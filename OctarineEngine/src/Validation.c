#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <VK2D/VK2D.h>
#include "oct/Validation.h"
#include "oct/Constants.h"

#define ANSI_COLOUR_RED     "\x1b[31m"
#define ANSI_COLOUR_GREEN   "\x1b[32m"
#define ANSI_COLOUR_YELLOW  "\x1b[33m"
#define ANSI_COLOUR_BLUE    "\x1b[34m"
#define ANSI_COLOUR_MAGENTA "\x1b[35m"
#define ANSI_COLOUR_CYAN    "\x1b[36m"
#define ANSI_COLOUR_RESET   "\x1b[0m"

// Constants
#define BUFFER_SIZE 1024
static char gErrorBuffer[BUFFER_SIZE]; // Buffer for error string
static Oct_Status gStatus;             // General status
static SDL_Mutex *gLogMutex;           // Mutex for logging
static SDL_Mutex *gStatusMutex;        // Mutex for status

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
        SDL_LockMutex(gLogMutex);
        printf("[" ANSI_COLOUR_RED "ERROR" ANSI_COLOUR_RESET "] " "%s\n", gErrorBuffer);
        fflush(stdout);
        SDL_UnlockMutex(gLogMutex);
        FILE *f = fopen("octarinedump.log", "a");
        fprintf(f, "\nCRASH REPORT\n===========\n%s\nError %" PRIu64 ": ", vk2dHostInformation(), gStatus);
        va_start(l, fmt);
        vfprintf(f, fmt, l);
        fprintf(f, "\n");
        va_end(l);
        fclose(f);
        abort();
    } else {
        SDL_LockMutex(gLogMutex);
        printf("[" ANSI_COLOUR_YELLOW "WARNING" ANSI_COLOUR_RESET "] " "%s\n", gErrorBuffer);
        fflush(stdout);
        SDL_UnlockMutex(gLogMutex);
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
    printf("[" ANSI_COLOUR_GREEN "LOG" ANSI_COLOUR_RESET "] ");
    va_start(l, fmt);
    vprintf(fmt, l);
    printf("\n");
    fflush(stdout);
    va_end(l);
    SDL_UnlockMutex(gLogMutex);
}
