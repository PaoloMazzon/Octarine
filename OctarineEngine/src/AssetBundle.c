#include <SDL3/SDL.h>
#include <physfs.h>

#include "oct/Core.h"
#include "oct/cJSON.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"
#include "oct/Assets.h"
#include "oct/Subsystems.h"

static uint32_t hash(const char *str) {
    uint32_t hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void _oct_PlaceAssetInBucket(Oct_AssetBundle bundle, Oct_Asset asset, const char *name) {
    const char *copy = mi_strdup(name);
    uint32_t bucketLocation = hash(copy) % OCT_BUCKET_SIZE;

    if (bundle->bucket[bucketLocation].asset == OCT_NO_ASSET || strcmp(bundle->bucket[bucketLocation].name, copy) == 0) {
        // This spot is empty
        bundle->bucket[bucketLocation].asset = asset;
        bundle->bucket[bucketLocation].name = copy;
    } else {
        // In case we find the spot in the linked list early
        bool foundSpot = false;

        // This spot is taken, find the end of the linked list
        Oct_AssetLink *current = &bundle->bucket[bucketLocation];
        while (current->next) {
            current = current->next;

            // In case the same name is just deep in the linked list
            if (strcmp(current->name, copy) == 0) {
                current->asset = asset;
                foundSpot = true;
                break;
            }
        }

        if (!foundSpot) {
            // Find a spot in the extended bucket list
            if (bundle->backupBucketCount == bundle->backupBucketSize) {
                void *temp = mi_realloc(bundle->backupBucket,
                                        sizeof(struct Oct_AssetLink_t) * (bundle->backupBucketSize + 10));
                if (!temp)
                    oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to reallocate backup bucket.");
                bundle->backupBucket = temp;
                bundle->backupBucketSize += 10;
                bundle->backupBucketCount++;
            }
            const int32_t extendedBucketSpot = bundle->backupBucketCount++;

            current->next = &bundle->backupBucket[extendedBucketSpot];
            bundle->backupBucket[extendedBucketSpot].asset = asset;
            bundle->backupBucket[extendedBucketSpot].name = copy;
        }
    }
}

OCTARINE_API void oct_FreeAssetBundle(Oct_AssetBundle bundle) {
    if (!bundle)
        return;
    for (int i = 0; i < OCT_BUCKET_SIZE; i++) {
        if (bundle->bucket[i].asset != OCT_NO_ASSET) {
            oct_FreeAsset(bundle->bucket[i].asset);
            mi_free((void*)bundle->bucket[i].name);
        }
    }
    for (int i = 0; i < bundle->backupBucketCount; i++) {
        if (bundle->backupBucket[i].asset != OCT_NO_ASSET) {
            oct_FreeAsset(bundle->backupBucket[i].asset);
            mi_free((void*)bundle->backupBucket[i].name);
        }
    }
    mi_free(bundle->bucket);
    mi_free(bundle->backupBucket);
    mi_free(bundle);
}

OCTARINE_API Oct_Bool oct_IsAssetBundleReady(Oct_AssetBundle bundle) {
    return SDL_GetAtomicInt(&bundle->bundleReady);
}

// For internal use
static Oct_Asset _oct_GetAssetUnblocking(Oct_AssetBundle bundle, const char *name) {
    // Get expected location
    uint32_t bucketLocation = hash(name) % OCT_BUCKET_SIZE;

    // Traverse linked list till we find it
    Oct_AssetLink *link = &bundle->bucket[bucketLocation];
    while (link) {
        if (link->name && strcmp(name, link->name) == 0)
            return link->asset;
        link = link->next;
    }

    return OCT_NO_ASSET;
}

OCTARINE_API Oct_Asset oct_GetAsset(Oct_AssetBundle bundle, const char *name) {
    // Wait till the bundle is loaded
    while (!SDL_GetAtomicInt(&bundle->bundleReady));

    return _oct_GetAssetUnblocking(bundle, name);
}

OCTARINE_API Oct_Bool oct_AssetExists(Oct_AssetBundle bundle, const char *name, Oct_AssetType type) {
    // Wait till the bundle is loaded
    while (!SDL_GetAtomicInt(&bundle->bundleReady));

    // Get expected location
    uint32_t bucketLocation = hash(name) % OCT_BUCKET_SIZE;

    // Traverse linked list till we find it
    Oct_AssetLink *link = &bundle->bucket[bucketLocation];
    while (link) {
        if (strcmp(name, link->name) == 0)
            return true;
        link = link->next;
    }

    return false;
}

void _oct_AssetCreateTexture(Oct_LoadCommand *load);
void _oct_AssetCreateAudio(Oct_LoadCommand *load);
void _oct_AssetCreateSprite(Oct_LoadCommand *load);
void _oct_AssetCreateFont(Oct_LoadCommand *load);
void _oct_AssetCreateFontAtlas(Oct_LoadCommand *load);
void _oct_AssetCreateBitmapFont(Oct_LoadCommand *load);
void _oct_LogError(const char *fmt, ...);
void _oct_DestroyAssetMetadata(Oct_Asset asset);
void _oct_FailLoad(Oct_Asset asset);
typedef enum {
    type_array,
    type_map,
    type_string,
    type_bool,
    type_num,
    type_null
} json_type;

// Returns null if item is null or item's type is not the specified type
static cJSON *jsonGetWithType(cJSON *item, json_type type) {
    if (!item)
        return null;
    if (type == type_array && cJSON_IsArray(item))
        return item;
    if (type == type_map && cJSON_IsObject(item))
        return item;
    if (type == type_string && cJSON_IsString(item))
        return item;
    if (type == type_bool && cJSON_IsBool(item))
        return item;
    if (type == type_num && cJSON_IsNumber(item))
        return item;
    if (type == type_null && cJSON_IsNull(item))
        return item;
    return null;
}

// Returns true if a string is in the exclude list
static Oct_Bool _oct_InExcludeList(cJSON *excludeList, const char *string) {
    int32_t size = cJSON_GetArraySize(excludeList);
    for (int32_t i = 0; i < size; i++) {
        cJSON *jsonString = jsonGetWithType(cJSON_GetArrayItem(excludeList, i), type_string);
        if (jsonString && strcmp(cJSON_GetStringValue(jsonString), string) == 0)
            return true;
    }
    return false;
}

// Returns true if the two strings are equal ignoring case
static Oct_Bool _oct_TextEqual(const char *s1, const char *s2) {
    if (!s1 || !s2)
        return false;
    if (strlen(s1) != strlen(s2))
        return false;
    for (int i = 0; i < strlen(s1); i++) {
        const char sc1 = s1[i] >= 'a' && s1[i] <= 'z' ? s1[i] - 32 : s1[i];
        const char sc2 = s2[i] >= 'a' && s2[i] <= 'z' ? s2[i] - 32 : s2[i];
        if (sc1 != sc2)
            return false;
    }
    return true;
}

static uint8_t *_oct_PhysFSGetFile(const char *filename, uint32_t *size) {
    PHYSFS_File *file = PHYSFS_openRead(filename);
    *size = PHYSFS_fileLength(file);
    uint8_t *buffer = mi_malloc(*size);
    if (!buffer)
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate buffer");
    PHYSFS_readBytes(file, buffer, *size);
    PHYSFS_close(file);
    return buffer;
}

void _oct_FileHandleCallback(void *buffer, uint32_t size) {
    mi_free(buffer);
}

// This will add the path of one filename to the other, returned string is only valid till next call
// for example, _oct_AddRootDir("image.png", "path/to/thing.json") -> "path/to/image.png"
const char *_oct_AddRootDir(const char *filename, const char *path) {
    static char buffer[512] = {0};
    if (!filename || !path)
        return null;

    // Find the last / (or \ for stupid fucking windows) in the path
    const char *lastSlash = strrchr(path, '/');
    if (!lastSlash)
        lastSlash = strrchr(path, '\\');

    // If there is no last slash then path is root-level, so just return filename
    if (!lastSlash)
        return filename;
    lastSlash++;

    // Buffer overflow bad
    if ((lastSlash - path) + strlen(filename) + 1 > 512)
        return "";

    // It is not root level so copy everything up to the last slash then add filename
    memcpy(buffer, path, (lastSlash - path));
    memcpy(buffer + ((lastSlash - path)), filename, strlen(filename));
    buffer[(lastSlash - path) + strlen(filename)] = 0;

    return buffer;
}

#define jsonGetNum(json, def) cJSON_IsNumber(json) ? cJSON_GetNumberValue(json) : 0

// Parses the json's frame data into a sprite's frame
void _oct_AddFrameData(Oct_SpriteFrame *frame, cJSON *frameJSON) {
    memset(frame, 0, sizeof(struct Oct_SpriteFrame_t));
    if (!cJSON_IsObject(frameJSON))
        return;

    cJSON *frameSize = jsonGetWithType(cJSON_GetObjectItem(frameJSON, "frame"), type_map);
    if (frameSize) {
        frame->position[0] = jsonGetNum(cJSON_GetObjectItem(frameSize, "x"), 0);
        frame->position[1] = jsonGetNum(cJSON_GetObjectItem(frameSize, "y"), 0);
        frame->size[0] = jsonGetNum(cJSON_GetObjectItem(frameSize, "w"), 0);
        frame->size[1] = jsonGetNum(cJSON_GetObjectItem(frameSize, "h"), 0);
    }

    frame->duration = jsonGetNum(jsonGetWithType(cJSON_GetObjectItem(frameJSON, "duration"), type_num), 100);
    frame->duration = frame->duration / 1000;
}

// This will check a given json to see if it is a json containing a spritesheet (exported by
// aseprite or some other program). If it is, it will add the spritesheet as an asset to the
// bundle under the json's name.
void _oct_CheckAndAddJSONSpriteSheet(Oct_AssetBundle bundle, const char *jsonFilename) {
    // Get json
    uint32_t size;
    uint8_t *jsonBuffer = _oct_PhysFSGetFile(jsonFilename, &size);
    cJSON *json = cJSON_ParseWithLength((void*)jsonBuffer, size);

    // Look for meta["image"]
    cJSON *meta = jsonGetWithType(cJSON_GetObjectItem(json, "meta"), type_map);
    cJSON *image = jsonGetWithType(cJSON_GetObjectItem(meta, "image"), type_string);

    // Check if we actually gottem
    Oct_Texture tex = OCT_NO_ASSET;
    if (image) {
        // Interpolate image filename to find it if this isn't the root
        const char *imageFilename = _oct_AddRootDir(cJSON_GetStringValue(image), jsonFilename);
        tex = _oct_GetAssetUnblocking(bundle, imageFilename);

        if (tex == OCT_NO_ASSET) {
            _oct_LogError("\"s\" is an invalid json for loading sprites, as the texture \"%s\" does not exist. Make sure the spritesheet texture is in the same directory as the json.", jsonFilename, cJSON_GetStringValue(image));
            cJSON_Delete(json);
            mi_free(jsonBuffer);
            return;
        }
    }
    if (tex != OCT_NO_ASSET) {
        // This is a valid spritesheet and we have the corresponding texture
        cJSON *frames = jsonGetWithType(cJSON_GetObjectItem(json, "frames"), type_array);
        if (!frames) {
            _oct_LogError("\"%s\" is an invalid json for loading sprites, make sure you are exporting a \"Array.\"", jsonFilename);
            cJSON_Delete(json);
            mi_free(jsonBuffer);
            return;
        }

        // Create the sprite
        Oct_Asset asset = _oct_AssetReserveSpace();
        Oct_SpriteData *data = &_oct_AssetGet(asset)->sprite;
        _oct_AssetGet(asset)->type = OCT_ASSET_TYPE_SPRITE;
        data->texture = tex;
        data->frameCount = cJSON_GetArraySize(frames);
        data->frame = 0;
        data->repeat = true;
        data->pause = false;
        data->lastTime = oct_Time();
        data->accumulator = 0;

        // Allocate the frames
        data->frames = mi_malloc(sizeof(struct Oct_SpriteFrame_t) * data->frameCount);
        if (!data->frames)
            oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate sprite frame data");

        // Loop each frame
        for (int i = 0; i < cJSON_GetArraySize(frames); i++) {
            _oct_AddFrameData(&data->frames[i], cJSON_GetArrayItem(frames, i));
        }

        // Make sprite ready
        SDL_SetAtomicInt(&_oct_AssetGet(asset)->loaded, 1);
        _oct_PlaceAssetInBucket(bundle, asset, jsonFilename);
    }

    // Cleanup
    cJSON_Delete(json);
    mi_free(jsonBuffer);
}

// Extends a path, ie _oct_ExtendPath("/", "folder", ...) -> "/folder/"
const char *_oct_ExtendPath(const char *root, const char *folder, char *buffer, int32_t size) {
    if (strlen(folder) + strlen(root) + 2 > size) return "";
    memcpy(buffer, root, strlen(root));
    memcpy(buffer + strlen(root), folder, strlen(folder));
    buffer[strlen(root) + strlen(folder)] = '/';
    buffer[strlen(root) + strlen(folder) + 1] = 0;
    return buffer;
}

// Same as above but for filenames _oct_ExtendPath("/", "folder", ...) -> "/folder/"
const char *_oct_ExtendFilename(const char *root, const char *folder, char *buffer, int32_t size) {
    if (strlen(folder) + strlen(root) + 1 > size) return "";
    memcpy(buffer, root, strlen(root));
    memcpy(buffer + strlen(root), folder, strlen(folder));
    buffer[strlen(root) + strlen(folder)] = 0;
    return buffer;
}

void _oct_EnumerateDirectory(Oct_AssetBundle bundle, cJSON *excludeList, const char *directory) {
    // Need temp memory to store directory stuff as to not stack overflow
    const uint32_t BUFFER_SIZE = 1024;
    char *filenameBuffer = mi_malloc(BUFFER_SIZE);
    char *directoryBuffer = mi_malloc(BUFFER_SIZE);
    if (!filenameBuffer || !directoryBuffer)
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate temporary directory memory.");

    // Iterate through primitive types first
    const char **fileList = (void*)PHYSFS_enumerateFiles(directory);
    for (int i = 0; fileList[i]; i++) {
        const char *completeFilename = _oct_ExtendFilename(directory, fileList[i], filenameBuffer, BUFFER_SIZE);

        // Ignore excluded files/directories
        // TODO: Directories in exclude list should be allowed to end with a '/'
        if (_oct_InExcludeList(excludeList, completeFilename))
            continue;

        PHYSFS_Stat stat;
        if (PHYSFS_stat(fileList[i], &stat) && stat.filetype == PHYSFS_FILETYPE_DIRECTORY) {
            _oct_EnumerateDirectory(bundle, excludeList, _oct_ExtendPath(directory, completeFilename, directoryBuffer, BUFFER_SIZE));
        }

        const char *extension = strrchr(completeFilename, '.');
        if (_oct_TextEqual(extension, ".mp3") || _oct_TextEqual(extension, ".wav") || _oct_TextEqual(extension, ".ogg")) {
            // Audio
            uint32_t size;
            uint8_t *buffer = _oct_PhysFSGetFile(completeFilename, &size);
            Oct_LoadCommand l;
            l.Audio.fileHandle.type = OCT_FILE_HANDLE_TYPE_FILE_BUFFER;
            l.Audio.fileHandle.buffer = buffer;
            l.Audio.fileHandle.size = size;
            l.Audio.fileHandle.name = completeFilename;
            l.Audio.fileHandle.callback = _oct_FileHandleCallback;
            l._assetID = _oct_AssetReserveSpace();
            _oct_PlaceAssetInBucket(bundle, l._assetID, completeFilename);
            _oct_AssetCreateAudio(&l);
        } else if (_oct_TextEqual(extension, ".jpg") || _oct_TextEqual(extension, ".jpeg") || _oct_TextEqual(extension, ".png") || _oct_TextEqual(extension, ".bmp")) {
            // Texture
            uint32_t size;
            uint8_t *buffer = _oct_PhysFSGetFile(completeFilename, &size);
            Oct_LoadCommand l;
            l.Texture.fileHandle.type = OCT_FILE_HANDLE_TYPE_FILE_BUFFER;
            l.Texture.fileHandle.buffer = buffer;
            l.Texture.fileHandle.size = size;
            l.Texture.fileHandle.name = completeFilename;
            l.Texture.fileHandle.callback = _oct_FileHandleCallback;
            l._assetID = _oct_AssetReserveSpace();
            _oct_PlaceAssetInBucket(bundle, l._assetID, completeFilename);
            _oct_AssetCreateTexture(&l);
        }
    }

    // // Find all json spritesheets
    for (int i = 0; fileList[i]; i++) {
        const char *completeFilename = _oct_ExtendFilename(directory, fileList[i], filenameBuffer, BUFFER_SIZE);

        if (_oct_InExcludeList(excludeList, completeFilename))
            continue;

        const char *extension = strrchr(completeFilename, '.');
        // This checks if a json is a spritesheet
        if (_oct_TextEqual(extension, ".json")) {
            _oct_CheckAndAddJSONSpriteSheet(bundle, completeFilename);
        }
    }
    PHYSFS_freeList(fileList);
    mi_free(filenameBuffer);
    mi_free(directoryBuffer);
}

void _oct_ParseFonts(Oct_AssetBundle bundle, cJSON *fonts) {
    if (!fonts) return;
    // TODO: This
}

void _oct_ParseBitmapFonts(Oct_AssetBundle bundle, cJSON *fonts) {
    if (!fonts) return;
    // TODO: This
}

void _oct_ParseSprites(Oct_AssetBundle bundle, cJSON *sprites) {
    if (!sprites) return;
    // TODO: This
}

void _oct_AssetCreateAssetBundle(Oct_LoadCommand *load) {
    // 1. Go through each file in the bundle and load the primitive types by their filenames
    // 2. Iterate through manifest.json and load the non-primitive types like sprites
    // 3. For each asset, create a load command for them and manually invoke their create function
    // 4. Set the ready atomic to 1
    if (PHYSFS_mount(load->AssetBundle.filename, NULL, 0)) {
        if (!PHYSFS_exists("manifest.json")) {
            _oct_LogError("Failed to load asset bundle from \"%s\", no manifest\n", load->AssetBundle.filename);
            _oct_FailLoad(OCT_NO_ASSET);
            SDL_SetAtomicInt(&load->AssetBundle.bundle->bundleReady, 1);
            return;
        }

        // Grab json for parsing as we go along
        uint32_t manifestBufferSize;
        uint8_t *manifestBuffer = _oct_PhysFSGetFile("manifest.json", &manifestBufferSize);
        cJSON *manifestJSON = cJSON_ParseWithLength((void*)manifestBuffer, manifestBufferSize);

        // Find the exclude list
        cJSON *excludeList = jsonGetWithType(cJSON_GetObjectItem(manifestJSON, "exclude"), type_array);

        // Recursively go through directories starting from root
        _oct_EnumerateDirectory(load->AssetBundle.bundle, excludeList, "");

        // Finally, go through the manifest to find other 2nd-order types (bitmap fonts, sprites, etc...)
        _oct_ParseFonts(load->AssetBundle.bundle, jsonGetWithType(cJSON_GetObjectItem(manifestJSON, "fonts"), type_array));
        _oct_ParseBitmapFonts(load->AssetBundle.bundle, jsonGetWithType(cJSON_GetObjectItem(manifestJSON, "bitmap fonts"), type_array));
        _oct_ParseSprites(load->AssetBundle.bundle, jsonGetWithType(cJSON_GetObjectItem(manifestJSON, "sprites"), type_array));

        // Cleanup
        cJSON_Delete(manifestJSON);
        mi_free(manifestBuffer);
        SDL_SetAtomicInt(&load->AssetBundle.bundle->bundleReady, 1);
    } else {
        SDL_SetAtomicInt(&load->AssetBundle.bundle->bundleReady, 1);
        _oct_LogError("Failed to load asset bundle from \"%s\"\n", load->AssetBundle.filename);
        _oct_FailLoad(OCT_NO_ASSET);
    }
}
