/// \brief Multi-threaded job system
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Queues a job to be completed at some point somewhere
///
/// This queues a job to be done by a worker thread. There are no guarantees when or on what thread the job
/// will be done on, it may even execute the job in this function depending on whats going on. In general,
/// the job system will do its best to keep work distributed across CPU cores.
OCTARINE_API void oct_QueueJob(const Oct_JobFunction job);

/// \brief Returns true if there are any jobs currently executing/in queue
OCTARINE_API Oct_Bool oct_JobsBusy();

/// \brief Waits until all active jobs are complete
OCTARINE_API void oct_WaitJobs();

#ifdef __cplusplus
};
#endif