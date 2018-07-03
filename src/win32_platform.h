#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define FORCE_INLINE __forceinline
#define align_struct(_byte) __declspec(align(_byte))
#define PLATFORM_SLEEP(ms) Sleep(ms)
#define CRASH __debugbreak()

//
// SIMD archtecture stuff
//
#ifdef _MSC_VER

//
// @TODO find a better way of detecting SSE3, SSE4.1 etc
//
#include <intrin.h>

// archtecture AMD64 (x64) supports SSE
#if  defined(_M_AMD64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1) || \
defined(__SSE__) || defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__)
#   include <mmintrin.h>
#   include <xmmintrin.h>
#   define DRAC_MMX 1
#   define DRAC_SSE 1
#endif /* AMD64 or x86 */

#if  defined(_M_AMD64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || \
defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__)
#   include <emmintrin.h>
#   define DRAC_SSE2 1
#endif /* AMD64 or x64 */

#else /* if we're using a compiler that doesn't do the weird MSVC macros for SSE */

#if defined(__SSE__) || defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__)
#   include <mmintrin.h>
#   include <xmmintrin.h>
#   define DRAC_MMX 1
#   define DRAC_SSE 1
#endif 

#if defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__)
#   include <emmintrin.h>
#   define DRAC_SSE2 1
#endif 

#endif /* _MSC_VER */

#if defined(__AVX__) || defined(__AVX2__)
#   include <immintrin.h>
#   define DRAC_AVX 1
#endif /* AVX or AVX2 */

#if defined(__AVX2__)
#   define DRAC_AVX2 1
#endif /* AVX2 */

#if defined(__SSE3__)
#   include <pmmintrin.h>
#   define DRAC_SSE3 1
#endif

#if defined(__SSSE3__)
#   include <tmmintrin.h>
#   define DRAC_SSSE3 1
#endif

#if defined(__SSE4_1__)
#   include <smmintrin.h>
#   define DRAC_SSE4_1 1
#endif

#if defined(__SSE4_2__)
#   include <nmmintrin.h>
#   define DRAC_SSE4_2 1
#endif


#define PLATFORM_LOG win32_print
#define PLATFORM_LOG_ERROR win32_print

#include "platform.h"
#include "drac_array.h"

extern SYSTEM_INFO Win32SystemInfo;
FUNCTION void win32_platform_init();

//
// Win32 Memory
//

FUNCTION PLATFORM_ALLOCATE_MEMORY(win32_allocate_virtual_memory);
FUNCTION PLATFORM_FREE_MEMORY(win32_free_virtual_memory);

//
// Win32 Multithreading
//

#define PLATFORM_WORK_QUEUE_LIMIT 256

struct PlatformWorkQueue
{
    HANDLE threadsLookForWorkSemaphore;
    PlatformWork work[PLATFORM_WORK_QUEUE_LIMIT];
    LONG completionGoal;
    LONG completionCount;
    LONG nextWriteIndex;
    LONG nextReadIndex;
};

struct Win32ThreadData
{
    PlatformWorkQueue* queue;
};

FUNCTION void win32_make_work_queue(PlatformWorkQueue* queue, 
                                    u32 threadCount,
                                    Win32ThreadData* threadData);
FUNCTION PLATFORM_ADD_WORK_TO_QUEUE(win32_add_work_to_queue);
FUNCTION PLATFORM_COMPLETE_ALL_WORK(win32_complete_all_work);
DWORD WINAPI win32_thread_proc(LPVOID lpParameter);


//
// Win32 file/console I/O
//

FUNCTION void win32_print(const char* buf);

struct Win32File
{
    HANDLE handle;
    WIN32_FIND_DATAA fileData;
};

struct Win32FileManager
{
    Array<Win32File> openFiles;
    Array<s32> freeFileIndices;
    s32 maxOpenFiles;
    
};

FUNCTION PLATFORM_OPEN_FILE(win32_open_file);
FUNCTION PLATFORM_CREATE_NEW_FILE(win32_create_new_file);
FUNCTION PLATFORM_READ_FROM_FILE(win32_read_from_file);
FUNCTION PLATFORM_CLOSE_FILE(win32_close_file);
FUNCTION PLATFORM_FILE_SIZE(win32_file_size);


inline u32
platform_thread_id() // handmade hero ep.182
{
    u8* tls = (u8*)__readgsqword(0x30);
    u32 threadId = *(u32*)(tls + 0x48);
    return threadId;
}

GLOBAL u32 MainThreadId;
inline u32 
platform_main_thread_id()
{
    return MainThreadId;
}

inline void
win32_platform_set_main_thread()
{
    LOCAL_STATIC bool32 setupOnce = BOOL_FALSE;
    assert(!setupOnce);
    MainThreadId = platform_thread_id();
    setupOnce = BOOL_TRUE;
}


#endif /* WIN32_PLATFORM_H */