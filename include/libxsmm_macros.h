/******************************************************************************
** Copyright (c) 2013-2019, Intel Corporation                                **
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
#ifndef LIBXSMM_MACROS_H
#define LIBXSMM_MACROS_H

#include "libxsmm_config.h"

/** Parameters the library was built for. */
#define LIBXSMM_CACHELINE LIBXSMM_CONFIG_CACHELINE
#define LIBXSMM_ALIGNMENT LIBXSMM_CONFIG_ALIGNMENT
#define LIBXSMM_ILP64 LIBXSMM_CONFIG_ILP64
#define LIBXSMM_SYNC LIBXSMM_CONFIG_SYNC
#define LIBXSMM_JIT LIBXSMM_CONFIG_JIT

/** Parameters of GEMM domain (static kernels, etc). */
#define LIBXSMM_PREFETCH LIBXSMM_CONFIG_PREFETCH
#define LIBXSMM_MAX_MNK LIBXSMM_CONFIG_MAX_MNK
#define LIBXSMM_MAX_DIM LIBXSMM_CONFIG_MAX_DIM
#define LIBXSMM_MAX_M LIBXSMM_CONFIG_MAX_M
#define LIBXSMM_MAX_N LIBXSMM_CONFIG_MAX_N
#define LIBXSMM_MAX_K LIBXSMM_CONFIG_MAX_K
#define LIBXSMM_FLAGS LIBXSMM_CONFIG_FLAGS
#define LIBXSMM_ALPHA LIBXSMM_CONFIG_ALPHA
#define LIBXSMM_BETA LIBXSMM_CONFIG_BETA

#if !defined(LIBXSMM_PLATFORM_SUPPORTED)
# if  (defined(__x86_64__) && 0 != (__x86_64__)) || \
      (defined(__amd64__) && 0 != (__amd64__)) || \
      (defined(_M_X64) || defined(_M_AMD64)) || \
      (defined(__i386__) && 0 != (__i386__)) || \
      (defined(_M_IX86))
#   define LIBXSMM_PLATFORM_SUPPORTED
# else /* JIT-generated code (among other issues) is not supported! */
#   error Intel Architecture or compatible CPU required!
# endif
#endif
#if !defined(LIBXSMM_BITS)
# if  (defined(__SIZEOF_PTRDIFF_T__) && 4 < (__SIZEOF_PTRDIFF_T__)) || \
      (defined(__SIZE_MAX__) && (4294967295U < (__SIZE_MAX__))) || \
      (defined(__x86_64__) && 0 != (__x86_64__)) || \
      (defined(__amd64__) && 0 != (__amd64__)) || \
      (defined(_M_X64) || defined(_M_AMD64)) || \
      (defined(_WIN64))
#   define LIBXSMM_BITS 64
# elif defined(NDEBUG) /* not for production use! */
#   error LIBXSMM is only supported on a 64-bit platform!
# else /* JIT-generated code (among other issues) is not supported! */
#   define LIBXSMM_BITS 32
# endif
#endif

#define LIBXSMM_STRINGIFY2(SYMBOL) #SYMBOL
#define LIBXSMM_STRINGIFY(SYMBOL) LIBXSMM_STRINGIFY2(SYMBOL)
#define LIBXSMM_TOSTRING(SYMBOL) LIBXSMM_STRINGIFY(SYMBOL)
#define LIBXSMM_CONCATENATE1(A, B) A##B
#define LIBXSMM_CONCATENATE(A, B) LIBXSMM_CONCATENATE1(A, B)
#define LIBXSMM_CONCATENATE2(A, B, C) LIBXSMM_CONCATENATE(LIBXSMM_CONCATENATE(A, B), C)
#define LIBXSMM_CONCATENATE3(A, B, C, D) LIBXSMM_CONCATENATE(LIBXSMM_CONCATENATE2(A, B, C), D)
#define LIBXSMM_FSYMBOL(SYMBOL) LIBXSMM_CONCATENATE(SYMBOL, _)
#define LIBXSMM_UNIQUE(NAME) LIBXSMM_CONCATENATE(NAME, __LINE__)
#define LIBXSMM_EXPAND(...) __VA_ARGS__
#define LIBXSMM_ELIDE(...)

#define LIBXSMM_VERSION2(MAJOR, MINOR) ((MAJOR) * 10000 + (MINOR) * 100)
#define LIBXSMM_VERSION3(MAJOR, MINOR, UPDATE) (LIBXSMM_VERSION2(MAJOR, MINOR) + (UPDATE))
#define LIBXSMM_VERSION4(MAJOR, MINOR, UPDATE, PATCH) \
  (((MAJOR/*7b*/) << 24) | ((MINOR/*5b*/) << 19) | ((UPDATE/*5b*/) << 14) | (PATCH/*14b*/))
#define LIBXSMM_VERSION41(VERSION) (((VERSION) >> 24))
#define LIBXSMM_VERSION42(VERSION) (((VERSION) >> 19) & 0x1F)
#define LIBXSMM_VERSION43(VERSION) (((VERSION) >> 14) & 0x1F)
#define LIBXSMM_VERSION44(VERSION) (((VERSION)) & 0x3FFF)

#if defined(__cplusplus)
# define LIBXSMM_VARIADIC ...
# define LIBXSMM_EXTERN_KEYWORD extern "C"
# define LIBXSMM_EXTERN LIBXSMM_EXTERN_KEYWORD
# define LIBXSMM_EXTERN_C LIBXSMM_EXTERN_KEYWORD
# define LIBXSMM_INLINE_KEYWORD inline
# define LIBXSMM_INLINE LIBXSMM_INLINE_KEYWORD
# if defined(__GNUC__) || defined(_CRAYC)
#   define LIBXSMM_CALLER __PRETTY_FUNCTION__
# elif defined(_MSC_VER)
#   define LIBXSMM_CALLER __FUNCDNAME__
#   define LIBXSMM_FUNCNAME __FUNCTION__
# else
#   define LIBXSMM_CALLER __FUNCNAME__
# endif
#else
# define LIBXSMM_VARIADIC
# define LIBXSMM_EXTERN_KEYWORD extern
# define LIBXSMM_EXTERN LIBXSMM_EXTERN_KEYWORD
# define LIBXSMM_EXTERN_C
# if defined(__STDC_VERSION__) && (199901L <= __STDC_VERSION__) /*C99*/
#   define LIBXSMM_PRAGMA(DIRECTIVE) _Pragma(LIBXSMM_STRINGIFY(DIRECTIVE))
#   define LIBXSMM_CALLER __func__
#   define LIBXSMM_RESTRICT restrict
#   define LIBXSMM_INLINE_KEYWORD inline
# elif defined(_MSC_VER)
#   define LIBXSMM_CALLER __FUNCDNAME__
#   define LIBXSMM_FUNCNAME __FUNCTION__
#   define LIBXSMM_INLINE_KEYWORD __inline
#   define LIBXSMM_INLINE_FIXUP
# elif defined(__GNUC__) && !defined(__STRICT_ANSI__)
#   define LIBXSMM_CALLER __PRETTY_FUNCTION__
# endif
# if !defined(LIBXSMM_INLINE_KEYWORD)
#   define LIBXSMM_INLINE_KEYWORD
#   define LIBXSMM_INLINE_FIXUP
# endif
# define LIBXSMM_INLINE static LIBXSMM_INLINE_KEYWORD
#endif /*__cplusplus*/
#if !defined(LIBXSMM_CALLER)
# define LIBXSMM_CALLER NULL
#endif
#if !defined(LIBXSMM_FUNCNAME)
# define LIBXSMM_FUNCNAME LIBXSMM_CALLER
#endif
#if !defined(LIBXSMM_CALLER_ID)
# if defined(__GNUC__) || 1
#   define LIBXSMM_CALLER_ID ((const void*)((uintptr_t)libxsmm_hash_string(LIBXSMM_CALLER)))
# else /* assume no string-pooling (perhaps unsafe) */
#   define LIBXSMM_CALLER_ID LIBXSMM_CALLER
# endif
#endif

#if !defined(LIBXSMM_UNPACKED) && (defined(_CRAYC) || defined(LIBXSMM_OFFLOAD_BUILD))
# define LIBXSMM_UNPACKED
#endif

#if defined(_WIN32) && !defined(__GNUC__) && !defined(__clang__)
# define LIBXSMM_ATTRIBUTE(A) __declspec(A)
# if defined(__cplusplus)
#   define LIBXSMM_INLINE_ALWAYS __forceinline
# else
#   define LIBXSMM_INLINE_ALWAYS static __forceinline
# endif
# define LIBXSMM_ALIGNED(DECL, N) LIBXSMM_ATTRIBUTE(align(N)) DECL
# if !defined(LIBXSMM_UNPACKED)
#   define LIBXSMM_PACKED(TYPE) LIBXSMM_PRAGMA(pack(1)) TYPE
# endif
# define LIBXSMM_CDECL __cdecl
#elif (defined(__GNUC__) || defined(__clang__) || defined(__PGI))
# define LIBXSMM_ATTRIBUTE(A) __attribute__((A))
# define LIBXSMM_INLINE_ALWAYS LIBXSMM_ATTRIBUTE(always_inline) LIBXSMM_INLINE
# define LIBXSMM_ALIGNED(DECL, N) DECL LIBXSMM_ATTRIBUTE(aligned(N))
# if !defined(LIBXSMM_UNPACKED)
#   define LIBXSMM_PACKED(TYPE) TYPE LIBXSMM_ATTRIBUTE(__packed__)
# endif
# define LIBXSMM_CDECL LIBXSMM_ATTRIBUTE(cdecl)
#else
# define LIBXSMM_ATTRIBUTE(A)
# define LIBXSMM_INLINE_ALWAYS LIBXSMM_INLINE
# define LIBXSMM_ALIGNED(DECL, N) DECL
# define LIBXSMM_CDECL
#endif
#if !defined(LIBXSMM_PACKED)
# define LIBXSMM_PACKED(TYPE) TYPE
# if !defined(LIBXSMM_UNPACKED)
#   define LIBXSMM_UNPACKED
# endif
#endif

#if defined(__INTEL_COMPILER)
# if !defined(__INTEL_COMPILER_UPDATE)
#   define LIBXSMM_INTEL_COMPILER __INTEL_COMPILER
# else
#   define LIBXSMM_INTEL_COMPILER (__INTEL_COMPILER + __INTEL_COMPILER_UPDATE)
# endif
#elif defined(__INTEL_COMPILER_BUILD_DATE)
# define LIBXSMM_INTEL_COMPILER ((__INTEL_COMPILER_BUILD_DATE / 10000 - 2000) * 100)
#endif

#if defined(LIBXSMM_OFFLOAD_BUILD) && \
  defined(__INTEL_OFFLOAD) && (!defined(_WIN32) || (1400 <= LIBXSMM_INTEL_COMPILER))
# define LIBXSMM_OFFLOAD(A) LIBXSMM_ATTRIBUTE(target(A))
# define LIBXSMM_NO_OFFLOAD(RTYPE, FN, ...) ((RTYPE (*)(LIBXSMM_VARIADIC))(FN))(__VA_ARGS__)
# if !defined(LIBXSMM_OFFLOAD_TARGET)
#   define LIBXSMM_OFFLOAD_TARGET mic
# endif
#else
# define LIBXSMM_OFFLOAD(A)
# define LIBXSMM_NO_OFFLOAD(RTYPE, FN, ...) (FN)(__VA_ARGS__)
#endif
#define LIBXSMM_RETARGETABLE LIBXSMM_OFFLOAD(LIBXSMM_OFFLOAD_TARGET)

#if !defined(__STATIC) && !defined(_WINDLL) && (defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__))
# define __STATIC
#endif

/* may include Clang and other compatible compilers */
#if defined(__GNUC__) && !defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
# define LIBXSMM_VISIBILITY_INTERNAL LIBXSMM_ATTRIBUTE(visibility("internal"))
# define LIBXSMM_VISIBILITY_HIDDEN LIBXSMM_ATTRIBUTE(visibility("hidden"))
# define LIBXSMM_VISIBILITY_PUBLIC LIBXSMM_ATTRIBUTE(visibility("default"))
#endif
#if !defined(LIBXSMM_VISIBILITY_INTERNAL)
# define LIBXSMM_VISIBILITY_INTERNAL
#endif
#if !defined(LIBXSMM_VISIBILITY_HIDDEN)
# define LIBXSMM_VISIBILITY_HIDDEN
#endif
#if !defined(LIBXSMM_VISIBILITY_PUBLIC)
# define LIBXSMM_VISIBILITY_PUBLIC
#endif
#if !defined(LIBXSMM_VISIBILITY_PRIVATE)
# define LIBXSMM_VISIBILITY_PRIVATE LIBXSMM_VISIBILITY_HIDDEN
#endif

/* Windows Dynamic Link Library (DLL) */
#if !defined(__STATIC) && (defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__))
# define LIBXSMM_VISIBILITY_EXPORT LIBXSMM_ATTRIBUTE(dllexport)
# define LIBXSMM_VISIBILITY_IMPORT LIBXSMM_ATTRIBUTE(dllimport)
#endif
#if !defined(LIBXSMM_VISIBILITY_EXPORT)
# define LIBXSMM_VISIBILITY_EXPORT LIBXSMM_EXTERN_C LIBXSMM_VISIBILITY_PUBLIC
#endif
#if !defined(LIBXSMM_VISIBILITY_IMPORT)
# define LIBXSMM_VISIBILITY_IMPORT LIBXSMM_EXTERN_KEYWORD LIBXSMM_VISIBILITY_PUBLIC
#endif

#if defined(LIBXSMM_API) /* header-only mode */
# define LIBXSMM_API_INTERN LIBXSMM_API
# define LIBXSMM_API_EXPORT LIBXSMM_API
# define LIBXSMM_APIVAR_PUBLIC(DECL) LIBXSMM_EXTERN_C LIBXSMM_RETARGETABLE DECL; LIBXSMM_RETARGETABLE DECL
# define LIBXSMM_APIVAR(DECL) LIBXSMM_APIVAR_PUBLIC(DECL)
# define LIBXSMM_APIEXT_INTERN LIBXSMM_API_INTERN
# define LIBXSMM_APIEXT LIBXSMM_APIEXT_INTERN
#else /* classic ABI */
# if defined(LIBXSMM_BUILD_EXT)
#   define LIBXSMM_API LIBXSMM_VISIBILITY_IMPORT LIBXSMM_RETARGETABLE
#   define LIBXSMM_API_INTERN LIBXSMM_VISIBILITY_PRIVATE LIBXSMM_RETARGETABLE
#   define LIBXSMM_API_EXPORT LIBXSMM_EXTERN_C LIBXSMM_API
#   define LIBXSMM_APIVAR_PUBLIC(DECL) LIBXSMM_API DECL; LIBXSMM_API DECL
#   define LIBXSMM_APIVAR(DECL) LIBXSMM_EXTERN_C LIBXSMM_VISIBILITY_PRIVATE LIBXSMM_RETARGETABLE DECL; \
                                LIBXSMM_EXTERN_C LIBXSMM_VISIBILITY_PRIVATE LIBXSMM_RETARGETABLE DECL
#   define LIBXSMM_APIEXT LIBXSMM_VISIBILITY_EXPORT LIBXSMM_RETARGETABLE
#   define LIBXSMM_APIEXT_INTERN LIBXSMM_API_INTERN
# elif defined(LIBXSMM_BUILD)
#   define LIBXSMM_API LIBXSMM_VISIBILITY_EXPORT LIBXSMM_RETARGETABLE
#   define LIBXSMM_API_INTERN LIBXSMM_VISIBILITY_PRIVATE LIBXSMM_RETARGETABLE
#   define LIBXSMM_API_EXPORT LIBXSMM_API
#   define LIBXSMM_APIVAR_PUBLIC(DECL) LIBXSMM_VISIBILITY_EXPORT LIBXSMM_RETARGETABLE DECL; \
                                       LIBXSMM_VISIBILITY_EXPORT LIBXSMM_RETARGETABLE DECL
#   define LIBXSMM_APIVAR(DECL) LIBXSMM_EXTERN_C LIBXSMM_VISIBILITY_PRIVATE LIBXSMM_RETARGETABLE DECL; \
                                LIBXSMM_EXTERN_C LIBXSMM_VISIBILITY_PRIVATE LIBXSMM_RETARGETABLE DECL
#   define LIBXSMM_APIEXT LIBXSMM_VISIBILITY_IMPORT LIBXSMM_RETARGETABLE
# else /* import */
#   define LIBXSMM_API LIBXSMM_VISIBILITY_IMPORT LIBXSMM_RETARGETABLE
#   define LIBXSMM_API_INTERN LIBXSMM_API
#   define LIBXSMM_API_EXPORT LIBXSMM_API
#   define LIBXSMM_APIVAR_PUBLIC(DECL) LIBXSMM_VISIBILITY_IMPORT LIBXSMM_RETARGETABLE DECL; \
                                       LIBXSMM_VISIBILITY_IMPORT LIBXSMM_RETARGETABLE DECL
#   define LIBXSMM_APIVAR(DECL) LIBXSMM_EXTERN_C LIBXSMM_VISIBILITY_PRIVATE LIBXSMM_RETARGETABLE DECL; \
                                LIBXSMM_EXTERN_C LIBXSMM_VISIBILITY_PRIVATE LIBXSMM_RETARGETABLE DECL
#   define LIBXSMM_APIEXT LIBXSMM_API
# endif
#endif
#if !defined(_WIN32)
# define LIBXSMM_APIVAR_PRIVATE(DECL) LIBXSMM_APIVAR(LIBXSMM_ALIGNED(DECL, LIBXSMM_CONFIG_CACHELINE))
# define LIBXSMM_APIVAR_ALIGNED(DECL) LIBXSMM_APIVAR_PUBLIC(LIBXSMM_ALIGNED(DECL, LIBXSMM_CONFIG_CACHELINE))
# define LIBXSMM_APIVAR_ARRAY(DECL, N) LIBXSMM_APIVAR(LIBXSMM_ALIGNED(DECL[N], LIBXSMM_CONFIG_CACHELINE))
#else /* Windows */
# define LIBXSMM_APIVAR_PRIVATE(DECL) LIBXSMM_APIVAR(DECL)
# define LIBXSMM_APIVAR_ALIGNED(DECL) LIBXSMM_APIVAR_PUBLIC(DECL)
# define LIBXSMM_APIVAR_ARRAY(DECL, N) LIBXSMM_APIVAR(DECL[N])
#endif
#define LIBXSMM_API_INLINE LIBXSMM_EXTERN_C LIBXSMM_INLINE LIBXSMM_RETARGETABLE

#if !defined(LIBXSMM_RESTRICT)
# if ((defined(__GNUC__) && !defined(__CYGWIN32__)) || defined(LIBXSMM_INTEL_COMPILER)) && !defined(_WIN32)
#   define LIBXSMM_RESTRICT __restrict__
# elif defined(_MSC_VER) || defined(LIBXSMM_INTEL_COMPILER)
#   define LIBXSMM_RESTRICT __restrict
# else
#   define LIBXSMM_RESTRICT
# endif
#endif /*LIBXSMM_RESTRICT*/

#if !defined(LIBXSMM_PRAGMA)
# if defined(LIBXSMM_INTEL_COMPILER) || defined(_MSC_VER)
#   define LIBXSMM_PRAGMA(DIRECTIVE) __pragma(DIRECTIVE)
# else
#   define LIBXSMM_PRAGMA(DIRECTIVE)
# endif
#endif /*LIBXSMM_PRAGMA*/

#if !defined(LIBXSMM_OPENMP_SIMD) && (defined(_OPENMP) && (201307 <= _OPENMP)) /*OpenMP 4.0*/
# if defined(LIBXSMM_INTEL_COMPILER)
#   if (1500 <= LIBXSMM_INTEL_COMPILER)
#     define LIBXSMM_OPENMP_SIMD
#   endif
# elif defined(__GNUC__)
#   if LIBXSMM_VERSION3(4, 9, 0) <= LIBXSMM_VERSION3(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#     define LIBXSMM_OPENMP_SIMD
#   endif
# else
#   define LIBXSMM_OPENMP_SIMD
# endif
#endif

#if !defined(LIBXSMM_INTEL_COMPILER) || (LIBXSMM_INTEL_COMPILER < 9900)
# if defined(LIBXSMM_OPENMP_SIMD)
#   define LIBXSMM_PRAGMA_SIMD_REDUCTION(EXPRESSION) LIBXSMM_PRAGMA(omp simd reduction(EXPRESSION))
#   define LIBXSMM_PRAGMA_SIMD_COLLAPSE(N) LIBXSMM_PRAGMA(omp simd collapse(N))
#   define LIBXSMM_PRAGMA_SIMD_PRIVATE(A, ...) LIBXSMM_PRAGMA(omp simd private(A, __VA_ARGS__))
#   define LIBXSMM_PRAGMA_SIMD LIBXSMM_PRAGMA(omp simd)
# elif defined(__INTEL_COMPILER)
#   define LIBXSMM_PRAGMA_SIMD_REDUCTION(EXPRESSION) LIBXSMM_PRAGMA(simd reduction(EXPRESSION))
#   define LIBXSMM_PRAGMA_SIMD_COLLAPSE(N) LIBXSMM_PRAGMA(simd collapse(N))
#   define LIBXSMM_PRAGMA_SIMD_PRIVATE(A, ...) LIBXSMM_PRAGMA(simd private(A, __VA_ARGS__))
#   define LIBXSMM_PRAGMA_SIMD LIBXSMM_PRAGMA(simd)
# endif
#endif
#if !defined(LIBXSMM_PRAGMA_SIMD)
# define LIBXSMM_PRAGMA_SIMD_REDUCTION(EXPRESSION)
# define LIBXSMM_PRAGMA_SIMD_COLLAPSE(N)
# define LIBXSMM_PRAGMA_SIMD_PRIVATE(A, ...)
# define LIBXSMM_PRAGMA_SIMD
#endif

#if defined(__INTEL_COMPILER)
# define LIBXSMM_PRAGMA_NONTEMPORAL_VARS(A, ...) LIBXSMM_PRAGMA(vector nontemporal(A, __VA_ARGS__))
# define LIBXSMM_PRAGMA_NONTEMPORAL LIBXSMM_PRAGMA(vector nontemporal)
# define LIBXSMM_PRAGMA_VALIGNED LIBXSMM_PRAGMA(vector aligned)
# define LIBXSMM_PRAGMA_NOVECTOR LIBXSMM_PRAGMA(novector)
# define LIBXSMM_PRAGMA_FORCEINLINE LIBXSMM_PRAGMA(forceinline)
# define LIBXSMM_PRAGMA_LOOP_COUNT(MIN, MAX, AVG) LIBXSMM_PRAGMA(loop_count min=MIN max=MAX avg=AVG)
# define LIBXSMM_PRAGMA_UNROLL_AND_JAM(N) LIBXSMM_PRAGMA(unroll_and_jam(N))
# define LIBXSMM_PRAGMA_UNROLL_N(N) LIBXSMM_PRAGMA(unroll(N))
# define LIBXSMM_PRAGMA_UNROLL LIBXSMM_PRAGMA(unroll)
# define LIBXSMM_PRAGMA_VALIGNED_VAR(A) LIBXSMM_ASSUME_ALIGNED(A, LIBXSMM_ALIGNMENT);
/*# define LIBXSMM_UNUSED(VARIABLE) LIBXSMM_PRAGMA(unused(VARIABLE))*/
#else
# define LIBXSMM_PRAGMA_NONTEMPORAL_VARS(A, ...)
# define LIBXSMM_PRAGMA_NONTEMPORAL
# define LIBXSMM_PRAGMA_VALIGNED_VAR(A)
# define LIBXSMM_PRAGMA_VALIGNED
# define LIBXSMM_PRAGMA_NOVECTOR
# define LIBXSMM_PRAGMA_FORCEINLINE
# define LIBXSMM_PRAGMA_LOOP_COUNT(MIN, MAX, AVG)
# define LIBXSMM_PRAGMA_UNROLL_AND_JAM(N)
# define LIBXSMM_PRAGMA_UNROLL
#endif
#if !defined(LIBXSMM_PRAGMA_UNROLL_N)
# if defined(__GNUC__) && (LIBXSMM_VERSION3(8, 3, 0) <= LIBXSMM_VERSION3(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__))
#   define LIBXSMM_PRAGMA_UNROLL_N(N) LIBXSMM_PRAGMA(GCC unroll N)
# else
#   define LIBXSMM_PRAGMA_UNROLL_N(N)
# endif
#endif

#if defined(LIBXSMM_INTEL_COMPILER)
# define LIBXSMM_PRAGMA_OPTIMIZE_OFF LIBXSMM_PRAGMA(optimize("", off))
# define LIBXSMM_PRAGMA_OPTIMIZE_ON  LIBXSMM_PRAGMA(optimize("", on))
#elif defined(__clang__)
# define LIBXSMM_PRAGMA_OPTIMIZE_OFF LIBXSMM_PRAGMA(clang optimize off)
# define LIBXSMM_PRAGMA_OPTIMIZE_ON  LIBXSMM_PRAGMA(clang optimize on)
#elif defined(__GNUC__)
# define LIBXSMM_PRAGMA_OPTIMIZE_OFF LIBXSMM_PRAGMA(GCC push_options) LIBXSMM_PRAGMA(GCC optimize("O0"))
# define LIBXSMM_PRAGMA_OPTIMIZE_ON  LIBXSMM_PRAGMA(GCC pop_options)
#else
# define LIBXSMM_PRAGMA_OPTIMIZE_OFF
# define LIBXSMM_PRAGMA_OPTIMIZE_ON
#endif

#if defined(_OPENMP) && (200805 <= _OPENMP) /*OpenMP 3.0*/ \
 && defined(NDEBUG) /* CCE complains for debug builds */
# define LIBXSMM_OPENMP_COLLAPSE(N) collapse(N)
#else
# define LIBXSMM_OPENMP_COLLAPSE(N)
#endif

/** LIBXSMM_UP2POT rounds up to the next power of two (POT). */
#define LIBXSMM_UP2POT_01(N) ((N) | ((N) >> 1))
#define LIBXSMM_UP2POT_02(N) (LIBXSMM_UP2POT_01(N) | (LIBXSMM_UP2POT_01(N) >> 2))
#define LIBXSMM_UP2POT_04(N) (LIBXSMM_UP2POT_02(N) | (LIBXSMM_UP2POT_02(N) >> 4))
#define LIBXSMM_UP2POT_08(N) (LIBXSMM_UP2POT_04(N) | (LIBXSMM_UP2POT_04(N) >> 8))
#define LIBXSMM_UP2POT_16(N) (LIBXSMM_UP2POT_08(N) | (LIBXSMM_UP2POT_08(N) >> 16))
#define LIBXSMM_UP2POT_32(N) (LIBXSMM_UP2POT_16(N) | (LIBXSMM_UP2POT_16(N) >> 32))
#define LIBXSMM_UP2POT(N) (LIBXSMM_UP2POT_32((unsigned long long)(N) - LIBXSMM_MIN(1, N)) + LIBXSMM_MIN(1, N))
#define LIBXSMM_LO2POT(N) (LIBXSMM_UP2POT_32((unsigned long long)(N) >> 1) + LIBXSMM_MIN(1, N))

#define LIBXSMM_UP2(N, NPOT) ((((uintptr_t)N) + ((NPOT) - 1)) & ~((NPOT) - 1))
#define LIBXSMM_UP(N, UP) ((((N) + (UP) - 1) / (UP)) * (UP))
#define LIBXSMM_ABS(A) (0 <= (A) ? (A) : -(A))
#define LIBXSMM_MIN(A, B) ((A) < (B) ? (A) : (B))
#define LIBXSMM_MAX(A, B) ((A) < (B) ? (B) : (A))
#define LIBXSMM_MOD(A, N) ((A) % (N))
#define LIBXSMM_MOD2(A, NPOT) ((A) & ((NPOT) - 1))
#define LIBXSMM_DELTA(T0, T1) ((T0) < (T1) ? ((T1) - (T0)) : ((T0) - (T1)))
#define LIBXSMM_CLMP(VALUE, LO, HI) ((LO) < (VALUE) ? ((VALUE) <= (HI) ? (VALUE) : LIBXSMM_MIN(VALUE, HI)) : LIBXSMM_MAX(LO, VALUE))
#define LIBXSMM_SIZEOF(START, LAST) (((const char*)(LAST)) - ((const char*)(START)) + sizeof(*LAST))
#define LIBXSMM_FEQ(A, B) ((A) == (B))
#define LIBXSMM_NEQ(A, B) ((A) != (B))
#define LIBXSMM_ISNAN(A)  LIBXSMM_NEQ(A, A)
#define LIBXSMM_NOTNAN(A) LIBXSMM_FEQ(A, A)
#define LIBXSMM_ROUNDX(TYPE, A) ((TYPE)((long long)(0 <= (A) ? ((double)(A) + 0.5) : ((double)(A) - 0.5))))

/** Makes some functions available independent of C99 support. */
#if defined(__STDC_VERSION__) && (199901L <= __STDC_VERSION__) /*C99*/
# if defined(__PGI)
#   define LIBXSMM_POWF(A, B) ((float)pow((double)(A), (double)(B)))
# else
#   define LIBXSMM_POWF(A, B) powf(A, B)
# endif
# define LIBXSMM_FREXPF(A, B) frexpf(A, B)
# define LIBXSMM_ROUNDF(A) roundf(A)
# define LIBXSMM_ROUND(A) round(A)
# define LIBXSMM_TANHF(A) tanhf(A)
# define LIBXSMM_SQRTF(A) sqrtf(A)
# define LIBXSMM_EXP2F(A) exp2f(A)
# define LIBXSMM_LOG2F(A) log2f(A)
# define LIBXSMM_EXP2(A) exp2(A)
# define LIBXSMM_LOG2(A) log2(A)
# define LIBXSMM_EXPF(A) expf(A)
# define LIBXSMM_LOGF(A) logf(A)
#else
# define LIBXSMM_POWF(A, B) ((float)pow((double)(A), (double)(B)))
# define LIBXSMM_FREXPF(A, B) ((float)frexp((double)(A), B))
# define LIBXSMM_ROUNDF(A) LIBXSMM_ROUNDX(float, A)
# define LIBXSMM_ROUND(A) LIBXSMM_ROUNDX(double, A)
# define LIBXSMM_TANHF(A) ((float)tanh((double)(A)))
# define LIBXSMM_SQRTF(A) ((float)sqrt((double)(A)))
# define LIBXSMM_EXP2F(A) LIBXSMM_POWF(2, A)
# define LIBXSMM_LOG2F(A) ((float)LIBXSMM_LOG2((double)(A)))
# define LIBXSMM_EXP2(A) pow(2.0, (double)(A))
# define LIBXSMM_LOG2(A) (log(A) * (1.0 / (M_LN2)))
# define LIBXSMM_EXPF(A) ((float)exp((double)(A)))
# define LIBXSMM_LOGF(A) ((float)log((double)(A)))

#endif

#if defined(LIBXSMM_INTEL_COMPILER)
# if (1700 <= LIBXSMM_INTEL_COMPILER)
#   define LIBXSMM_ASSUME(EXPRESSION) __assume(EXPRESSION)
# else
#   define LIBXSMM_ASSUME(EXPRESSION) assert(EXPRESSION)
# endif
#elif defined(_MSC_VER)
# define LIBXSMM_ASSUME(EXPRESSION) __assume(EXPRESSION)
#elif defined(__GNUC__) && !defined(_CRAYC) && (LIBXSMM_VERSION3(4, 5, 0) <= LIBXSMM_VERSION3(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__))
# define LIBXSMM_ASSUME(EXPRESSION) do { if (!(EXPRESSION)) __builtin_unreachable(); } while(0)
#else
# define LIBXSMM_ASSUME(EXPRESSION) assert(EXPRESSION)
#endif

#if defined(__INTEL_COMPILER)
# define LIBXSMM_ASSUME_ALIGNED(A, N) __assume_aligned(A, N)
#else
# define LIBXSMM_ASSUME_ALIGNED(A, N) assert(0 == ((uintptr_t)(A)) % (N))
#endif
#define LIBXSMM_ALIGN(POINTER, ALIGNMENT/*POT*/) ((POINTER) + (LIBXSMM_UP2((uintptr_t)(POINTER), ALIGNMENT) - ((uintptr_t)(POINTER))) / sizeof(*(POINTER)))
#define LIBXSMM_FOLD2(POINTER, ALIGNMENT, NPOT) LIBXSMM_MOD2(((uintptr_t)(POINTER) / (ALIGNMENT)), NPOT)

#if defined(_MSC_VER) && !defined(__clang__) && !defined(LIBXSMM_INTEL_COMPILER) /* account for incorrect handling of __VA_ARGS__ */
# define LIBXSMM_SELECT_ELEMENT(INDEX1/*one-based*/, .../*elements*/) LIBXSMM_CONCATENATE(LIBXSMM_SELECT_ELEMENT_, INDEX1)LIBXSMM_EXPAND((__VA_ARGS__))
#else
# define LIBXSMM_SELECT_ELEMENT(INDEX1/*one-based*/, .../*elements*/) LIBXSMM_CONCATENATE(LIBXSMM_SELECT_ELEMENT_, INDEX1)(__VA_ARGS__)
#endif
#define LIBXSMM_SELECT_ELEMENT_1(E0, E1, E2, E3, E4, E5, E6, E7) E0
#define LIBXSMM_SELECT_ELEMENT_2(E0, E1, E2, E3, E4, E5, E6, E7) E1
#define LIBXSMM_SELECT_ELEMENT_3(E0, E1, E2, E3, E4, E5, E6, E7) E2
#define LIBXSMM_SELECT_ELEMENT_4(E0, E1, E2, E3, E4, E5, E6, E7) E3
#define LIBXSMM_SELECT_ELEMENT_5(E0, E1, E2, E3, E4, E5, E6, E7) E4
#define LIBXSMM_SELECT_ELEMENT_6(E0, E1, E2, E3, E4, E5, E6, E7) E5
#define LIBXSMM_SELECT_ELEMENT_7(E0, E1, E2, E3, E4, E5, E6, E7) E6
#define LIBXSMM_SELECT_ELEMENT_8(E0, E1, E2, E3, E4, E5, E6, E7) E7
#define LIBXSMM_SELECT_HEAD_AUX(A, ...) (A)
#define LIBXSMM_SELECT_HEAD(...) LIBXSMM_EXPAND(LIBXSMM_SELECT_HEAD_AUX(__VA_ARGS__, 0/*dummy*/))
#define LIBXSMM_SELECT_TAIL(A, ...) __VA_ARGS__

/**
 * For VLAs, check EXACTLY for C99 since a C11-conforming compiler may not provide VLAs.
 * However, some compilers (Intel) may signal support for VLA even with strict ANSI (C89).
 * To ultimately disable VLA-support, define LIBXSMM_NO_VLA (make VLA=0).
 * VLA-support is signaled by LIBXSMM_VLA.
 */
#if !defined(LIBXSMM_VLA) && !defined(LIBXSMM_NO_VLA) && !defined(__PGI) && ((defined(__STDC_VERSION__) && (199901L/*C99*/ == __STDC_VERSION__ || \
   (!defined(__STDC_NO_VLA__) && 199901L/*C99*/ < __STDC_VERSION__))) || (defined(__GNUC__) && !defined(__STRICT_ANSI__) && !defined(__cplusplus)) /*|| \
    (defined(LIBXSMM_INTEL_COMPILER) && !defined(_WIN32) && !defined(__cplusplus)) || (defined(__INTEL_COMPILER) && !defined(_WIN32))*/)
# define LIBXSMM_VLA
#endif

/**
 * LIBXSMM_INDEX1 calculates the linear address for a given set of (multiple) indexes/bounds.
 * Syntax: LIBXSMM_INDEX1(<ndims>, <i0>, ..., <i(ndims-1)>, <s1>, ..., <s(ndims-1)>).
 * Please note that the leading dimension (s0) is omitted in the above syntax!
 * TODO: support leading dimension (pitch/stride).
 */
#if defined(_MSC_VER) && !defined(__clang__) /* account for incorrect handling of __VA_ARGS__ */
# define LIBXSMM_INDEX1(NDIMS, ...) LIBXSMM_CONCATENATE(LIBXSMM_INDEX1_, NDIMS)LIBXSMM_EXPAND((__VA_ARGS__))
#else
# define LIBXSMM_INDEX1(NDIMS, ...) LIBXSMM_CONCATENATE(LIBXSMM_INDEX1_, NDIMS)(__VA_ARGS__)
#endif
#define LIBXSMM_INDEX1_1(...) ((size_t)LIBXSMM_SELECT_HEAD(__VA_ARGS__))
#define LIBXSMM_INDEX1_2(I0, I1, S1) (LIBXSMM_INDEX1_1(I0) * ((size_t)S1) + (size_t)I1)
#define LIBXSMM_INDEX1_3(I0, I1, I2, S1, S2) (LIBXSMM_INDEX1_2(I0, I1, S1) * ((size_t)S2) + (size_t)I2)
#define LIBXSMM_INDEX1_4(I0, I1, I2, I3, S1, S2, S3) (LIBXSMM_INDEX1_3(I0, I1, I2, S1, S2) * ((size_t)S3) + (size_t)I3)
#define LIBXSMM_INDEX1_5(I0, I1, I2, I3, I4, S1, S2, S3, S4) (LIBXSMM_INDEX1_4(I0, I1, I2, I3, S1, S2, S3) * ((size_t)S4) + (size_t)I4)
#define LIBXSMM_INDEX1_6(I0, I1, I2, I3, I4, I5, S1, S2, S3, S4, S5) (LIBXSMM_INDEX1_5(I0, I1, I2, I3, I4, S1, S2, S3, S4) * ((size_t)S5) + (size_t)I5)
#define LIBXSMM_INDEX1_7(I0, I1, I2, I3, I4, I5, I6, S1, S2, S3, S4, S5, S6) (LIBXSMM_INDEX1_6(I0, I1, I2, I3, I4, I5, S1, S2, S3, S4, S5) * ((size_t)S6) + (size_t)I6)
#define LIBXSMM_INDEX1_8(I0, I1, I2, I3, I4, I5, I6, I7, S1, S2, S3, S4, S5, S6, S7) (LIBXSMM_INDEX1_7(I0, I1, I2, I3, I4, I5, I6, S1, S2, S3, S4, S5, S6) * ((size_t)S7) + (size_t)I7)

/**
 * LIBXSMM_VLA_DECL declares an array according to the given set of (multiple) bounds.
 * Syntax: LIBXSMM_VLA_DECL(<ndims>, <elem-type>, <var-name>, <init>, <s1>, ..., <s(ndims-1)>).
 * The element type can be "const" or otherwise qualified; initial value must be (const)element-type*.
 * Please note that the syntax is similar to LIBXSMM_INDEX1, and the leading dimension (s0) is omitted!
 *
 * LIBXSMM_VLA_ACCESS gives the array element according to the given set of (multiple) indexes/bounds.
 * Syntax: LIBXSMM_VLA_ACCESS(<ndims>, <array>, <i0>, ..., <i(ndims-1)>, <s1>, ..., <s(ndims-1)>).
 * Please note that the syntax is similar to LIBXSMM_INDEX1, and the leading dimension (s0) is omitted!
 */
#if !defined(LIBXSMM_VLA_POSTFIX)
# define LIBXSMM_VLA_POSTFIX _
#endif
#if defined(LIBXSMM_VLA)
# define LIBXSMM_VLA_ACCESS(NDIMS, ARRAY, ...) LIBXSMM_VLA_ACCESS_Z(NDIMS, LIBXSMM_CONCATENATE(ARRAY, LIBXSMM_VLA_POSTFIX), LIBXSMM_VLA_ACCESS_X, __VA_ARGS__)
# define LIBXSMM_VLA_ACCESS_X(S) + 0 * (S)
# define LIBXSMM_VLA_ACCESS_Y(...)
# define LIBXSMM_VLA_ACCESS_Z(NDIMS, ARRAY, XY, ...) LIBXSMM_CONCATENATE(LIBXSMM_VLA_ACCESS_, NDIMS)(ARRAY, XY, __VA_ARGS__)
# define LIBXSMM_VLA_ACCESS_0(ARRAY, XY, ...) (ARRAY)/*scalar*/
# define LIBXSMM_VLA_ACCESS_1(ARRAY, XY, ...) ((ARRAY)[LIBXSMM_SELECT_HEAD(__VA_ARGS__)])
# define LIBXSMM_VLA_ACCESS_2(ARRAY, XY, I0, I1, ...) (((ARRAY) XY(__VA_ARGS__))[I0][I1])
# define LIBXSMM_VLA_ACCESS_3(ARRAY, XY, I0, I1, I2, S1, ...) (((ARRAY) XY(S1) XY(__VA_ARGS__))[I0][I1][I2])
# define LIBXSMM_VLA_ACCESS_4(ARRAY, XY, I0, I1, I2, I3, S1, S2, ...) (((ARRAY) XY(S1) XY(S2) XY(__VA_ARGS__))[I0][I1][I2][I3])
# define LIBXSMM_VLA_ACCESS_5(ARRAY, XY, I0, I1, I2, I3, I4, S1, S2, S3, ...) (((ARRAY) XY(S1) XY(S2) XY(S3) XY(__VA_ARGS__))[I0][I1][I2][I3][I4])
# define LIBXSMM_VLA_ACCESS_6(ARRAY, XY, I0, I1, I2, I3, I4, I5, S1, S2, S3, S4, ...) (((ARRAY) XY(S1) XY(S2) XY(S3) XY(S4) XY(__VA_ARGS__))[I0][I1][I2][I3][I4][I5])
# define LIBXSMM_VLA_ACCESS_7(ARRAY, XY, I0, I1, I2, I3, I4, I5, I6, S1, S2, S3, S4, S5, ...) (((ARRAY) XY(S1) XY(S2) XY(S3) XY(S4) XY(S5) XY(__VA_ARGS__))[I0][I1][I2][I3][I4][I5][I6])
# define LIBXSMM_VLA_ACCESS_8(ARRAY, XY, I0, I1, I2, I3, I4, I5, I6, I7, S1, S2, S3, S4, S5, S6, ...) (((ARRAY) XY(S1) XY(S2) XY(S3) XY(S4) XY(S5) XY(S6) XY(__VA_ARGS__))[I0][I1][I2][I3][I4][I5][I6][I7])
# define LIBXSMM_VLA_DECL(NDIMS, ELEMENT_TYPE, ARRAY_VAR, .../*initial value, and bounds*/) \
    ELEMENT_TYPE LIBXSMM_VLA_ACCESS_Z(LIBXSMM_SELECT_ELEMENT(NDIMS, 0, 1, 2, 3, 4, 5, 6, 7), *LIBXSMM_RESTRICT LIBXSMM_CONCATENATE(ARRAY_VAR, LIBXSMM_VLA_POSTFIX), \
      LIBXSMM_VLA_ACCESS_Y, LIBXSMM_SELECT_TAIL(__VA_ARGS__, 0)/*bounds*/, LIBXSMM_SELECT_TAIL(__VA_ARGS__, 0)/*dummy*/) = \
   (ELEMENT_TYPE LIBXSMM_VLA_ACCESS_Z(LIBXSMM_SELECT_ELEMENT(NDIMS, 0, 1, 2, 3, 4, 5, 6, 7), *, \
      LIBXSMM_VLA_ACCESS_Y, LIBXSMM_SELECT_TAIL(__VA_ARGS__, 0)/*bounds*/, LIBXSMM_SELECT_TAIL(__VA_ARGS__, 0)/*dummy*/))LIBXSMM_SELECT_HEAD(__VA_ARGS__)
#else /* calculate linear index */
# define LIBXSMM_VLA_ACCESS(NDIMS, ARRAY, ...) LIBXSMM_CONCATENATE(ARRAY, LIBXSMM_VLA_POSTFIX)[LIBXSMM_INDEX1(NDIMS, __VA_ARGS__)]
# define LIBXSMM_VLA_DECL(NDIMS, ELEMENT_TYPE, ARRAY_VAR, .../*initial value, and bounds*/) \
    ELEMENT_TYPE *LIBXSMM_RESTRICT LIBXSMM_CONCATENATE(ARRAY_VAR, LIBXSMM_VLA_POSTFIX) = /*(ELEMENT_TYPE*)*/LIBXSMM_SELECT_HEAD(__VA_ARGS__) \
    + 0 * LIBXSMM_INDEX1(NDIMS, LIBXSMM_SELECT_TAIL(__VA_ARGS__, LIBXSMM_SELECT_TAIL(__VA_ARGS__, 0))) /* dummy-shift to "sink" unused arguments */
#endif

/** Access an array of TYPE with Byte-measured stride. */
#define LIBXSMM_ACCESS(TYPE, ARRAY, STRIDE) (*(TYPE*)((char*)(ARRAY) + (STRIDE)))

#if !defined(LIBXSMM_UNUSED)
# if 0
#   define LIBXSMM_UNUSED(VARIABLE) LIBXSMM_PRAGMA(unused(VARIABLE))
# else
#   define LIBXSMM_UNUSED(VARIABLE) (void)(VARIABLE)
# endif
#endif

#if defined(_OPENMP)
# define LIBXSMM_PRAGMA_OMP(...) LIBXSMM_PRAGMA(omp __VA_ARGS__)
#else
# define LIBXSMM_PRAGMA_OMP(...)
#endif
#if defined(_OPENMP) && defined(_MSC_VER) && !defined(__clang__) && !defined(LIBXSMM_INTEL_COMPILER)
# define LIBXSMM_OMP_VAR(A) LIBXSMM_UNUSED(A) /* suppress warning about "unused" variable */
#else
# define LIBXSMM_OMP_VAR(A)
#endif

#if (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__) && !defined(__MINGW32__)
# define LIBXSMM_ATTRIBUTE_WEAK_IMPORT LIBXSMM_ATTRIBUTE(weak_import)
# define LIBXSMM_ATTRIBUTE_WEAK LIBXSMM_ATTRIBUTE(weak)
#else
# define LIBXSMM_ATTRIBUTE_WEAK
# define LIBXSMM_ATTRIBUTE_WEAK_IMPORT
#endif

#if !defined(LIBXSMM_NO_CTOR) && !defined(LIBXSMM_CTOR) && \
    (defined(LIBXSMM_BUILD) && !defined(__STATIC)) && \
    (defined(__GNUC__) || defined(__clang__))
# define LIBXSMM_ATTRIBUTE_CTOR LIBXSMM_ATTRIBUTE(constructor)
# define LIBXSMM_ATTRIBUTE_DTOR LIBXSMM_ATTRIBUTE(destructor)
# define LIBXSMM_CTOR
#else
# define LIBXSMM_ATTRIBUTE_CTOR
# define LIBXSMM_ATTRIBUTE_DTOR
#endif

#if defined(__GNUC__) || (defined(LIBXSMM_INTEL_COMPILER) && !defined(_WIN32))
# define LIBXSMM_ATTRIBUTE_MALLOC LIBXSMM_ATTRIBUTE(malloc)
# define LIBXSMM_ATTRIBUTE_UNUSED LIBXSMM_ATTRIBUTE(unused)
# define LIBXSMM_ATTRIBUTE_USED LIBXSMM_ATTRIBUTE(used)
#else
# define LIBXSMM_ATTRIBUTE_MALLOC
# define LIBXSMM_ATTRIBUTE_UNUSED
# define LIBXSMM_ATTRIBUTE_USED
#endif
#if defined(__GNUC__)
# define LIBXSMM_MAY_ALIAS LIBXSMM_ATTRIBUTE(__may_alias__)
#else
# define LIBXSMM_MAY_ALIAS
#endif

#if !defined(LIBXSMM_MKTEMP_PATTERN)
# define LIBXSMM_MKTEMP_PATTERN "XXXXXX"
#endif
#if defined(_WIN32) && 0
# define LIBXSMM_SNPRINTF(S, N, ...) _snprintf_s(S, N, _TRUNCATE, __VA_ARGS__)
# define setenv(NAME, VALUE, OVERWRITE) _putenv(NAME "=" VALUE)
#elif defined(__STDC_VERSION__) && (199901L <= __STDC_VERSION__ || defined(__GNUC__))
# define LIBXSMM_SNPRINTF(S, N, ...) snprintf(S, N, __VA_ARGS__)
#else
# define LIBXSMM_SNPRINTF(S, N, ...) sprintf((S) + /*unused*/(N) * 0, __VA_ARGS__)
#endif
#if (0 == LIBXSMM_SYNC)
# define LIBXSMM_FLOCK(FILE)
# define LIBXSMM_FUNLOCK(FILE)
#elif defined(_WIN32)
# define LIBXSMM_FLOCK(FILE) _lock_file(FILE)
# define LIBXSMM_FUNLOCK(FILE) _unlock_file(FILE)
#elif !defined(__CYGWIN__)
# define LIBXSMM_FLOCK(FILE) flockfile(FILE)
# define LIBXSMM_FUNLOCK(FILE) funlockfile(FILE)
#else /* Only available with __CYGWIN__ *and* C++0x. */
# define LIBXSMM_FLOCK(FILE)
# define LIBXSMM_FUNLOCK(FILE)
#endif

/** Synchronize console output */
#define LIBXSMM_STDIO_ACQUIRE() LIBXSMM_FLOCK(stdout); LIBXSMM_FLOCK(stderr)
#define LIBXSMM_STDIO_RELEASE() LIBXSMM_FUNLOCK(stderr); LIBXSMM_FUNLOCK(stdout)

/** Below group is to fix-up some platform/compiler specifics. */
#if defined(_WIN32)
# if !defined(_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES)
#   define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
# endif
# if !defined(_CRT_SECURE_NO_DEPRECATE)
#   define _CRT_SECURE_NO_DEPRECATE 1
# endif
# if !defined(_USE_MATH_DEFINES)
#   define _USE_MATH_DEFINES 1
# endif
# if !defined(WIN32_LEAN_AND_MEAN)
#   define WIN32_LEAN_AND_MEAN 1
# endif
# if !defined(NOMINMAX)
#   define NOMINMAX 1
# endif
# if defined(__INTEL_COMPILER) && (190023506 <= _MSC_FULL_VER)
#   define __builtin_huge_val() HUGE_VAL
#   define __builtin_huge_valf() HUGE_VALF
#   define __builtin_nan nan
#   define __builtin_nanf nanf
#   define __builtin_nans nan
#   define __builtin_nansf nanf
#   if defined(__cplusplus)
#     define _CMATH_
#   endif
# endif
#endif
#if !defined(_GNU_SOURCE) /*&& defined(__GNUC__)*/
# define _GNU_SOURCE
#endif
#if !defined(__STDC_FORMAT_MACROS)
# define __STDC_FORMAT_MACROS
#endif
#if defined(__clang__) && !defined(__extern_always_inline)
# define __extern_always_inline LIBXSMM_INLINE
#endif
#if defined(LIBXSMM_INLINE_FIXUP) && !defined(inline)
# define inline LIBXSMM_INLINE_KEYWORD
#endif

#if (0 != LIBXSMM_SYNC) && !defined(_REENTRANT)
# define _REENTRANT
#endif

#if !defined(__has_feature) && !defined(__clang__)
# define __has_feature(A) 0
#endif
#if !defined(__has_builtin) && !defined(__clang__)
# define __has_builtin(A) 0
#endif

#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(push,target(LIBXSMM_OFFLOAD_TARGET))
#endif
#if !defined(LIBXSMM_ASSERT)
# include <assert.h>
# if defined(NDEBUG)
#   define LIBXSMM_ASSERT(EXPR) LIBXSMM_ASSUME(EXPR)
# else
#   define LIBXSMM_ASSERT(EXPR) assert(EXPR)
# endif
#endif
#if !defined(LIBXSMM_ASSERT_MSG)
# define LIBXSMM_ASSERT_MSG(EXPR, MSG) assert((EXPR) && (0 != *(MSG)))
#endif
#if !defined(LIBXSMM_EXPECT)
# if defined(NDEBUG)
#   define LIBXSMM_EXPECT(RESULT, EXPR) (EXPR)
# else
#   define LIBXSMM_EXPECT(RESULT, EXPR) LIBXSMM_ASSERT((RESULT) == (EXPR))
# endif
#endif
#if !defined(LIBXSMM_EXPECT_NOT)
# if defined(NDEBUG)
#   define LIBXSMM_EXPECT_NOT(RESULT, EXPR) (EXPR)
# else
#   define LIBXSMM_EXPECT_NOT(RESULT, EXPR) LIBXSMM_ASSERT((RESULT) != (EXPR))
# endif
#endif
#if defined(LIBXSMM_GLIBC_FPTYPES)
# if defined(__cplusplus)
#   undef __USE_MISC
#   if !defined(LIBXSMM_NO_LIBM)
#     include <math.h>
#   endif
#   if !defined(_DEFAULT_SOURCE)
#     define _DEFAULT_SOURCE
#   endif
#   if !defined(_BSD_SOURCE)
#     define _BSD_SOURCE
#   endif
# elif !defined(__PURE_INTEL_C99_HEADERS__)
#   define __PURE_INTEL_C99_HEADERS__
# endif
#endif
#include <stddef.h>
#include <stdint.h>
#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(pop)
#endif

/* block must be after including above header files */
#if defined(__GLIBC__) && defined(__GLIBC_MINOR__) && LIBXSMM_VERSION2(__GLIBC__, __GLIBC_MINOR__) < LIBXSMM_VERSION2(2, 26)
/* _Float128 was introduced with GNU GCC 7.0. */
# if !defined(_Float128) && !defined(__SIZEOF_FLOAT128__) && defined(__GNUC__) && !defined(__cplusplus) && defined(__linux__)
#   define _Float128 __float128
# endif
# if !defined(LIBXSMM_GLIBC_FPTYPES) && defined(__GNUC__) && !defined(__cplusplus) && defined(__linux__) \
  && (LIBXSMM_VERSION3(7, 0, 0) > LIBXSMM_VERSION3(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__) \
  || defined(LIBXSMM_INTEL_COMPILER)) && 0 /* TODO */
#   define LIBXSMM_GLIBC_FPTYPES
# endif
# if !defined(_Float128X) && defined(LIBXSMM_GLIBC_FPTYPES)
#   define _Float128X _Float128
# endif
# if !defined(_Float32) && defined(LIBXSMM_GLIBC_FPTYPES)
#   define _Float32 float
# endif
# if !defined(_Float32x) && defined(LIBXSMM_GLIBC_FPTYPES)
#   define _Float32x _Float32
# endif
# if !defined(_Float64) && defined(LIBXSMM_GLIBC_FPTYPES)
#   define _Float64 double
# endif
# if !defined(_Float64x) && defined(LIBXSMM_GLIBC_FPTYPES)
#   define _Float64x _Float64
# endif
#endif

#endif /*LIBXSMM_MACROS_H*/

