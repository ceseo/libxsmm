/******************************************************************************
** Copyright (c) 2018-2019, Intel Corporation                                **
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
#include "libxsmm_main.h"

#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(push,target(LIBXSMM_OFFLOAD_TARGET))
#endif
#include <string.h>
#include <stdio.h>
#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(pop)
#endif

#if !defined(LIBXSMM_PRODUCT_LIMIT)
# define LIBXSMM_PRODUCT_LIMIT 1024
#endif


LIBXSMM_API libxsmm_gemm_descriptor* libxsmm_dgemm_descriptor_init(libxsmm_descriptor_blob* blob,
  libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k, libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc,
  double alpha, double beta, int flags, int prefetch)
{
  union {
    libxsmm_gemm_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  if  (LIBXSMM_GEMM_NO_BYPASS(flags, alpha, beta)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(lda, ldb, ldc)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(m, n, k))
  {
    result.blob = blob;
    LIBXSMM_GEMM_DESCRIPTOR(*result.ptr, LIBXSMM_GEMM_PRECISION(double),
      flags, m, n, k, lda, ldb, ldc, alpha, beta, prefetch);
  }
  else { /* quiet error (unsupported) */
    result.ptr = NULL;
  }
  return result.ptr;
}


LIBXSMM_API libxsmm_gemm_descriptor* libxsmm_sgemm_descriptor_init(libxsmm_descriptor_blob* blob,
  libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k, libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc,
  float alpha, float beta, int flags, int prefetch)
{
  union {
    libxsmm_gemm_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  if  (LIBXSMM_GEMM_NO_BYPASS(flags, alpha, beta)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(lda, ldb, ldc)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(m, n, k))
  {
    result.blob = blob;
    LIBXSMM_GEMM_DESCRIPTOR(*result.ptr, LIBXSMM_GEMM_PRECISION(float),
      flags, m, n, k, lda, ldb, ldc, alpha, beta, prefetch);
  }
  else { /* unsupported */
    result.ptr = NULL;
  }
  return result.ptr;
}


LIBXSMM_API libxsmm_gemm_descriptor* libxsmm_wigemm_descriptor_init(libxsmm_descriptor_blob* blob,
  libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k, libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc,
  int alpha, int beta, int flags, int prefetch)
{
  union {
    libxsmm_gemm_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  if  (LIBXSMM_GEMM_NO_BYPASS(flags, alpha, beta)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(lda, ldb, ldc)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(m, n, k))
  {
    result.blob = blob;
    LIBXSMM_GEMM_DESCRIPTOR2(*result.ptr, LIBXSMM_GEMM_PRECISION(short), LIBXSMM_GEMM_PRECISION(int),
      flags, m, n, k, lda, ldb, ldc, alpha, beta, prefetch);
  }
  else { /* unsupported */
    result.ptr = NULL;
  }
  return result.ptr;
}


LIBXSMM_API libxsmm_gemm_descriptor* libxsmm_wsgemm_descriptor_init(libxsmm_descriptor_blob* blob,
  libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k, libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc,
  float alpha, float beta, int flags, int prefetch)
{
  union {
    libxsmm_gemm_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  if (LIBXSMM_GEMM_NO_BYPASS(flags, alpha, beta)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(lda, ldb, ldc)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(m, n, k))
  {
    result.blob = blob;
    LIBXSMM_GEMM_DESCRIPTOR2(*result.ptr, LIBXSMM_GEMM_PRECISION(short), LIBXSMM_GEMM_PRECISION(float),
      flags, m, n, k, lda, ldb, ldc, alpha, beta, prefetch);
  }
  else { /* unsupported */
    result.ptr = NULL;
  }
  return result.ptr;
}


LIBXSMM_API libxsmm_gemm_descriptor* libxsmm_bsgemm_descriptor_init(libxsmm_descriptor_blob* blob,
  libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k, libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc,
  float alpha, float beta, int flags, int prefetch)
{
  union {
    libxsmm_gemm_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  if (LIBXSMM_GEMM_NO_BYPASS(flags, alpha, beta)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(lda, ldb, ldc)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(m, n, k))
  {
    result.blob = blob;
    LIBXSMM_GEMM_DESCRIPTOR2(*result.ptr, LIBXSMM_GEMM_PRECISION(libxsmm_bfloat16), LIBXSMM_GEMM_PRECISION(float),
      flags, m, n, k, lda, ldb, ldc, alpha, beta, prefetch);
  }
  else { /* unsupported */
    result.ptr = NULL;
  }
  return result.ptr;
}


LIBXSMM_API libxsmm_gemm_descriptor* libxsmm_bgemm_descriptor_init(libxsmm_descriptor_blob* blob,
  libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k, libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc,
  float alpha, float beta, int flags, int prefetch)
{
  union {
    libxsmm_gemm_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  if (LIBXSMM_GEMM_NO_BYPASS(flags, alpha, beta)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(lda, ldb, ldc)
    && LIBXSMM_GEMM_NO_BYPASS_DIMS(m, n, k))
  {
    result.blob = blob;
    LIBXSMM_GEMM_DESCRIPTOR2(*result.ptr, LIBXSMM_GEMM_PRECISION(libxsmm_bfloat16), LIBXSMM_GEMM_PRECISION(libxsmm_bfloat16),
      flags, m, n, k, lda, ldb, ldc, alpha, beta, prefetch);
  }
  else { /* unsupported */
    result.ptr = NULL;
  }
  return result.ptr;
}


LIBXSMM_API libxsmm_gemm_descriptor* libxsmm_gemm_descriptor_dinit(libxsmm_descriptor_blob* blob,
  libxsmm_gemm_precision precision, libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k,
  libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc, double alpha, double beta,
  int flags, int prefetch)
{
  return libxsmm_gemm_descriptor_dinit2(blob, precision, precision, m, n, k, lda, ldb, ldc, alpha, beta, flags, prefetch);
}


LIBXSMM_API libxsmm_gemm_descriptor* libxsmm_gemm_descriptor_dinit2(libxsmm_descriptor_blob* blob,
  libxsmm_gemm_precision iprec, libxsmm_gemm_precision oprec, libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k,
  libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc, double alpha, double beta,
  int flags, int prefetch)
{
  /* avoid warning about potentially uninitialized variable (initialize outside of control flow) */
  libxsmm_gemm_descriptor* result = NULL;
  switch (iprec) {
    case LIBXSMM_GEMM_PRECISION_F64: {
      LIBXSMM_ASSERT(iprec == oprec);
      result = libxsmm_dgemm_descriptor_init(blob, m, n, k, lda, ldb, ldc,
        alpha, beta, flags, prefetch);
    } break;
    case LIBXSMM_GEMM_PRECISION_F32: {
      LIBXSMM_ASSERT(iprec == oprec);
      result = libxsmm_sgemm_descriptor_init(blob, m, n, k, lda, ldb, ldc,
        (float)alpha, (float)beta, flags, prefetch);
    } break;
    case LIBXSMM_GEMM_PRECISION_I16: {
      if (LIBXSMM_GEMM_PRECISION_I32 == oprec) {
        result = libxsmm_wigemm_descriptor_init(blob, m, n, k, lda, ldb, ldc,
          (int)alpha, (int)beta, flags, prefetch);
      }
      else {
        LIBXSMM_ASSERT(LIBXSMM_GEMM_PRECISION_F32 == oprec);
        result = libxsmm_wsgemm_descriptor_init(blob, m, n, k, lda, ldb, ldc,
          (float)alpha, (float)beta, flags, prefetch);
      }
    } break;
    case LIBXSMM_GEMM_PRECISION_BF16: {
      if (LIBXSMM_GEMM_PRECISION_F32 == oprec) {
        result = libxsmm_bsgemm_descriptor_init(blob, m, n, k, lda, ldb, ldc,
          (float)alpha, (float)beta, flags, prefetch);
      } else if (LIBXSMM_GEMM_PRECISION_BF16 == oprec) {
        result = libxsmm_bgemm_descriptor_init(blob, m, n, k, lda, ldb, ldc,
          (float)alpha, (float)beta, flags, prefetch);
      }
    } break;
    default: {
      static int error_once = 0;
      if (0 != libxsmm_verbosity /* library code is expected to be mute */
        && 1 == LIBXSMM_ATOMIC_ADD_FETCH(&error_once, 1, LIBXSMM_ATOMIC_RELAXED))
      {
        fprintf(stderr, "LIBXSMM ERROR: GEMM precision is not supported!\n");
      }
    }
  }
  return result;
}


LIBXSMM_API libxsmm_gemm_descriptor* libxsmm_gemm_descriptor_init(libxsmm_descriptor_blob* blob,
  libxsmm_gemm_precision precision, libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k,
  libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc, const void* alpha, const void* beta,
  int flags, int prefetch)
{
  return libxsmm_gemm_descriptor_init2(blob, precision, precision, m, n, k, lda, ldb, ldc, alpha, beta, flags, prefetch);
}


LIBXSMM_API libxsmm_gemm_descriptor* libxsmm_gemm_descriptor_init2(libxsmm_descriptor_blob* blob,
  libxsmm_gemm_precision iprec, libxsmm_gemm_precision oprec, libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k,
  libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc, const void* alpha, const void* beta,
  int flags, int prefetch)
{
  return libxsmm_gemm_descriptor_init3(blob, iprec, oprec, m, n, k, lda, ldb, ldc, alpha, beta, flags, prefetch,
    NULL/*dalpha*/, NULL/*dbeta*/);
}


LIBXSMM_API libxsmm_gemm_descriptor* libxsmm_gemm_descriptor_init3(libxsmm_descriptor_blob* blob,
  libxsmm_gemm_precision iprec, libxsmm_gemm_precision oprec, libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k,
  libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc, const void* alpha, const void* beta,
  int flags, int prefetch, double* dalpha, double* dbeta)
{
  /* avoid warning about potentially uninitialized variable (initialize outside of control flow) */
  libxsmm_gemm_descriptor* result = NULL;
  switch (iprec) {
    case LIBXSMM_GEMM_PRECISION_F64: {
      const double aa = (NULL != alpha ? *((const double*)alpha) : (LIBXSMM_ALPHA));
      const double bb = (NULL != beta  ? *((const double*)beta)  : (LIBXSMM_BETA));
      LIBXSMM_ASSERT(LIBXSMM_GEMM_PRECISION_F64 == oprec);
      result = libxsmm_dgemm_descriptor_init(blob, m, n, k, lda, ldb, ldc, aa, bb, flags, prefetch);
      if (NULL != dalpha) *dalpha = aa;
      if (NULL != dbeta) *dbeta = bb;
    } break;
    case LIBXSMM_GEMM_PRECISION_F32: {
      const float aa = (NULL != alpha ? *((const float*)alpha) : (LIBXSMM_ALPHA));
      const float bb = (NULL != beta  ? *((const float*)beta)  : (LIBXSMM_BETA));
      LIBXSMM_ASSERT(LIBXSMM_GEMM_PRECISION_F32 == oprec);
      result = libxsmm_sgemm_descriptor_init(blob, m, n, k, lda, ldb, ldc, aa, bb, flags, prefetch);
      if (NULL != dalpha) *dalpha = (double)aa;
      if (NULL != dbeta) *dbeta = (double)bb;
    } break;
    case LIBXSMM_GEMM_PRECISION_I16: {
      if (LIBXSMM_GEMM_PRECISION_I32 == oprec) {
        /**
         * Take alpha and beta as short data although wgemm works on integers.
         * However, alpha and beta are only JIT-supported for certain values,
         * and the call-side may not distinct different input and output types
         * (integer/short), hence it is safer to only read short data.
         */
        const short aa = (short)(NULL != alpha ? *((const short*)alpha) : (LIBXSMM_ALPHA));
        const short bb = (short)(NULL != beta  ? *((const short*)beta)  : (LIBXSMM_BETA));
        result = libxsmm_wigemm_descriptor_init(blob, m, n, k, lda, ldb, ldc, aa, bb, flags, prefetch);
        if (NULL != dalpha) *dalpha = (double)aa;
        if (NULL != dbeta) *dbeta = (double)bb;
      }
      else {
        const float aa = (NULL != alpha ? *((const float*)alpha) : (LIBXSMM_ALPHA));
        const float bb = (NULL != beta  ? *((const float*)beta)  : (LIBXSMM_BETA));
        LIBXSMM_ASSERT(LIBXSMM_GEMM_PRECISION_F32 == oprec);
        result = libxsmm_wsgemm_descriptor_init(blob, m, n, k, lda, ldb, ldc, aa, bb, flags, prefetch);
        if (NULL != dalpha) *dalpha = (double)aa;
        if (NULL != dbeta) *dbeta = (double)bb;
      }
    } break;
    case LIBXSMM_GEMM_PRECISION_BF16: {
      if (LIBXSMM_GEMM_PRECISION_F32 == oprec) {
        const float aa = (NULL != alpha ? *((const float*)alpha) : (LIBXSMM_ALPHA));
        const float bb = (NULL != beta  ? *((const float*)beta)  : (LIBXSMM_BETA));
        result = libxsmm_bsgemm_descriptor_init(blob, m, n, k, lda, ldb, ldc, aa, bb, flags, prefetch);
        if (NULL != dalpha) *dalpha = (double)aa;
        if (NULL != dbeta) *dbeta = (double)bb;
      } else if (LIBXSMM_GEMM_PRECISION_BF16 == oprec) {
        const float aa = (NULL != alpha ? *((const float*)alpha) : (LIBXSMM_ALPHA));
        const float bb = (NULL != beta  ? *((const float*)beta)  : (LIBXSMM_BETA));
        result = libxsmm_bgemm_descriptor_init(blob, m, n, k, lda, ldb, ldc, aa, bb, flags, prefetch);
        if (NULL != dalpha) *dalpha = (double)aa;
        if (NULL != dbeta) *dbeta = (double)bb;
      }
    } break;
    default: {
      static int error_once = 0;
      if (0 != libxsmm_verbosity /* library code is expected to be mute */
        && 1 == LIBXSMM_ATOMIC_ADD_FETCH(&error_once, 1, LIBXSMM_ATOMIC_RELAXED))
      {
        fprintf(stderr, "LIBXSMM ERROR: GEMM precision is not supported!\n");
      }
    }
  }
  return result;
}


LIBXSMM_API libxsmm_trans_descriptor* libxsmm_trans_descriptor_init(libxsmm_descriptor_blob* blob,
  unsigned int typesize, unsigned int m, unsigned int n, unsigned int ldo)
{
  union {
    libxsmm_trans_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  LIBXSMM_DESCRIPTOR_CLEAR(blob);
  result.blob = blob;
  result.ptr->typesize = (unsigned char)typesize;
  result.ptr->ldo = ldo;
  result.ptr->m = m;
  result.ptr->n = n;
  return result.ptr;
}


LIBXSMM_API libxsmm_mcopy_descriptor* libxsmm_mcopy_descriptor_init(libxsmm_descriptor_blob* blob,
  unsigned int typesize, unsigned int m, unsigned int n, unsigned int ldo,
  unsigned int ldi, int flags, int prefetch, const int* unroll)
{
  union {
    libxsmm_mcopy_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  if (0 == LIBXSMM_MOD2(typesize, 4)) { /* TODO: more general kernel */
    const unsigned int typescale = typesize / 4;
    LIBXSMM_DESCRIPTOR_CLEAR(blob);
    result.blob = blob;
    result.ptr->unroll_level = (unsigned char)((NULL == unroll || 0 >= *unroll) ? 2/*default*/ : LIBXSMM_MIN(*unroll, 64));
    result.ptr->typesize = (unsigned char)/*typesize*/4;
    result.ptr->prefetch = (unsigned char)prefetch;
    result.ptr->flags = (unsigned char)flags;
    result.ptr->ldi = ldi * typescale;
    result.ptr->ldo = ldo * typescale;
    result.ptr->m = m * typescale;
    result.ptr->n = n;
  }
  else {
    result.ptr = NULL;
  }
  return result.ptr;
}

LIBXSMM_API libxsmm_trsm_descriptor* libxsmm_trsm_descriptor_init(libxsmm_descriptor_blob* blob,
  unsigned int typesize, libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint lda, libxsmm_blasint ldb,
  const void* alpha, char transa, char diag, char side, char uplo, int layout)
{
  union {
    libxsmm_trsm_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  LIBXSMM_DESCRIPTOR_CLEAR(blob);
  result.blob = blob;
  result.ptr->typesize = (unsigned char)typesize;
  result.ptr->lda = (unsigned char)lda;
  result.ptr->ldb = (unsigned char)ldb;
  result.ptr->m = (unsigned char)m;
  result.ptr->n = (unsigned char)n;
  result.ptr->transa = transa;
  result.ptr->diag = diag;
  result.ptr->side = side;
  result.ptr->uplo = uplo;
  result.ptr->layout = (unsigned char)layout;
  switch (typesize) {
    case 4: {
      result.ptr->alpha.s = (NULL != alpha ? (*(const float*)alpha) : ((float)LIBXSMM_ALPHA));
    } break;
    case 8: {
      result.ptr->alpha.d = (NULL != alpha ? (*(const double*)alpha) : ((double)LIBXSMM_ALPHA));
    } break;
    default: /* TODO: generate warning */;
  }
  return result.ptr;
}


LIBXSMM_API libxsmm_trmm_descriptor* libxsmm_trmm_descriptor_init(libxsmm_descriptor_blob* blob,
  unsigned int typesize, libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint lda, libxsmm_blasint ldb,
  const void* alpha, char transa, char diag, char side, char uplo, int layout)
{
  union {
    libxsmm_trmm_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  LIBXSMM_DESCRIPTOR_CLEAR(blob);
  result.blob = blob;
  result.ptr->typesize = (unsigned char)typesize;
  result.ptr->lda = (unsigned char)lda;
  result.ptr->ldb = (unsigned char)ldb;
  result.ptr->m = (unsigned char)m;
  result.ptr->n = (unsigned char)n;
  result.ptr->transa = transa;
  result.ptr->diag = diag;
  result.ptr->side = side;
  result.ptr->uplo = uplo;
  result.ptr->layout = (unsigned char)layout;
  switch (typesize) {
    case 4: {
      result.ptr->alpha.s = (NULL != alpha ? (*(const float*)alpha) : ((float)LIBXSMM_ALPHA));
    } break;
    case 8: {
      result.ptr->alpha.d = (NULL != alpha ? (*(const double*)alpha) : ((double)LIBXSMM_ALPHA));
    } break;
    default: /* TODO: generate warning */;
  }
  return result.ptr;
}


LIBXSMM_API libxsmm_pgemm_descriptor* libxsmm_pgemm_descriptor_init(libxsmm_descriptor_blob* blob,
  unsigned int typesize, libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint k, libxsmm_blasint lda, libxsmm_blasint ldb, libxsmm_blasint ldc,
  const void* alpha, char transa, char transb, int layout)
{
  union {
    libxsmm_pgemm_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  LIBXSMM_DESCRIPTOR_CLEAR(blob);
  result.blob = blob;
  result.ptr->typesize = (unsigned char)typesize;
  result.ptr->lda = (unsigned char)lda;
  result.ptr->ldb = (unsigned char)ldb;
  result.ptr->ldc = (unsigned char)ldc;
  result.ptr->m = (unsigned char)m;
  result.ptr->n = (unsigned char)n;
  result.ptr->k = (unsigned char)k;
  result.ptr->transa = transa;
  result.ptr->transb = transb;
  result.ptr->layout = (unsigned char)layout;
  if ( typesize == 4 ) {
    float *alpha_val = (float*)alpha;
    if ( *alpha_val == 1.0 ) result.ptr->alpha_val = 0;
    else if ( *alpha_val == -1.0 ) result.ptr->alpha_val = 1;
    else {
      printf("Warning: real*4 alpha value should be 1.0 or -1.0\n");
      exit(-1);
    }
  } else {
    double *alpha_val = (double*)alpha;
    if ( *alpha_val == 1.0 ) result.ptr->alpha_val = 0;
    else if ( *alpha_val == -1.0 ) result.ptr->alpha_val = 1;
    else {
      printf("Warning: real*8 alpha value should be 1.0 or -1.0\n");
      exit(-1);
    }
  }
  return result.ptr;
}


LIBXSMM_API libxsmm_getrf_descriptor* libxsmm_getrf_descriptor_init(libxsmm_descriptor_blob* blob,
  unsigned int typesize, libxsmm_blasint m, libxsmm_blasint n, libxsmm_blasint lda, int layout)
{
  union {
    libxsmm_getrf_descriptor* ptr;
    libxsmm_descriptor_blob* blob;
  } result;
  LIBXSMM_DESCRIPTOR_CLEAR(blob);
  result.blob = blob;
  result.ptr->typesize = (unsigned char)typesize;
  result.ptr->lda = (unsigned char)lda;
  result.ptr->m = (unsigned char)m;
  result.ptr->n = (unsigned char)n;
  result.ptr->layout = (unsigned char)layout;
  return result.ptr;
}


LIBXSMM_API size_t libxsmm_gcd(size_t a, size_t b)
{
  while (0 != b) {
    const size_t r = a % b;
    a = b; b = r;
  }
  return 0 != a ? a : 1;
}


LIBXSMM_API size_t libxsmm_lcm(size_t a, size_t b)
{
  const size_t gcd = libxsmm_gcd(a, b);
  return 0 != gcd ? ((a / gcd) * b) : 0;
}


LIBXSMM_API int libxsmm_primes_u32(unsigned int num, unsigned int num_factors_n32[])
{
  unsigned int c = num, i;
  int n = 0;
  if (0 < c && 0 == (c & 1)) { /* non-zero even */
    unsigned int j = c / 2;
    while (c == (2 * j)) {
      num_factors_n32[n++] = 2;
      c = j; j /= 2;
    }
  }
  for (i = 3; i <= c; i += 2) {
    unsigned int j = c / i;
    while (c == (i * j)) {
      num_factors_n32[n++] = i;
      c = j; j /= i;
    }
    if ((i * i) > num) {
      break;
    }
  }
  if (1 < c && 0 != n) {
    num_factors_n32[n++] = c;
  }
  return n;
}


LIBXSMM_API_INLINE unsigned int internal_product_limit(unsigned int product, unsigned int limit)
{
  unsigned int fact[32], maxp = limit, result = 1;
  int i, n;
  /* attempt to lower the memory requirement for DP; can miss best solution */
  if (LIBXSMM_PRODUCT_LIMIT < limit) {
    const unsigned int minfct = (limit + limit - 1) / LIBXSMM_PRODUCT_LIMIT;
    const unsigned int maxfct = (unsigned int)libxsmm_gcd(product, limit);
    result = maxfct;
    if (minfct < maxfct) {
      n = libxsmm_primes_u32(result, fact);
      for (i = 0; i < n; ++i) {
        if (minfct < fact[i]) {
          result = fact[i];
          break;
        }
      }
    }
    maxp /= result;
  }
  if (LIBXSMM_PRODUCT_LIMIT >= maxp) {
    unsigned int k[2][LIBXSMM_PRODUCT_LIMIT], *k0 = k[0], *k1 = k[1], *kt, p;
    n = libxsmm_primes_u32(product / result, fact);
    /* initialize table with trivial factor */
    for (p = 0; p <= maxp; ++p) k[0][p] = 1;
    k[0][0] = k[1][0] = 1;
    for (i = 1; i <= n; ++i) {
      for (p = 1; p <= maxp; ++p) {
        const unsigned int f = fact[i - 1], h = k0[p];
        if (p < f) {
          k1[p] = h;
        }
        else {
          const unsigned int g = f * k0[p / f];
          k1[p] = LIBXSMM_MAX(g, h);
        }
      }
      kt = k0; k0 = k1; k1 = kt;
    }
    result *= k0[maxp];
  }
  else { /* trivial approximation */
    n = libxsmm_primes_u32(product, fact);
    for (i = 0; i < n; ++i) {
      const unsigned int f = result * fact[i];
      if (f <= limit) {
        result = f;
      }
      else break;
    }
  }
  return result;
}


LIBXSMM_API unsigned int libxsmm_product_limit(unsigned int product, unsigned int limit, int is_lower)
{
  unsigned int result;
  if (1 < limit) { /* check for fast-path */
    result = internal_product_limit(product, limit);
  }
  else {
    result = limit;
  }
  if (0 != is_lower && limit < product) {
    if (result < limit) {
      result = internal_product_limit(product, 2 * limit - 1);
    }
    if (result < limit) {
      result = product;
    }
    LIBXSMM_ASSERT(limit <= result);
  }
  if (product < result) {
    result = product;
  }
  LIBXSMM_ASSERT(result <= product);
  return result;
}

