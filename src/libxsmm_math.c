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
#include <libxsmm_mhd.h>
#include "libxsmm_main.h"

#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(push,target(LIBXSMM_OFFLOAD_TARGET))
#endif
#if !defined(LIBXSMM_NO_LIBM)
# include <math.h>
#endif
#include <string.h>
#include <stdio.h>
#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(pop)
#endif


LIBXSMM_API int libxsmm_matdiff(libxsmm_matdiff_info* info,
  libxsmm_datatype datatype, libxsmm_blasint m, libxsmm_blasint n, const void* ref, const void* tst,
  const libxsmm_blasint* ldref, const libxsmm_blasint* ldtst)
{
  int result = EXIT_SUCCESS, result_swap = 0, result_nan = 0;
  libxsmm_blasint ldr = (NULL == ldref ? m : *ldref), ldt = (NULL == ldtst ? m : *ldtst);
  if (NULL == ref && NULL != tst) { ref = tst; tst = NULL; result_swap = 1; }
  if (NULL != ref && NULL != info && m <= ldr && m <= ldt) {
    libxsmm_blasint mm = m, nn = n;
    double inf;
    if (1 == n) { mm = ldr = ldt = 1; nn = m; } /* ensure row-vector shape to standardize results */
    libxsmm_matdiff_clear(info);
    inf = info->min_ref;
    switch (datatype) {
      case LIBXSMM_DATATYPE_F64: {
#       define LIBXSMM_MATDIFF_TEMPLATE_ELEM_TYPE double
#       include "template/libxsmm_matdiff.tpl.c"
#       undef  LIBXSMM_MATDIFF_TEMPLATE_ELEM_TYPE
      } break;
      case LIBXSMM_DATATYPE_F32: {
#       define LIBXSMM_MATDIFF_TEMPLATE_ELEM_TYPE float
#       include "template/libxsmm_matdiff.tpl.c"
#       undef  LIBXSMM_MATDIFF_TEMPLATE_ELEM_TYPE
      } break;
      case LIBXSMM_DATATYPE_I32: {
#       define LIBXSMM_MATDIFF_TEMPLATE_ELEM_TYPE int
#       include "template/libxsmm_matdiff.tpl.c"
#       undef  LIBXSMM_MATDIFF_TEMPLATE_ELEM_TYPE
      } break;
      case LIBXSMM_DATATYPE_I16: {
#       define LIBXSMM_MATDIFF_TEMPLATE_ELEM_TYPE short
#       include "template/libxsmm_matdiff.tpl.c"
#       undef  LIBXSMM_MATDIFF_TEMPLATE_ELEM_TYPE
      } break;
      case LIBXSMM_DATATYPE_I8: {
#       define LIBXSMM_MATDIFF_TEMPLATE_ELEM_TYPE signed char
#       include "template/libxsmm_matdiff.tpl.c"
#       undef  LIBXSMM_MATDIFF_TEMPLATE_ELEM_TYPE
      } break;
      default: {
        static int error_once = 0;
        if (0 != libxsmm_verbosity /* library code is expected to be mute */
          && 1 == LIBXSMM_ATOMIC_ADD_FETCH(&error_once, 1, LIBXSMM_ATOMIC_RELAXED))
        {
          fprintf(stderr, "LIBXSMM ERROR: unsupported data-type requested!\n");
        }
        result = EXIT_FAILURE;
      }
    }
    LIBXSMM_ASSERT((0 <= info->m && 0 <= info->n) || (0 > info->m && 0 > info->n));
    LIBXSMM_ASSERT(info->m < mm && info->n < nn);
    if (EXIT_SUCCESS == result) {
      const char *const env = getenv("LIBXSMM_DUMP");
      LIBXSMM_INIT
      if (NULL != env && 0 != *env && '0' != *env) {
        if ('-' != *env || (0 <= info->m && 0 <= info->n)) {
          const char *const defaultname = (('0' < *env && '9' >= *env) || '-' == *env) ? "libxsmm_dump" : env;
          const libxsmm_mhd_elemtype type_src = (libxsmm_mhd_elemtype)datatype;
          const libxsmm_mhd_elemtype type_dst = LIBXSMM_MIN(LIBXSMM_MHD_ELEMTYPE_F32, type_src);
          const int envi = atoi(env), reshape = (1 < envi || -1 > envi);
          size_t shape[2], size[2];
          char filename[256];
          if (0 == reshape) {
            shape[0] = (size_t)mm; shape[1] = (size_t)nn;
            size[0] = (size_t)ldr; size[1] = (size_t)nn;
          }
          else { /* reshape */
            const size_t x = (size_t)mm * (size_t)nn;
            const size_t y = (size_t)libxsmm_isqrt2_u32((unsigned int)x);
            shape[0] = x / y; shape[1] = y;
            size[0] = shape[0];
            size[1] = shape[1];
          }
          LIBXSMM_SNPRINTF(filename, sizeof(filename), "%s-%p-ref.mhd", defaultname, ref);
          libxsmm_mhd_write(filename, NULL/*offset*/, shape, size, 2/*ndims*/, 1/*ncomponents*/,
            type_src, &type_dst, ref, NULL/*header_size*/, NULL/*extension_header*/,
            NULL/*extension*/, 0/*extension_size*/);
          if (NULL != tst) {
            if (0 == reshape) {
              size[0] = (size_t)ldt;
              size[1] = (size_t)nn;
            }
            LIBXSMM_SNPRINTF(filename, sizeof(filename), "%s-%p-tst.mhd", defaultname, ref/*adopt ref-ptr*/);
            libxsmm_mhd_write(filename, NULL/*offset*/, shape, size, 2/*ndims*/, 1/*ncomponents*/,
              type_src, &type_dst, tst, NULL/*header_size*/, NULL/*extension_header*/,
              NULL/*extension*/, 0/*extension_size*/);
            if ('-' == *env && '1' < env[1]) {
              printf("LIBXSMM MATDIFF (%s): m=%lli n=%lli ldi=%lli ldo=%lli failed.\n",
                libxsmm_typename(datatype), (long long)m, (long long)n, (long long)ldr, (long long)ldt);
            }
          }
        }
        else if ('-' == *env && '1' < env[1] && NULL != tst) {
          printf("LIBXSMM MATDIFF (%s): m=%lli n=%lli ldi=%lli ldo=%lli passed.\n",
            libxsmm_typename(datatype), (long long)m, (long long)n, (long long)ldr, (long long)ldt);
        }
      }
      if (0 == result_nan) {
        info->normf_rel = libxsmm_dsqrt(info->normf_rel);
        info->l2_abs = libxsmm_dsqrt(info->l2_abs);
        info->l2_rel = libxsmm_dsqrt(info->l2_rel);
      }
      else if (1 == result_nan) {
        /* in case of NaN in test-set, statistics is not set to inf (ref/test) */
        info->norm1_abs = info->norm1_rel = info->normi_abs = info->normi_rel = info->normf_rel
                        = info->linf_abs = info->linf_rel = info->l2_abs = info->l2_rel
                        = inf;
      }
      if (1 == n) {
        const libxsmm_blasint tmp = info->m;
        info->m = info->n;
        info->n = tmp;
      }
      if (0 != result_swap) {
        info->min_tst = info->min_ref;
        info->min_ref = 0;
        info->max_tst = info->max_ref;
        info->max_ref = 0;
        info->avg_tst = info->avg_ref;
        info->avg_ref = 0;
        info->var_tst = info->var_ref;
        info->var_ref = 0;
        info->l1_tst = info->l1_ref;
        info->l1_ref = 0;
      }
    }
  }
  else {
    result = EXIT_FAILURE;
  }
  return result;
}


LIBXSMM_API void libxsmm_matdiff_reduce(libxsmm_matdiff_info* output, const libxsmm_matdiff_info* input)
{
  LIBXSMM_ASSERT(NULL != output && NULL != input);
  if (output->linf_abs < input->linf_abs) {
    output->linf_abs = input->linf_abs;
    LIBXSMM_ASSERT(0 <= input->m);
    output->m = input->m;
    LIBXSMM_ASSERT(0 <= input->n);
    output->n = input->n;
  }
  if (output->norm1_abs < input->norm1_abs) {
    output->norm1_abs = input->norm1_abs;
  }
  if (output->norm1_rel < input->norm1_rel) {
    output->norm1_rel = input->norm1_rel;
  }
  if (output->normi_abs < input->normi_abs) {
    output->normi_abs = input->normi_abs;
  }
  if (output->normi_rel < input->normi_rel) {
    output->normi_rel = input->normi_rel;
  }
  if (output->normf_rel < input->normf_rel) {
    output->normf_rel = input->normf_rel;
  }
  if (output->linf_rel < input->linf_rel) {
    output->linf_rel = input->linf_rel;
  }
  if (output->l2_abs < input->l2_abs) {
    output->l2_abs = input->l2_abs;
  }
  if (output->l2_rel < input->l2_rel) {
    output->l2_rel = input->l2_rel;
  }
  if (output->var_ref < input->var_ref) {
    output->var_ref = input->var_ref;
  }
  if (output->var_tst < input->var_tst) {
    output->var_tst = input->var_tst;
  }
  if (output->max_ref < input->max_ref) {
    output->max_ref = input->max_ref;
  }
  if (output->max_tst < input->max_tst) {
    output->max_tst = input->max_tst;
  }
  if (output->min_ref > input->min_ref) {
    output->min_ref = input->min_ref;
  }
  if (output->min_tst > input->min_tst) {
    output->min_tst = input->min_tst;
  }
  output->avg_ref = 0.5 * (output->avg_ref + input->avg_ref);
  output->avg_tst = 0.5 * (output->avg_tst + input->avg_tst);
  output->l1_ref += input->l1_ref;
  output->l1_tst += input->l1_tst;
}


LIBXSMM_API void libxsmm_matdiff_clear(libxsmm_matdiff_info* info)
{
  if (NULL != info) {
    union { int raw; float value; } inf;
#if defined(INFINITY) && /*overflow warning*/!defined(_CRAYC)
    inf.value = (float)(INFINITY);
#else
    inf.raw = 0x7F800000;
#endif
    memset(info, 0, sizeof(*info)); /* nullify */
    /* no location discovered yet with a difference */
    info->m = info->n = -1;
    /* initial minimum/maximum of reference/test */
    info->min_ref = info->min_tst = +inf.value;
    info->max_ref = info->max_tst = -inf.value;
  }
}


LIBXSMM_API size_t libxsmm_shuffle(unsigned int n)
{
  const unsigned int s = (0 != (n & 1) ? ((n / 2 - 1) | 1) : ((n / 2) & ~1));
  const unsigned int d = (0 != (n & 1) ? 1 : 2);
  unsigned int result = (1 < n ? 1 : 0), i;
  for (i = (d < n ? (n - 1) : 0); d < i; i -= d) {
    unsigned int c = (s <= i ? (i - s) : (s - i));
    unsigned int a = n, b = c;
    do {
      const unsigned int r = a % b;
      a = b;
      b = r;
    } while (0 != b);
    if (1 == a) {
      result = c;
      if (2 * c <= n) {
        i = d; /* break */
      }
    }
  }
  assert((0 == result && 1 >= n) || (result < n && 1 == libxsmm_gcd(result, n)));
  return result;
}


LIBXSMM_API unsigned int libxsmm_isqrt_u64(unsigned long long x)
{
  unsigned long long b; unsigned int y = 0, s;
  for (s = 0x80000000/*2^31*/; 0 < s; s >>= 1) {
    b = y | s; y |= (b * b <= x ? s : 0);
  }
  return y;
}


LIBXSMM_API unsigned int libxsmm_isqrt_u32(unsigned int x)
{
  unsigned int b; unsigned int y = 0; int s;
  for (s = 0x40000000/*2^30*/; 0 < s; s >>= 2) {
    b = y | s; y >>= 1;
    if (b <= x) { x -= b; y |= s; }
  }
  return y;
}


LIBXSMM_API unsigned int libxsmm_isqrt2_u32(unsigned int x)
{
  return libxsmm_product_limit(x, libxsmm_isqrt_u32(x), 0/*is_lower*/);
}


LIBXSMM_API LIBXSMM_INTRINSICS(LIBXSMM_X86_GENERIC) double libxsmm_dsqrt(double x)
{
#if defined(LIBXSMM_INTRINSICS_X86) && !defined(__PGI)
  const __m128d a = LIBXSMM_INTRINSICS_MM_UNDEFINED_PD();
  const double result = _mm_cvtsd_f64(_mm_sqrt_sd(a, _mm_set_sd(x)));
#elif !defined(LIBXSMM_NO_LIBM)
  const double result = sqrt(x);
#else /* fall-back */
  double result, y = x;
  if (LIBXSMM_NEQ(0, x)) {
    do {
      result = y;
      y = 0.5 * (y + x / y);
    } while (LIBXSMM_NEQ(result, y));
  }
  result = y;
#endif
  return result;
}


LIBXSMM_API LIBXSMM_INTRINSICS(LIBXSMM_X86_GENERIC) float libxsmm_ssqrt(float x)
{
#if defined(LIBXSMM_INTRINSICS_X86)
  const float result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
#elif !defined(LIBXSMM_NO_LIBM)
  const float result = LIBXSMM_SQRTF(x);
#else /* fall-back */
  float result, y = x;
  if (LIBXSMM_NEQ(0, x)) {
    do {
      result = y;
      y = 0.5f * (y + x / y);
    } while (LIBXSMM_NEQ(result, y));
  }
  result = y;
#endif
  return result;
}


LIBXSMM_API unsigned int libxsmm_icbrt_u64(unsigned long long x)
{
  unsigned long long b; unsigned int y = 0; int s;
  for (s = 63; 0 <= s; s -= 3) {
    y += y; b = ((unsigned long long)y + 1) * 3 * y + 1ULL;
    if (b <= (x >> s)) { x -= b << s; ++y; }
  }
  return y;
}


LIBXSMM_API unsigned int libxsmm_icbrt_u32(unsigned int x)
{
  unsigned int b; unsigned int y = 0; int s;
  for (s = 30; 0 <= s; s -= 3) {
    y += y; b = 3 * y * (y + 1) + 1;
    if (b <= (x >> s)) { x -= b << s; ++y; }
  }
  return y;
}

#if defined(LIBXSMM_NO_LIBM)
/* Implementation based on Claude Baumann's product (http://www.convict.lu/Jeunes/ultimate_stuff/exp_ln_2.htm).
 * Exponential function, which exposes the number of iterations taken in the main case (1...22).
 */
LIBXSMM_API_INLINE float internal_math_sexp2(float x, int maxiter)
{
  static const float lut[] = { /* tabulated powf(2.f, powf(2.f, -index)) */
    2.00000000f, 1.41421354f, 1.18920708f, 1.09050775f, 1.04427373f, 1.02189720f, 1.01088929f, 1.00542986f,
    1.00271130f, 1.00135469f, 1.00067711f, 1.00033855f, 1.00016928f, 1.00008464f, 1.00004232f, 1.00002110f,
    1.00001061f, 1.00000525f, 1.00000262f, 1.00000131f, 1.00000072f, 1.00000036f, 1.00000012f
  };
  const int lut_size = sizeof(lut) / sizeof(*lut), lut_size1 = lut_size - 1;
  int sign, temp, unbiased, exponent, mantissa;
  union { int i; float s; } result;

  result.s = x;
  sign = (0 == (result.i & 0x80000000) ? 0 : 1);
  temp = result.i & 0x7FFFFFFF; /* clear sign */
  unbiased = (temp >> 23) - 127; /* exponent */
  exponent = -unbiased;
  mantissa = (temp << 8) | 0x80000000;

  if (lut_size1 >= exponent) {
    if (lut_size1 != exponent) { /* multiple lookups needed */
      if (7 >= unbiased) { /* not a degenerated case */
        const int n = (0 >= maxiter || lut_size1 <= maxiter) ? lut_size1 : maxiter;
        int i = 1;
        if (0 > unbiased) { /* regular/main case */
          LIBXSMM_ASSERT(0 <= exponent && exponent < lut_size);
          result.s = lut[exponent]; /* initial value */
          i = exponent + 1; /* next LUT offset */
        }
        else {
          result.s = 2.f; /* lut[0] */
          i = 1; /* next LUT offset */
        }
        for (; i <= n && 0 != mantissa; ++i) {
          mantissa <<= 1;
          if (0 != (mantissa & 0x80000000)) { /* check MSB */
            LIBXSMM_ASSERT(0 <= i && i < lut_size);
            result.s *= lut[i]; /* TODO: normalized multiply */
          }
        }
        for (i = 0; i < unbiased; ++i) { /* compute squares */
          result.s *= result.s;
        }
        if (0 != sign) { /* negative value, so reciprocal */
          result.s = 1.f / result.s;
        }
      }
      else { /* out of range */
#if defined(INFINITY) && /*overflow warning*/!defined(_CRAYC)
        result.s = (0 == sign ? ((float)(INFINITY)) : 0.f);
#else
        result.i = (0 == sign ? 0x7F800000 : 0);
#endif
      }
    }
    else if (0 == sign) {
      result.s = lut[lut_size1];
    }
    else { /* reciprocal */
      result.s = 1.f / lut[lut_size1];
    }
  }
  else {
    result.s = 1.f; /* case 2^0 */
  }
  return result.s;
}
#endif


LIBXSMM_API float libxsmm_sexp2(float x)
{
#if !defined(LIBXSMM_NO_LIBM)
  return LIBXSMM_EXP2F(x);
#else /* fall-back */
  return internal_math_sexp2(x, 20/*compromise*/);
#endif
}


LIBXSMM_API float libxsmm_sexp2_u8(unsigned char x)
{
  union { int i; float s; } result;
  if (128 > x) {
    if (31 < x) {
      const float r32 = 2.f * ((float)(1U << 31)); /* 2^32 */
      const int n = x >> 5;
      int i;
      result.s = r32;
      for (i = 1; i < n; ++i) result.s *= r32;
      result.s *= (1U << (x - (n << 5)));
    }
    else {
      result.s = (float)(1U << x);
    }
  }
  else {
#if defined(INFINITY) && /*overflow warning*/!defined(_CRAYC)
    result.s = (float)(INFINITY);
#else
    result.i = 0x7F800000;
#endif
  }
  return result.s;
}


LIBXSMM_API float libxsmm_sexp2_i8(signed char x)
{
  union { int i; float s; } result;
  if (-128 != x) {
    const signed char ux = (signed char)LIBXSMM_ABS(x);
    if (31 < ux) {
      const float r32 = 2.f * ((float)(1U << 31)); /* 2^32 */
      const int n = ux >> 5;
      int i;
      result.s = r32;
      for (i = 1; i < n; ++i) result.s *= r32;
      result.s *= (1U << (ux - (n << 5)));
    }
    else {
      result.s = (float)(1U << ux);
    }
    if (ux != x) { /* signed */
      result.s = 1.f / result.s;
    }
  }
  else {
    result.i = 0x200000;
  }
  return result.s;
}


LIBXSMM_API float libxsmm_sexp2_i8i(int x)
{
  LIBXSMM_ASSERT(-128 <= x && x <= 127);
  return libxsmm_sexp2_i8((signed char)x);
}


#if defined(LIBXSMM_BUILD) && (!defined(LIBXSMM_NOFORTRAN) || defined(__clang_analyzer__))

/* implementation provided for Fortran 77 compatibility */
LIBXSMM_API void LIBXSMM_FSYMBOL(libxsmm_matdiff)(libxsmm_matdiff_info* /*info*/,
  const int* /*datatype*/, const libxsmm_blasint* /*m*/, const libxsmm_blasint* /*n*/, const void* /*ref*/, const void* /*tst*/,
  const libxsmm_blasint* /*ldref*/, const libxsmm_blasint* /*ldtst*/);
LIBXSMM_API void LIBXSMM_FSYMBOL(libxsmm_matdiff)(libxsmm_matdiff_info* info,
  const int* datatype, const libxsmm_blasint* m, const libxsmm_blasint* n, const void* ref, const void* tst,
  const libxsmm_blasint* ldref, const libxsmm_blasint* ldtst)
{
  static int error_once = 0;
  if ((NULL == datatype || LIBXSMM_DATATYPE_UNSUPPORTED <= *datatype || 0 > *datatype || NULL == m
    || EXIT_SUCCESS != libxsmm_matdiff(info, (libxsmm_datatype)*datatype, *m, *(NULL != n ? n : m), ref, tst, ldref, ldtst))
    && 0 != libxsmm_verbosity && 1 == LIBXSMM_ATOMIC_ADD_FETCH(&error_once, 1, LIBXSMM_ATOMIC_RELAXED))
  {
    fprintf(stderr, "LIBXSMM ERROR: invalid arguments for libxsmm_matdiff specified!\n");
  }
}


/* implementation provided for Fortran 77 compatibility */
LIBXSMM_API void LIBXSMM_FSYMBOL(libxsmm_matdiff_reduce)(libxsmm_matdiff_info* /*output*/, const libxsmm_matdiff_info* /*input*/);
LIBXSMM_API void LIBXSMM_FSYMBOL(libxsmm_matdiff_reduce)(libxsmm_matdiff_info* output, const libxsmm_matdiff_info* input)
{
  libxsmm_matdiff_reduce(output, input);
}


/* implementation provided for Fortran 77 compatibility */
LIBXSMM_API void LIBXSMM_FSYMBOL(libxsmm_matdiff_clear)(libxsmm_matdiff_info* /*info*/);
LIBXSMM_API void LIBXSMM_FSYMBOL(libxsmm_matdiff_clear)(libxsmm_matdiff_info* info)
{
  libxsmm_matdiff_clear(info);
}


/* implementation provided for Fortran 77 compatibility */
LIBXSMM_API void LIBXSMM_FSYMBOL(libxsmm_shuffle)(long long* /*coprime*/, const int* /*n*/);
LIBXSMM_API void LIBXSMM_FSYMBOL(libxsmm_shuffle)(long long* coprime, const int* n)
{
#if !defined(NDEBUG)
  static int error_once = 0;
  if (NULL != coprime && NULL != n && 0 <= *n)
#endif
  {
    *coprime = (long long)(libxsmm_shuffle((unsigned int)(*n)) & 0x7FFFFFFF);
  }
#if !defined(NDEBUG)
  else if (0 != libxsmm_verbosity /* library code is expected to be mute */
    && 1 == LIBXSMM_ATOMIC_ADD_FETCH(&error_once, 1, LIBXSMM_ATOMIC_RELAXED))
  {
    fprintf(stderr, "LIBXSMM ERROR: invalid arguments for libxsmm_shuffle specified!\n");
  }
#endif
}

#endif /*defined(LIBXSMM_BUILD) && (!defined(LIBXSMM_NOFORTRAN) || defined(__clang_analyzer__))*/

