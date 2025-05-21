#include <SDL3/SDL.h>
#include <physfs.h>

#include "oct/cJSON.h"
#include "oct/Common.h"
#include "oct/Opaque.h"
#include "oct/Validation.h"
#include "oct/Assets.h"

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
    for (int i = 0; i < OCT_BUCKET_SIZE; i++) {
        if (bundle->bucket[i].asset != OCT_NO_ASSET)
            oct_FreeAsset(bundle->bucket[i].asset);
    }
    for (int i = 0; i < bundle->backupBucketCount; i++) {
        if (bundle->backupBucket[i].asset != OCT_NO_ASSET)
            oct_FreeAsset(bundle->backupBucket[i].asset);
    }
    mi_free(bundle->bucket);
    mi_free(bundle->backupBucket);
    mi_free(bundle);
}

OCTARINE_API Oct_Bool oct_IsAssetBundleReady(Oct_AssetBundle bundle) {
    return SDL_GetAtomicInt(&bundle->bundleReady);
}

OCTARINE_API Oct_Asset oct_GetAsset(Oct_AssetBundle bundle, const char *name) {
    // Wait till the bundle is loaded
    while (!SDL_GetAtomicInt(&bundle->bundleReady));

    // Get expected location
    uint32_t bucketLocation = hash(name) % OCT_BUCKET_SIZE;

    // Traverse linked list till we find it
    Oct_AssetLink *link = &bundle->bucket[bucketLocation];
    while (link) {
        if (strcmp(name, link->name) == 0)
            return link->asset;
        link = link->next;
    }

    return OCT_NO_ASSET;
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

void _oct_AssetCreateAssetBundle(Oct_LoadCommand *load) {
    // 1. Go through each file in the bundle and load the primitive types by their filenames
    // 2. Iterate through manifest.json and load the non-primitive types like sprites
    // 3. For each asset, create a load command for them and manually invoke their create function
    // 4. Set the ready atomic to 1
    if (PHYSFS_mount(load->AssetBundle.filename, NULL, 0)) {
        if (!PHYSFS_exists("manifest.json")) {
            _oct_LogError("Failed to load asset bundle from \"%s\", no manifest\n", load->AssetBundle.filename);
            return;
        }

        // Grab json for parsing as we go along
        PHYSFS_File *manifestFile = PHYSFS_openRead("manifest.json");
        uint32_t manifestBufferSize = PHYSFS_fileLength(manifestFile);
        uint8_t *manifestBuffer = mi_malloc(manifestBufferSize);
        if (!manifestBuffer)
            oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate manifest buffer");
        PHYSFS_readBytes(manifestFile, manifestBuffer, manifestBufferSize);
        PHYSFS_close(manifestFile);
        cJSON *manifestJSON = cJSON_ParseWithLength((void*)manifestBuffer, manifestBufferSize);

        // Find the exclude list
        cJSON *excludeList = jsonGetWithType(cJSON_GetObjectItem(manifestJSON, "exclude"), type_array);

        // Iterate through primitive types first
        // TODO: This

        // Iterate over advanced types (sprites, fonts, ...)
        // TODO: This

        // Cleanup
        cJSON_Delete(manifestJSON);
        mi_free(manifestBuffer);
        SDL_SetAtomicInt(&load->AssetBundle.bundle->bundleReady, 1);
    } else {
        _oct_LogError("Failed to load asset bundle from \"%s\"\n", load->AssetBundle.filename);
    }
}
