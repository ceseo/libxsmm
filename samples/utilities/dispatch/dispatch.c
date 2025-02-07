/******************************************************************************
** Copyright (c) 2016-2019, Intel Corporation                                **
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
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if defined(_OPENMP)
# include <omp.h>
#endif
#if defined(__MKL)
# include <mkl.h>
#endif
#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(pop)
#endif

#if !defined(MKLJIT) && defined(mkl_jit_create_dgemm) && \
  !defined(_WIN32) /* check this manually under Windows */
# define MKLJIT
#endif
#if (!defined(INTEL_MKL_VERSION) || (20190003 <= INTEL_MKL_VERSION)) && \
  !defined(_WIN32) /* TODO: Windows calling convention */
# define CHECK
#endif
#if !defined(MAXSIZE)
# define MAXSIZE LIBXSMM_MAX_M
#endif


typedef struct triplet { libxsmm_blasint m, n, k; } triplet;

LIBXSMM_INLINE void unique(triplet* mnk, int* size)
{
  if (NULL != mnk && NULL != size && 0 < *size) {
    triplet *const first = mnk, *last = mnk + ((size_t)*size - 1), *i;
    for (i = mnk + 1; mnk < last; ++mnk, i = mnk + 1) {
      while (i <= last) {
        if (i->m != mnk->m || i->n != mnk->n || i->k != mnk->k) {
          i++; /* skip */
        }
        else { /* copy */
          *i = *last--;
        }
      }
    }
    *size = (int)(last - first + 1);
  }
}


/**
 * This (micro-)benchmark measures the duration needed to dispatch a kernel.
 * Various durations are measured: time to generate the code, to dispatch
 * from cache, and to dispatch from the entire database. The large total
 * number of kernels may also stress the in-memory database.
 * When building with "make MKL=1", the benchmark exercises JIT capability of
 * Intel MKL. However, the measured "dispatch" durations cannot be compared
 * with LIBXSMM because MKL's JIT-interface does not provide a function to
 * query a kernel for a set of GEMM-arguments. The implicit JIT-dispatch
 * on the other hand does not expose the time to query the kernel.
 */
int main(int argc, char* argv[])
{
#if defined(_OPENMP)
  const int max_nthreads = omp_get_max_threads();
#else
  const int max_nthreads = 1;
#endif
  const int default_minsize = 4;
#if !defined(INTEL_MKL_VERSION) || (20190003 <= INTEL_MKL_VERSION)
  const int default_maxsize = MAXSIZE;
#else
  const int default_maxsize = 16;
#endif
  int size_total = LIBXSMM_MAX((1 < argc && 0 < atoi(argv[1])) ? atoi(argv[1]) : 10000/*default*/, 2);
  const int size_local = LIBXSMM_CLMP((2 < argc && 0 < atoi(argv[2])) ? atoi(argv[2]) : 4/*default*/, 1, size_total);
  const int nthreads = LIBXSMM_CLMP(3 < argc ? atoi(argv[3]) : 1/*default*/, 1, max_nthreads);
  const int nrepeat = LIBXSMM_MAX(4 < argc ? atoi(argv[4]) : 1/*default*/, 1);
  const libxsmm_blasint maxsize = LIBXSMM_CLMP((5 < argc && 0 < atoi(argv[5])) ? atoi(argv[5]) : default_maxsize, 1, MAXSIZE);
  const libxsmm_blasint minsize = LIBXSMM_CLMP((6 < argc && 0 < atoi(argv[6])) ? atoi(argv[6]) : default_minsize, 1, maxsize);
  const libxsmm_blasint range = maxsize - minsize + 1;
  libxsmm_timer_tickint start, tcall, tcgen, tdsp0 = 0, tdsp1 = 0;
  int result = EXIT_SUCCESS;

#if 0 != LIBXSMM_JIT
  if (LIBXSMM_X86_SSE3 > libxsmm_get_target_archid()) {
    fprintf(stderr, "\n\tWarning: JIT support is not available at runtime!\n");
  }
#else
  fprintf(stderr, "\n\tWarning: JIT support has been disabled at build time!\n");
#endif
#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload target(LIBXSMM_OFFLOAD_TARGET)
#endif
  {
    triplet *const rnd = (triplet*)(0 < size_total ? malloc(sizeof(triplet) * size_total) : NULL);
    const size_t shuffle = libxsmm_shuffle(size_total);
    const double alpha = 1, beta = 1;
    int i, n;

#if defined(MKLJIT)
    void* *const jitter = malloc(size_total * sizeof(void*));
    if (NULL == jitter) exit(EXIT_FAILURE);
#else
    const int prefetch = LIBXSMM_GEMM_PREFETCH_NONE;
    const int flags = LIBXSMM_GEMM_FLAG_NONE;
#endif
    if (NULL == rnd) exit(EXIT_FAILURE);

    /* generate set of random numbers outside of any parallel region */
    for (i = 0; i < size_total; ++i) {
      const int r1 = rand(), r2 = rand(), r3 = rand();
      rnd[i].m = (1 < range ? (LIBXSMM_MOD(r1, range) + minsize) : minsize);
      rnd[i].n = (1 < range ? (LIBXSMM_MOD(r2, range) + minsize) : minsize);
      rnd[i].k = (1 < range ? (LIBXSMM_MOD(r3, range) + minsize) : minsize);
#if defined(MKLJIT)
      jitter[i] = NULL;
#endif
    }
    unique(rnd, &size_total);

    printf("Dispatching total=%i and local=%i kernels using %i thread%s...", size_total, size_local,
      1 >= nthreads ? 1 : nthreads,
      1 >= nthreads ? "" : "s");

    /* first invocation may initialize some internals */
    libxsmm_init(); /* subsequent calls are not doing any work */
    start = libxsmm_timer_tick();
    for (n = 0; n < nrepeat; ++n) {
      for (i = 0; i < size_total; ++i) {
        /* measure call overhead of an "empty" function (not inlined) */
        libxsmm_init();
      }
    }
    tcall = libxsmm_timer_ncycles(start, libxsmm_timer_tick());

    /* trigger code generation to subsequently measure only dispatch time */
    start = libxsmm_timer_tick();
    for (i = 0; i < size_local; ++i) {
#if defined(MKLJIT)
      mkl_cblas_jit_create_dgemm(jitter,
        MKL_COL_MAJOR, MKL_NOTRANS/*transa*/, MKL_NOTRANS/*transb*/,
        rnd[i].m, rnd[i].n, rnd[i].k, alpha, rnd[i].m, rnd[i].k, beta, rnd[i].m);
      mkl_jit_get_dgemm_ptr(jitter[i]); /* to include lookup time */
#else
      libxsmm_dmmdispatch(rnd[i].m, rnd[i].n, rnd[i].k, &rnd[i].m, &rnd[i].k, &rnd[i].m, &alpha, &beta, &flags, &prefetch);
#endif
    }
    tcgen = libxsmm_timer_ncycles(start, libxsmm_timer_tick());

    /* measure duration for dispatching (cached) kernel; MKL: no "dispatch" just unwrapping the jitter */
#if defined(_OPENMP)
    if (1 < nthreads) {
      for (n = 0; n < nrepeat; ++n) {
#       pragma omp parallel num_threads(nthreads) private(i)
        {
#         pragma omp master
          start = libxsmm_timer_tick();
#         pragma omp for
          for (i = 0; i < size_total; ++i) {
            const int j = LIBXSMM_MOD(i, size_local);
#if defined(MKLJIT)
            mkl_jit_get_dgemm_ptr(jitter[j]);
#else
            libxsmm_dmmdispatch(rnd[j].m, rnd[j].n, rnd[j].k, &rnd[j].m, &rnd[j].k, &rnd[j].m, &alpha, &beta, &flags, &prefetch);
#endif
          }
#         pragma omp master
          tdsp1 += libxsmm_timer_ncycles(start, libxsmm_timer_tick());
        }
      }
    }
    else
#endif
    {
      for (n = 0; n < nrepeat; ++n) {
        start = libxsmm_timer_tick();
        for (i = 0; i < size_total; ++i) {
          const int j = LIBXSMM_MOD(i, size_local);
#if defined(MKLJIT)
          mkl_jit_get_dgemm_ptr(jitter[j]);
#else
          libxsmm_dmmdispatch(rnd[j].m, rnd[j].n, rnd[j].k, &rnd[j].m, &rnd[j].k, &rnd[j].m, &alpha, &beta, &flags, &prefetch);
#endif
        }
        tdsp1 += libxsmm_timer_ncycles(start, libxsmm_timer_tick());
      }
    }

    /* measure duration for code-generation */
#if defined(_OPENMP)
    if (1 < nthreads) {
#     pragma omp parallel num_threads(nthreads) private(i)
      {
#       pragma omp master
        start = libxsmm_timer_tick();
#       pragma omp for
        for (i = size_local; i < size_total; ++i) {
#if defined(MKLJIT)
          mkl_cblas_jit_create_dgemm(jitter + i,
            MKL_COL_MAJOR, MKL_NOTRANS/*transa*/, MKL_NOTRANS/*transb*/,
            rnd[i].m, rnd[i].n, rnd[i].k, alpha, rnd[i].m, rnd[i].k, beta, rnd[i].m);
          mkl_jit_get_dgemm_ptr(jitter[i]);
#else
          libxsmm_dmmdispatch(rnd[i].m, rnd[i].n, rnd[i].k, &rnd[i].m, &rnd[i].k, &rnd[i].m, &alpha, &beta, &flags, &prefetch);
#endif
        }
#       pragma omp master
        tcgen += libxsmm_timer_ncycles(start, libxsmm_timer_tick());
      }
    }
    else
#endif
    {
      start = libxsmm_timer_tick();
      for (i = size_local; i < size_total; ++i) {
#if defined(MKLJIT)
        mkl_cblas_jit_create_dgemm(jitter + i,
          MKL_COL_MAJOR, MKL_NOTRANS/*transa*/, MKL_NOTRANS/*transb*/,
          rnd[i].m, rnd[i].n, rnd[i].k, alpha, rnd[i].m, rnd[i].k, beta, rnd[i].m);
        mkl_jit_get_dgemm_ptr(jitter[i]);
#else
        libxsmm_dmmdispatch(rnd[i].m, rnd[i].n, rnd[i].k, &rnd[i].m, &rnd[i].k, &rnd[i].m, &alpha, &beta, &flags, &prefetch);
#endif
      }
      tcgen += libxsmm_timer_ncycles(start, libxsmm_timer_tick());
    }

    /* measure dispatching previously generated kernel (likely non-cached) */
#if defined(_OPENMP)
    if (1 < nthreads) {
      for (n = 0; n < nrepeat; ++n) {
#       pragma omp parallel num_threads(nthreads) private(i)
        {
#         pragma omp master
          start = libxsmm_timer_tick();
#         pragma omp for
          for (i = 0; i < size_total; ++i) {
            const int j = (int)LIBXSMM_MOD(shuffle * i, size_total);
#if defined(MKLJIT)
            mkl_jit_get_dgemm_ptr(jitter[j]);
#else
            libxsmm_dmmdispatch(rnd[j].m, rnd[j].n, rnd[j].k, &rnd[j].m, &rnd[j].k, &rnd[j].m, &alpha, &beta, &flags, &prefetch);
#endif
          }
#         pragma omp master
          tdsp0 += libxsmm_timer_ncycles(start, libxsmm_timer_tick());
        }
      }
    }
    else
#endif
    {
      for (n = 0; n < nrepeat; ++n) {
        start = libxsmm_timer_tick();
        for (i = 0; i < size_total; ++i) {
          const int j = (int)LIBXSMM_MOD(shuffle * i, size_total);
#if defined(MKLJIT)
          mkl_jit_get_dgemm_ptr(jitter[j]);
#else
          libxsmm_dmmdispatch(rnd[j].m, rnd[j].n, rnd[j].k, &rnd[j].m, &rnd[j].k, &rnd[j].m, &alpha, &beta, &flags, &prefetch);
#endif
        }
        tdsp0 += libxsmm_timer_ncycles(start, libxsmm_timer_tick());
      }
    }

#if defined(CHECK)
    { /* calculate l1-norm for manual validation */
      double a[LIBXSMM_MAX_M*LIBXSMM_MAX_M];
      double b[LIBXSMM_MAX_M*LIBXSMM_MAX_M];
      double c[LIBXSMM_MAX_M*LIBXSMM_MAX_M];
      libxsmm_matdiff_info check;
      libxsmm_matdiff_clear(&check);
      LIBXSMM_MATINIT(double, 0, a, maxsize, maxsize, maxsize, 1.0);
      LIBXSMM_MATINIT(double, 0, b, maxsize, maxsize, maxsize, 1.0);
      LIBXSMM_MATINIT(double, 0, c, maxsize, maxsize, maxsize, 1.0);
      for (i = 0; i < size_total; ++i) {
        const int j = (int)LIBXSMM_MOD(shuffle * i, size_total);
        libxsmm_matdiff_info diff;
# if defined(MKLJIT)
        const dgemm_jit_kernel_t kernel = mkl_jit_get_dgemm_ptr(jitter[j]);
# else
        const libxsmm_dmmfunction kernel = libxsmm_dmmdispatch(rnd[j].m, rnd[j].n, rnd[j].k,
          &rnd[j].m, &rnd[j].k, &rnd[j].m, &alpha, &beta, &flags, &prefetch);
# endif
        if (NULL != kernel) {
# if defined(MKLJIT)
          kernel(jitter[j], a, b, c);
# else
          if (LIBXSMM_GEMM_PREFETCH_NONE == prefetch) kernel(a, b, c); else kernel(a, b, c, a, b, c);
# endif
          result = libxsmm_matdiff(&diff, LIBXSMM_DATATYPE(double), rnd[j].m, rnd[j].n, NULL, c, &rnd[j].m, &rnd[j].m);
        }
        else {
          result = EXIT_FAILURE;
        }
        if (EXIT_SUCCESS == result) {
          libxsmm_matdiff_reduce(&check, &diff);
        }
        else {
          printf(" m=%u n=%u k=%u kernel=%" PRIuPTR, (unsigned int)rnd[j].m, (unsigned int)rnd[j].n, (unsigned int)rnd[j].k, (uintptr_t)kernel);
          i = size_total + 1; /* break */
        }
      }
      if (i <= size_total) {
        printf(" check=%f\n", check.l1_tst);
      }
      else {
        printf(" <- ERROR!\n");
      }
    }
#else
    printf("\n");
#endif /*defined(CHECK)*/
    free(rnd); /* release random numbers */
#if defined(MKLJIT) /* release dispatched code */
    for (i = 0; i < size_total; ++i) mkl_jit_destroy(jitter[i]);
    free(jitter); /* release array used to store dispatched code */
#endif
  }

  tcall = (tcall + (size_t)size_total * nrepeat - 1) / ((size_t)size_total * nrepeat);
  tdsp0 = (tdsp0 + (size_t)size_total * nrepeat - 1) / ((size_t)size_total * nrepeat);
  tdsp1 = (tdsp1 + (size_t)size_total * nrepeat - 1) / ((size_t)size_total * nrepeat);
  tcgen = (tcgen + size_total - 1) / size_total;
  if (0 < tcall && 0 < tdsp0 && 0 < tdsp1 && 0 < tcgen) {
    const double tcall_ns = 1E9 * libxsmm_timer_duration(0, tcall), tcgen_ns = 1E9 * libxsmm_timer_duration(0, tcgen);
    const double tdsp0_ns = 1E9 * libxsmm_timer_duration(0, tdsp0), tdsp1_ns = 1E9 * libxsmm_timer_duration(0, tdsp1);
    printf("\tfunction-call (false): %.0f ns (call/s %.0f MHz, %" PRIuPTR " cycles)\n", tcall_ns, 1E3 / tcall_ns, (uintptr_t)libxsmm_timer_ncycles(0, tcall));
    printf("\tdispatch (ro/cached): %.0f ns (call/s %.0f MHz, %" PRIuPTR " cycles)\n", tdsp1_ns, 1E3 / tdsp1_ns, (uintptr_t)libxsmm_timer_ncycles(0, tdsp1));
    printf("\tdispatch (ro): %.0f ns (call/s %.0f MHz, %" PRIuPTR " cycles)\n", tdsp0_ns, 1E3 / tdsp0_ns, (uintptr_t)libxsmm_timer_ncycles(0, tdsp0));
    if (1E6 < tcgen_ns) {
      printf("\tcode-gen (rw): %.0f ms (call/s %.0f Hz)\n", 1E-6 * tcgen_ns, 1E9 / tcgen_ns);
    }
    else if (1E3 < tcgen_ns) {
      printf("\tcode-gen (rw): %.0f us (call/s %.0f kHz)\n", 1E-3 * tcgen_ns, 1E6 / tcgen_ns);
    }
    else {
      printf("\tcode-gen (rw): %.0f ns (call/s %.0f MHz)\n", tcgen_ns, 1E3 / tcgen_ns);
    }
  }
  printf("Finished\n");

  return result;
}

