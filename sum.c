#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif


#define CLOCK_RATE_GHZ 2.26e9

static __inline__ uint64_t RDTSC()
{
    uint32_t hi, lo;
    __asm__ volatile
	(
		"rdtsc"
		: "=a"(lo), "=d"(hi)
	);
	return (((uint64_t)hi) << 32) | ((uint64_t)lo);
}

static int sum_naive(int n, int *a)
{
    int sum = 0;
	for (int i = 0; i < n; i++)
	{
		sum += a[i];
	}
	return sum;
}

static int sum_unrolled(int n, int *a)
{
    int sum = 0;

    // unrolled loop
	for (int i = 0; i < n / 4 * 4; i += 4)
    {
        sum += a[i+0];
        sum += a[i+1];
        sum += a[i+2];
        sum += a[i+3];
    }

    // tail case
	for (int i = n / 4 * 4; i < n; i++)
	{
		sum += a[i];
	}

    return sum;
}

static int sum_vectorized(int n, int *a)
{
    // WRITE YOUR VECTORIZED CODE HERE
    __m128i vecSum = _mm_setzero_si128();
    __m128i curVec;
    int finalVec[4]; //= malloc(4*sizeof(int));
    int sum = 0;
    int i = 0;
    for(i=0; i<n-n%4; i+=4){
    	curVec = _mm_loadu_si128((__m128i*)(a+i));
    	vecSum = _mm_add_epi32(vecSum, curVec);
    }
    _mm_storeu_si128((__m128i*)finalVec, vecSum);
    for(int j=0; j<4; j++){
    	sum += finalVec[j];
    }
    for(i=n-n%4; i<n; i++){
    	sum += a[i];
    }
    return sum;

}

static int sum_vectorized_unrolled(int n, int *a)
{
    // UNROLL YOUR VECTORIZED CODE HERE
    int sum = 0;
    int size = (n-n%4)/4;
    int* t=malloc(4*sizeof(int));
    __m128i one;
    __m128i vectorSum = _mm_setzero_si128();
    for (int p = 0; p < (n-n%4)/4; p +=4){
        if ((p*4) < (n-n%4)){
            one = _mm_loadu_si128((__m128i*)(a+p*4));
            vectorSum= _mm_add_epi32(vectorSum, one);
            if (((p+1)*4) < (n-n%4)){
                one = _mm_loadu_si128((__m128i*)(a+(p+1)*4));
                vectorSum= _mm_add_epi32(vectorSum, one);
                if (((p+2)*4)< (n-n%4)){
                    one = _mm_loadu_si128((__m128i*)(a+(p+2)*4));
                    vectorSum= _mm_add_epi32(vectorSum, one);
                    if (((p+3)*4) < (n-n%4)){
                        one = _mm_loadu_si128((__m128i*)(a+(p+3)*4));
                        vectorSum= _mm_add_epi32(vectorSum, one);
                    }else break;
                }else break;
            }else break;
        }else break;
    }
    _mm_storeu_si128((__m128i *)t, vectorSum);
    for (int q = 0; q< 4; q++){
        sum += t[q];
    }
    for (int r = n-n%4; r<n; r++){
        sum += a[r];
    }
    return sum;
}

void benchmark(int n, int *a, int(*computeSum)(int, int*), char *name)
{
    // warm up cache
    int sum = computeSum(n, a);

    // measure
    uint64_t beginCycle = RDTSC();
    sum += computeSum(n, a);
	uint64_t cycles = RDTSC() - beginCycle;
    
    double microseconds = cycles/CLOCK_RATE_GHZ*1e6;
    
    // print results
	printf("%20s: ", name);
	if (sum == 2 * sum_naive(n, a))
	{
		printf("%.2f microseconds\n", microseconds);
	}
	else
	{
		printf("ERROR!\n");
	}
}

int main(int argc, char **argv)
{
    const int n = 7777;
    
    // initialize the array with random values
	srand48(time(NULL));
	int a[n] __attribute__((aligned(16)));
	for (int i = 0; i < n; i++)
	{
		a[i] = lrand48();
	}
    
    // benchmark series of codes
	benchmark(n, a, sum_naive, "naive");
	benchmark(n, a, sum_unrolled, "unrolled");
	benchmark(n, a, sum_vectorized, "vectorized");
	benchmark(n, a, sum_vectorized_unrolled, "vectorized unrolled");

    return 0;
}

