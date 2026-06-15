#ifndef THREAD_WRAP_H
#define THREAD_WRAP_H

#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /* ============================================================
       Common types
       ============================================================ */

#ifdef _WIN32

    typedef HANDLE thread_t;
    typedef CRITICAL_SECTION thread_mutex_t;
    typedef DWORD thread_ret_t;
    typedef LPVOID thread_arg_t;

#define THREAD_CALLCONV WINAPI
#define THREAD_FUNC(name) DWORD WINAPI name(LPVOID arg)
#define THREAD_RETURN(value) return (DWORD)(uintptr_t)(value)
#define THREAD_NULL_RETURN return 0

#else

    typedef pthread_t thread_t;
    typedef pthread_mutex_t thread_mutex_t;
    typedef void* thread_ret_t;
    typedef void* thread_arg_t;

#define THREAD_CALLCONV
#define THREAD_FUNC(name) void* name(void* arg)
#define THREAD_RETURN(value) return (void*)(uintptr_t)(value)
#define THREAD_NULL_RETURN return NULL

#endif

    typedef thread_ret_t(THREAD_CALLCONV* thread_func_t)(thread_arg_t);

    /* ============================================================
       Thread options
       ============================================================ */

    typedef struct thread_options_t {
        size_t stack_size;         /* 0 = default */
        uint64_t affinity_mask;    /* 0 = no affinity request */
    } thread_options_t;

    /* ============================================================
       API
       ============================================================ */

    static int thread_create(thread_t* thread, thread_func_t func, void* arg);
    static int thread_create_ex(thread_t* thread,
        thread_func_t func,
        void* arg,
        const thread_options_t* opts);
    static int thread_join(thread_t thread);
    static int thread_set_affinity(thread_t thread, uint64_t affinity_mask);
    static void thread_sleep_ms(unsigned int ms);

    static int thread_mutex_init(thread_mutex_t* mutex);
    static int thread_mutex_lock(thread_mutex_t* mutex);
    static int thread_mutex_unlock(thread_mutex_t* mutex);
    static int thread_mutex_destroy(thread_mutex_t* mutex);

    /* ============================================================
       Implementation
       ============================================================ */

    static int thread_create(thread_t* thread, thread_func_t func, void* arg)
    {
        return thread_create_ex(thread, func, arg, NULL);
    }

#ifdef _WIN32

    static int thread_set_affinity(thread_t thread, uint64_t affinity_mask)
    {
        if (affinity_mask == 0) {
            return 0;
        }

        DWORD_PTR result = SetThreadAffinityMask(thread, (DWORD_PTR)affinity_mask);
        return (result == 0) ? -1 : 0;
    }

    static int thread_create_ex(thread_t* thread,
        thread_func_t func,
        void* arg,
        const thread_options_t* opts)
    {
        SIZE_T stack_size = 0;
        if (opts && opts->stack_size > 0) {
            stack_size = (SIZE_T)opts->stack_size;
        }

        HANDLE h = CreateThread(
            NULL,
            stack_size,
            (LPTHREAD_START_ROUTINE)func,
            (LPVOID)arg,
            0,
            NULL
        );

        if (h == NULL) {
            return -1;
        }

        if (opts && opts->affinity_mask != 0) {
            if (thread_set_affinity(h, opts->affinity_mask) != 0) {
                CloseHandle(h);
                return -1;
            }
        }

        *thread = h;
        return 0;
    }

    static int thread_join(thread_t thread)
    {
        DWORD result = WaitForSingleObject(thread, INFINITE);
        if (result != WAIT_OBJECT_0) {
            return -1;
        }

        return CloseHandle(thread) ? 0 : -1;
    }

    static void thread_sleep_ms(unsigned int ms)
    {
        Sleep(ms);
    }

    static int thread_mutex_init(thread_mutex_t* mutex)
    {
        InitializeCriticalSection(mutex);
        return 0;
    }

    static int thread_mutex_lock(thread_mutex_t* mutex)
    {
        EnterCriticalSection(mutex);
        return 0;
    }

    static int thread_mutex_unlock(thread_mutex_t* mutex)
    {
        LeaveCriticalSection(mutex);
        return 0;
    }

    static int thread_mutex_destroy(thread_mutex_t* mutex)
    {
        DeleteCriticalSection(mutex);
        return 0;
    }

#else

    static int thread_set_affinity(thread_t thread, uint64_t affinity_mask)
    {
        if (affinity_mask == 0) {
            return 0;
        }

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        for (int i = 0; i < 64; ++i) {
            if ((affinity_mask >> i) & 1ULL) {
                CPU_SET(i, &cpuset);
            }
        }

        return pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
    }

    static int thread_create_ex(thread_t* thread,
        thread_func_t func,
        void* arg,
        const thread_options_t* opts)
    {
        pthread_attr_t attr;
        pthread_attr_t* attr_ptr = NULL;
        int rc;

        if (opts) {
            rc = pthread_attr_init(&attr);
            if (rc != 0) {
                return rc;
            }
            attr_ptr = &attr;

            if (opts->stack_size > 0) {
                rc = pthread_attr_setstacksize(&attr, opts->stack_size);
                if (rc != 0) {
                    pthread_attr_destroy(&attr);
                    return rc;
                }
            }
        }

        rc = pthread_create(thread, attr_ptr, func, arg);

        if (attr_ptr) {
            pthread_attr_destroy(&attr);
        }

        if (rc != 0) {
            return rc;
        }

        if (opts && opts->affinity_mask != 0) {
            rc = thread_set_affinity(*thread, opts->affinity_mask);
            if (rc != 0) {
                return rc;
            }
        }

        return 0;
    }

    static int thread_join(thread_t thread)
    {
        return pthread_join(thread, NULL);
    }

    static void thread_sleep_ms(unsigned int ms)
    {
        usleep(ms * 1000);
    }

    static int thread_mutex_init(thread_mutex_t* mutex)
    {
        return pthread_mutex_init(mutex, NULL);
    }

    static int thread_mutex_lock(thread_mutex_t* mutex)
    {
        return pthread_mutex_lock(mutex);
    }

    static int thread_mutex_unlock(thread_mutex_t* mutex)
    {
        return pthread_mutex_unlock(mutex);
    }

    static int thread_mutex_destroy(thread_mutex_t* mutex)
    {
        return pthread_mutex_destroy(mutex);
    }

#endif

#ifdef __cplusplus
}
#endif

#endif