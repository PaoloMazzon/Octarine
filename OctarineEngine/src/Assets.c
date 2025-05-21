#include <stdarg.h>
#include <SDL3/SDL.h>
#include <VK2D/VK2D.h>
#include <stdio.h>

#include "oct/Common.h"
#include "oct/Allocators.h"
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
#define ERROR_BUFFER_SIZE 1024
static char gErrorMessage[ERROR_BUFFER_SIZE];
static SDL_Mutex *gErrorMessageMutex;
static SDL_AtomicInt gErrorHasOccurred;
static TTF_TextEngine *gTextEngine;

///////////////////////////////// ASSET CREATION HELP /////////////////////////////////
void _oct_LogError(const char *fmt, ...) {
    va_list l;
    va_start(l, fmt);
    const int len = strlen(gErrorMessage);
    vsnprintf(&gErrorMessage[len], ERROR_BUFFER_SIZE - len - 1, fmt, l);
    va_end(l);
}

// Destroys metadata for an asset when its being freed
void _oct_DestroyAssetMetadata(Oct_Asset asset) {
    SDL_SetAtomicInt(&gAssets[asset].loaded, 0);
    SDL_SetAtomicInt(&gAssets[asset].reserved, 0);
    SDL_SetAtomicInt(&gAssets[asset].failed, 0);
    SDL_AddAtomicInt(&gAssets[ASSET_INDEX(asset)].generation, 1);
}

void _oct_FailLoad(Oct_Asset asset) {
    SDL_SetAtomicInt(&gAssets[asset].failed, 1);
    SDL_SetAtomicInt(&gAssets[asset].reserved, 1);
    SDL_SetAtomicInt(&gErrorHasOccurred, 1);
}

///////////////////////////////// ASSET CREATION /////////////////////////////////
void _oct_AssetCreateTexture(Oct_LoadCommand *load) {
    VK2DTexture tex = vk2dTextureLoad(load->Texture.filename);
    if (tex) {
        gAssets[ASSET_INDEX(load->_assetID)].texture = tex;
        gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_TEXTURE;
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
    } else {
        _oct_FailLoad(load->_assetID);
        _oct_LogError("Failed to load texture \"%s\"\n", load->Texture.filename);
    }
}

static void _oct_AssetCreateSurface(Oct_LoadCommand *load) {
    VK2DTexture tex = vk2dTextureCreate(load->Surface.dimensions[0], load->Surface.dimensions[1]);
    if (tex) {
        gAssets[ASSET_INDEX(load->_assetID)].texture = tex;
        gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_TEXTURE;
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
    } else {
        _oct_FailLoad(load->_assetID);
        _oct_LogError("Failed to create surface of dimensions %.2f/%.2f\n", load->Surface.dimensions[0], load->Surface.dimensions[1]);
    }
}

static void _oct_AssetCreateCamera(Oct_LoadCommand *load) {
    VK2DCameraIndex camIndex = vk2dCameraCreate(vk2dCameraGetSpec(VK2D_DEFAULT_CAMERA));
    if (camIndex != VK2D_INVALID_CAMERA) {
        gAssets[ASSET_INDEX(load->_assetID)].camera = camIndex;
        gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_CAMERA;
        SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
    } else {
        _oct_FailLoad(load->_assetID);
        _oct_LogError("Failed to create camera\n");
    }
}

void _oct_AssetCreateAudio(Oct_LoadCommand *load) {
    // Find file extension
    const char *extension = strrchr(load->Audio.filename, '.');
    extension = extension == null ? "" : extension;
    uint8_t *data = null;
    uint32_t dataSize = 0;
    SDL_AudioSpec spec;

    if (strcmp(extension, ".ogg") == 0) {
        // TODO: Implement ogg loading
        _oct_FailLoad(load->_assetID);
        _oct_LogError("Failed to load audio sample \"%s\", ogg is not yet supported.\n", load->Audio.filename);
    } else if (strcmp(extension, ".wav") == 0) {
        // Use SDL to load wavs
        if (!SDL_LoadWAV(load->Audio.filename, &spec, &data, &dataSize)) {
            _oct_FailLoad(load->_assetID);
            _oct_LogError("Failed to load audio sample \"%s\", SDL Error: %s.\n", load->Audio.filename, SDL_GetError());
        }
    } else if (strcmp(extension, ".mp3") == 0) {
        // TODO: Implement mp3 loading
        _oct_FailLoad(load->_assetID);
        _oct_LogError("Failed to load audio sample \"%s\", mp3 is not yet supported.\n", load->Audio.filename);
    } else {
        _oct_FailLoad(load->_assetID);
        _oct_LogError("Failed to load audio sample \"%s\", unrecognized extension.\n", load->Audio.filename);
    }

    // Convert format if we found good data
    if (data) {
        int32_t newSize;
        uint8_t *newSamples = _oct_AudioConvertFormat(data, dataSize, &newSize, &spec);
        if (newSamples) {
            gAssets[ASSET_INDEX(load->_assetID)].audio.size = newSize;
            gAssets[ASSET_INDEX(load->_assetID)].audio.data = newSamples;
            SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
        } else {
            _oct_FailLoad(load->_assetID);
            _oct_LogError("Failed to load audio sample \"%s\", failed to convert format, SDL Error %s.\n", load->Audio.filename, SDL_GetError());
        }
        SDL_free(data);
    }
}

void _oct_AssetCreateSprite(Oct_LoadCommand *load) {
    Oct_SpriteData *data = &gAssets[ASSET_INDEX(load->_assetID)].sprite;
    gAssets[ASSET_INDEX(load->_assetID)].type = OCT_ASSET_TYPE_SPRITE;
    data->texture = load->Sprite.texture;
    data->ownsTexture = false;
    data->frameCount = load->Sprite.frameCount;
    data->frame = 0;
    data->repeat = load->Sprite.repeat;
    data->pause = false;
    data->delay = 1.0 / load->Sprite.fps;
    data->lastTime = oct_Time();
    data->accumulator = 0;
    data->startPos[0] = load->Sprite.startPos[0];
    data->startPos[1] = load->Sprite.startPos[1];
    data->frameSize[0] = load->Sprite.frameSize[0];
    data->frameSize[1] = load->Sprite.frameSize[1];
    data->padding[0] = load->Sprite.padding[0];
    data->padding[1] = load->Sprite.padding[1];
    data->xStop = load->Sprite.xStop;
    SDL_SetAtomicInt(&gAssets[ASSET_INDEX(load->_assetID)].loaded, 1);
}

void _oct_AssetCreateFont(Oct_LoadCommand *load) {
    Oct_AssetData *data = &gAssets[ASSET_INDEX(load->_assetID)];
    Oct_FontData *fnt = &data->font;
    data->type = OCT_ASSET_TYPE_FONT;
    Oct_Bool error = false;

    // Load all fonts
    for (int i = 0; i < OCT_FALLBACK_FONT_MAX; i++) {
        if (load->Font.filename[i] != null) {
            fnt->font[i] = TTF_OpenFont(load->Font.filename[i], load->Font.size);

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
    } else {
        _oct_FailLoad(load->_assetID);
        _oct_LogError("Failed to create font \"%s\", TTF error %s\n", load->Font.filename[0], SDL_GetError());
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
    if (_oct_AssetGet(load->FontAtlas.font)->type != OCT_ASSET_TYPE_FONT || !SDL_GetAtomicInt(&_oct_AssetGet(load->FontAtlas.font)->loaded)) {
        _oct_LogError("Atlas cannot be created without a font.\n");
        _oct_FailLoad(asset);
        return;
    }
    Oct_FontData *fntData = &_oct_AssetGet(load->FontAtlas.font)->font;

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
        TTF_GetStringSize(fntData->font[0], string, 0, &w, &h);
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
    atlas->img = vk2dImageFromPixels(vk2dRendererGetDevice(), tempSurface->pixels, imgWidth, imgHeight, true);
    atlas->atlas = vk2dTextureLoadFromImage(atlas->img);
    SDL_SetAtomicInt(&gAssets[ASSET_INDEX(asset)].loaded, 1);
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

    const uint32_t glyphCount = load->BitmapFont.unicodeEnd - load->BitmapFont.unicodeStart;
    asset->fontAtlas.atlases[0].img = null;
    asset->fontAtlas.atlases[0].atlas = vk2dTextureLoad(load->BitmapFont.filename);
    asset->fontAtlas.atlases[0].glyphs = mi_malloc(sizeof(struct Oct_FontGlyphData_t) * glyphCount);
    asset->fontAtlas.atlases[0].unicodeStart = load->BitmapFont.unicodeStart;
    asset->fontAtlas.atlases[0].unicodeEnd = load->BitmapFont.unicodeEnd;
    asset->fontAtlas.spaceSize = load->BitmapFont.cellSize[0];
    asset->fontAtlas.newLineSize = load->BitmapFont.cellSize[1];

    if (!asset->fontAtlas.atlases[0].atlas) {
        _oct_FailLoad(load->_assetID);
        _oct_LogError("Failed to create bitmap font from \"%s\", VK2D error: %s\n", load->BitmapFont.filename, vk2dStatusMessage());
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
}

void _oct_AssetCreateAssetBundle(Oct_LoadCommand *load);

///////////////////////////////// ASSET DESTRUCTION /////////////////////////////////
static void _oct_AssetDestroyTexture(Oct_Asset asset) {
    vk2dRendererWait();
    vk2dTextureFree(gAssets[ASSET_INDEX(asset)].texture);
    _oct_DestroyAssetMetadata(asset);
}

static void _oct_AssetDestroyCamera(Oct_Asset asset) {
    vk2dCameraSetState(gAssets[ASSET_INDEX(asset)].camera, VK2D_CAMERA_STATE_DELETED);
    _oct_DestroyAssetMetadata(asset);
}

static void _oct_AssetDestroySprite(Oct_Asset asset) {
    if (gAssets[ASSET_INDEX(asset)].sprite.ownsTexture) {
        // TODO: Free texture if sprite owns it
    }
    _oct_DestroyAssetMetadata(asset);
}

static void _oct_AssetDestroyAudio(Oct_Asset asset) {
    SDL_free(gAssets[ASSET_INDEX(asset)].audio.data);
    _oct_DestroyAssetMetadata(asset);
}

void _oct_AssetDestroyFont(Oct_Asset asset) {
    for (int i = 0; i < OCT_FALLBACK_FONT_MAX; i++) {
        if (gAssets[ASSET_INDEX(asset)].font.font[i])
            TTF_CloseFont(gAssets[ASSET_INDEX(asset)].font.font[i]);
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
    } else if (load->type == OCT_LOAD_COMMAND_TYPE_FREE) {
        if (SDL_GetAtomicInt(&gAssets[load->_assetID].loaded))
            _oct_AssetDestroy(load->_assetID);
    }
}

Oct_AssetType _oct_AssetType(Oct_Asset asset) {
    return SDL_GetAtomicInt(&gAssets[ASSET_INDEX(asset)].loaded) ? gAssets[ASSET_INDEX(asset)].type : OCT_ASSET_TYPE_NONE;
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

OCTARINE_API const char *oct_AssetErrorMessage(Oct_Allocator allocator) {
    char *out = null;
    SDL_LockMutex(gErrorMessageMutex);
    const int slen = strlen(gErrorMessage);
    const int len = slen >= ERROR_BUFFER_SIZE ? ERROR_BUFFER_SIZE - 1 : slen;

    // Allocate new string and copy it
    out = oct_Malloc(allocator, len + 1);
    if (out) {
        strncpy(out, gErrorMessage, len);
        out[len] = 0;
        SDL_SetAtomicInt(&gErrorHasOccurred, 0);
        gErrorMessage[0] = 0;
    }

    SDL_UnlockMutex(gErrorMessageMutex);
    return out;
}

OCTARINE_API const char *oct_AssetGetErrorMessage(int *size, char *buffer) {
    if (buffer) {
        const int slen = strlen(gErrorMessage);
        const int len = slen >= ERROR_BUFFER_SIZE ? ERROR_BUFFER_SIZE - 1 : slen;

        // Allocate new string and copy it
        strncpy(buffer, gErrorMessage, len);
        buffer[len] = 0;

        gErrorMessage[0] = 0;
        SDL_SetAtomicInt(&gErrorHasOccurred, 0);
        SDL_UnlockMutex(gErrorMessageMutex);
    } else if (size) {
        SDL_LockMutex(gErrorMessageMutex);
        const int slen = strlen(gErrorMessage);
        const int len = slen >= ERROR_BUFFER_SIZE ? ERROR_BUFFER_SIZE - 1 : slen;
        *size = len + 1;
    }
    return buffer;
}

OCTARINE_API Oct_Bool oct_AssetLoadHasFailed() {
    return SDL_GetAtomicInt(&gErrorHasOccurred);
}
