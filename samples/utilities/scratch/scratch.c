/******************************************************************************
** Copyright (c) 2017-2019, Intel Corporation                                **
** All rights reserved.                                                      **
**                                                                           **
** Redistribution and use in source and binary forms, with or without        **
** modification, are permitted provided that the following conditions        **
** are met:                                                                  **
** 1. Redistributions of source code must retain the above copyright         **
**    notice, this list of conditions and the following disclaimer.          **
** 2. Redistributions in binary form must reproduce the above copyright      **
**    notice, this list of conditions and the following disclaimer in the    **
**    documentation and/or other materials provided with the distribution.   **
** 3. Neither the name of the copyright holder nor the names of its          **
**    contributors may be used to endorse or promote products derived        **
**    from this software without specific prior written permission.          **
**                                                                           **
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       **
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         **
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR     **
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT      **
** HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    **
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED  **
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR    **
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    **
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      **
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        **
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              **
******************************************************************************/
/* Hans Pabst (Intel Corp.)
******************************************************************************/
#include <libxsmm.h>

#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(push,target(LIBXSMM_OFFLOAD_TARGET))
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if defined(_OPENMP)
# include <omp.h>
#endif
#if defined(__TBB)
# include <tbb/scalable_allocator.h>
#endif
#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(pop)
#endif

#if !defined(MAX_MALLOC_MB)
# define MAX_MALLOC_MB 100
#endif
#if !defined(MAX_MALLOC_N)
# define MAX_MALLOC_N 24
#endif


void* malloc_offsite(size_t size);


int main(int argc, char* argv[])
{
#if defined(_OPENMP)
  const int max_nthreads = omp_get_max_threads();
#else
  const int max_nthreads = 1;
#endif
  const int ncycles = LIBXSMM_MAX(1 < argc ? atoi(argv[1]) : 100, 1);
  const int max_nallocs = LIBXSMM_CLMP(2 < argc ? atoi(argv[2]) : 4, 1, MAX_MALLOC_N);
  const int nthreads = LIBXSMM_CLMP(3 < argc ? atoi(argv[3]) : 1, 1, max_nthreads);
  const char *const env_check = getenv("CHECK");
  const double check = LIBXSMM_ABS(NULL == env_check ? 0 : atof(env_check));
  unsigned int nallocs = 0, nerrors0 = 0, nerrors1 = 0;
  int r[MAX_MALLOC_N], i;
  int max_size = 0;

  /* generate set of random number for parallel region */
  for (i = 0; i < (MAX_MALLOC_N); ++i) r[i] = rand();

  /* count number of calls according to randomized scheme */
  for (i = 0; i < ncycles; ++i) {
    const int count = r[i%(MAX_MALLOC_N)] % max_nallocs + 1;
    int mbytes = 0, j;
    for (j = 0; j < count; ++j) {
      const int k = (i * count + j) % (MAX_MALLOC_N);
      mbytes += (r[k] % (MAX_MALLOC_MB) + 1);
    }
    if (max_size < mbytes) max_size = mbytes;
    nallocs += count;
  }
  assert(0 != nallocs);

  fprintf(stdout, "Running %i cycles with max. %i malloc+free (%u calls) using %i thread%s...\n",
    ncycles, max_nallocs, nallocs, 1 >= nthreads ? 1 : nthreads, 1 >= nthreads ? "" : "s");

#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload target(LIBXSMM_OFFLOAD_TARGET)
#endif
  {
    const char *const longlife_env = getenv("LONGLIFE");
    const int enable_longlife = ((NULL == longlife_env || 0 == *longlife_env) ? 0 : atoi(longlife_env));
    void* longlife = (0 == enable_longlife ? NULL : malloc_offsite((MAX_MALLOC_MB) << 20));
    libxsmm_timer_tickint d0 = 0, d1 = 0;
    int scratch = 0, local = 0;
    libxsmm_scratch_info info;

    libxsmm_init();

#if defined(_OPENMP)
#   pragma omp parallel for num_threads(nthreads) private(i) reduction(+:d1,nerrors1)
#endif
    for (i = 0; i < ncycles; ++i) {
      const int count = r[i%(MAX_MALLOC_N)] % max_nallocs + 1;
      void* p[MAX_MALLOC_N];
      int j;
      assert(count <= MAX_MALLOC_N);
      for (j = 0; j < count; ++j) {
        const int k = (i * count + j) % (MAX_MALLOC_N);
        const size_t nbytes = ((size_t)r[k] % (MAX_MALLOC_MB) + 1) << 20;
        const libxsmm_timer_tickint t1 = libxsmm_timer_tick();
        p[j] = libxsmm_aligned_scratch(nbytes, 0/*auto*/);
        d1 += libxsmm_timer_ncycles(t1, libxsmm_timer_tick());
        if (NULL == p[j]) {
          ++nerrors1;
        }
        else if (0 != check) {
          memset(p[j], j, nbytes);
        }
      }
      for (j = 0; j < count; ++j) {
        libxsmm_free(p[j]);
      }
    }
    libxsmm_free(longlife);
    if (EXIT_SUCCESS == libxsmm_get_scratch_info(&info) && 0 < info.size) {
      scratch = (int)(1.0 * info.size / (1ULL << 20) + 0.5);
      local = (int)(1.0 * info.local / (1ULL << 20) + 0.5);
      fprintf(stdout, "\nScratch: %i+%i MB (mallocs=%lu, pools=%u)\n",
        scratch, local, (unsigned long int)info.nmallocs, info.npools);
      libxsmm_release_scratch(); /* suppress LIBXSMM's termination message about scratch */
    }

#if defined(__TBB)
    longlife = (0 == enable_longlife ? NULL : scalable_malloc((MAX_MALLOC_MB) << 20));
#else
    longlife = (0 == enable_longlife ? NULL : malloc((MAX_MALLOC_MB) << 20));
#endif
#if defined(_OPENMP)
#   pragma omp parallel for num_threads(nthreads) private(i) reduction(+:d0,nerrors0)
#endif
    for (i = 0; i < ncycles; ++i) {
      const int count = r[i % (MAX_MALLOC_N)] % max_nallocs + 1;
      void* p[MAX_MALLOC_N];
      int j;
      assert(count <= MAX_MALLOC_N);
      for (j = 0; j < count; ++j) {
        const int k = (i * count + j) % (MAX_MALLOC_N);
        const size_t nbytes = ((size_t)r[k] % (MAX_MALLOC_MB) + 1) << 20;
        const libxsmm_timer_tickint t1 = libxsmm_timer_tick();
#if defined(__TBB)
        p[j] = scalable_malloc(nbytes);
#else
        p[j] = malloc(nbytes);
#endif
        d0 += libxsmm_timer_ncycles(t1, libxsmm_timer_tick());
        if (NULL == p[j]) {
          ++nerrors0;
        }
        else if (0 != check) {
          memset(p[j], j, nbytes);
        }
      }
      for (j = 0; j < count; ++j) {
#if defined(__TBB)
        scalable_free(p[j]);
#else
        free(p[j]);
#endif
      }
    }
#if defined(__TBB)
    scalable_free(longlife);
#else
    free(longlife);
#endif

    if (0 != d0 && 0 != d1 && 0 < nallocs) {
      const double dcalls = libxsmm_timer_duration(0, d0);
      const double dalloc = libxsmm_timer_duration(0, d1);
      const double scratch_freq = 1E-3 * nallocs / dalloc;
      const double malloc_freq = 1E-3 * nallocs / dcalls;
      const double speedup = scratch_freq / malloc_freq;
      fprintf(stdout, "\tlibxsmm scratch calls/s: %.1f kHz\n", scratch_freq);
      fprintf(stdout, "Malloc: %i MB\n", max_size);
      fprintf(stdout, "\tstd.malloc+free calls/s: %.1f kHz\n", malloc_freq);
      fprintf(stdout, "Fair (size vs. speed): %.1fx\n",
        max_size * speedup / LIBXSMM_MAX(scratch + local, max_size));
      fprintf(stdout, "Scratch Speedup: %.1fx\n", speedup);
    }
  }

  if (0 != nerrors0 || 0 != nerrors1) {
    fprintf(stdout, "FAILED (errors: malloc=%u libxsmm=%u)\n", nerrors0, nerrors1);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


void* malloc_offsite(size_t size) { return libxsmm_aligned_scratch(size, 0/*auto*/); }

