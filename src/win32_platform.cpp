#ifndef WIN32_PLATFORM_CPP
#define WIN32_PLATFORM_CPP

#include "win32_platform.h"
#include "platform.h"

SYSTEM_INFO Win32SystemInfo;

FUNCTION void
win32_platform_init()
{
    Platform.AllocateMemory = &win32_allocate_virtual_memory;
    Platform.FreeMemory = &win32_free_virtual_memory;
    Platform.AddWorkToQueue = &win32_add_work_to_queue;
    Platform.CompleteAllWork = &win32_complete_all_work;
    Platform.OpenFile = &win32_open_file;
    Platform.CreateNewFile = &win32_create_new_file;
    Platform.ReadFromFile = &win32_read_from_file;
    Platform.CloseFile = &win32_close_file;
    Platform.FileSize = &win32_file_size;
    
    GetSystemInfo(&Win32SystemInfo);
    win32_platform_set_main_thread();
    assert(platform_main_thread_id() == GetCurrentThreadId());
}

//
// Win32 Memory
//

FUNCTION PLATFORM_ALLOCATE_MEMORY(win32_allocate_virtual_memory)
{
    const u64 pageSize = Win32SystemInfo.dwPageSize;
    const u64 sizeofAllocationHeader = align_64(sizeof(MemoryAllocation));
    size = align_n(size, pageSize);
    
    // VirtualProtect affects all pages of a memory range, 
    // so the protection size needs to be a full page
    u64 underflowGuard = 0;
    u64 overflowGuard = 0;
    if (ALLOCATION_OVERFLOW_GUARD & flags)
        overflowGuard = pageSize;
    
    u64 adjustedSize = size + underflowGuard + overflowGuard;
    byte* data = (byte*)VirtualAlloc(0, 
                                     adjustedSize, 
                                     MEM_COMMIT | MEM_RESERVE,
                                     PAGE_READWRITE);
    assert(data);
    
    MemoryAllocation* mem = (MemoryAllocation*)data;
    mem->size = size;
    mem->ptr = data + sizeofAllocationHeader;
    mem->flags = flags;
    mem->userData = null;
    
    if (ALLOCATION_OVERFLOW_GUARD & flags)
    {
        byte* guardBegin = data + adjustedSize - pageSize;
        memset(guardBegin, 0xA, pageSize); // 1010 repeating
        DWORD oldProtection; // PAGE_GUARD ????
        BOOL successfullyGuarded = VirtualProtect(guardBegin, 
                                                  1,
                                                  PAGE_NOACCESS,
                                                  &oldProtection);
        assert(successfullyGuarded);
    }    
    // on windows, VirtualAlloc guarantees that the memory is already zeroed
#if 0
    if (ALLOCATION_ZERO_INIT & flags) memzero(mem->ptr, mem->size);
#endif    
    
    return mem;
}

FUNCTION PLATFORM_FREE_MEMORY(win32_free_virtual_memory)
{
    if (ALLOCATION_OVERFLOW_GUARD & allocation->flags)
    {
        DWORD oldProtection;
        VirtualProtect(allocation->ptr + allocation->size, 1, PAGE_READWRITE, &oldProtection);
    }
    VirtualFree(allocation, 0, MEM_RELEASE | MEM_DECOMMIT);
}

#if 0
//
// @TODO using the heap is bad, lol
//       this is a tiny baby program, I'll start refactoring this out to 
//       real memory allocation when queen starts requesting more features
//
GLOBAL HANDLE ProcessHeap = null;

FUNCTION void* 
win32_heap_alloc(u64 numbytes) 
{
    if (!ProcessHeap) ProcessHeap = GetProcessHeap();
    numbytes = align_8(numbytes);
    return HeapAlloc(ProcessHeap, HEAP_ZERO_MEMORY, numbytes);
}

FUNCTION void* 
win32_heap_realloc(void* data, u64 numbytes) 
{
    numbytes = align_8(numbytes);
    
    void* firstTryRealloc =
        HeapReAlloc(ProcessHeap, 
                    HEAP_ZERO_MEMORY | HEAP_REALLOC_IN_PLACE_ONLY, 
                    data, 
                    numbytes);
    
    if (firstTryRealloc) return firstTryRealloc;
    
    return HeapReAlloc(ProcessHeap, 
                       HEAP_ZERO_MEMORY, 
                       data, 
                       numbytes);
}

FUNCTION void 
win32_heap_free(void* data)
{
    HeapFree(ProcessHeap, 0, data);
}
#endif


FUNCTION void 
win32_print(const char* buf)
{
    LOCAL_STATIC HANDLE StdOut = null;
    if (!StdOut) 
    {
        StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    WriteFile(StdOut, 
              buf, (DWORD)strlen(buf),
              null, null);
}


//
// Win32 Multithreading
//

FUNCTION bool32
win32_find_and_do_queue_work(PlatformWorkQueue* queue)
{
    bool32 foundWork = BOOL_FALSE;
    if (queue->nextReadIndex == queue->nextWriteIndex)
    {
        foundWork = BOOL_FALSE;
        return foundWork;
    }
    
    foundWork = BOOL_TRUE;
    u32 originalReadIndex = queue->nextReadIndex;
    u32 newReadIndex = (queue->nextReadIndex + 1) % PLATFORM_WORK_QUEUE_LIMIT;
    
    // InterlockedCompareExchange is an x64 instruction that atomically compares
    // the value at the address in arg1 against arg3, and if they are equal,
    // set the value at arg1 to arg2
    // Then returns the original value at the address
    //docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-interlockedcompareexchange
    u32 nextReadIndex = InterlockedCompareExchange((LONG volatile*)&queue->nextReadIndex,
                                                   newReadIndex,
                                                   originalReadIndex);
    if (nextReadIndex == originalReadIndex)
    {
        PlatformWork workOnDeck = queue->work[nextReadIndex];
        workOnDeck.callback(queue, workOnDeck.userData);
        InterlockedIncrement((LONG volatile*)&queue->completionCount);
    }
    return foundWork;
}

DWORD WINAPI
win32_thread_proc(LPVOID lpParameter)
{
    Win32ThreadData* threadData = (Win32ThreadData*)lpParameter;
    PlatformWorkQueue* queue = threadData->queue;
    
    for(;;)
    {
        if (!win32_find_and_do_queue_work(queue))
        {
            // grabs and increments semaphore, block while sem == max
            WaitForSingleObject(queue->threadsLookForWorkSemaphore, INFINITE);
        }
    }
    
    //return 0;
}

FUNCTION void
win32_make_work_queue(PlatformWorkQueue* queue, 
                      u32 threadCount,
                      Win32ThreadData* threadData)
{
    queue->completionGoal = 0;
    queue->completionCount = 0;
    queue->nextWriteIndex = 0;
    queue->nextReadIndex = 0;
    queue->threadsLookForWorkSemaphore = CreateSemaphoreExA(null, // security attr
                                                            0, // initial count
                                                            threadCount, // max count
                                                            null, // name
                                                            0, // flags; must be 0
                                                            SEMAPHORE_ALL_ACCESS);
    for(u32 i = 0; i < threadCount; ++i)
    {
        threadData[i].queue = queue;
        
        DWORD threadId;
        HANDLE threadHandle = CreateThread(null, // security attributes
                                           megabytes(1), // stack size
                                           &win32_thread_proc, &threadData[i], 
                                           0, // create flags
                                           &threadId);
        // closing the handle doesn't kill the thread
        // it just lets windows know it can get rid of the memory allocated to that 
        // thread handle
        CloseHandle(threadHandle);
    }
}

FUNCTION PLATFORM_ADD_WORK_TO_QUEUE(win32_add_work_to_queue)
{
    assert_msg(platform_main_thread_id() == platform_thread_id(),
               "Currently we only support adding work from the main thread!");
    
    u32 newWriteIndex;
    {
        // @UPDATE use InterlockCompareExchange for if we want any thread to 
        //       add work to the queue
        newWriteIndex = (queue->nextWriteIndex + 1) % PLATFORM_WORK_QUEUE_LIMIT;
        // @TODO should this be an assert, or a call to complete all work???
        assert_msg(newWriteIndex != queue->nextReadIndex,
                   "Wrote too many things to the work queue!");
        // @NOTE completionGoal has the possibility to wrap around past INT_MAX
        //     if we never call win32_complete_all_work.
        //     However, this doesn't matter because we never compare with 
        //     less/greater than only equality, and completionCount will wrap with it
        queue->work[queue->nextWriteIndex] = *work;
        queue->completionGoal += 1;
    } 
    
    _WriteBarrier();
    queue->nextWriteIndex = newWriteIndex;
    ReleaseSemaphore(queue->threadsLookForWorkSemaphore, 1, 0);
}

FUNCTION PLATFORM_COMPLETE_ALL_WORK(win32_complete_all_work)
{
    // using != compare accounts for potential (lol) wrapping past INT_MAX
    while(queue->completionGoal != queue->completionCount)
    {
        win32_find_and_do_queue_work(queue);
    }
    queue->completionGoal = 0;
    queue->completionCount = 0;
}


//
// Win32 file system
//

GLOBAL Win32FileManager FileManager;

FUNCTION void
win32_make_file_manager(s32 maxOpenFiles)
{
    NOT_IMPLEMENTED;
    
    FileManager = {};
    FileManager.maxOpenFiles = maxOpenFiles;
    
    u64 allocationSize = maxOpenFiles*sizeof(Win32File) + maxOpenFiles*sizeof(s32);
    
}

FUNCTION PLATFORM_OPEN_FILE(win32_open_file)
{
    NOT_IMPLEMENTED;
    return {};
}

FUNCTION PLATFORM_CREATE_NEW_FILE(win32_create_new_file)
{
    NOT_IMPLEMENTED;
    return {};
}

FUNCTION PLATFORM_READ_FROM_FILE(win32_read_from_file)
{
    NOT_IMPLEMENTED;
    return {};
}

FUNCTION PLATFORM_CLOSE_FILE(win32_close_file)
{
    NOT_IMPLEMENTED;
}

FUNCTION PLATFORM_FILE_SIZE(win32_file_size)
{
    NOT_IMPLEMENTED;
    return {};
}

#endif /* WIN32_PLATFORM_CPP */
