#include <SDL3/SDL.h>
#include <VK2D/VK2D.h>
#include <stdio.h>

#include "oct/stb_vorbis.h"
#include "oct/Common.h"
#include "oct/Opaque.h"
#include "oct/Core.h"
#include "oct/Validation.h"
#include "oct/Subsystems.h"

// All assets
static Oct_AssetData gAssets[OCT_MAX_ASSETS];

// Returns the actual index in the asset array given an asset id
#define ASSET_INDEX(asset) (asset & INT32_MAX)
#define ASSET_GENERATION(asset) (asset >> 32)

// Error message in case an asset load fails
static SDL_Mutex *gErrorMessageMutex;
static SDL_AtomicInt gErrorHasOccurred;
static TTF_TextEngine *gTextEngine;

// For text
#define TEXT_BUFFER_SIZE 1024
char gTextBuffer[TEXT_BUFFER_SIZE];

///////////////////////////////// ASSET CREATION HELP /////////////////////////////////

// Destroys metadata for an asset when its being freed
void _oct_DestroyAssetMetadata(Oct_Asset asset) {
    SDL_SetAtomicInt(&gAssets[asset].loaded, 0);
    SDL_SetAtomicInt(&gAssets[asset].reserved, 0);
    SDL_SetAtomicInt(&gAssets[asset].failed, 0);
    SDL_AddAtomicInt(&gAssets[ASSET_INDEX(asset)].generation, 1);
}

void _oct_FailLoad(Oct_Asset asset) {
    if (asset != OCT_NO_ASSET) {
        SDL_SetAtomicInt(&gAssets[asset].failed, 1);
        SDL_SetAtomicInt(&gAssets[asset].reserved, 1);
    }
    SDL_SetAtomicInt(&gErrorHasOccurred, 1);
}

static uint8_t* _oct_ReadFile(const char *filename, uint32_t *size) {
    FILE* file = fopen(filename, "rb");
    unsigned char *buffer = NULL;
    *size = 0;

    if (file != NULL) {
        // Find file size
        fseek(file, 0, SEEK_END); // not portable but idc
        *size = ftell(file);
        rewind(file);

        buffer = mi_malloc(*size);

        if (buffer != NULL) {
            // Fill the buffer
            fread(buffer, 1, *size, file);
        } else {
            oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Unable to allocate buffer for file \"%s\".", filename);
        }
        fclose(file);
    }

    return buffer;
}

static uint8_t *_oct_GetBufferFromHandle(Oct_FileHandle *handle, uint32_t *size) {
    if (handle->type == OCT_FILE_HANDLE_TYPE_FILENAME) {
        return _oct_ReadFile(handle->filename, size);
    } else {
        *size = handle->size;
        return handle->buffer;
    }
}

static void _oct_CleanupBufferFromHandle(Oct_FileHandle *handle, uint8_t *buffer) {
    if (handle->type == OCT_FILE_HANDLE_TYPE_FILENAME) {
        mi_free(buffer);
    } else {
        if (handle->callback)
            handle->callback(buffer, handle->size);
    }
}

// Returns a string representing a file handle from a string, only valid till next call
static const char *_oct_FileHandleName(Oct_FileHandle *handle) {
    static char buffer[512] = {0};
    if (handle->type == OCT_FILE_HANDLE_TYPE_NONE) {
        SDL_snprintf(buffer, 511, "[none]");
    }
    if (handle->type == OCT_FILE_HANDLE_TYPE_FILENAME) {
        SDL_snprintf(buffer, 511, "\"%s\"", handle->filename);
    }
    if (handle->type == OCT_FILE_HANDLE_TYPE_FILE_BUFFER) {
        if (handle->name)
            SDL_snprintf(buffer, 511, "[\"%s\", buffer 0x%p, size %i]", handle->name, (void*)handle->buffer, handle->size);
        else
            SDL_snprintf(buffer, 511, "[Buffer 0x%p, size %i]", (void*)handle->buffer, handle->size);
    }
    return buffer;
}

void _oct_RegisterAssetName(Oct_Asset asset, Oct_FileHandle *handle) {
    strncpy(gAssets[ASSET_INDEX(asset)].name, _oct_FileHandleName(handle), OCT_ASSET_NAME_SIZE - 1);
}

///////////////////////////////// ASSET CREATION /////////////////////////////////
void _oct_AssetCreateTexture(Oct_LoadCommand *load) {
    uint32_t size;
    uint8_t *buffer = _oct_GetBufferFromHandle(&load->Texture.fileHandle, &size);
    VK2DTexture tex = vk2dTextureFrom(buffer, size);
    _oct_CleanupBufferFromHandle(&load->Texture.fileHandle, buffer);
    if (tex) {
        gAssets[ASSET_INDEX(load->_assetID)].texture.tex = tex;
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].texture.width, (float)vk2dTextureWidth(tex));
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].texture.height, (float)vk2dTextureHeight(tex));
        gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_TEXTURE;
        _oct_RegisterAssetName(load->_assetID, &load->Texture.fileHandle);
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
    } else {
        _oct_FailLoad(load->_assetID);
        oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Failed to load texture %s", _oct_FileHandleName(&load->Texture.fileHandle));
    }
}

static void _oct_AssetCreateSurface(Oct_LoadCommand *load) {
    VK2DTexture tex = vk2dTextureCreate(load->Surface.dimensions[0], load->Surface.dimensions[1]);
    if (tex) {
        gAssets[ASSET_INDEX(load->_assetID)].texture.tex = tex;
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].texture.width, (float)vk2dTextureWidth(tex));
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].texture.height, (float)vk2dTextureHeight(tex));
        gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_TEXTURE;
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
        snprintf(gAssets[ASSET_INDEX(load->_assetID)].name, OCT_ASSET_NAME_SIZE - 1, "Size: %.2fx%.2f", load->Surface.dimensions[0], load->Surface.dimensions[1]);
    } else {
        _oct_FailLoad(load->_assetID);
        oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Failed to create surface of dimensions %.2f/%.2f", load->Surface.dimensions[0], load->Surface.dimensions[1]);
    }
}

static void _oct_AssetCreateCamera(Oct_LoadCommand *load) {
    VK2DCameraIndex camIndex = vk2dCameraCreate(vk2dCameraGetSpec(VK2D_DEFAULT_CAMERA));
    if (camIndex != VK2D_INVALID_CAMERA) {
        gAssets[ASSET_INDEX(load->_assetID)].camera = camIndex;
        gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_CAMERA;
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
        snprintf(gAssets[ASSET_INDEX(load->_assetID)].name, OCT_ASSET_NAME_SIZE - 1, "<Camera>");
    } else {
        _oct_FailLoad(load->_assetID);
        oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Failed to create camera");
    }
}

void _oct_AssetCreateAudio(Oct_LoadCommand *load) {
    // Find file extension
    uint32_t fileBufferSize;
    uint8_t *fileBuffer = _oct_GetBufferFromHandle(&load->Audio.fileHandle, &fileBufferSize);
    uint8_t *data = null;
    uint32_t dataSize = 0;
    SDL_AudioSpec spec;
    Oct_Bool sdlFree = false;

    if (fileBufferSize < 4) {
        _oct_CleanupBufferFromHandle(&load->Audio.fileHandle, fileBuffer);
        _oct_FailLoad(load->_assetID);
        oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Failed to load audio sample %s, ogg is not yet supported.", _oct_FileHandleName(&load->Audio.fileHandle));
        return;
    }

    const uint16_t MP3_SIG = 0xFFFB;
    if (memcmp(fileBuffer, "OggS", 4) == 0) {
        int channels, sampleRate;
        int samples = stb_vorbis_decode_memory(fileBuffer, fileBufferSize, &channels, &sampleRate, &data);
        if (samples < 0) {
            _oct_FailLoad(load->_assetID);
            oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Failed to load audio sample %s with vorbis", _oct_FileHandleName(&load->Audio.fileHandle), SDL_GetError());
            return;
        }
        spec.freq = sampleRate;
        spec.channels = channels;
        spec.format = SDL_AUDIO_S16;
        dataSize = samples * channels * 2;
    } else if (memcmp(fileBuffer, "RIFF", 4) == 0 && fileBufferSize >= 12 && memcmp(fileBuffer + 8, "WAVE", 4) == 0) {
        // Use SDL to load wavs
        SDL_IOStream *io = SDL_IOFromConstMem(fileBuffer, fileBufferSize);
        if (!SDL_LoadWAV_IO(io, true, &spec, &data, &dataSize)) {
            _oct_FailLoad(load->_assetID);
            oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Failed to load audio sample %s, SDL Error: %s.", _oct_FileHandleName(&load->Audio.fileHandle), SDL_GetError());
            return;
        }
        sdlFree = true;
    } else if (memcmp(fileBuffer, "ID3", 4) == 0 || memcmp(fileBuffer, &MP3_SIG, 2) == 0) {
        // TODO: Implement mp3 loading
        _oct_FailLoad(load->_assetID);
        return;
        oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Failed to load audio sample %s, mp3 is not yet supported.", _oct_FileHandleName(&load->Audio.fileHandle));
    } else {
        _oct_FailLoad(load->_assetID);
        oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Failed to load audio sample %s, unrecognized extension.", _oct_FileHandleName(&load->Audio.fileHandle));
        return;
    }

    _oct_CleanupBufferFromHandle(&load->Audio.fileHandle, fileBuffer);

    // Convert format if we found good data
    if (data) {
        int32_t newSize;
        uint8_t *newSamples = _oct_AudioConvertFormat(data, dataSize, &newSize, &spec);
        if (newSamples) {
            gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_AUDIO;
            gAssets[ASSET_INDEX(load->_assetID)].audio.size = newSize;
            gAssets[ASSET_INDEX(load->_assetID)].audio.data = newSamples;
            SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
            _oct_RegisterAssetName(load->_assetID, &load->Audio.fileHandle);
        } else {
            _oct_FailLoad(load->_assetID);
            oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Failed to load audio sample %s, failed to convert format, SDL Error %s.", _oct_FileHandleName(&load->Audio.fileHandle), SDL_GetError());
        }
        if (sdlFree)
            SDL_free(data);
        else
            free(data);
    }
}

void _oct_AssetCreateSprite(Oct_LoadCommand *load) {
    Oct_SpriteData *data = &gAssets[ASSET_INDEX(load->_assetID)].sprite;
    gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_SPRITE;
    data->texture = load->Sprite.texture;
    data->frameCount = load->Sprite.frameCount;

    // Allocate the frames
    data->frames = mi_malloc(sizeof(struct Oct_SpriteFrame_t) * data->frameCount);
    if (!data->frames)
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate sprite frame data");

    // Get the texture
    Oct_AssetData *texData = _oct_AssetGetSafe(load->Sprite.texture, OCT_ASSET_TYPE_TEXTURE);
    if (!texData) {
        _oct_FailLoad(load->_assetID);
        oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Sprite cannot be created with invalid texture (%" PRIu64 ")", load->Sprite.texture);
    }
    data->duration = 1.0 / load->Sprite.fps;

    // Fill frame data
    for (int i = 0; i < data->frameCount; i++) {
        const int totalHorizontal = i * (load->Sprite.frameSize[0] + load->Sprite.padding[0]);
        const int lineBreaks = (int)(load->Sprite.startPos[0] + totalHorizontal) / (int)(vk2dTextureWidth(texData->texture.tex) - load->Sprite.xStop);
        float x;
        if (lineBreaks == 0)
            x = (float)((int)(load->Sprite.startPos[0] + (totalHorizontal - (load->Sprite.padding[0] * lineBreaks))) % (int)(vk2dTextureWidth(texData->texture.tex) - load->Sprite.xStop));
        else
            x = (float)(load->Sprite.xStop + ((int)(load->Sprite.startPos[0] + (totalHorizontal - (load->Sprite.padding[0] * lineBreaks))) % (int)(vk2dTextureWidth(texData->texture.tex) - load->Sprite.xStop)));
        data->frames[i].position[0] = x;
        data->frames[i].position[1] = lineBreaks * load->Sprite.frameSize[1];
        data->frames[i].size[0] = load->Sprite.frameSize[0];;
        data->frames[i].size[1] = load->Sprite.frameSize[1];;
    }

    SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
    snprintf(gAssets[ASSET_INDEX(load->_assetID)].name, OCT_ASSET_NAME_SIZE - 1, "%ix%i, %i frames", (int)load->Sprite.frameSize[0], (int)load->Sprite.frameSize[1], load->Sprite.frameCount);
}

void _oct_AssetCreateFont(Oct_LoadCommand *load) {
    Oct_AssetData *data = &gAssets[ASSET_INDEX(load->_assetID)];
    Oct_FontData *fnt = &data->font;
    data->type = OCT_ASSET_TYPE_FONT;
    Oct_Bool error = false;

    // Load all fonts
    for (int i = 0; i < OCT_FALLBACK_FONT_MAX; i++) {
        if (load->Font.fileHandles[i].type != OCT_FILE_HANDLE_TYPE_NONE) {
            // Fonts expect the io stream to live with the font so we gotta copy it
            uint32_t size;
            uint8_t *buffer = _oct_GetBufferFromHandle(&load->Font.fileHandles[i], &size);
            fnt->buffers[i] = mi_malloc(size);
            memcpy(fnt->buffers[i], buffer, size);
            SDL_IOStream *io = SDL_IOFromConstMem(fnt->buffers[i], size);

            // Load font[i]
            fnt->font[i] = TTF_OpenFontIO(io, true, 10);

            // We ditch the original cuz we don't know if we can have it lingering
            _oct_CleanupBufferFromHandle(&load->Font.fileHandles[i], buffer);

            // Make sure font didn't explode
            if (fnt->font[i] == null || (i > 0 && !TTF_AddFallbackFont(fnt->font[0], fnt->font[i]))) {
                error = true;

                // Clean up previous fonts
                for (int j = i - 1; j >= 0; j--) {
                    TTF_CloseFont(fnt->font[j]);
                }
                break;
            }
        }
    }

    if (!error) {
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
        _oct_RegisterAssetName(load->_assetID, &load->Font.fileHandles[0]);
    } else {
        _oct_FailLoad(load->_assetID);
        oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Failed to create font %s, TTF error %s", _oct_FileHandleName(&load->Font.fileHandles[0]), SDL_GetError());
    }
}

void _oct_AssetCreateFontAtlas(Oct_LoadCommand *load) {
    // How to do this:
    //   1. Find the dimensions of each glyph in range
    //   2. Calculate the total dimensions of the atlas
    //   3. Create a surface of that size
    //   4. Copy each glyph into that surface
    //   5. Upload the surface to a VK2D texture

    // First check if we are adding to an existing bitmap or creating one
    Oct_BitmapFontData *fnt = null;
    Oct_Asset asset = 0;
    if (load->FontAtlas.atlas != OCT_NO_ASSET) {
        // Free the reserved asset
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].reserved, 0);
        fnt = &gAssets[ASSET_INDEX(load->FontAtlas.atlas)].fontAtlas;
        asset = ASSET_INDEX(load->FontAtlas.atlas);
    } else {
        // Prepare the new asset slot
        fnt = &gAssets[ASSET_INDEX(load->_assetID)].fontAtlas;
        fnt->atlasCount = 0;
        fnt->atlases = null;
        asset = ASSET_INDEX(load->_assetID);
    }
    gAssets[ASSET_INDEX(asset)].type = OCT_ASSET_TYPE_FONT_ATLAS;

    // Check if the passed font exists
    Oct_AssetData *fontAsset = _oct_AssetGetSafe(load->FontAtlas.font, OCT_ASSET_TYPE_FONT);
    if (!fontAsset) {
        oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Atlas cannot be created without a font.");
        _oct_FailLoad(asset);
        return;
    }
    Oct_FontData *fntData = &fontAsset->font;

    // Resize font
    TTF_SetFontSize(fntData->font[0], load->FontAtlas.size);

    // Add new atlas to atlas list
    void *newAtlas = mi_realloc(fnt->atlases, (fnt->atlasCount + 1) * sizeof(struct Oct_FontAtlasData_t));
    if (!newAtlas) {
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to reallocate font atlas list.");
    }
    fnt->atlases = newAtlas;
    fnt->atlasCount++;
    Oct_FontAtlasData *atlas = &fnt->atlases[fnt->atlasCount - 1];
    atlas->unicodeEnd = load->FontAtlas.unicodeEnd;
    atlas->unicodeStart = load->FontAtlas.unicodeStart;
    atlas->atlas = null;
    fnt->font = load->FontAtlas.font;

    // Allocate glyph list
    const uint32_t glyphCount = load->FontAtlas.unicodeEnd - load->FontAtlas.unicodeStart;
    atlas->glyphs = mi_malloc(sizeof(struct Oct_FontGlyphData_t) * glyphCount);
    if (!atlas->glyphs)
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate font glyph list.");

    // Find space dimensions
    char space[5] = {0};
    int garbage1, garbage2, garbage3, garbage4;
    int spaceSize;
    SDL_UCS4ToUTF8(32, (void*)space);
    TTF_GetGlyphMetrics(
            fntData->font[0],
            *((uint32_t*)space),
            &garbage1, &garbage2,
            &garbage3, &garbage4,
            &spaceSize);
    fnt->spaceSize = spaceSize;

    // Find dimensions of each glyph
    int x = 0;
    int y = 0;
    int tallestGlyph = 0;
    const int textureWidthCap = 2048;
    Oct_Bool oversize = false;
    for (uint32_t i = 0; i < glyphCount; i++) {
        // Find glyph metrics
        char string[5] = {0};
        int w, h;
        SDL_UCS4ToUTF8(atlas->unicodeStart + i, (void*)string);
        if (!TTF_GetStringSize(fntData->font[0], string, 0, &w, &h)) {
            oct_Raise(OCT_STATUS_SDL_ERROR, false, "Failed to get character size, SDL error: %s", SDL_GetError());
        }
        TTF_GetGlyphMetrics(
                fntData->font[0],
                *((uint32_t*)string),
                &atlas->glyphs[i].minBB[0], &atlas->glyphs[i].maxBB[0],
                &atlas->glyphs[i].minBB[1], &atlas->glyphs[i].maxBB[1],
                &atlas->glyphs[i].advance);

        // Keep textures to a certain width
        int drawX = 0;
        if (x + w > textureWidthCap) {
            x = 0;
            y += tallestGlyph + 1;
            oversize = true;
        } else {
            drawX = x;
            x += w + 1;
        }
        tallestGlyph = h > tallestGlyph ? h : tallestGlyph;

        // Record x/y position in the output texture
        atlas->glyphs[i].location.position[0] = drawX;
        atlas->glyphs[i].location.position[1] = y;
        atlas->glyphs[i].location.size[0] = w;
        atlas->glyphs[i].location.size[1] = h;
    }
    fnt->newLineSize = tallestGlyph;

    // Create the surface that will hold all the glyphs
    const int imgWidth = oversize ? textureWidthCap : x;
    const int imgHeight = y + tallestGlyph;
    SDL_Surface *tempSurface = SDL_CreateSurface(imgWidth, imgHeight, SDL_PIXELFORMAT_RGBA8888);
    if (!tempSurface)
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to create SDL surface, SDL error %s", SDL_GetError());

    // Copy glyphs into surface
    for (uint32_t i = 0; i < glyphCount; i++) {
        char string[5] = {0};
        SDL_UCS4ToUTF8(atlas->unicodeStart + i, (void*)string);
        TTF_Text *t = TTF_CreateText(gTextEngine, fntData->font[0], string, 0);
        Oct_Bool e = TTF_DrawSurfaceText(t, atlas->glyphs[i].location.position[0], atlas->glyphs[i].location.position[1], tempSurface);
        TTF_DestroyText(t);

        if (!t || !e)
            oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to text or copy text, TTF error %s", SDL_GetError());
    }

    // Copy the atlas surface to a VK2D texture/cleanup
    // TODO: Possible memory problem here, SDL_SaveBMP reports incorrect surfaces sometimes
    SDL_SaveBMP(tempSurface, "surf.bmp");
    atlas->img = vk2dImageFromPixels(vk2dRendererGetDevice(), tempSurface->pixels, imgWidth, imgHeight, true);
    atlas->atlas = vk2dTextureLoadFromImage(atlas->img);
    SDL_SetAtomicInt(&gAssets[ASSET_INDEX(asset)].loaded, 1);
    snprintf(gAssets[ASSET_INDEX(load->_assetID)].name, OCT_ASSET_NAME_SIZE - 1, "Font %" PRIu64 ", size %.2f, U+%04X - U+%04X", load->FontAtlas.font, load->FontAtlas.size, load->FontAtlas.unicodeStart, load->FontAtlas.unicodeEnd);
    SDL_DestroySurface(tempSurface);
}

// Bitmap fonts are just font atlases
void _oct_AssetCreateBitmapFont(Oct_LoadCommand *load) {
    Oct_AssetData *asset = &gAssets[ASSET_INDEX(load->_assetID)];
    asset->type = OCT_ASSET_TYPE_FONT_ATLAS;
    asset->fontAtlas.atlases = mi_malloc(sizeof(struct Oct_FontAtlasData_t));
    asset->fontAtlas.atlasCount = 1;
    asset->fontAtlas.font = OCT_NO_ASSET;

    if (!asset->fontAtlas.atlases)
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate font atlas list");

    uint32_t size;
    uint8_t *buffer = _oct_GetBufferFromHandle(&load->BitmapFont.fileHandle, &size);

    const uint32_t glyphCount = load->BitmapFont.unicodeEnd - load->BitmapFont.unicodeStart;
    asset->fontAtlas.atlases[0].img = null;
    asset->fontAtlas.atlases[0].atlas = vk2dTextureFrom(buffer, size);
    asset->fontAtlas.atlases[0].glyphs = mi_malloc(sizeof(struct Oct_FontGlyphData_t) * glyphCount);
    asset->fontAtlas.atlases[0].unicodeStart = load->BitmapFont.unicodeStart;
    asset->fontAtlas.atlases[0].unicodeEnd = load->BitmapFont.unicodeEnd;
    asset->fontAtlas.spaceSize = load->BitmapFont.cellSize[0];
    asset->fontAtlas.newLineSize = load->BitmapFont.cellSize[1];

    _oct_CleanupBufferFromHandle(&load->BitmapFont.fileHandle, buffer);

    if (!asset->fontAtlas.atlases[0].atlas) {
        _oct_FailLoad(load->_assetID);
        oct_Raise(OCT_STATUS_FAILED_ASSET, false, "Failed to create bitmap font %s, VK2D error: %s", _oct_FileHandleName(&load->BitmapFont.fileHandle), vk2dStatusMessage());
        mi_free(asset->fontAtlas.atlases[0].glyphs);
        mi_free(asset->fontAtlas.atlases);
        return;
    } else if (!asset->fontAtlas.atlases[0].glyphs) {
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate glyph list");
    }

    // Copy glyphs to position
    for (int i = 0; i < glyphCount; i++) {
        Oct_FontGlyphData *glyph = &asset->fontAtlas.atlases[0].glyphs[i];
        glyph->advance = load->BitmapFont.cellSize[0];
        glyph->location.size[0] = load->BitmapFont.cellSize[0];
        glyph->location.size[1] = load->BitmapFont.cellSize[1];
        glyph->location.position[0] = (int)(load->BitmapFont.cellSize[0] * i) % (int)vk2dTextureWidth(asset->fontAtlas.atlases[0].atlas);
        glyph->location.position[1] = ((int)(load->BitmapFont.cellSize[0] * i) / (int)vk2dTextureWidth(asset->fontAtlas.atlases[0].atlas)) * load->BitmapFont.cellSize[1];
        glyph->maxBB[0] = load->BitmapFont.cellSize[0];
        glyph->maxBB[1] = load->BitmapFont.cellSize[1];
        glyph->minBB[0] = 0;
        glyph->minBB[1] = 0;
    }
    SDL_SetAtomicInt(&asset->loaded, 1);
    _oct_RegisterAssetName(load->_assetID, &load->BitmapFont.fileHandle);
}

void _oct_AssetCreateAssetBundle(Oct_LoadCommand *load);

///////////////////////////////// ASSET DESTRUCTION /////////////////////////////////
static void _oct_AssetDestroyTexture(Oct_Asset asset) {
    vk2dRendererWait();
    vk2dTextureFree(gAssets[ASSET_INDEX(asset)].texture.tex);
    _oct_DestroyAssetMetadata(asset);
}

static void _oct_AssetDestroyCamera(Oct_Asset asset) {
    vk2dCameraSetState(gAssets[ASSET_INDEX(asset)].camera, VK2D_CAMERA_STATE_DELETED);
    _oct_DestroyAssetMetadata(asset);
}

static void _oct_AssetDestroySprite(Oct_Asset asset) {
    mi_free(gAssets[ASSET_INDEX(asset)].sprite.frames);
    _oct_DestroyAssetMetadata(asset);
}

static void _oct_AssetDestroyAudio(Oct_Asset asset) {
    SDL_free(gAssets[ASSET_INDEX(asset)].audio.data);
    _oct_DestroyAssetMetadata(asset);
}

void _oct_AssetDestroyFont(Oct_Asset asset) {
    for (int i = 0; i < OCT_FALLBACK_FONT_MAX; i++) {
        if (gAssets[ASSET_INDEX(asset)].font.font[i]) {
            TTF_CloseFont(gAssets[ASSET_INDEX(asset)].font.font[i]);
            if (gAssets[ASSET_INDEX(asset)].font.buffers[i])
                mi_free(gAssets[ASSET_INDEX(asset)].font.buffers[i]);
        }
    }
    _oct_DestroyAssetMetadata(asset);
}

void _oct_AssetDestroyFontAtlas(Oct_Asset asset) {
    for (int i = 0; i < gAssets[ASSET_INDEX(asset)].fontAtlas.atlasCount; i++) {
        vk2dTextureFree(gAssets[ASSET_INDEX(asset)].fontAtlas.atlases[i].atlas);
        vk2dImageFree(gAssets[ASSET_INDEX(asset)].fontAtlas.atlases[i].img);
        mi_free(gAssets[ASSET_INDEX(asset)].fontAtlas.atlases[i].glyphs);
    }
    mi_free(gAssets[ASSET_INDEX(asset)].fontAtlas.atlases);
    _oct_DestroyAssetMetadata(asset);
}


static void _oct_AssetDestroy(Oct_Asset asset) {
    if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_TEXTURE) {
        _oct_AssetDestroyTexture(asset);
    } else if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_CAMERA) {
        _oct_AssetDestroyCamera(asset);
    } else if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_SPRITE) {
        _oct_AssetDestroySprite(asset);
    } else if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_AUDIO) {
        _oct_AssetDestroyAudio(asset);
    } else if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_FONT) {
        _oct_AssetDestroyFont(asset);
    } else if (gAssets[ASSET_INDEX(asset)].type == OCT_ASSET_TYPE_FONT_ATLAS) {
        _oct_AssetDestroyFontAtlas(asset);
    }

}

///////////////////////////////// INTERNAL /////////////////////////////////
void _oct_AssetsInit() {
    gErrorMessageMutex = SDL_CreateMutex();
    if (!TTF_Init()) {
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to initialize SDL TTF, SDL error %s", SDL_GetError());
    }
    gTextEngine = TTF_CreateSurfaceTextEngine();
    if (!gTextEngine) {
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to initialize SDL TTF, SDL error %s", SDL_GetError());
    }
    oct_Log("Asset system initialized.");
}

void _oct_AssetsProcessCommand(Oct_Command *cmd) {
    Oct_LoadCommand *load = &cmd->loadCommand;
    if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_TEXTURE) {
        _oct_AssetCreateTexture(load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_CREATE_SURFACE) {
        _oct_AssetCreateSurface(load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_CREATE_CAMERA) {
        _oct_AssetCreateCamera(load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_SPRITE) {
        _oct_AssetCreateSprite(load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_AUDIO) {
        _oct_AssetCreateAudio(load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_FONT) {
        _oct_AssetCreateFont(load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_CREATE_FONT_ATLAS) {
        _oct_AssetCreateFontAtlas(load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_BITMAP_FONT) {
        _oct_AssetCreateBitmapFont(load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_LOAD_ASSET_BUNDLE) {
        _oct_AssetCreateAssetBundle(load);
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_FREE) {
        if (SDL_GetAtomicInt(&gAssets[load->_assetID].loaded))
            _oct_AssetDestroy(load->_assetID);
    }
}

Oct_AssetType _oct_AssetType(Oct_Asset asset) {
    return SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].loaded) ? gAssets[ASSET_INDEX(asset)].type : OCT_ASSET_TYPE_NONE;
}

int _oct_AssetGeneration(Oct_Asset asset) {
    return SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].generation);
}

const char *_oct_AssetTypeString(Oct_Asset asset) {
    Oct_AssetType type = SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].loaded) ? gAssets[ASSET_INDEX(asset)].type : OCT_ASSET_TYPE_NONE;
    if (type == OCT_ASSET_TYPE_NONE)
        return "None";
    if (type == OCT_ASSET_TYPE_TEXTURE)
        return "Texture";
    if (type == OCT_ASSET_TYPE_FONT)
        return "Font";
    if (type == OCT_ASSET_TYPE_FONT_ATLAS)
        return "Font Atlas";
    if (type == OCT_ASSET_TYPE_AUDIO)
        return "Audio";
    if (type == OCT_ASSET_TYPE_SPRITE)
        return "Sprite";
    if (type == OCT_ASSET_TYPE_CAMERA)
        return "Camera";
    if (type == OCT_ASSET_TYPE_ANY)
        return "Any";
    return "";
}

const char *_oct_AssetName(Oct_Asset asset) {
    return gAssets[ASSET_INDEX(asset)].name;
}

Oct_AssetData *_oct_AssetGet(Oct_Asset asset) {
    return &gAssets[ASSET_INDEX(asset)];
}

Oct_AssetData *_oct_AssetGetSafe(Oct_Asset asset, Oct_AssetType type) {

    if (ASSET_INDEX(asset) >= OCT_MAX_ASSETS ||
        ASSET_GENERATION(asset) != SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].generation) ||
        gAssets[ASSET_INDEX(asset)].type != type ||
        !SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].loaded))
        return null;
    return &gAssets[ASSET_INDEX(asset)];
}

void _oct_AssetsEnd() {
    // Delete all the assets still loaded
    for (int i = 0; i < OCT_MAX_ASSETS; i++)
        if (SDL_GetAtomicInt(&gAssets[i].loaded))
            _oct_AssetDestroy(i);

    TTF_DestroySurfaceTextEngine(gTextEngine);
    TTF_Quit();
    SDL_DestroyMutex(gErrorMessageMutex);
}

Oct_Asset _oct_AssetReserveSpace() {
    for (int i = 0; i < OCT_MAX_ASSETS; i++) {
        if (!SDL_GetAtomicInt(&gAssets[i].reserved)) {
            SDL_SetAtomicInt(&gAssets[i].reserved, 1);
            const int64_t gen = SDL_GetAtomicInt(&gAssets[i].generation);
            return i + ((gen) << 32);
        }
    }
}

///////////////////////////////// EXTERNAL /////////////////////////////////
OCTARINE_API Oct_Bool oct_AssetLoaded(Oct_Asset asset) {
    const Oct_Bool loaded = SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].loaded);
    const int64_t gen = SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].generation);
    const Oct_Bool matchesGeneration = gen == asset >> 32;
    return loaded && matchesGeneration;
}

OCTARINE_API Oct_Bool oct_AssetLoadFailed(Oct_Asset asset) {
    bool failed = SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].failed);
    if (failed) {
        _oct_DestroyAssetMetadata(asset);
    }
    return failed;
}

OCTARINE_API Oct_Bool oct_AssetLoadHasFailed() {
    return SDL_GetAtomicInt(&gErrorHasOccurred);
}

OCTARINE_API float oct_TextureWidth(Oct_Texture tex) {
    while (SDL_GetAtomicInt(&gAssets[ASSET_INDEX(tex)].reserved) && !(SDL_GetAtomicInt(&gAssets[ASSET_INDEX(tex)].loaded) || SDL_GetAtomicInt(&gAssets[ASSET_INDEX(tex)].failed)));
    Oct_AssetData *d = _oct_AssetGetSafe(tex, OCT_ASSET_TYPE_TEXTURE);
    if (d)
        return (float) SDL_GetAtomicInt(&d->texture.width);
    return 0;
}

OCTARINE_API float oct_TextureHeight(Oct_Texture tex) {
    while (SDL_GetAtomicInt(&gAssets[ASSET_INDEX(tex)].reserved) && !(SDL_GetAtomicInt(&gAssets[ASSET_INDEX(tex)].loaded) || SDL_GetAtomicInt(&gAssets[ASSET_INDEX(tex)].failed)));
    Oct_AssetData *d = _oct_AssetGetSafe(tex, OCT_ASSET_TYPE_TEXTURE);
    if (d)
        return (float) SDL_GetAtomicInt(&d->texture.height);
    return 0;
}

OCTARINE_API void oct_GetTextSize(Oct_FontAtlas atlas, Oct_Vec2 outSize, float scale, const char *fmt, ...) {
    while (SDL_GetAtomicInt(&gAssets[ASSET_INDEX(atlas)].reserved) && !(SDL_GetAtomicInt(&gAssets[ASSET_INDEX(atlas)].loaded) || SDL_GetAtomicInt(&gAssets[ASSET_INDEX(atlas)].failed)));
    va_list l;
    va_start(l, fmt);
    SDL_vsnprintf(gTextBuffer, TEXT_BUFFER_SIZE - 1, fmt, l);
    va_end(l);
    outSize[0] = 0;
    outSize[1] = 0;

    Oct_AssetData *asset = _oct_AssetGetSafe(atlas, OCT_ASSET_TYPE_FONT_ATLAS);
    if (!asset)
        return;
    Oct_BitmapFontData *atlasData = &asset->fontAtlas;

    Oct_AssetData *fontAsset = atlas != OCT_NO_ASSET ? _oct_AssetGetSafe(atlasData->font, OCT_ASSET_TYPE_FONT) : null;
    if (!fontAsset)
        return;
    Oct_FontData *font = &fontAsset->font;

    const char *t = gTextBuffer;
    uint32_t codePoint = SDL_StepUTF8(&t, null);
    float x = 0;
    float y = 0;
    uint32_t previousCodePoint = UINT32_MAX;
    while (codePoint) {
        if (codePoint == '\n') {
            x = 0;
            y += atlasData->newLineSize * scale;

            codePoint = SDL_StepUTF8(&t, null);
            previousCodePoint = UINT32_MAX;
            continue;
        } else if (codePoint == ' ') {
            x += atlasData->spaceSize * scale;
            codePoint = SDL_StepUTF8(&t, null);
            previousCodePoint = UINT32_MAX;
            continue;
        }

        // For each character, check each layer in the atlas until we either run out
        // of layers, or find an atlas that contains the given character
        int layer = -1;
        for (int i = 0; i < atlasData->atlasCount; i++) {
            if (codePoint >= atlasData->atlases[i].unicodeStart && codePoint < atlasData->atlases[i].unicodeEnd) {
                layer = i;
                break;
            }
        }

        if (layer != -1) {
            vk2dRendererDrawTexture(
                    atlasData->atlases[layer].atlas,
                    x, y,
                    scale, scale,
                    0, 0, 0,
                    atlasData->atlases[layer].glyphs[codePoint - atlasData->atlases[layer].unicodeStart].location.position[0],
                    atlasData->atlases[layer].glyphs[codePoint - atlasData->atlases[layer].unicodeStart].location.position[1],
                    atlasData->atlases[layer].glyphs[codePoint - atlasData->atlases[layer].unicodeStart].location.size[0],
                    atlasData->atlases[layer].glyphs[codePoint - atlasData->atlases[layer].unicodeStart].location.size[1]
            );
            x += atlasData->atlases[layer].glyphs[codePoint - atlasData->atlases[layer].unicodeStart].advance * scale;
            if (outSize[0] < x) outSize[0] = x;
            if (outSize[1] < y + (atlasData->atlases[layer].glyphs[codePoint - atlasData->atlases[layer].unicodeStart].location.size[1] * scale)) outSize[1] = y + (atlasData->atlases[layer].glyphs[codePoint - atlasData->atlases[layer].unicodeStart].location.size[1] * scale);

            // Find additional kerning
            if (previousCodePoint != UINT32_MAX && font) {
                int kern;
                TTF_GetGlyphKerning(font->font[0], previousCodePoint, codePoint, &kern);
                x += kern;
            }
            previousCodePoint = codePoint;
        }

        codePoint = SDL_StepUTF8(&t, null);
    }
}
