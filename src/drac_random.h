#ifndef DRAC_RANDOM_H
#define DRAC_RANDOM_H

#ifndef FUNCTION
#define FUNCTION
#endif

#if !defined(DRAC_RANDOM_PCG)
#define DRAC_RANDOM_PCG 1
#endif

#include <stdint.h>

FUNCTION void     seed_rand(uint64_t seed);
FUNCTION float    rand_f32(float lowerBound = 0.0f, float upperBound = 1.0f);FUNCTION double   rand_f64(double lowerBound = 0.0f, double upperBound = 1.0f);
FUNCTION int32_t  rand_i32(int32_t lowerBound, int32_t upperBound);
FUNCTION uint32_t rand_32_bits();


#endif /* DRAC_RANDOM_H */
