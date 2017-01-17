
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) NGINX, Inc.
 */

#include <nxt_main.h>


#if (NXT_THREADS)
static void nxt_job_thread_trampoline(nxt_thread_t *thr, void *obj, void *data);
static void nxt_job_thread_return_handler(nxt_thread_t *thr, void *obj,
    void *data);
#endif


void *
nxt_job_create(nxt_mem_pool_t *mp, size_t size)
{
    size_t     cache_size;
    nxt_job_t  *job;

    if (mp == NULL) {
        mp = nxt_mem_pool_create(256);

        if (nxt_slow_path(mp == NULL)) {
            return NULL;
        }

        job = nxt_mem_zalloc(mp, size);
        cache_size = 0;

    } else {
        job = nxt_mem_cache_zalloc0(mp, size);
        cache_size = size;
    }

    if (nxt_fast_path(job != NULL)) {
        job->cache_size = (uint16_t) cache_size;
        job->mem_pool = mp;
        nxt_job_set_name(job, "job");
    }

    /* Allow safe nxt_queue_remove() in nxt_job_destroy(). */
    nxt_queue_self(&job->link);

    return job;
}


void
nxt_job_init(nxt_job_t *job, size_t size)
{
    nxt_memzero(job, size);

    nxt_job_set_name(job, "job");

    nxt_queue_self(&job->link);
}


void
nxt_job_destroy(void *data)
{
    nxt_job_t  *job;

    job = data;

    nxt_queue_remove(&job->link);

    if (job->cache_size == 0) {

        if (job->mem_pool != NULL) {
            nxt_mem_pool_destroy(job->mem_pool);
        }

    } else {
        nxt_mem_cache_free0(job->mem_pool, job, job->cache_size);
    }
}


nxt_int_t
nxt_job_cleanup_add(nxt_mem_pool_t *mp, nxt_job_t *job)
{
    nxt_mem_pool_cleanup_t  *mpcl;

    mpcl = nxt_mem_pool_cleanup(mp, 0);

    if (nxt_fast_path(mpcl != NULL)) {
        mpcl->handler = nxt_job_destroy;
        mpcl->data = job;
        return NXT_OK;
    }

    return NXT_ERROR;
}


/*
 * The (void *) casts in nxt_thread_pool_post() and nxt_event_engine_post()
 * calls and to the "nxt_work_handler_t" are required by Sun C.
 */

void
nxt_job_start(nxt_thread_t *thr, nxt_job_t *job, nxt_work_handler_t handler)
{
    nxt_log_debug(thr->log, "%s start", job->name);

#if (NXT_THREADS)

    if (job->thread_pool != NULL) {
        nxt_int_t  ret;

        job->engine = thr->engine;
        ret = nxt_thread_pool_post(job->thread_pool, nxt_job_thread_trampoline,
                                   job, (void *) handler, job->log);
        if (ret == NXT_OK) {
            return;
        }

        handler = job->abort_handler;
    }

#endif

    handler(thr, job, job->data);
}


#if (NXT_THREADS)

/* A trampoline function is called by a thread pool thread. */

static void
nxt_job_thread_trampoline(nxt_thread_t *thr, void *obj, void *data)
{
    nxt_job_t           *job;
    nxt_work_handler_t  handler;

    job = obj;
    handler = (nxt_work_handler_t) data;

    nxt_log_debug(thr->log, "%s thread", job->name);

    if (nxt_slow_path(job->cancel)) {
        nxt_job_return(thr, job, job->abort_handler);

    } else {
        handler(thr, job, job->data);
    }
}

#endif


void
nxt_job_return(nxt_thread_t *thr, nxt_job_t *job, nxt_work_handler_t handler)
{
    nxt_log_debug(thr->log, "%s return", job->name);

#if (NXT_THREADS)

    if (job->engine != NULL) {
        /* A return function is called in thread pool thread context. */
        nxt_event_engine_post(job->engine, nxt_job_thread_return_handler,
                              job, (void *) handler, job->log);
        return;
    }

#endif

    if (nxt_slow_path(job->cancel)) {
        nxt_log_debug(thr->log, "%s cancellation", job->name);
        handler = job->abort_handler;
    }

    nxt_thread_work_queue_push(thr, &thr->work_queue.main,
                               handler, job, job->data, thr->log);
}


#if (NXT_THREADS)

static void
nxt_job_thread_return_handler(nxt_thread_t *thr, void *obj, void *data)
{
    nxt_job_t           *job;
    nxt_work_handler_t  handler;

    job = obj;
    handler = (nxt_work_handler_t) data;

    if (nxt_slow_path(job->cancel)) {
        nxt_log_debug(thr->log, "%s cancellation", job->name);
        handler = job->abort_handler;
    }

    handler(thr, job, job->data);
}

#endif
