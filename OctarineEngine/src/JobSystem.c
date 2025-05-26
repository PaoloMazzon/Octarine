#include <SDL3/SDL.h>
#include "oct/Validation.h"
#include "oct/JobSystem.h"
#include "oct/Opaque.h"
#include "oct/Subsystems.h"

typedef struct Job_t {
    Oct_JobFunction job;
    void *ptr;
} Job;

// Globals
#define JOB_RINGBUFFER_SIZE  250
SDL_Thread **gJobThreads;
uint32_t gJobThreadCount;
SDL_AtomicInt gThreadsWorking;

// Ring buffer, uses a mutex because any thread can access it
static Job gRingBuffer[JOB_RINGBUFFER_SIZE];
static int32_t gRingBufferTail; // read head, if tail == head then there is no more data to read
static int32_t gRingBufferHead; // write head, if head == tail - 1 the buffer is full and no more writing can be done
static SDL_Mutex *gRingMutex;

////////////////////////////////// RING BUFFER //////////////////////////////////
// Returns false if the push fails (ring buffer full)
static Oct_Bool ringBufferPush(Job *job) {
    Oct_Bool cooked = true;
    SDL_LockMutex(gRingMutex);
    if (gRingBufferHead != (gRingBufferTail - 1) % JOB_RINGBUFFER_SIZE) {
        // there is a spot we can write to
        memcpy(&gRingBuffer[gRingBufferHead], job, sizeof(struct Job_t));
        cooked = false;
        gRingBufferHead = (gRingBufferHead + 1) % JOB_RINGBUFFER_SIZE;
    }
    SDL_UnlockMutex(gRingMutex);
    return !cooked;
}

// Returns false if the pop fails (there are no jobs available)
static Oct_Bool ringBufferPop(Job *job) {
    Oct_Bool cooked = true;
    SDL_LockMutex(gRingMutex);
    if (gRingBufferTail != gRingBufferHead) {
        // there is a spot we can read from
        memcpy(job, &gRingBuffer[gRingBufferTail], sizeof(struct Job_t));
        cooked = false;
        gRingBufferTail = (gRingBufferTail + 1) % JOB_RINGBUFFER_SIZE;
    }
    SDL_UnlockMutex(gRingMutex);
    return !cooked;
}

////////////////////////////////// INTERNAL //////////////////////////////////

static int jobThread(void *ptr) {
    Oct_Context ctx = _oct_GetCtx();
    Job job;
    while (!SDL_GetAtomicInt(&ctx->quit)) {
        // Look for work
        if (ringBufferPop(&job)) {
            job.job(job.ptr);
            SDL_AddAtomicInt(&gThreadsWorking, -1);
        }

        // To not totally destroy the core in the case that the job queue is empty
        SDL_Delay(1);
    }
    return 0;
}

void _oct_JobsInit() {
    // Mutex
    gRingMutex = SDL_CreateMutex();
    if (!gRingMutex)
        oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to create job system rinbugger mutex, SDL error %s", SDL_GetError());

    // There are 4 threads in octarine: logic, render, audio, and clock. ideally we want 1 thread per core, so
    // if there are 16 cores we would have 12 job threads. If there are less cores than minimum threads + 4 we
    // will just use the minimum.
    int availableCoreCount = SDL_GetNumLogicalCPUCores() - 4;
    gJobThreadCount = 1;//availableCoreCount > OCT_MINIMUM_JOB_THREADS ? availableCoreCount : OCT_MINIMUM_JOB_THREADS;
    gJobThreads = mi_malloc(sizeof(SDL_Thread *) * gJobThreadCount);
    /*for (int i = 0; i < gJobThreadCount; i++) {
        gJobThreads[i] = SDL_CreateThread(jobThread, "Job thread", null);
        if (!gJobThreads[i])
            oct_Raise(OCT_STATUS_SDL_ERROR, true, "Failed to create job thread, SDL error %s", SDL_GetError());
    }*/
}

void _oct_JobsUpdate() {
    // TODO: This
}

void _oct_JobsEnd() {
    SDL_DestroyMutex(gRingMutex);
    for (int i = 0; i < gJobThreadCount; i++) {
        SDL_WaitThread(gJobThreads[i], NULL);
    }
    mi_free(gJobThreads);
}

////////////////////////////////// PUBLIC API //////////////////////////////////

OCTARINE_API void oct_QueueJob(Oct_JobFunction job, void *data) {
    // This will be decremented once this job is complete
    SDL_AddAtomicInt(&gThreadsWorking, 1);

    // If the job queue is full, the job will just be executed right away lmao
    Job jobStruct = {.job = job, .ptr = data};
    if (!ringBufferPush(&jobStruct))
        job(data);
}

OCTARINE_API Oct_Bool oct_JobsBusy() {
    return SDL_GetAtomicInt(&gThreadsWorking) > 0;
}

OCTARINE_API void oct_WaitJobs() {
    while (oct_JobsBusy());
}
