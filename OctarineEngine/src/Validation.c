#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <VK2D/VK2D.h>
#include "oct/Validation.h"
#include "oct/Constants.h"

// Constants
#define BUFFER_SIZE 1024
static char gErrorBuffer[BUFFER_SIZE]; // Buffer for error string
static Oct_Status gStatus;             // General status
static SDL_mutex *gLogMutex;           // Mutex for logging
static SDL_mutex *gStatusMutex;        // Mutex for status

void _oct_ValidationInit() {
    gLogMutex = SDL_CreateMutex();
    gStatusMutex = SDL_CreateMutex();
}

void _oct_ValidationEnd() {
    SDL_DestroyMutex(gLogMutex);
    SDL_DestroyMutex(gStatusMutex);
}

OCTARINE_API void oct_Raise(Oct_Status status, Oct_Bool fatal, const char *fmt, ...) {
    SDL_LockMutex(gStatusMutex);
    va_list l;
    va_start(l, fmt);
    vsnprintf(gErrorBuffer, BUFFER_SIZE - 1, fmt, l);
    va_end(l);
    if (fatal) {
        FILE *f = fopen("octarinedump.log", "a");
        fprintf(f, "\nCRASH REPORT\n===========\n%s", vk2dHostInformation());
        va_start(l, fmt);
        vfprintf(f, fmt, l);
        fprintf(f, "\n");
        va_end(l);
        fclose(f);
        abort();
    }
    gStatus |= status;
    SDL_UnlockMutex(gStatusMutex);
}

OCTARINE_API Oct_Status oct_GetStatus() {
    Oct_Status s;
    SDL_LockMutex(gStatusMutex);
    s = gStatus;
    SDL_UnlockMutex(gStatusMutex);
    return s;
}

OCTARINE_API const char *oct_GetError() {
    return gErrorBuffer; // todo - this is unsafe lmao
}

OCTARINE_API void oct_Log(const char *fmt, ...) {
    SDL_LockMutex(gLogMutex);
    va_list l;
    va_start(l, fmt);
    vprintf(fmt, l);
    fflush(stdout);
    va_end(l);
    SDL_UnlockMutex(gLogMutex);
}
