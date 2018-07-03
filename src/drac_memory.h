#ifndef DRAC_MEMORY_H
#define DRAC_MEMORY_H

#include "platform.h"
#include "drac_string.h"
#include "drac_array.h"

struct MemoryArena;
struct TemporaryStack;

struct MemoryArena
{
    MemoryAllocation* allocation; // dynamic
    MemoryAllocation* extras;
    TemporaryStack* crntStack;
    byte* cursor;
    u64 minAllocationSize;
};

struct TemporaryStack
{
    MemoryArena* arena;
    MemoryAllocation* allocation; 
    TemporaryStack* prev; 
    byte* cursor;
};

#if DEBUG
#define ARENA_ALLOCATION_FLAGS (ALLOCATION_OVERFLOW_GUARD)
#else
#define ARENA_ALLOCATION_FLAGS (0)
#endif

FUNCTION inline MemoryArena* 
allocate_memory_arena(u64 minSize = 0)
{
    if (!minSize) minSize = megabytes(1);
    u64 bootstrapHeaderSize = align_64(sizeof(MemoryArena));
    
    MemoryAllocation* allocation = Platform.AllocateMemory(minSize + bootstrapHeaderSize, 
                                                           ARENA_ALLOCATION_FLAGS);
    
    if (minSize < (allocation->size - bootstrapHeaderSize))
    {
        minSize = allocation->size - bootstrapHeaderSize;
    }
    
    MemoryArena* arena = (MemoryArena*)allocation->ptr;
    arena->allocation = allocation;
    arena->extras = null;
    arena->crntStack = null;
    arena->cursor = allocation->ptr + bootstrapHeaderSize;
    arena->minAllocationSize = minSize;
    return arena;
}

FUNCTION void
free_memory_arena(MemoryArena* arena)
{
    assert(arena && arena->allocation);
    MemoryAllocation* lastAllocation = arena->allocation;
    do
    {
        MemoryAllocation* previous = (MemoryAllocation*)lastAllocation->userData;
        Platform.FreeMemory(lastAllocation);
        lastAllocation = previous;
    } while(lastAllocation);
    
    MemoryAllocation* extraAllocation = arena->extras;
    while(extraAllocation)
    {
        MemoryAllocation* previous = (MemoryAllocation*)extraAllocation->userData;
        Platform.FreeMemory(extraAllocation);
        extraAllocation = previous;
    }
    *arena = {};
}

FUNCTION inline void
free_unused_memory(MemoryArena* arena)
{
    assert(arena);
    MemoryAllocation* extraAllocation = arena->extras;
    while(extraAllocation);
    {
        MemoryAllocation* previous = (MemoryAllocation*)extraAllocation->userData;
        Platform.FreeMemory(extraAllocation);
        extraAllocation = previous;
    }
    arena->extras = null;
}

FUNCTION inline u64
adjust_to_arena_alignment(u64 alignment)
{
    assert_power_of_2(alignment);
    alignment = max(8, alignment);
    return alignment;
}

FUNCTION inline byte*
base_ptr(MemoryArena* arena)
{
    assert(arena && arena->allocation);
    return arena->allocation->ptr;
}

FUNCTION inline byte*
next_ptr(MemoryArena* arena, u64 alignment = 8)
{
    assert(arena && arena->allocation);
    alignment = adjust_to_arena_alignment(alignment);
    uptr alignedCursor = reinterpret_cast<uptr>(arena->cursor);
    alignedCursor = align_n(alignedCursor, alignment);
    return reinterpret_cast<byte*>(alignedCursor);
}

FUNCTION inline bool32
check_arena(MemoryArena* arena, u64 numBytes, u64 alignment = 8)
{
    alignment = adjust_to_arena_alignment(alignment);
    numBytes = align_n(numBytes, alignment);
    byte* nextPtr = next_ptr(arena, alignment);
    byte* endPtr = arena->allocation->ptr + arena->allocation->size;
    u64 remainingSize = endPtr - nextPtr;
    return (remainingSize > 0) && (numBytes <= remainingSize);
}

FUNCTION byte* 
push_size(MemoryArena* arena, u64 numBytes, u64 alignment = 8)
{
    alignment = adjust_to_arena_alignment(alignment);
    numBytes = align_n(numBytes, alignment);
    
    if (!check_arena(arena, numBytes, alignment))
    {
        u64 newAllocationSize = max(numBytes + alignment, arena->minAllocationSize);
        MemoryAllocation* next = arena->extras;
        MemoryAllocation* thingBeforeNext = null;
        while(next && next->size < newAllocationSize)
        {
            thingBeforeNext = next;
            next = (MemoryAllocation*)next->userData;
        }
        
        MemoryAllocation* newAllocation = null;
        if (next)
        {
            if (thingBeforeNext)
            {
                thingBeforeNext->userData = next->userData;
            }
            newAllocation = next;
        }
        else
        {
            newAllocation = Platform.AllocateMemory(newAllocationSize,
                                                    ARENA_ALLOCATION_FLAGS);
        }
        newAllocation->userData = arena->allocation;
        arena->allocation = newAllocation;
        arena->cursor = arena->allocation->ptr;
    }
    
    byte* alignedMem = next_ptr(arena, alignment);
    arena->cursor = alignedMem + numBytes;
    return alignedMem;
}

template<typename T> FUNCTION inline
T* push(MemoryArena* arena)
{
    T* s = reinterpret_cast<T*>(push_size(arena, sizeof(T), alignof(T)));
    *s = {};
    return s;
}

template<typename T> FUNCTION inline
Array<T> push_array(MemoryArena* arena, u64 count)
{
    T* mem = reinterpret_cast<T*>(push_size(arena, sizeof(T) * count, alignof(T)));
    Array<T> a = array_from_buffer<T>(mem, sizeof(T) * count);
    assert(a.capacity == count);
    return a;
}

FUNCTION inline String
push_string(MemoryArena* arena, u64 capacity)
{
    void* buf = push_size(arena, capacity);
    String str = string_from_buffer(buf, capacity);
    return str;
}

FUNCTION inline void
begin_temp_stack(MemoryArena* arena)
{
    TemporaryStack dataToReload;
    dataToReload.arena = arena;
    dataToReload.allocation = arena->allocation;
    dataToReload.prev = arena->crntStack;
    dataToReload.cursor = arena->cursor;
    
    // need to store the current data before pushing the new stack
    // because the push call will move the cursor & potentially allocate mem
    TemporaryStack* stack = push<TemporaryStack>(arena);
    assert(stack);
    *stack = dataToReload;
    arena->crntStack = stack;
}

FUNCTION void
end_temp_stack(MemoryArena* arena)
{
    TemporaryStack* dataToReload = arena->crntStack;
    assert(dataToReload && dataToReload->arena == arena);
    while(dataToReload->allocation != arena->allocation)
    {
        auto crntAllocation = arena->allocation;
        auto prevAllocation = (MemoryAllocation*)crntAllocation->userData;
        assert(prevAllocation);
        arena->allocation = prevAllocation;
        crntAllocation->userData = arena->extras;
        arena->extras = crntAllocation;
    }
    arena->cursor = dataToReload->cursor;
    arena->crntStack = dataToReload->prev;
}

#endif /* DRAC_MEMORY_H */