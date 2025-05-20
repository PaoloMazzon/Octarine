#include <SDL3/SDL.h>

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
