/*
 *   This matrix multiply driver was originally written by Jason Riedy.
 *     Most of the code (and comments) are his, but there are several
 *       things I've hacked up.  It seemed to work for him, so any errors
 *         are probably mine.
 *
 *           Ideally, you won't need to change this file.  You may want to change
 *             a few settings to speed debugging runs, but remember to change back
 *               to the original settings during final testing.
 *
 *                 The output format: "Size: %u\tmflop/s: %g\n"
 *                 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <float.h>
#include <math.h>

#include <sys/types.h>
#include <sys/resource.h>

#include <unistd.h>
#include <sys/time.h>

#include "mm_malloc.h"
//#include <mkl.h>

//#include "papi_wrapper.h"

/*
 *   We try to run enough iterations to get reasonable timings.  The matrices
 *     are multiplied at least MIN_RUNS times.  If that doesn't take MIN_CPU_SECS
 *       seconds, then we double the number of iterations and try again.
 *
 *         You may want to modify these to speed debugging...
 *         */
#define MIN_RUNS     10
#define MIN_CPU_SECS 1.00000001

#define NN 512

//#define PAPI

/*
 *   Your function _MUST_ have the following signature:
 */

double ddot( const int, const double *, const int, const double *, const int); 


double ddot_ref( const int N, const double *a, const int incx, const double *b, const int incy)
{
        int i;
	long int ops = 0;
	long int mem = 0;

	double res = 0.;

	for (i = 0; i < N; ++i)
	{
		res += a[i]*b[i];
		ops += 2;
		mem += 2;
	}
	printf("ref ops = %ld, ", ops); 
	printf("ref mem = %ld, ", mem);
	printf("AI = %f\n", (double) ops/(mem*sizeof(double)));
	return res;
}	

double mysecond()
{
	struct timeval  tp;
	struct timezone tzp;
	int i;

	i = gettimeofday(&tp,&tzp);
	return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}
//
	void 
vector_init (double *A, const int N, double val)
{
	int i;

	for (i = 0; i < N; ++i) 
	{
		A[i] = val*drand48();
		//printf("A[%d] = %f\n", i, A[i]);
		//A[i] = M*val/N;
	}
}

	double
time_ddot (const int N, 
		const double *A, const int incx,
		const double *B, const int incy)
{
	double mflops, mflop_s;
	double secs = -1;

	int num_iterations = MIN_RUNS;
	int i;
	double dot;

	while (secs < MIN_CPU_SECS) 
	{
		double cpu_time = 0;
		for (i = 0; i < num_iterations; ++i) 
		{
			cpu_time -= mysecond();
			dot = ddot(N, A, incx, B, incy);
			cpu_time += mysecond();
		}

		mflops  = 2.0 * num_iterations*N/1.0e6;
		secs    = cpu_time;
		mflop_s = mflops/secs;
		//
		num_iterations *= 2;
	}
	printf("ddot = %f, ", dot);
	return mflop_s;
}
//
//
//
double time_ddot_blas(const int N, const double *A, const int incx, 
		const double *B, const int incy)
{
        double mflops, mflop_s;
        double secs = -1;

	int num_iterations = MIN_RUNS;
	int i;
	double dot;

	while (secs < MIN_CPU_SECS) 
	{
		double cpu_time = 0;
		//
		for (i = 0; i < num_iterations; ++i)
		{
			cpu_time -= mysecond();	
			dot = ddot_(&N, A, &incx, B, &incy);
			cpu_time += mysecond();
		}
		//
		mflops  = 2.0*num_iterations*N/1.0e6;
		secs    = cpu_time;
		mflop_s = mflops/secs;
		//
		num_iterations *= 2;
	}
	printf("ddot = %f, ", dot);
	return mflop_s;
}

	int
main (int argc, char *argv[])
{
	double mflop_s, mflop_b;

	int N;

	if (argc == 2)
	{
		N    = atoi(argv[1]);
	}
	else
	{
		N = NN;
	}

	int incx = 1;
	int incy = 1;

	double* A  = (double*) _mm_malloc(N*sizeof(double), 32);
	double* B  = (double*) _mm_malloc(N*sizeof(double), 32);

	vector_init(A,  N, 1.);
	//init_t(A, M, K, 1.);
	vector_init(B,  N, 1.);
	//
	printf("Size: %g GB\t", 2*N*8/1024./1024./1024.); fflush(stdout);
	//
	mflop_s = time_ddot(N, A, incx, B, incy);    
	printf ("Gflop/s: %g\n ", mflop_s/1000.); fflush(stdout);
	//
#if 1
	mflop_b = time_ddot_blas(N, A, incx, B, incy);
	printf ("blas Gflops: %g\n", mflop_b/1000);

#endif
	printf("naive = %f", ddot_ref(N, A, incx, B, incy)); 
	printf("\n");
	_mm_free(A);
	_mm_free(B);


	return 0;
}
