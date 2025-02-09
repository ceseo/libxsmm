/******************************************************************************
** Copyright (c) 2015-2019, Intel Corporation                                **
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
#ifndef LIBXSMM_TYPEDEFS_H
#define LIBXSMM_TYPEDEFS_H

#include "libxsmm_macros.h"

/** Check ILP64 configuration for sanity. */
#if !defined(LIBXSMM_ILP64) || (0 == LIBXSMM_ILP64 && defined(MKL_ILP64))
# error "Inconsistent ILP64 configuration detected!"
#elif (0 != LIBXSMM_ILP64 && !defined(MKL_ILP64))
# define MKL_ILP64
#endif
#if (0 != LIBXSMM_ILP64)
# define LIBXSMM_BLASINT_NBITS 64
# define LIBXSMM_BLASINT long long
#else /* LP64 */
# define LIBXSMM_BLASINT_NBITS 32
# define LIBXSMM_BLASINT int
#endif

/** Generic prefetches; similar to LIBXSMM_PREFETCH_AUTO (libxsmm_frontend.h) */
#define LIBXSMM_PREFETCH_SIGONLY 1
#define LIBXSMM_PREFETCH_NONE 0

/** Helper macro for type names. */
#define LIBXSMM_TYPENAME(TYPE) LIBXSMM_STRINGIFY(LIBXSMM_CONCATENATE(LIBXSMM_TYPENAME_, TYPE))
#define LIBXSMM_TYPENAME_double f64
#define LIBXSMM_TYPENAME_float f32
#define LIBXSMM_TYPENAME_libxsmm_bfloat16 bf16
#define LIBXSMM_TYPENAME_int i32
#define LIBXSMM_TYPENAME_short i16
#define LIBXSMM_TYPENAME_char i8

/** Helper macro for type information: INFO := { FP }. */
#define LIBXSMM_TYPEINFO(TYPE, INFO) LIBXSMM_CONCATENATE3(LIBXSMM_TYPEINFO_, INFO, _, TYPE)
#define LIBXSMM_TYPEINFO_FP_double 1
#define LIBXSMM_TYPEINFO_FP_float 1
#define LIBXSMM_TYPEINFO_FP_libxsmm_bfloat16 1
#define LIBXSMM_TYPEINFO_FP_int 0
#define LIBXSMM_TYPEINFO_FP_short 0
#define LIBXSMM_TYPEINFO_FP_char 0

/** Helper macro for type postfixes. */
#define LIBXSMM_TYPESYMBOL(TYPE) LIBXSMM_CONCATENATE(LIBXSMM_TYPESYMBOL_, TYPE)
#define LIBXSMM_TYPESYMBOL_double F64
#define LIBXSMM_TYPESYMBOL_float F32
#define LIBXSMM_TYPESYMBOL_libxsmm_bfloat16 BF16
#define LIBXSMM_TYPESYMBOL_int I32
#define LIBXSMM_TYPESYMBOL_short I16
#define LIBXSMM_TYPESYMBOL_char I8

#define LIBXSMM_TYPESIZE(ENUM) ( \
  ((int)(ENUM)) == LIBXSMM_DATATYPE_F64  ? 8 : ( \
  ((int)(ENUM)) == LIBXSMM_DATATYPE_F32  ? 4 : ( \
  ((int)(ENUM)) == LIBXSMM_DATATYPE_BF16 ? 2 : ( \
  ((int)(ENUM)) == LIBXSMM_DATATYPE_I32  ? 4 : ( \
  ((int)(ENUM)) == LIBXSMM_DATATYPE_I16  ? 2 : ( \
  ((int)(ENUM)) == LIBXSMM_DATATYPE_I8   ? 1 : ( \
  0/*invalid*/)))))))

/* Get input or output precision */
#define LIBXSMM_GETENUM_INP(SRC) ((SRC) & 0x0F)
#define LIBXSMM_GETENUM_OUT(SRC) (0 == ((SRC) >> 4) ? LIBXSMM_GETENUM_INP(SRC) : ((SRC) >> 4))
/* Get/Set input and output precision */
#define LIBXSMM_GETENUM(INP, OUT) (((INP) == (OUT)) ? (INP) : ((INP) | ((OUT) << 4)))
#define LIBXSMM_SETENUM(DST, INP, OUT) DST = LIBXSMM_GETENUM(INP, OUT)

/* Construct an enumerator (libxsmm_datatype) from a built-in type (float, double, etc.). */
#define LIBXSMM_DATATYPE(TYPE) LIBXSMM_CONCATENATE(LIBXSMM_DATATYPE_, LIBXSMM_TYPESYMBOL(TYPE))
/* Construct a type-id from built-in input/output types (float, double, etc.). */
#define LIBXSMM_DATATYPE2(ITYPE, OTYPE) LIBXSMM_GETENUM(LIBXSMM_DATATYPE(ITYPE), LIBXSMM_DATATYPE(OTYPE))

/* Construct an enumerator (libxsmm_gemm_precision) from a built-in type (float, double, etc.). */
#define LIBXSMM_GEMM_PRECISION(TYPE) LIBXSMM_CONCATENATE(LIBXSMM_GEMM_PRECISION_, LIBXSMM_TYPESYMBOL(TYPE))
/* Construct GEMM-precision from built-in input/output types (float, double, etc.). */
#define LIBXSMM_GEMM_PRECISION2(ITYPE, OTYPE) (libxsmm_gemm_precision)LIBXSMM_GETENUM( \
  LIBXSMM_GEMM_PRECISION(ITYPE), LIBXSMM_GEMM_PRECISION(OTYPE))

/** Maximum size available to store a descriptor/blob (GEMM, MCOPY, TRANS, TRSM, TRMM). */
#if !defined(LIBXSMM_DESCRIPTOR_MAXSIZE)
# define LIBXSMM_DESCRIPTOR_MAXSIZE 64
#endif
/** Size of the descriptor considered as unique signature. */
#if !defined(LIBXSMM_DESCRIPTOR_SIGSIZE)
# define LIBXSMM_DESCRIPTOR_SIGSIZE LIBXSMM_DESCRIPTOR_MAXSIZE
#endif


/* Support for Bfloat16 */
typedef unsigned short libxsmm_bfloat16;

LIBXSMM_EXTERN_C typedef union LIBXSMM_RETARGETABLE libxsmm_bfloat16_hp {
  libxsmm_bfloat16 i[2];
  float f;
} libxsmm_bfloat16_hp;

#if defined(__cplusplus)
namespace tensorflow { struct bfloat16; }
#endif /*__cplusplus*/

/** Integer type for LAPACK/BLAS (LP64: 32-bit, and ILP64: 64-bit). */
typedef LIBXSMM_BLASINT libxsmm_blasint;

/** Type representing sufficient storage space for a GEMM handle. */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_gemm_blob { char data[128]; } libxsmm_gemm_blob;
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_gemm_handle libxsmm_gemm_handle;

/** Type representing sufficient storage space for descriptors (GEMM, TCOPY, MCOPY). */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_descriptor_blob {
  char data[LIBXSMM_DESCRIPTOR_MAXSIZE];
} libxsmm_descriptor_blob;

/** Structure storing arguments of GEMM-like routines. */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_gemm_descriptor libxsmm_gemm_descriptor;
/** Structure storing arguments of the matrix-copy routine. */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_mcopy_descriptor libxsmm_mcopy_descriptor;
/** Structure storing arguments of the transpose routine. */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_trans_descriptor libxsmm_trans_descriptor;
/** Structure storing arguments of packed TRSM. */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_trsm_descriptor libxsmm_trsm_descriptor;
/** Structure storing arguments of packed TRMM. */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_trmm_descriptor libxsmm_trmm_descriptor;
/** Structure storing arguments of packed GETRF. */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_getrf_descriptor libxsmm_getrf_descriptor;
/** Structure storing arguments of packed GEMM. */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_pgemm_descriptor libxsmm_pgemm_descriptor;

/** Enumerates element/data types. */
typedef enum libxsmm_datatype {
  LIBXSMM_DATATYPE_F64,
  LIBXSMM_DATATYPE_F32,
  LIBXSMM_DATATYPE_BF16,
  LIBXSMM_DATATYPE_I64,
  LIBXSMM_DATATYPE_I32,
  LIBXSMM_DATATYPE_I16,
  LIBXSMM_DATATYPE_I8,
  LIBXSMM_DATATYPE_UNSUPPORTED
} libxsmm_datatype;

/** Denotes the precision/data type of GEMM. */
typedef enum libxsmm_gemm_precision {
  LIBXSMM_GEMM_PRECISION_F64  = LIBXSMM_DATATYPE_F64,
  LIBXSMM_GEMM_PRECISION_F32  = LIBXSMM_DATATYPE_F32,
  LIBXSMM_GEMM_PRECISION_BF16 = LIBXSMM_DATATYPE_BF16,
  LIBXSMM_GEMM_PRECISION_I32  = LIBXSMM_DATATYPE_I32,
  LIBXSMM_GEMM_PRECISION_I16  = LIBXSMM_DATATYPE_I16,
  LIBXSMM_GEMM_PRECISION_I8   = LIBXSMM_DATATYPE_I8
} libxsmm_gemm_precision;

/** Flag enumeration which can be binary ORed. */
typedef enum libxsmm_gemm_flags {
  LIBXSMM_GEMM_FLAG_NONE = 0,
  /** Transpose matrix A. */
  LIBXSMM_GEMM_FLAG_TRANS_A = 1,
  /** Transpose matrix B. */
  LIBXSMM_GEMM_FLAG_TRANS_B = 2,
  /** Transpose matrix A and B. */
  LIBXSMM_GEMM_FLAG_TRANS_AB = LIBXSMM_GEMM_FLAG_TRANS_A | LIBXSMM_GEMM_FLAG_TRANS_B,
#if 0
  /** Alpha=0|1 */
  LIBXSMM_GEMM_FLAG_ALPHA_0 = 4,
  /** Alpha=neg|pos */
  LIBXSMM_GEMM_FLAG_ALPHA_S = 8,
#endif
  /** Beta=0|1 */
  LIBXSMM_GEMM_FLAG_BETA_0 = 16,
#if 0
  /** Beta=neg|pos */
  LIBXSMM_GEMM_FLAG_BETA_S = 32,
#endif
  /** Generate aligned load instructions. */
  LIBXSMM_GEMM_FLAG_ALIGN_A = 64,
  /** Aligned load/store instructions. */
  LIBXSMM_GEMM_FLAG_ALIGN_C = 128,
  /** Batch-reduce Ai * Bi. */
  LIBXSMM_GEMM_FLAG_BATCH_REDUCE = 256,
  /** Aligned C matrix, but using NTS Hint when storing */
  LIBXSMM_GEMM_FLAG_ALIGN_C_NTS_HINT = 640,
  LIBXSMM_GEMM_FLAG_ALIGN_C_NTS_HINT_BATCH_REDUCE        = LIBXSMM_GEMM_FLAG_BATCH_REDUCE | LIBXSMM_GEMM_FLAG_ALIGN_C_NTS_HINT,
  LIBXSMM_GEMM_FLAG_ALIGN_C_NTS_HINT_BETA_0              = LIBXSMM_GEMM_FLAG_BETA_0       | LIBXSMM_GEMM_FLAG_ALIGN_C_NTS_HINT,
  LIBXSMM_GEMM_FLAG_ALIGN_C_NTS_HINT_BETA_0_BATCH_REDUCE = LIBXSMM_GEMM_FLAG_BETA_0       | LIBXSMM_GEMM_FLAG_ALIGN_C_NTS_HINT | LIBXSMM_GEMM_FLAG_BATCH_REDUCE,
  /** Marker flag; do not use. */
  LIBXSMM_GEMM_FLAG_INVALID = 1024
} libxsmm_gemm_flags;

/** Flag enumeration which can be binary ORed. */
typedef enum libxsmm_gemm_handle_flags {
  LIBXSMM_GEMM_HANDLE_FLAG_AUTO   = 0,
  LIBXSMM_GEMM_HANDLE_FLAG_COPY_A = 1,
  LIBXSMM_GEMM_HANDLE_FLAG_COPY_B = 2,
  LIBXSMM_GEMM_HANDLE_FLAG_COPY_C = 4
} libxsmm_gemm_handle_flags;

/** Auto-batch flags (can be ORed) applicable to mmbatch_begin/mmbatch_end. */
typedef enum libxsmm_mmbatch_flags {
  /** Handle recorded batch unsynchronized-parallel. */
  LIBXSMM_MMBATCH_FLAG_DEFAULT      = LIBXSMM_GEMM_FLAG_INVALID * 0,
  /** Synchronize among C matrices. */
  LIBXSMM_MMBATCH_FLAG_SYNCHRONIZED = LIBXSMM_GEMM_FLAG_INVALID * 1,
  /** Handle recorded batch sequentially. */
  LIBXSMM_MMBATCH_FLAG_SEQUENTIAL   = LIBXSMM_GEMM_FLAG_INVALID * 2,
  /** Only record a statistic of potential SMMs. */
  LIBXSMM_MMBATCH_FLAG_STATISTIC    = LIBXSMM_GEMM_FLAG_INVALID * 4
} libxsmm_mmbatch_flags;

/** Enumeration of the available prefetch strategies. */
typedef enum libxsmm_gemm_prefetch_type {
  /** No prefetching and no prefetch fn. signature. */
  LIBXSMM_GEMM_PREFETCH_NONE               = LIBXSMM_PREFETCH_NONE,
  /** Only function prefetch signature. */
  LIBXSMM_GEMM_PREFETCH_SIGONLY            = LIBXSMM_PREFETCH_SIGONLY,
  /** Prefetch PA using accesses to A. */
  LIBXSMM_GEMM_PREFETCH_AL2                = 2,
  /** Prefetch PA (aggressive). */
  LIBXSMM_GEMM_PREFETCH_AL2_JPST           = 4,
  /** Prefetch PB using accesses to C. */
  LIBXSMM_GEMM_PREFETCH_BL2_VIA_C          = 8,
  /** Prefetch A ahead. */
  LIBXSMM_GEMM_PREFETCH_AL2_AHEAD          = 16,
  LIBXSMM_GEMM_PREFETCH_AL2BL2_VIA_C       = LIBXSMM_GEMM_PREFETCH_BL2_VIA_C | LIBXSMM_GEMM_PREFETCH_AL2,
  LIBXSMM_GEMM_PREFETCH_AL2BL2_VIA_C_JPST  = LIBXSMM_GEMM_PREFETCH_BL2_VIA_C | LIBXSMM_GEMM_PREFETCH_AL2_JPST,
  LIBXSMM_GEMM_PREFETCH_AL2BL2_VIA_C_AHEAD = LIBXSMM_GEMM_PREFETCH_BL2_VIA_C | LIBXSMM_GEMM_PREFETCH_AL2_AHEAD,
  /** Prefetch PA/PB/PC in L1 (using accesses to A, B, C) */
  LIBXSMM_GEMM_PREFETCH_AL1                = 32,
  LIBXSMM_GEMM_PREFETCH_BL1                = 64,
  LIBXSMM_GEMM_PREFETCH_CL1                = 128,
  LIBXSMM_GEMM_PREFETCH_AL1_BL1            = LIBXSMM_GEMM_PREFETCH_AL1 | LIBXSMM_GEMM_PREFETCH_BL1,
  LIBXSMM_GEMM_PREFETCH_BL1_CL1            = LIBXSMM_GEMM_PREFETCH_BL1 | LIBXSMM_GEMM_PREFETCH_CL1,
  LIBXSMM_GEMM_PREFETCH_AL1_CL1            = LIBXSMM_GEMM_PREFETCH_AL1 | LIBXSMM_GEMM_PREFETCH_CL1,
  LIBXSMM_GEMM_PREFETCH_AL1_BL1_CL1        = LIBXSMM_GEMM_PREFETCH_AL1_BL1 | LIBXSMM_GEMM_PREFETCH_CL1,
  /** Backward compatibility: AL2CL2BL2_VIA_C is an alias for AL2BL2_VIA_C (Eigen library). */
  LIBXSMM_PREFETCH_AL2CL2BL2_VIA_C         = LIBXSMM_GEMM_PREFETCH_AL2BL2_VIA_C
} libxsmm_gemm_prefetch_type;

/** Flag enumeration which can be binary ORed. */
typedef enum libxsmm_matcopy_flags {
  /** If set, then use zero matrix as source */
  LIBXSMM_MATCOPY_FLAG_ZERO_SOURCE = 1
} libxsmm_matcopy_flags;

/** Flag enumeration which can be binary ORed. */
typedef enum libxsmm_convolution_prefetch_type {
  /** no prefetch */
  LIBXSMM_CONVOLUTION_PREFETCH_NONE = 0,
  /** prefetch input into L1 */
  LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L1 = 1,
  /** prefetch weight into L2 */
  LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L2 = 2,
  /** prefetch output into L1 */
  LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L1 = 4,
  /** prefetch weight into L1 */
  LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L1 = 8,
  /** prefetch output into L2 */
  LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L2 = 16,
  /** prefetch input into L2 */
  LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L2 = 32,
  /** combination 1: all */
  LIBXSMM_CONVOLUTION_PREFETCH_ALL = LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L2 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L2 | LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L2,
  /** combination 2: no weight */
  LIBXSMM_CONVOLUTION_PREFETCH_NO_WEIGHT = LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L1,
  /** combination 3: no output */
  LIBXSMM_CONVOLUTION_PREFETCH_NO_OUTPUT = LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L2,
  /** combination 4: no output L2 */
  LIBXSMM_CONVOLUTION_PREFETCH_NO_OUTPUT_L2 = LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L2 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L1  | LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L2,
  /** combination 5: no input L2 */
  LIBXSMM_CONVOLUTION_PREFETCH_NO_INPUT_L2 = LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L2 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L2,
  /** combination 7: no output L2  and no input L2*/
  LIBXSMM_CONVOLUTION_PREFETCH_NO_OUTPUT_NO_INPUT_L2 = LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L2 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L1,
  /** combination 8: no output L2  and no input L2 and no weight L2*/
  LIBXSMM_CONVOLUTION_PREFETCH_NO_OUTPUT_NO_INPUT_NO_WEIGHT_L2 = LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L1,
  /** combination 9: no output L2 no weight L2 */
  LIBXSMM_CONVOLUTION_PREFETCH_NO_OUTPUT_NO_WEIGHT_L2 = LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L1  | LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L2,
  /** combination 10: no input and no output L1 */
  LIBXSMM_CONVOLUTION_PREFETCH_NO_OUTPUT_NO_INPUT_L1 = LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L2 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L2 | LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L2,
  /** combination 11: no weight L2 */
  LIBXSMM_CONVOLUTION_PREFETCH_NO_WEIGHT_L2 = LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L2 | LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L2,
  /** combination 12: no input L1 */
  LIBXSMM_CONVOLUTION_PREFETCH_NO_INPUT_L1 = LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L2 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L2 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L2,
  /** combination 12: no input L1 no weight L2*/
  LIBXSMM_CONVOLUTION_PREFETCH_NO_INPUT_L1_NO_WEIGHT_L2 = LIBXSMM_CONVOLUTION_PREFETCH_INPUT_L2 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_WEIGHT_L1 | LIBXSMM_CONVOLUTION_PREFETCH_OUTPUT_L2
} libxsmm_convolution_prefetch_type;

typedef enum libxsmm_dnn_tensor_format {
  /* use LIBXSMM internal format, we need to copy data into that */
  LIBXSMM_DNN_TENSOR_FORMAT_LIBXSMM  = 1,
  /* use NHWC format internally, this allows no-copy operations */
  LIBXSMM_DNN_TENSOR_FORMAT_NHWC     = 2,
  /* use NCHW format internally, this will include shadow copies, not preferred */
  LIBXSMM_DNN_TENSOR_FORMAT_NCHW     = 4,
  /* use RSCK format internally, this allows no-copy operations */
  LIBXSMM_DNN_TENSOR_FORMAT_RSCK     = 8,
  /* use KCRS format internally, this will include shadow copies, not preferred */
  LIBXSMM_DNN_TENSOR_FORMAT_KCRS     = 16,
  LIBXSMM_DNN_TENSOR_FORMAT_CK       = 32,
  LIBXSMM_DNN_TENSOR_FORMAT_CKPACKED = 64,
  LIBXSMM_DNN_TENSOR_FORMAT_NCPACKED = 128,
  LIBXSMM_DNN_TENSOR_FORMAT_NC       = 256
} libxsmm_dnn_tensor_format;

typedef enum libxsmm_dnn_internal_format {
  /* use LIBXSMM internal format NC_bHWc */
  LIBXSMM_DNN_TENSOR_FORMAT_LIBXSMM_1 = 1,
  /* use LIBXSMM internal format C_bN_bHWnc */
  LIBXSMM_DNN_TENSOR_FORMAT_LIBXSMM_2 = 2,
  /* use LIBXSMM internal format HWN_bC_bnc */
  LIBXSMM_DNN_TENSOR_FORMAT_LIBXSMM_3 = 3
} libxsmm_dnn_internal_format;

/** Denotes the element/pixel type of an image/channel. */
typedef enum libxsmm_dnn_datatype {
  LIBXSMM_DNN_DATATYPE_F64  = LIBXSMM_DATATYPE_F64,
  LIBXSMM_DNN_DATATYPE_F32  = LIBXSMM_DATATYPE_F32,
  LIBXSMM_DNN_DATATYPE_BF16 = LIBXSMM_DATATYPE_BF16,
  LIBXSMM_DNN_DATATYPE_I32  = LIBXSMM_DATATYPE_I32,
  LIBXSMM_DNN_DATATYPE_I16  = LIBXSMM_DATATYPE_I16,
  LIBXSMM_DNN_DATATYPE_I8   = LIBXSMM_DATATYPE_I8
} libxsmm_dnn_datatype;

typedef enum libxsmm_dnn_conv_option {
  /* we get default settings */
  LIBXSMM_DNN_CONV_OPTION_NONE = 0,
  /* overwrite results buffer (set it to zero before running the operations) */
  LIBXSMM_DNN_CONV_OPTION_OVERWRITE = 1,
  /* activations are stored unsigned */
  /* @TODO check if we still need this option */
  LIBXSMM_DNN_CONV_OPTION_ACTIVATION_UNSIGNED = 2,
  /* reduce filters externally to op */
  LIBXSMM_DNN_CONV_OPTION_UPD_NO_FILTER_REDUCE = 4,
  /* external filter transpose to bwd convolutions */
  LIBXSMM_DNN_CONV_OPTION_BWD_NO_FILTER_TRANSPOSE = 8,
  /* external filter transpose to bwd convolutions */
  LIBXSMM_DNN_CONV_OPTION_UPD_NO_INPUT_TRANSPOSE = 16,
  /* Down-convert for BF16 using RNE rounding */
  LIBXSMM_DNN_CONV_OPTION_F32_BF16_CVT_RNE = 32,
  /* compound types */
  LIBXSMM_DNN_CONV_OPTION_F32_BF16_CVT_RNE_OVERWRITE = LIBXSMM_DNN_CONV_OPTION_OVERWRITE | LIBXSMM_DNN_CONV_OPTION_F32_BF16_CVT_RNE,
  LIBXSMM_DNN_CONV_OPTION_ACTIVATION_UNSIGNED_OVERWRITE = LIBXSMM_DNN_CONV_OPTION_ACTIVATION_UNSIGNED | LIBXSMM_DNN_CONV_OPTION_OVERWRITE,
  LIBXSMM_DNN_CONV_OPTION_UPD_NO_FILTER_REDUCE_OVERWRITE = LIBXSMM_DNN_CONV_OPTION_UPD_NO_FILTER_REDUCE | LIBXSMM_DNN_CONV_OPTION_OVERWRITE,
  LIBXSMM_DNN_CONV_OPTION_BWD_NO_FILTER_TRANSPOSE_OVERWRITE = LIBXSMM_DNN_CONV_OPTION_OVERWRITE | LIBXSMM_DNN_CONV_OPTION_BWD_NO_FILTER_TRANSPOSE,
  LIBXSMM_DNN_CONV_OPTION_UPD_NO_INPUT_TRANSPOSE_OVERWRITE = LIBXSMM_DNN_CONV_OPTION_OVERWRITE | LIBXSMM_DNN_CONV_OPTION_UPD_NO_INPUT_TRANSPOSE,
  LIBXSMM_DNN_CONV_OPTION_NO_TRANSPOSES_OVERWRITE = LIBXSMM_DNN_CONV_OPTION_OVERWRITE | LIBXSMM_DNN_CONV_OPTION_BWD_NO_FILTER_TRANSPOSE | LIBXSMM_DNN_CONV_OPTION_UPD_NO_INPUT_TRANSPOSE
} libxsmm_dnn_conv_option;

typedef enum libxsmm_dnn_fusedbatchnorm_fuse_order {
  /* the fuse order is: 1. BN, 2. element-wise 3. RELU */
  LIBXSMM_DNN_FUSEDBN_ORDER_BN_ELTWISE_RELU = 0
} libxsmm_dnn_fusedbatchnorm_fuse_order;

typedef enum libxsmm_dnn_fusedbatchnorm_fuse_op {
  /* the fuse order is: 1. BN, 2. element-wise 3. RELU */
  LIBXSMM_DNN_FUSEDBN_OPS_BN = 1,
  LIBXSMM_DNN_FUSEDBN_OPS_BNSCALE = 2,
  LIBXSMM_DNN_FUSEDBN_OPS_ELTWISE = 4,
  LIBXSMM_DNN_FUSEDBN_OPS_RELU = 8,
  LIBXSMM_DNN_FUSEDBN_OPS_BN_ELTWISE = LIBXSMM_DNN_FUSEDBN_OPS_BN | LIBXSMM_DNN_FUSEDBN_OPS_ELTWISE,
  LIBXSMM_DNN_FUSEDBN_OPS_BN_RELU = LIBXSMM_DNN_FUSEDBN_OPS_BN | LIBXSMM_DNN_FUSEDBN_OPS_RELU,
  LIBXSMM_DNN_FUSEDBN_OPS_BN_ELTWISE_RELU = LIBXSMM_DNN_FUSEDBN_OPS_BN | LIBXSMM_DNN_FUSEDBN_OPS_ELTWISE | LIBXSMM_DNN_FUSEDBN_OPS_RELU,
  LIBXSMM_DNN_FUSEDBN_OPS_BNSCALE_ELTWISE = LIBXSMM_DNN_FUSEDBN_OPS_BNSCALE | LIBXSMM_DNN_FUSEDBN_OPS_ELTWISE,
  LIBXSMM_DNN_FUSEDBN_OPS_BNSCALE_RELU = LIBXSMM_DNN_FUSEDBN_OPS_BNSCALE | LIBXSMM_DNN_FUSEDBN_OPS_RELU,
  LIBXSMM_DNN_FUSEDBN_OPS_BNSCALE_ELTWISE_RELU = LIBXSMM_DNN_FUSEDBN_OPS_BNSCALE | LIBXSMM_DNN_FUSEDBN_OPS_ELTWISE | LIBXSMM_DNN_FUSEDBN_OPS_RELU
} libxsmm_dnn_fusedbatchnorm_fuse_op;

LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_dnn_fusedbatchnorm_desc {
  int N;                                     /* number of images in mini-batch */
  int C;                                     /* number of input feature maps */
  int H;                                     /* height of input image */
  int W;                                     /* width of input image */
  int u;                                     /* vertical stride */
  int v;                                     /* horizontal stride */
  int pad_h_in;                              /* height of physical zero-padding in input buffer */
  int pad_w_in;                              /* width of physical zero-padding in input buffer */
  int pad_h_out;                             /* height of physical zero-padding in output buffer */
  int pad_w_out;                             /* width of physical zero-padding in output buffer */
  int threads;                               /* number of threads used */
  libxsmm_dnn_datatype datatype_in;          /* datatype used for all input related buffers */
  libxsmm_dnn_datatype datatype_out;         /* datatype used for all output related buffers */
  libxsmm_dnn_datatype datatype_stats;       /* datatype used for all stats related buffers */
  libxsmm_dnn_tensor_format buffer_format;   /* format which is for activation buffers */
  libxsmm_dnn_fusedbatchnorm_fuse_order fuse_order; /* additional options */
  libxsmm_dnn_fusedbatchnorm_fuse_op fuse_ops;      /* used ops into convolutions */
} libxsmm_dnn_fusedbatchnorm_desc;

/** Structure storing the convolution argument description. */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_MAY_ALIAS libxsmm_convolution_forward_descriptor {
  unsigned int kh;                              /* kernel height */
  unsigned int kw;                              /* kernel width */
  unsigned int unroll_kh;                       /* kernel height, unrolled */
  unsigned int unroll_kw;                       /* kernel width, unrolled */
  unsigned int blocks_ofm;
  unsigned int blocks_ifm;
  unsigned int blocks_ifm_blocking;
  unsigned int ofm_block;                       /* should be VLEN */
  unsigned int ifm_block;                       /* should be VLEN */
  unsigned int ifm_block_hp;
  unsigned int ofm_block_lp;
  unsigned int ofh_padded;                      /* this we need for 2D register block */
  unsigned int ofw_padded;                      /* this we use for 1D and 2D register block */
  unsigned int ofh_rb;                          /* UR, register block of ofh */
  unsigned int ofw_rb;                          /* UR, register block of ofw */
  unsigned int ifh_padded;                      /* this we need for 2D register block */
  unsigned int ifw_padded;                      /* this we use for 1D and 2D register block */
  unsigned int stride_h;                        /* this we use for offsets in the input */
  unsigned int stride_w;                        /* this we use for offsets in the input */
  unsigned int fm_lp_block;                     /* additional blocking for low precision datatypes of ifm */
  unsigned int use_nts;                         /* non-zero if intent is to overwrite the output buffer using streaming stores */
  unsigned int weight_stride;
  unsigned int use_fwd_generator_for_bwd;
  unsigned int stride_h_store;
  unsigned int stride_w_store;
  unsigned int extra_L2_prefetching;
  unsigned int input_L2_prefetching;
  unsigned int lookahead;
  unsigned int compute_batch_stats_fwd;
  unsigned int compute_batch_stats_bwd;
  unsigned int compute_eltwise;
  libxsmm_dnn_fusedbatchnorm_desc* pre_bn;
  libxsmm_dnn_fusedbatchnorm_desc* post_bn;
  unsigned int compute_max;
  unsigned int perform_relu_in_kernel;
  unsigned int n_variants;
  unsigned int f32_bf16_cvt_rne;                /* non-zero if in case of bf16 we perform RNE rounding when converting down from f32 in JIT sequence */
  libxsmm_dnn_tensor_format format;
  libxsmm_dnn_conv_option option;
  libxsmm_dnn_datatype datatype;
  libxsmm_dnn_datatype datatype_itm;
  libxsmm_convolution_prefetch_type prefetch;   /* prefetch type, can be ORed vales of libxsmm_convolution_prefetch_type */
} libxsmm_convolution_forward_descriptor;

/** Backward convolution argument descriptor (similar to forward descriptor). */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_MAY_ALIAS libxsmm_convolution_backward_descriptor {
  unsigned int kh;                              /* kernel height */
  unsigned int kw;                              /* kernel width */
  unsigned int unroll_kh;                       /* kernel height, unrolled */
  unsigned int unroll_kw;                       /* kernel width, unrolled */
  unsigned int blocks_ofm;
  unsigned int blocks_ifm;
  unsigned int ofm_block;                       /* should be VLEN */
  unsigned int ifm_block;                       /* should be VLEN */
  unsigned int ofh_padded;                      /* this we need for 2D register block */
  unsigned int ofw_padded;                      /* this we use for 1D and 2D register block */
  unsigned int ofh_rb;                          /* UR, register block of ofh */
  unsigned int ofw_rb;                          /* UR, register block of ofw */
  unsigned int ifh_padded;                      /* this we need for 2D register block */
  unsigned int ifw_padded;                      /* this we use for 1D and 2D register block */
  unsigned int stride_h;                        /* this we use for offsets in the input */
  unsigned int stride_w;                        /* this we use for offsets in the input */
  unsigned int ofw;
  unsigned int fm_lp_block;                    /* additional blocking for low precision datatypes of ifm */
  libxsmm_dnn_tensor_format format;
  libxsmm_dnn_conv_option option;
  libxsmm_dnn_datatype datatype;
  libxsmm_dnn_datatype datatype_itm;
  libxsmm_convolution_prefetch_type prefetch;   /* prefetch type, can be ORed vales of libxsmm_convolution_prefetch_type */
} libxsmm_convolution_backward_descriptor;

/** Structure storing the convolution weight update argument description. */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_MAY_ALIAS libxsmm_convolution_weight_update_descriptor {
  unsigned int kw;                              /* kernel width */
  unsigned int kh;                              /* kernel height */
  unsigned int blocks_ofm;
  unsigned int blocks_ifm;
  unsigned int ofm_block;                       /* should be VLEN */
  unsigned int ifm_block;                       /* should be VLEN */
  unsigned int ifm_block_hp;
  unsigned int ofm_block_lp;
  unsigned int ofh_padded;                      /* this we need for 2D register block */
  unsigned int ofw_padded;                      /* this we use for 1D and 2D register block */
  unsigned int ofh_rb;                          /* UR, register block of ofh */
  unsigned int ofw_rb;                          /* UR, register block of ofw */
  unsigned int ifh_padded;                      /* this we need for 2D register block */
  unsigned int ifw_padded;                      /* this we use for 1D and 2D register block */
  unsigned int stride_h;                        /* this we use for offsets in the input */
  unsigned int stride_w;                        /* this we use for offsets in the input */
  unsigned int fm_lp_block;
  unsigned int ifm_unroll;                      /* this we use to unroll ifm loop */
  unsigned int ofh;                             /* upper bound of oj loop */
  unsigned int ofh_unroll;                      /* this we use to unroll ofh loop */
  unsigned int ofw;                             /* upper bound of oi loop */
  unsigned int ofw_unroll;                      /* this we use to unroll ofw loop */
  unsigned int blocks_h;
  unsigned int blocks_img;
  unsigned int use_nts;
  unsigned int transpose_ofw_ifm;               /* transpose ofw and ifm */
  unsigned int ofw_fake_pixels;
  unsigned int use_fastpath;
  unsigned int ncopies;                         /* number of reduction copies, probably nthreads */
  unsigned int avoid_output_trans;
  unsigned int f32_bf16_cvt_rne;                /* non-zero if in case of bf16 we perform RNE rounding when converting down from f32 in JIT sequence */
  libxsmm_dnn_tensor_format format;
  libxsmm_dnn_conv_option option;
  libxsmm_dnn_datatype datatype;
  libxsmm_dnn_datatype datatype_itm;
  libxsmm_convolution_prefetch_type prefetch;   /* prefetch type, can be ORed vales of libxsmm_convolution_prefetch_type */
} libxsmm_convolution_weight_update_descriptor;

/** Specialized function with fused alpha and beta arguments, and optional prefetch locations (double-precision). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_dmmfunction)(const double* a, const double* b, double* c, ...);
/** Specialized function with fused alpha and beta arguments, and optional prefetch locations (single-precision). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_smmfunction)(const float* a, const float* b, float* c, ...);
/** Specialized function with fused alpha and beta arguments, and optional prefetch locations (low-precision). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_wimmfunction)(const short* a, const short* b, int* c, ...);
/** Specialized function with fused alpha and beta arguments, and optional prefetch locations (low-precision). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_wsmmfunction)(const short* a, const short* b, float* c, ...);
/** Specialized function with fused alpha and beta arguments, and optional prefetch locations (bf16, fp32-accumulate). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_bsmmfunction)(const libxsmm_bfloat16* a, const libxsmm_bfloat16* b, float* c, ...);
/** Specialized function with fused alpha and beta arguments, and optional prefetch locations (bf16, fp32-accumulate). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_bmmfunction)(const libxsmm_bfloat16* a, const libxsmm_bfloat16* b, libxsmm_bfloat16* c, ...);

LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_dmmfunction_reducebatch)(const double** a, const double** b, double* c, const unsigned long long* count, ...);
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_smmfunction_reducebatch)(const float** a, const float** b, float* c, const unsigned long long* count, ...);
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_bsmmfunction_reducebatch)(const libxsmm_bfloat16** a, const libxsmm_bfloat16** b, float* c, const unsigned long long* count, ...);
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_bmmfunction_reducebatch)(const libxsmm_bfloat16** a, const libxsmm_bfloat16** b, libxsmm_bfloat16* c, const unsigned long long* count, ...);

/** Function type which is either libxsmm_smmfunction or libxsmm_dmmfunction (weak-typed). */
LIBXSMM_EXTERN_C typedef union LIBXSMM_RETARGETABLE libxsmm_xmmfunction {
  void (*xmm)(const void* a, const void* b, void* c, ...);
  void (*xbm)(const void** a, const void** b, void* c, const unsigned long long* count, ...);
  libxsmm_dmmfunction dmm; libxsmm_smmfunction smm; libxsmm_wimmfunction wimm; libxsmm_wsmmfunction wsmm; libxsmm_bsmmfunction bsmm; libxsmm_bmmfunction bmm;
  libxsmm_dmmfunction_reducebatch dmr; libxsmm_smmfunction_reducebatch smr; libxsmm_bsmmfunction_reducebatch bsmr; libxsmm_bmmfunction_reducebatch bmr;
} libxsmm_xmmfunction;

/** Determines the kernel kind. */
typedef enum libxsmm_kernel_kind {
  /** Matrix multiplication kernel */
  LIBXSMM_KERNEL_KIND_MATMUL  = 0,
  /** Matcopy kernel kind */
  LIBXSMM_KERNEL_KIND_MCOPY   = 1,
  /** Transpose kernel kind */
  LIBXSMM_KERNEL_KIND_TRANS   = 2,
  /** GEMM/packed kernel kind */
  LIBXSMM_KERNEL_KIND_PGEMM   = 3,
  /** GEMM/packed kernel kind */
  LIBXSMM_KERNEL_KIND_GETRF   = 4,
  /** TRMM kernel kind */
  LIBXSMM_KERNEL_KIND_TRMM    = 5,
  /** TRSM kernel kind */
  LIBXSMM_KERNEL_KIND_TRSM    = 6,
  /** Not a JIT kernel */
  LIBXSMM_KERNEL_KIND_INVALID = 7
} libxsmm_kernel_kind;

/** Specialized function for matrix-copy (weak-typed). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_xmcopyfunction)(
  const void* in, const unsigned int* ldi, void* out, const unsigned int* ldo, ...);

/** Specialized function for transpose (weak-typed). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_xtransfunction)(
  const void* in, const unsigned int* ldi, void* out, const unsigned int* ldo);

/** Specialized function for packed GEMM (weak-typed). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_pgemm_xfunction)(
  const void* a, const void* b, void* c);

/** Specialized function for packed GEMM (weak-typed). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_getrf_xfunction)(
  const void* a, const void* b, void* c);

/** Specialized function for TRMM (weak-typed). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_trmm_xfunction)(
  const void* a, const void* b, void* c);

/** Specialized function for TRSM (weak-typed). */
LIBXSMM_EXTERN_C typedef LIBXSMM_RETARGETABLE void (*libxsmm_trsm_xfunction)(
  const void* a, const void* b, void* c);

/** Structure to receive information about GEMM-kernels (libxsmm_get_mmkernel_info). */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_mmkernel_info {
  /** Input/output data-type */
  libxsmm_gemm_precision iprecision, oprecision;
  /** Prefetch strategy. */
  libxsmm_gemm_prefetch_type prefetch;
  /** Leading dimensions. */
  unsigned int lda, ldb, ldc;
  /** Extents/shape. */
  unsigned int m, n, k;
  /** Set of flags. */
  int flags;
} libxsmm_mmkernel_info;

/** Structure to receive information about transpose-kernels (libxsmm_get_transkernel_info). */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_transkernel_info {
  /** LD, M, and N. */
  unsigned int ldo, m, n;
  /** Size of data element. */
  unsigned int typesize;
} libxsmm_transkernel_info;

/** Structure to receive information about matrix-copy kernels (libxsmm_get_mcopykernel_info). */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_mcopykernel_info {
  /** LDx, M, and N. */
  unsigned int ldi, ldo, m, n;
  /** Size of data element. */
  unsigned int typesize;
  /** Boolean value. */
  int prefetch;
  /** Set of flags. */
  int flags;
} libxsmm_mcopykernel_info;

/** Structure to receive information about the code registry status (libxsmm_get_registry_info). */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_registry_info {
  size_t capacity, size, nbytes, nstatic, ncache;
} libxsmm_registry_info;

#endif /*LIBXSMM_TYPEDEFS_H*/

