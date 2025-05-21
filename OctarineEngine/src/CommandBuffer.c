#include <mimalloc.h>
#include "oct/Validation.h"
#include "oct/CommandBuffer.h"
#include "oct/Opaque.h"
#include "oct/Constants.h"
#include "oct/Subsystems.h"
#include "oct/Allocators.h"

// Quadruple buffer command buffer allocators so the logic thread is never writing to a buffer the render thread
// is reading from
Oct_Allocator gCommandBufferAllocators[4];
int gCommandBufferAllocatorCurrent;

// Wraps an index, returns index + 1 unless index == len - 1 in which case it returns 0
static inline int32_t nextIndex(int32_t index, int32_t len) {
    return index == len - 1 ? 0 : index + 1;
}

// Places a command into the ring buffer at tail, stalling if its full
static inline void pushCommand(Oct_Command *command) {
    Oct_Context ctx = _oct_GetCtx();

    // Busy loop if the buffer is full
    int tail = SDL_GetAtomicInt(&ctx->RingBuffer.tail);
    while (nextIndex(tail, ctx->initInfo->ringBufferSize) == SDL_GetAtomicInt(&ctx->RingBuffer.head)) {
        volatile int i;
    }

    // Insert new element
    memcpy(&ctx->RingBuffer.commands[tail], command, sizeof(struct Oct_Command_t));
    SDL_SetAtomicInt(&ctx->RingBuffer.tail, nextIndex(tail, ctx->initInfo->ringBufferSize));
}

// Allocates some memory into the command buffer allocator for the current frame, returns new memory location
void *_oct_CopyIntoFrameMemory(void *data, int32_t size) {
    void *mem = oct_Malloc(gCommandBufferAllocators[gCommandBufferAllocatorCurrent], size);
    if (mem) {
        memcpy(mem, data, size);
        return mem;
    } else {
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to copy memory into frame memory, size %i bytes", size);
    }
    return null;
}

void *_oct_GetFrameMemory(int32_t size) {
    void *mem = oct_Malloc(gCommandBufferAllocators[gCommandBufferAllocatorCurrent], size);
    if (mem) {
        return mem;
    } else {
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to copy memory into frame memory, size %i bytes", size);
    }
    return null;
}

void _oct_CommandBufferInit() {
    Oct_Context ctx = _oct_GetCtx();

    // Init ring buffer
    ctx->RingBuffer.commands = mi_malloc(sizeof(struct Oct_Command_t) * OCT_RING_BUFFER_SIZE);
    if (ctx->RingBuffer.commands) {
        SDL_SetAtomicInt(&ctx->RingBuffer.head, 0);
        SDL_SetAtomicInt(&ctx->RingBuffer.tail, 0);
    } else {
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate ringbuffer.");
    }

    // Init memory quad buffer
    for (int i = 0; i < 4; i++) {
        gCommandBufferAllocators[i] = oct_CreateVirtualPageAllocator();
        if (gCommandBufferAllocators[i] == null)
            oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to create command buffer allocator.");
    }
    oct_Log("Command buffer system initialized.");
}

void _oct_CommandBufferBeginFrame() {
    // Cycle command buffer allocator
    gCommandBufferAllocatorCurrent = (gCommandBufferAllocatorCurrent + 1) % 4;
    oct_ResetAllocator(gCommandBufferAllocators[gCommandBufferAllocatorCurrent]);

    // Tell render thread that new frame is starting
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .metaCommand = {
                    .sType = OCT_STRUCTURE_TYPE_META_COMMAND,
                    .type = OCT_META_COMMAND_TYPE_START_FRAME
            }
    };
    pushCommand(&cmd);
}

void _oct_CommandBufferEndFrame() {
    // Tell render thread that current frame is done
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .metaCommand = {
                    .sType = OCT_STRUCTURE_TYPE_META_COMMAND,
                    .type = OCT_META_COMMAND_TYPE_END_FRAME
            }
    };
    pushCommand(&cmd);
}

bool _oct_CommandBufferPop(Oct_Command *out) {
    Oct_Context ctx = _oct_GetCtx();

    if (SDL_GetAtomicInt(&ctx->RingBuffer.head) != SDL_GetAtomicInt(&ctx->RingBuffer.tail)) {
        memcpy(out, &ctx->RingBuffer.commands[SDL_GetAtomicInt(&ctx->RingBuffer.head)], sizeof(struct Oct_Command_t));
        SDL_SetAtomicInt(&ctx->RingBuffer.head, nextIndex(SDL_GetAtomicInt(&ctx->RingBuffer.head), ctx->initInfo->ringBufferSize));
        return true;
    }
    return false;
}

void _oct_CommandBufferEnd() {
    Oct_Context ctx = _oct_GetCtx();
    mi_free(ctx->RingBuffer.commands);
    for (int i = 0; i < 4; i++)
        oct_FreeAllocator(gCommandBufferAllocators[i]);
}

OCTARINE_API void oct_Draw(Oct_DrawCommand *draw) {
    draw->sType = OCT_STRUCTURE_TYPE_DRAW_COMMAND;
    draw->pNext = null;
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .drawCommand = *draw
    };
    pushCommand(&cmd);
}

OCTARINE_API void oct_WindowUpdate(Oct_WindowCommand *windowUpdate) {
    windowUpdate->sType = OCT_STRUCTURE_TYPE_WINDOW_COMMAND;
    windowUpdate->pNext = null;
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .windowCommand = *windowUpdate
    };
    pushCommand(&cmd);
}

OCTARINE_API Oct_Sound oct_AudioUpdate(Oct_AudioCommand *audioCommand) {
    audioCommand->sType = OCT_STRUCTURE_TYPE_AUDIO_COMMAND;
    audioCommand->pNext = null;
    if (audioCommand->type == OCT_AUDIO_COMMAND_TYPE_PLAY_SOUND)
        audioCommand->Play._soundID = _oct_ReserveSound();
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .audioCommand = *audioCommand
    };
    pushCommand(&cmd);
    if (audioCommand->type == OCT_AUDIO_COMMAND_TYPE_PLAY_SOUND)
        return audioCommand->Play._soundID;
    return audioCommand->Update.sound;
}

OCTARINE_API Oct_Asset oct_Load(Oct_LoadCommand *load) {
    load->sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND;
    load->pNext = null;
    load->_assetID = _oct_AssetReserveSpace();
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = *load
    };
    pushCommand(&cmd);
    return load->_assetID;
}

OCTARINE_API Oct_Texture oct_LoadTexture(const char *filename) {
    Oct_Asset id = _oct_AssetReserveSpace();
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = {
                    .sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND,
                    .type = OCT_LOAD_COMMAND_TYPE_LOAD_TEXTURE,
                    .pNext = null,
                    ._assetID = id,
                    .Texture.fileHandle = {
                            .type = OCT_FILE_HANDLE_TYPE_FILENAME,
                            .filename = _oct_CopyIntoFrameMemory((void*)filename, strlen(filename) + 1)
                    }
            }
    };
    pushCommand(&cmd);
    return id;
}

OCTARINE_API Oct_Font oct_LoadFont(const char *filename, float size) {
    Oct_Asset id = _oct_AssetReserveSpace();
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = {
                    .sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND,
                    .type = OCT_LOAD_COMMAND_TYPE_LOAD_FONT,
                    .pNext = null,
                    ._assetID = id,
                    .Font.fileHandles[0] = {
                            .type = OCT_FILE_HANDLE_TYPE_FILENAME,
                            .filename = _oct_CopyIntoFrameMemory((void*)filename, strlen(filename) + 1),
                    },
                    .Font.size = size
            }
    };
    pushCommand(&cmd);
    return id;
}

OCTARINE_API Oct_Texture oct_CreateSurface(Oct_Vec2 size) {
    Oct_Asset id = _oct_AssetReserveSpace();
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = {
                    .sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND,
                    .type = OCT_LOAD_COMMAND_TYPE_CREATE_SURFACE,
                    .pNext = null,
                    ._assetID = id,
                    .Surface.dimensions = {size[0], size[1]}
            }
    };
    pushCommand(&cmd);
    return id;
}

OCTARINE_API Oct_Camera oct_CreateCamera() {
    Oct_Asset id = _oct_AssetReserveSpace();
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = {
                    .sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND,
                    .type = OCT_LOAD_COMMAND_TYPE_CREATE_CAMERA,
                    .pNext = null,
                    ._assetID = id,
            }
    };
    pushCommand(&cmd);
    return id;
}

OCTARINE_API Oct_Sprite oct_LoadSprite(Oct_Texture tex, int32_t frameCount, double fps, Oct_Vec2 startPos, Oct_Vec2 frameSize) {
    Oct_Asset id = _oct_AssetReserveSpace();
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = {
                    .sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND,
                    .type = OCT_LOAD_COMMAND_TYPE_LOAD_SPRITE,
                    .pNext = null,
                    ._assetID = id,
                    .Sprite = {
                            .texture = tex,
                            .frameCount = frameCount,
                            .repeat = true,
                            .fps = fps,
                            .startPos = {startPos[0], startPos[1]},
                            .frameSize = {frameSize[0], frameSize[1]},
                            .padding = {0, 0},
                            .xStop = 0
                    }
            }
    };
    pushCommand(&cmd);
    return id;
}

OCTARINE_API Oct_FontAtlas oct_CreateFontAtlas(Oct_Font font, Oct_FontAtlas atlas, uint32_t unicodeStart, uint32_t unicodeEnd) {
    Oct_Asset id = _oct_AssetReserveSpace();
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = {
                    .sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND,
                    .type = OCT_LOAD_COMMAND_TYPE_CREATE_FONT_ATLAS,
                    .pNext = null,
                    ._assetID = id,
                    .FontAtlas = {
                            .font = font,
                            .atlas = atlas,
                            .unicodeStart = unicodeStart,
                            .unicodeEnd = unicodeEnd
                    }
            }
    };
    pushCommand(&cmd);
    if (atlas != OCT_NO_ASSET)
        return atlas;
    return id;
}

OCTARINE_API Oct_FontAtlas oct_LoadBitmapFont(const char *filename, Oct_Vec2 cellSize, uint32_t unicodeStart, uint32_t unicodeEnd) {
    Oct_Asset id = _oct_AssetReserveSpace();
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = {
                    .sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND,
                    .type = OCT_LOAD_COMMAND_TYPE_LOAD_BITMAP_FONT,
                    .pNext = null,
                    ._assetID = id,
                    .BitmapFont = {
                            .fileHandle = {
                                    .type = OCT_FILE_HANDLE_TYPE_FILENAME,
                                    .filename = _oct_CopyIntoFrameMemory((void*)filename, strlen(filename) + 1),
                            },
                            .cellSize = {cellSize[0], cellSize[1]},
                            .unicodeStart = unicodeStart,
                            .unicodeEnd = unicodeEnd
                    }
            }
    };
    pushCommand(&cmd);
    return id;
}

OCTARINE_API void oct_FreeAsset(Oct_Asset asset) {
    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = {
                    .sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND,
                    ._assetID = asset,
                    .type = OCT_LOAD_COMMAND_TYPE_FREE
            }
    };
    pushCommand(&cmd);
}

void _oct_CommandBufferDispatch() {
    Oct_Command cmd = {0};
    while (_oct_CommandBufferPop(&cmd)) {
        // Find out the kind of command this is
        const Oct_StructureType sType = OCT_STRUCTURE_TYPE(&cmd.topOfUnion);

        // Dispatch to the proper subsystem
        if (sType == OCT_STRUCTURE_TYPE_META_COMMAND) {
            // Meta commands are dispatched everywhere
            _oct_AudioProcessCommand(&cmd);
            _oct_DrawingProcessCommand(&cmd);
            _oct_WindowProcessCommand(&cmd);
        } else if (sType == OCT_STRUCTURE_TYPE_DRAW_COMMAND) {
            _oct_DrawingProcessCommand(&cmd);
        } else if (sType == OCT_STRUCTURE_TYPE_WINDOW_COMMAND) {
            _oct_WindowProcessCommand(&cmd);
        } else if (sType == OCT_STRUCTURE_TYPE_AUDIO_COMMAND) {
            _oct_AudioProcessCommand(&cmd);
        } else if (sType == OCT_STRUCTURE_TYPE_LOAD_COMMAND) {
            _oct_AssetsProcessCommand(&cmd);
        }
    }
}

OCTARINE_API void *oct_CopyFrameData(void *data, int32_t size) {
    return _oct_CopyIntoFrameMemory(data, size);
}

OCTARINE_API Oct_AssetBundle oct_LoadAssetBundle(const char *filename) {
    Oct_AssetBundle bundle = mi_zalloc(sizeof(struct Oct_AssetBundle_t));
    if (!bundle)
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate asset bundle.");

    // Allocate bucket
    bundle->bucket = mi_zalloc(sizeof(struct Oct_AssetLink_t) * OCT_BUCKET_SIZE);
    if (!bundle->bucket)
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate asset bundle bucket.");
    for (int32_t i = 0; i < OCT_BUCKET_SIZE; i++)
        bundle->bucket[i].asset = OCT_NO_ASSET;

    Oct_Command cmd = {
            .sType = OCT_STRUCTURE_TYPE_COMMAND,
            .loadCommand = {
                    .sType = OCT_STRUCTURE_TYPE_LOAD_COMMAND,
                    .type = OCT_LOAD_COMMAND_TYPE_LOAD_ASSET_BUNDLE,
                    .pNext = null,
                    ._assetID = OCT_NO_ASSET,
                    .AssetBundle = {
                            .filename = _oct_CopyIntoFrameMemory((void*)filename, strlen(filename) + 1),
                            .bundle = bundle
                    }
            }
    };
    pushCommand(&cmd);

    return bundle;
}
