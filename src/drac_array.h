#ifndef DRAC_ARRAY_H
#define DRAC_ARRAY_H

#include "platform.h"

#define ARRAY_FUNCTION template<typename T> FUNCTION
#define ARRAY_MIN_ALLOCATED 8

#define STACK_ARRAY_LENGTH CONCAT(_temparrlen_,__LINE__)
#define STACK_ARRAY_BUFFER CONCAT(_temparr_,__LINE__)

#define STACK_ARRAY_COUNT_ZERO(arrayName, T, count)\
T STACK_ARRAY_BUFFER [count]; \
Array<T> arrayName = array_from_buffer<T>(sizeof(STACK_ARRAY_BUFFER), STACK_ARRAY_BUFFER)

#define STACK_ARRAY_COUNT_N(arrayName, T, count_) \
STACK_ARRAY_COUNT_ZERO(arrayName, T, count_); arrayName.count = count_

#define for_array(array_) for_each(it, array_)

#define for_each(it_name, array_) \
for(auto it_name = make_iterator(&array_); \
!it_name.is_end(); \
++it_name) 


template<typename T>
struct Array
{
    T* items;
    s32 capacity;
    s32 count;
    
    inline T& operator[] (s32 index) const
    {
        assert(index < count && index > -1);
        return items[index];
    }
};


template<typename T>
struct ArrayIterator
{
    Array<T>* array;
    s32 index;
    
    inline T& operator*() const
    {
        assert(index >= 0 && (!array->count || index < array->count));
        return array->items[index];
    }
    inline T* operator->() const
    {
        assert(index >= 0 && (!array->count || index < array->count));
        return &array->items[index];
    }
    inline ArrayIterator<T>& operator++()
    {
        ++index;
        return *this;
    }
    inline ArrayIterator<T>& operator--()
    {
        --index;
        return *this;
    }
    inline bool32 is_end()
    {
        return index >= array->count;
    }
    inline bool32 is_beginning()
    {
        return index <= 0;
    }
};

ARRAY_FUNCTION inline ArrayIterator<T>
make_iterator(Array<T>* array)
{
    ArrayIterator<T> it;
    it.array = array;
    it.index = 0;
    return it;
}

ARRAY_FUNCTION  Array<T>
array_from_buffer(void* buffer, u64 bufferNumBytes)
{
    u64 alignOfT = alignof(T);
    byte* alignedBuffer = (byte*)align_n((u64)buffer, alignOfT);
    u64 trashedBytes = alignedBuffer - (byte*)buffer;
    bufferNumBytes -= trashedBytes;
    
    u32 buffersArrayItemCount = (u32)(bufferNumBytes) / sizeof(T);
    assert_msg(buffersArrayItemCount > 0,
               "Not enough room in the Array buffer for even a single item!");
    
    Array<T> array;
    array.capacity = buffersArrayItemCount;
    array.count = 0;
    array.items = (T*)alignedBuffer;
    return array;
}

ARRAY_FUNCTION FORCE_INLINE bool32
array_check_n_more_items(Array<T>* array, s32 countItems)
{
    return array->capacity >= (array->count + countItems);
}

ARRAY_FUNCTION s32 
array_add(Array<T>* array, const T& item)
{
    if (!array_check_n_more_items(array, 1)) return -1;
    
    array->items[array->count] = item;
    array->count += 1;
    return array->count - 1;
}

ARRAY_FUNCTION s32
array_add_range(Array<T>* array, const T* items, u32 countItems)
{
    if (!array_check_n_more_items(array, countItems)) return -1;
    
    memcpy(array->items + array->count, items, countItems * sizeof(T));
    array->count += countItems;
    return array->count - countItems;
}

ARRAY_FUNCTION inline s32
array_add_array(Array<T>* dst, const Array<T>* src)
{
    return array_add_range(dst, src->count, src->items);
}

ARRAY_FUNCTION s32
array_insert_unordered(Array<T>* array, T& item, s32 index)
{
    if (!array_check_n_more_items(array, 1)) return -1;
    
    array->items[array->count] = array->items[index];
    array->items[index] = item;
    array->count += 1;
    return index;
}

ARRAY_FUNCTION s32
array_insert_ordered(Array<T>* array, T& item, s32 index)
{
    if (!array_check_n_more_items(array, 1)) return -1;
    
    memmove(&array->items[index + 1], 
            &array->items[index],
            (array->count - index) * sizeof(T));
    array->items[index] = item;
    array->count += 1;
    return index;
}

ARRAY_FUNCTION void
array_remove_index_ordered(Array<T>* array, s32 index)
{
    assert(index >= 0 && index < array->count);
    
    if ((index + 1) < array->count) 
    {
        memmove(&array->items[index],
                &array->items[index + 1],
                (array->count - (index + 1)) * sizeof(T));
    }
    
    array->count -= 1;
#if DEBUG
    array->items[array->count] = {};
#endif
}

ARRAY_FUNCTION void
array_remove_index_unordered(Array<T>* array, s32 index)
{
    assert(index >= 0 && index < array->count);
    
    s32 lastIndex = array->count - 1;
    array->items[index] = array->items[lastIndex];
    array->count = lastIndex;
#if DEBUG
    array->items[lastIndex] = {};
#endif
}

ARRAY_FUNCTION void
array_remove_range_ordered(Array<T>* array, s32 indexFirst, s32 removeCount)
{
    assert(indexFirst >= 0 && (indexFirst + removeCount) <= array->count);
    
    s32 moveCount = array->count - (indexFirst + removeCount);
    assert(moveCount >= 0);
    if (moveCount > 0)
    {
        memmove(&array->items[indexFirst],
                &array->items[indexFirst + removeCount],
                moveCount * sizeof(T));
    }
    
#if DEBUG
    memzero(&array->items[array->count - removeCount], removeCount * sizeof(T));
#endif
    array->count -= removeCount;
}

ARRAY_FUNCTION void
array_remove_range_unordered(Array<T>* array, s32 indexFirst, s32 removeCount)
{
    assert(indexFirst >= 0 && (indexFirst + removeCount) <= array->count);
    
    s32 moveCount = min(array->count - (indexFirst + removeCount), removeCount);
    assert(moveCount >= 0);
    if (moveCount > 0)
    {
        memmove(&array->items[indexFirst],
                &array->items[array->count - moveCount],
                moveCount * sizeof(T));
    }
    
#if DEBUG
    memzero(&array->items[array->count - removeCount], removeCount * sizeof(T));
#endif
    array->count -= removeCount;
}

ARRAY_FUNCTION bool32
array_remove_item_ordered(Array<T>* array, T& item)
{
    s32 index = array_find(array, item);
    if (-1 == index) return BOOL_FALSE;
    return array_remove_index_ordered(a, index);
}

ARRAY_FUNCTION bool32
remove_item_unordered(Array<T>* array, T& item)
{
    s32 index = array_find(array, item);
    if (-1 == index) return BOOL_FALSE;
    return remove_index_unordered(a, index);
}

ARRAY_FUNCTION void
array_reset(Array<T>* array)
{
    array->count = 0;
#if DEBUG
    memzero(array->items, sizeof(T) * array->count);
#endif
}

ARRAY_FUNCTION s32
array_find(Array<T>* array, T& item)
{
    for_array(array)
    {
        if (*it == item) return it.index;
    }
    return -1;
}

ARRAY_FUNCTION s32
find_in_buffer(T* buffer, u32 count, T& item)
{
    for(u32 i = 0; i < count; ++i)
    {
        if (buffer[i] == item) return i;
    }
    return -1;
}

template<typename ArrayType, typename MatchFunctionType>
FUNCTION s32
array_find(Array<ArrayType>* array, ArrayType& item, MatchFunctionType& equals_func)
{
    for(s32 i = 0; i < array->count; ++i)
    {
        if (equals_func(item, array->items[i])) return i;
    }
    return -1;
}

ARRAY_FUNCTION inline bool32
array_contains(Array<T>* a, T& item)
{
    return array_find(a, item) > -1;
}


#endif /* DRAC_ARRAY_H */
