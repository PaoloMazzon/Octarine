#include <SDL3/SDL.h>
#include "oct/Validation.h"
#include "oct/JobSystem.h"

typedef struct Job_t {
    Oct_JobFunction job;
} Job;

// Globals
#define JOB_RINGBUFFER_SIZE  250

// Ring buffer, uses a mutex because any thread can access it
static Job gRingBuffer[JOB_RINGBUFFER_SIZE];
static int32_t gRingBufferTail; // read head, if tail == head then there is no more data to read
static int32_t gRingBufferHead; // write head, if head == tail - 1 the buffer is full and no more writing can be done
static SDL_Mutex *gRingMutex;

////////////////////////////////// RING BUFFER //////////////////////////////////
// Returns false if the push fails (ring buffer full)
Oct_Bool ringBufferPush(Job *job) {
    Oct_Bool cooked = true;
    SDL_LockMutex(gRingMutex);
    if (gRingBufferHead != (gRingBufferTail - 1) % JOB_RINGBUFFER_SIZE) {
        // there is a spot we can write to
        memcpy(job, &gRingBuffer[gRingBufferHead], sizeof(struct Job_t));
        cooked = false;
        gRingBufferHead++;
    }
    SDL_UnlockMutex(gRingMutex);
    return !cooked;
}

// Returns false if the pop fails (there are no jobs available)
Oct_Bool ringBufferPop(Job *job) {
    Oct_Bool cooked = true;
    SDL_LockMutex(gRingMutex);
    if (gRingBufferTail != gRingBufferHead) {
        // there is a spot we can read from
        memcpy(&gRingBuffer[gRingBufferTail], job, sizeof(struct Job_t));
        cooked = false;
        gRingBufferTail++;
    }
    SDL_UnlockMutex(gRingMutex);
    return !cooked;
}

////////////////////////////////// INTERNAL //////////////////////////////////

void _oct_JobsInit() {
    gRingMutex = SDL_CreateMutex();
    if (!gRingMutex)
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to create job system rinbugger mutex, SDL error %s", SDL_GetError());
}

void _oct_JobsUpdate() {
    // TODO: This
}

void _oct_JobsEnd() {
    SDL_DestroyMutex(gRingMutex);
}

////////////////////////////////// PUBLIC API //////////////////////////////////

OCTARINE_API void oct_QueueJob(const Oct_JobFunction job) {
    // TODO: This
}

OCTARINE_API Oct_Bool oct_JobsBusy() {
    return false;// TODO: This
}

OCTARINE_API void oct_WaitJobs() {
    // TODO: This
}
