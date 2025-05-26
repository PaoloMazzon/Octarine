/// \brief Multi-threaded job system
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Queues a job to be completed
///
/// Jobs are functions that will be completed (usually) on a job thread. There is no guarantee when the queued job
/// will be completed/started or on what thread, as they go into a job queue (mostly, if the job queue is full, this
/// function will just execute the job right away).
OCTARINE_API void oct_QueueJob(Oct_JobFunction job, void *data);

/// \brief Returns true if there are any jobs currently executing/in queue
OCTARINE_API Oct_Bool oct_JobsBusy();

/// \brief Waits until all active jobs are complete
OCTARINE_API void oct_WaitJobs();

#ifdef __cplusplus
};
#endif