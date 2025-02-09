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
#ifndef LIBXSMM_MATH_H
#define LIBXSMM_MATH_H

#include "libxsmm_typedefs.h"


/**
 * Structure of differences with matrix norms according
 * to http://www.netlib.org/lapack/lug/node75.html).
 */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_matdiff_info {
  /** One-norm */         double norm1_abs, norm1_rel;
  /** Infinity-norm */    double normi_abs, normi_rel;
  /** Froebenius-norm */  double normf_rel;
  /** Maximum difference, and L2-norm (both absolute and relative). */
  double linf_abs, linf_rel, l2_abs, l2_rel;
  /** Statistics: sum/l1, min., max., arith. avg., and variance. */
  double l1_ref, min_ref, max_ref, avg_ref, var_ref;
  /** Statistics: sum/l1, min., max., arith. avg., and variance. */
  double l1_tst, min_tst, max_tst, avg_tst, var_tst;
  /** Location (m, n) of largest difference (linf_abs). */
  libxsmm_blasint m, n;
} libxsmm_matdiff_info;

/**
 * Utility function to calculate a collection of scalar differences between two matrices (libxsmm_matdiff_info).
 * The location (m, n) of the largest difference (linf_abs) is recorded (also in case of NaN). In case of NaN,
 * differences are set to infinity. If no difference is discovered, the location (m, n) is negative (OOB).
 */
LIBXSMM_API int libxsmm_matdiff(libxsmm_matdiff_info* info,
  libxsmm_datatype datatype, libxsmm_blasint m, libxsmm_blasint n, const void* ref, const void* tst,
  const libxsmm_blasint* ldref, const libxsmm_blasint* ldtst);

/**
 * Reduces input into output such that the difference is maintained or increased (max function).
 * The very first (initial) output should be zeroed (libxsmm_matdiff_clear).
 */
LIBXSMM_API void libxsmm_matdiff_reduce(libxsmm_matdiff_info* output, const libxsmm_matdiff_info* input);
/** Clears the given info-structure e.g., for the initial reduction-value (libxsmm_matdiff_reduce). */
LIBXSMM_API void libxsmm_matdiff_clear(libxsmm_matdiff_info* info);

/** Greatest common divisor (corner case: the GCD of 0 and 0 is 1). */
LIBXSMM_API size_t libxsmm_gcd(size_t a, size_t b);
/** Least common multiple. */
LIBXSMM_API size_t libxsmm_lcm(size_t a, size_t b);

/**
 * This function finds prime-factors (up to 32) of an unsigned integer in ascending order, and
 * returns the number of factors found (zero if the given number is prime and unequal to two).
 */
LIBXSMM_API int libxsmm_primes_u32(unsigned int num, unsigned int num_factors_n32[]);

/** Calculate co-prime number <= n/2 (except: libxsmm_shuffle(0|1) == 0). */
LIBXSMM_API size_t libxsmm_shuffle(unsigned int n);

/**
 * Divides the product into prime factors and selects factors such that the new product is within
 * the given limit (0/1-Knapsack problem) e.g., product=12=2*2*3 and limit=6 then result=2*3=6.
 * The limit is at least reached or exceeded with the minimal possible product (is_lower=true).
 */
LIBXSMM_API unsigned int libxsmm_product_limit(unsigned int product, unsigned int limit, int is_lower);

/** SQRT with Newton's method using integer arithmetic. */
LIBXSMM_API unsigned int libxsmm_isqrt_u64(unsigned long long x);
/** SQRT with Newton's method using integer arithmetic. */
LIBXSMM_API unsigned int libxsmm_isqrt_u32(unsigned int x);
/** Based on libxsmm_isqrt_u32, but actual factor of x. */
LIBXSMM_API unsigned int libxsmm_isqrt2_u32(unsigned int x);
/** SQRT with Newton's method using double-precision. */
LIBXSMM_API double libxsmm_dsqrt(double x);
/** SQRT with Newton's method using single-precision. */
LIBXSMM_API float libxsmm_ssqrt(float x);

/** CBRT with Newton's method using integer arithmetic. */
LIBXSMM_API unsigned int libxsmm_icbrt_u64(unsigned long long x);
/** CBRT with Newton's method using integer arithmetic. */
LIBXSMM_API unsigned int libxsmm_icbrt_u32(unsigned int x);

/** Single-precision approximation of exponential function (base 2). */
LIBXSMM_API float libxsmm_sexp2(float x);

/**
 * Exponential function (base 2), which is limited to unsigned 8-bit input values.
 * This function reproduces bit-accurate results (single-precision).
 */
LIBXSMM_API float libxsmm_sexp2_u8(unsigned char x);

/**
* Exponential function (base 2), which is limited to signed 8-bit input values.
* This function reproduces bit-accurate results (single-precision).
*/
LIBXSMM_API float libxsmm_sexp2_i8(signed char x);

/** Similar to libxsmm_sexp2_i8, but takes an integer as signed 8-bit value (check). */
LIBXSMM_API float libxsmm_sexp2_i8i(int x);

#endif /*LIBXSMM_MATH_H*/

