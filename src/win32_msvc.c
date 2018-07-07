
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#include <xmmintrin.h>
#include <emmintrin.h>

#define align_n(val, alignment) (((val) + ((alignment)-1)) & ~((alignment)-1))


#ifdef __cplusplus
extern "C" 
{
#endif
    
    int _fltused = 0x9875;
    
    //
    // strlen
    //
#if 0
#pragma function(strlen)
    size_t strlen(const char* str)
    {
#if _WIN64
#define mask01 0x0101010101010101
#define mask80 0x8080808080808080
#define PTR_MASK (sizeof(uint64_t) - 1)
        
        const char *p;
        const uint64_t *lp;
        int64_t va, vb;
        
        lp = (const uint64_t*)((uintptr_t)str & ~PTR_MASK);
        va = (*lp - mask01);
        vb = ((~*lp) & mask80);
        lp++;
        if (va & vb)
        {
            for (p = str; p < (const char *)lp; p++) {
                if (!*p) return (p - str);   
            }
        }
        for (;; ++lp) 
        {
            va = (*lp - mask01);
            vb = ((~*lp) & mask80);
            if (va & vb) 
            {
                uint64_t x = *lp;
                if(!(x & 0xFF)) return (char*)lp - str;
                if(!(x & 0xFF00)) return (char*)lp - str + 1;
                if(!(x & 0xFF0000)) return (char*)lp - str + 2;
                if(!(x & 0xFF000000)) return (char*)lp - str + 3;
                if(!(x & 0xFF00000000)) return (char*)lp - str + 4;
                if(!(x & 0xFF0000000000)) return (char*)lp - str + 5;
                if(!(x & 0xFF000000000000)) return (char*)lp - str + 6;
                if(!(x & 0xFF00000000000000)) return (char*)lp - str + 7;
            }
        }
        
#else 
#define mask01 0x01010101
#define mask80 0x80808080
#define PTR_MASK (sizeof(uint32_t) - 1)
        
        const char *p;
        const uint32_t *lp;
        int32_t va, vb;
        
        lp = (const uint32_t*)((uintptr_t)str & ~PTR_MASK);
        va = (*lp - mask01);
        vb = ((~*lp) & mask80);
        lp++;
        if (va & vb)
        {
            for (p = str; p < (const char *)lp; p++) {
                if (!*p) return (p - str);   
            }
        }
        for (;; ++lp) {
            va = (*lp - mask01);
            vb = ((~*lp) & mask80);
            if (va & vb) 
            {            
                uint32_t x = *lp;
                if(!(x & 0xFF)) return (char*)lp - str;
                if(!(x & 0xFF00)) return (char*)lp - str + 1;
                if(!(x & 0xFF0000)) return (char*)lp - str + 2;
                if(!(x & 0xFF000000)) return (char*)lp - str + 3;
            }
        }
        
#endif
        
#undef mask01
#undef mask80
#undef PTR_MASK
    }
#endif /* 0 */
    
    //
    // memset
    //
#if 0
#pragma function(memset)
    void* memset(void* dst, int32_t val, size_t numBytes)
    {
        const register int64_t regSize = sizeof(__m128i);
        
        int64_t extraFront = align_n((uintptr_t)dst, regSize) - (int64_t)dst;
        int64_t sseBytes = (numBytes - extraFront) & ~(regSize-1);
        int64_t extraBack = numBytes - sseBytes - extraFront;
        register char* ptr = (char*)dst;
        register char set = (char)val;
        
        __m128i xmm0;
        if (set) xmm0 = _mm_set1_epi8(set);
        else     xmm0 = _mm_setzero_si128();
        
        while (extraFront--)
            *ptr++ = set;
        
        int64_t stride = 8 * regSize;
        while(sseBytes >= stride)
        {
            _mm_stream_si128(((__m128i*)ptr) + 0, xmm0);
            _mm_stream_si128(((__m128i*)ptr) + 1, xmm0);
            _mm_stream_si128(((__m128i*)ptr) + 2, xmm0);
            _mm_stream_si128(((__m128i*)ptr) + 3, xmm0);
            _mm_stream_si128(((__m128i*)ptr) + 4, xmm0);
            _mm_stream_si128(((__m128i*)ptr) + 5, xmm0);
            _mm_stream_si128(((__m128i*)ptr) + 6, xmm0);
            _mm_stream_si128(((__m128i*)ptr) + 7, xmm0);
            ptr += stride;
            sseBytes -= stride;
        }
        
        while(sseBytes > (regSize - 1))
        {
            _mm_stream_si128(((__m128i*)ptr), xmm0);
            ptr += regSize;
            sseBytes -= regSize;
        }
        
        while (extraBack--)
            *ptr++ = set;
        
        return dst;
    } 
#endif
    
    //
    // memcpy
    //
#if 0
#pragma function(memcpy)
    void* memcpy(void* dst, void const* src, size_t numBytes)
    {
        if (((uintptr_t)dst % 16) != ((uintptr_t)src % 16))
        {
            char* ptr = (char*)dst;
            char* set = (char*)src;
            while (numBytes--) 
                *ptr++ = *set++;
        }
        else
        {
            int64_t extraFront = align_n((uintptr_t)dst, 16) - (int64_t)dst;
            int64_t middleBytes = (numBytes - extraFront) & ~(15);
            int64_t extraBack = numBytes - middleBytes - extraFront;
            char* ptr = (char*)dst;
            char* set = (char*)src;
            
            while (extraFront--) 
                *ptr++ = *set++;
            
            int64_t stride = 32;
            while (middleBytes >= stride)
            {
                _mm_stream_si128(((__m128i*)ptr) + 0, _mm_load_si128((__m128i*)set + 0));
                _mm_stream_si128(((__m128i*)ptr) + 1, _mm_load_si128((__m128i*)set + 1));
                ptr += stride;
                set += stride;
                middleBytes -= stride;
            }
            
            while(middleBytes > 0)
            {
                _mm_stream_si128((__m128i*)ptr, _mm_load_si128((__m128i*)set));
                ptr += 16;
                set += 16;
                middleBytes -= 16;
            }
            
            while (extraBack--) 
                *ptr++ = *set++;
        }
        return dst;
    }
#endif /* 0 */
    
    //
    // memmove
    //
    
#pragma function(memmove)
    void* memmove(void* dst, void const* src, size_t numBytes)
    {
        if ((uintptr_t)dst < (uintptr_t)src) 
        {
            char* ptr = (char*)dst;
            char* set = (char*)src;
            while(numBytes--) 
                *ptr++ = *set++;
        }
        else
        {
            char* ptr = (char*)dst + numBytes;
            char* set = (char*)src + numBytes;
            while(numBytes--)
                *ptr-- = *set--;
        }
        return dst;
    }
    
    
#undef align_n
    
#ifdef __cplusplus
}
#endif

#endif