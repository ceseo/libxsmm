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
/* Alexander Heinecke, Hans Pabst (Intel Corp.)
******************************************************************************/
#ifndef LIBXSMM_DNN_H
#define LIBXSMM_DNN_H

#include "libxsmm_typedefs.h"

#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(push,target(LIBXSMM_OFFLOAD_TARGET))
#endif
#include <stdlib.h>
#if !defined(NDEBUG)
# include <stdio.h>
#endif
#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(pop)
#endif

/** Opaque handles which represents convolutions and LIBXSMM datatypes */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_dnn_layer libxsmm_dnn_layer;
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_dnn_tensor libxsmm_dnn_tensor;
/** Opaque handles which represents LIBXSMM fusedbatchnorm */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_dnn_fusedbatchnorm libxsmm_dnn_fusedbatchnorm;
typedef unsigned int libxsmm_dnn_err_t;

/** Define error and warning codes */
#define LIBXSMM_DNN_SUCCESS                             0

#define LIBXSMM_DNN_WARN_FALLBACK                   90000
#define LIBXSMM_DNN_WARN_RNN_SUBOPTIMAL_N_BLOCKING  90001
#define LIBXSMM_DNN_WARN_RNN_SUBOPTIMAL_C_BLOCKING  90002
#define LIBXSMM_DNN_WARN_RNN_SUBOPTIMAL_K_BLOCKING  90003
#define LIBXSMM_DNN_WARN_FC_SUBOPTIMAL_N_BLOCKING   90004
#define LIBXSMM_DNN_WARN_FC_SUBOPTIMAL_C_BLOCKING   90005
#define LIBXSMM_DNN_WARN_FC_SUBOPTIMAL_K_BLOCKING   90006

#define LIBXSMM_DNN_ERR_GENERAL                    100000
#define LIBXSMM_DNN_ERR_CREATE_HANDLE              100001
#define LIBXSMM_DNN_ERR_UNSUPPORTED_DATATYPE       100002
#define LIBXSMM_DNN_ERR_INVALID_BLOCKING           100003
#define LIBXSMM_DNN_ERR_INVALID_HANDLE             100004
#define LIBXSMM_DNN_ERR_DATA_NOT_BOUND             100005
#define LIBXSMM_DNN_ERR_CREATE_TENSOR              100006
#define LIBXSMM_DNN_ERR_INVALID_TENSOR             100007
#define LIBXSMM_DNN_ERR_MISMATCH_TENSOR            100008
#define LIBXSMM_DNN_ERR_INVALID_HANDLE_TENSOR      100009
#define LIBXSMM_DNN_ERR_INVALID_KIND               100010
#define LIBXSMM_DNN_ERR_INVALID_FORMAT_NCHW        100011
#define LIBXSMM_DNN_ERR_UNSUPPORTED_DST_FORMAT     100012
#define LIBXSMM_DNN_ERR_UNSUPPORTED_SRC_FORMAT     100013
#define LIBXSMM_DNN_ERR_INVALID_FORMAT_CONVOLVE    100014
#define LIBXSMM_DNN_ERR_INVALID_FORMAT_KCRS        100015
#define LIBXSMM_DNN_ERR_INVALID_FORMAT_GENERAL     100016
#define LIBXSMM_DNN_ERR_CREATE_LAYOUT              100017
#define LIBXSMM_DNN_ERR_INVALID_LAYOUT             100018
#define LIBXSMM_DNN_ERR_UNSUPPORTED_ARCH           100019
#define LIBXSMM_DNN_ERR_SCRATCH_NOT_ALLOCED        100020
#define LIBXSMM_DNN_ERR_UNKNOWN_TENSOR_TYPE        100021
#define LIBXSMM_DNN_ERR_INVALID_ALGO               100022
#define LIBXSMM_DNN_ERR_INVALID_PADDING            100023
#define LIBXSMM_DNN_ERR_UNKNOWN_BIAS_TYPE          100024
#define LIBXSMM_DNN_ERR_MISMATCH_BIAS              100025
#define LIBXSMM_DNN_ERR_INVALID_HANDLE_BIAS        100026
#define LIBXSMM_DNN_ERR_TIME_STEPS_TOO_SMALL       100027
#define LIBXSMM_DNN_ERR_CREATE_LAYOUT_ARRAYS       100028
#define LIBXSMM_DNN_ERR_NOT_IMPLEMENTED            100029
#define LIBXSMM_DNN_ERR_FUSEBN_UNSUPPORTED_ORDER   100030
#define LIBXSMM_DNN_ERR_FUSEBN_UNSUPPORTED_FUSION  100031
#define LIBXSMM_DNN_ERR_INVALID_FORMAT_FUSEDBN     100032
#define LIBXSMM_DNN_ERR_UNSUPPORTED_POOLING        100033
#define LIBXSMM_DNN_ERR_INVALID_FORMAT_FC          100034
#define LIBXSMM_DNN_ERR_INVALID_RNN_TYPE           100035
#define LIBXSMM_DNN_ERR_RNN_INVALID_SEQ_LEN        100036

/** Kinds of supported compute flavor operations. */
typedef enum libxsmm_dnn_compute_kind {
  /** Forward path */
  LIBXSMM_DNN_COMPUTE_KIND_FWD,
  /** Backward path */
  LIBXSMM_DNN_COMPUTE_KIND_BWD,
  /** Updated weights. */
  LIBXSMM_DNN_COMPUTE_KIND_UPD,
  /** Backward and weightupdate combined, useful for RNNs */
  LIBXSMM_DNN_COMPUTE_KIND_BWDUPD,
  /** All routines, need for some init routines. */
  LIBXSMM_DNN_COMPUTE_KIND_ALL
} libxsmm_dnn_compute_kind;

/** type/meaning of dimension in a LIBXSMM DNN tensor */
typedef enum libxsmm_dnn_tensor_dimtype {
  /** Mini-batch */
  LIBXSMM_DNN_TENSOR_DIMTYPE_N,
  /** Image Height */
  LIBXSMM_DNN_TENSOR_DIMTYPE_H,
  /** Image Width */
  LIBXSMM_DNN_TENSOR_DIMTYPE_W,
  /** channels or input channels */
  LIBXSMM_DNN_TENSOR_DIMTYPE_C,
  /** output channels */
  LIBXSMM_DNN_TENSOR_DIMTYPE_K,
  /** kernel height */
  LIBXSMM_DNN_TENSOR_DIMTYPE_R,
  /** kernel width */
  LIBXSMM_DNN_TENSOR_DIMTYPE_S,
  /** sequence lenth counter */
  LIBXSMM_DNN_TENSOR_DIMTYPE_T,
  /** general counter */
  LIBXSMM_DNN_TENSOR_DIMTYPE_X
} libxsmm_dnn_tensor_dimtype;

/** types of different buffers */
typedef enum libxsmm_dnn_tensor_type {
  /** regular input buffer */
  LIBXSMM_DNN_REGULAR_INPUT,
  /** regular input buffer */
  LIBXSMM_DNN_REGULAR_INPUT_ADD,
  /** regular input buffer, transpose */
  LIBXSMM_DNN_REGULAR_INPUT_TRANS,
  /** gradient input buffer */
  LIBXSMM_DNN_GRADIENT_INPUT,
  /** gradient input buffer */
  LIBXSMM_DNN_GRADIENT_INPUT_ADD,
  /** regular output buffer */
  LIBXSMM_DNN_REGULAR_OUTPUT,
  /** gradient output buffer */
  LIBXSMM_DNN_GRADIENT_OUTPUT,
  /** general input type */
  LIBXSMM_DNN_INPUT,
  /** general output type */
  LIBXSMM_DNN_OUTPUT,
  /** general activation type */
  LIBXSMM_DNN_ACTIVATION,
  /* regular filter */
  LIBXSMM_DNN_REGULAR_FILTER,
  /* regular filter */
  LIBXSMM_DNN_REGULAR_FILTER_TRANS,
  /* gradient filter */
  LIBXSMM_DNN_GRADIENT_FILTER,
  /** general filter type */
  LIBXSMM_DNN_FILTER,
  /* regular bias */
  LIBXSMM_DNN_REGULAR_CHANNEL_BIAS,
  /* gradient bias */
  LIBXSMM_DNN_GRADIENT_CHANNEL_BIAS,
  /* bias */
  LIBXSMM_DNN_CHANNEL_BIAS,
  /* regular beta */
  LIBXSMM_DNN_REGULAR_CHANNEL_BETA,
  /* gradient beta */
  LIBXSMM_DNN_GRADIENT_CHANNEL_BETA,
  /* beta */
  LIBXSMM_DNN_CHANNEL_BETA,
  /* regular gamma */
  LIBXSMM_DNN_REGULAR_CHANNEL_GAMMA,
  /* gradient gamma */
  LIBXSMM_DNN_GRADIENT_CHANNEL_GAMMA,
  /* Gamma */
  LIBXSMM_DNN_CHANNEL_GAMMA,
  /* regular beta */
  LIBXSMM_DNN_CHANNEL_EXPECTVAL,
  /* regular beta */
  LIBXSMM_DNN_CHANNEL_RCPSTDDEV,
  /* variance */
  LIBXSMM_DNN_CHANNEL_VARIANCE,
  /** general bias type */
  LIBXSMM_DNN_CHANNEL_SCALAR,
  /** batch stats */
  LIBXSMM_DNN_BATCH_STATS,
  LIBXSMM_DNN_MAX_STATS_FWD,
  LIBXSMM_DNN_MAX_STATS_BWD,
  LIBXSMM_DNN_MAX_STATS_UPD,
  /** pooling mask */
  LIBXSMM_DNN_POOLING_MASK,
   /** general type, if needed might cause API issues in copy in/out API */
  LIBXSMM_DNN_TENSOR,

  /** regular input buffer */
  LIBXSMM_DNN_RNN_REGULAR_INPUT,
  /** regular previous cell state buffer */
  LIBXSMM_DNN_RNN_REGULAR_CS_PREV,
  /** regular previous hidden state buffer */
  LIBXSMM_DNN_RNN_REGULAR_HIDDEN_STATE_PREV,
  /** regular weight (LSTM: wi, wc, wf, wo) */
  LIBXSMM_DNN_RNN_REGULAR_WEIGHT,
  /** regular recurrent weight (LSTM: ri, rc, rf, ro) */
  LIBXSMM_DNN_RNN_REGULAR_RECUR_WEIGHT,
  /** regular weight (LSTM: wi, wc, wf, wo) */
  LIBXSMM_DNN_RNN_REGULAR_WEIGHT_TRANS,
  /** regular recurrent weight (LSTM: ri, rc, rf, ro) */
  LIBXSMM_DNN_RNN_REGULAR_RECUR_WEIGHT_TRANS,
  /** regular bias (LSTM: bi, bc, bf, bo) */
  LIBXSMM_DNN_RNN_REGULAR_BIAS,
  /** regular output cell state buffer */
  LIBXSMM_DNN_RNN_REGULAR_CS,
  /** regular hidden state buffer */
  LIBXSMM_DNN_RNN_REGULAR_HIDDEN_STATE,
  /** gradient input buffer */
  LIBXSMM_DNN_RNN_GRADIENT_INPUT,
  /** gradient previous cell state buffer */
  LIBXSMM_DNN_RNN_GRADIENT_CS_PREV,
  /** gradient previous hidden state buffer */
  LIBXSMM_DNN_RNN_GRADIENT_HIDDEN_STATE_PREV,
  /** gradient weight */
  LIBXSMM_DNN_RNN_GRADIENT_WEIGHT,
  /** gradient recurrent weight */
  LIBXSMM_DNN_RNN_GRADIENT_RECUR_WEIGHT,
  /** gradient bias */
  LIBXSMM_DNN_RNN_GRADIENT_BIAS,
  /** gradient output cell state buffer */
  LIBXSMM_DNN_RNN_GRADIENT_CS,
  /** gradient hidden state buffer */
  LIBXSMM_DNN_RNN_GRADIENT_HIDDEN_STATE,
  /** internal i buffer */
  LIBXSMM_DNN_RNN_INTERNAL_I,
  /** internal f buffer */
  LIBXSMM_DNN_RNN_INTERNAL_F,
  /** internal o buffer */
  LIBXSMM_DNN_RNN_INTERNAL_O,
  /** internal ci buffer */
  LIBXSMM_DNN_RNN_INTERNAL_CI,
  /** internal co buffer */
  LIBXSMM_DNN_RNN_INTERNAL_CO
} libxsmm_dnn_tensor_type;

/** layout descriptor to allow external data handling
    outside of LIBXSMM */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_dnn_tensor_datalayout {
  libxsmm_dnn_tensor_dimtype* dim_type;
  unsigned int* dim_size;
  unsigned int num_dims;
  libxsmm_dnn_tensor_format format;                /* format of activation buffer */
  libxsmm_dnn_internal_format custom_format;       /* internal classifier of format, an internal subgroup */
  libxsmm_dnn_datatype datatype;                   /* data type */
  libxsmm_dnn_tensor_type tensor_type;             /* tensor type */
} libxsmm_dnn_tensor_datalayout;

typedef enum libxsmm_dnn_conv_fuse_op {
  /* we fuse nothing into convolution */
  LIBXSMM_DNN_CONV_FUSE_NONE = 0,
  /* we fuse bias addition into convolution */
  LIBXSMM_DNN_CONV_FUSE_BIAS = 1,
  /* we fuse ReLU calculation into fwd convolution op */
  LIBXSMM_DNN_CONV_FUSE_RELU_FWD = 2,
  /* we fuse ReLU calculation into bwd convolution op */
  LIBXSMM_DNN_CONV_FUSE_RELU_BWD = 4,
  /* we fuse batch stats */
  LIBXSMM_DNN_CONV_FUSE_BATCH_STATS_FWD = 8,
  LIBXSMM_DNN_CONV_FUSE_MAX_STATS = 16,
  LIBXSMM_DNN_CONV_FUSE_BATCH_STATS_BWD = 32,
  LIBXSMM_DNN_CONV_FUSE_ELTWISE_BWD = 64,
  LIBXSMM_DNN_CONV_FUSE_BATCHNORM_STATS = 128,
  LIBXSMM_DNN_CONV_FUSE_BATCH_STATS_FWD_RELU_BWD = LIBXSMM_DNN_CONV_FUSE_RELU_BWD | LIBXSMM_DNN_CONV_FUSE_BATCH_STATS_FWD,
  LIBXSMM_DNN_CONV_FUSE_BATCH_STATS_FWD_RELU_BWD_AND_MAX = LIBXSMM_DNN_CONV_FUSE_BATCH_STATS_FWD_RELU_BWD | LIBXSMM_DNN_CONV_FUSE_MAX_STATS,
  LIBXSMM_DNN_CONV_FUSE_BATCH_STATS_FWD_AND_MAX = LIBXSMM_DNN_CONV_FUSE_BATCH_STATS_FWD |  LIBXSMM_DNN_CONV_FUSE_MAX_STATS,
  LIBXSMM_DNN_CONV_FUSE_RELU_BWD_AND_MAX = LIBXSMM_DNN_CONV_FUSE_RELU_BWD | LIBXSMM_DNN_CONV_FUSE_MAX_STATS,
    /* we fuse bias addition and ReLU into convolution op */
  LIBXSMM_DNN_CONV_FUSE_RELU = LIBXSMM_DNN_CONV_FUSE_RELU_FWD | LIBXSMM_DNN_CONV_FUSE_RELU_BWD,
  LIBXSMM_DNN_CONV_FUSE_BIAS_RELU = LIBXSMM_DNN_CONV_FUSE_BIAS | LIBXSMM_DNN_CONV_FUSE_RELU
} libxsmm_dnn_conv_fuse_op;

/** Type of algorithm used for convolutions. */
typedef enum libxsmm_dnn_conv_algo {
  /** let the library decide */
  LIBXSMM_DNN_CONV_ALGO_AUTO,
  /** direct convolution. */
  LIBXSMM_DNN_CONV_ALGO_DIRECT
} libxsmm_dnn_conv_algo;

/** Structure which describes the input and output of data (DNN). */
LIBXSMM_EXTERN_C typedef struct LIBXSMM_RETARGETABLE libxsmm_dnn_conv_desc {
  int N;                                    /* number of images in mini-batch */
  int C;                                    /* number of input feature maps */
  int H;                                    /* height of input image */
  int W;                                    /* width of input image */
  int K;                                    /* number of output feature maps */
  int R;                                    /* height of filter kernel */
  int S;                                    /* width of filter kernel */
  int u;                                    /* vertical stride */
  int v;                                    /* horizontal stride */
  int pad_h;                                /* height of logical rim padding to input
                                               for adjusting output height */
  int pad_w;                                /* width of logical rim padding to input
                                               for adjusting output width */
  int pad_h_in;                             /* height of zero-padding in input buffer,
                                               must equal to pad_h for direct conv */
  int pad_w_in;                             /* width of zero-padding in input buffer,
                                               must equal to pad_w for direct conv */
  int pad_h_out;                            /* height of zero-padding in output buffer */
  int pad_w_out;                            /* width of zero-padding in output buffer */
  int threads;                              /* number of threads to use when running
                                               convolution */
  libxsmm_dnn_datatype datatype_in;         /* datatypes used for all input related buffer */
  libxsmm_dnn_datatype datatype_out;        /* datatypes used for all output related buffer */
  libxsmm_dnn_tensor_format buffer_format;  /* format which is for buffer buffers */
  libxsmm_dnn_tensor_format filter_format;  /* format which is for filter buffers */
  libxsmm_dnn_conv_algo algo;               /* convolution algorithm used */
  libxsmm_dnn_conv_option options;          /* additional options */
  libxsmm_dnn_conv_fuse_op fuse_ops;        /* used ops into convolutions */
  libxsmm_dnn_fusedbatchnorm *pre_bn;       /* pointer to pre bn layer to accommodate bn fusion */
  libxsmm_dnn_fusedbatchnorm *post_bn;      /* pointer to post bn layer to accommodate bn fusion */
} libxsmm_dnn_conv_desc;

/** these are some quantization definitions, not sure if we want to
    move them into some main part of LIBXSMM */
/* @TODO check position of these declarations and defines */
typedef union LIBXSMM_RETARGETABLE libxsmm_intfloat {
  unsigned int ui;
  float f;
} libxsmm_intfloat;

/* F32 masking defines */
#define LIBXSNN_DNN_MASK_SIGN_F32      0x80000000
#define LIBXSMM_DNN_MASK_EXP_F32       0x7f800000
#define LIBXSMM_DNN_MASK_MANT_F32      0x007fffff
#define LIBXSMM_DNN_MASK_ABS_F32       0x7fffffff
#define LIBXSMM_DNN_MASK_FULL_F32      0xffffffff
#define LIBXSMM_DNN_MANT_SZ_F32        23
#define LIBXSMM_DNN_SZ_F32             32

/* DFP16 masking defines */
#define LIBXSMM_DNN_MANT_DFP16         15
#define LIXSMMM_DNN_RES_DFP16          libxsmm_sexp2_i8i(-(LIBXSMM_DNN_MANT_DFP16))

/* Quantization Rounding Defines */
#define LIBXSMM_DNN_QUANT_NO_ROUND       80000
#define LIBXSMM_DNN_QUANT_BIAS_ROUND     80001
#define LIBXSMM_DNN_QUANT_STOCH_ROUND    80002
#define LIBXSMM_DNN_QUANT_NEAREST_ROUND  80003
#define LIBXSMM_DNN_QUANT_FPHW_ROUND     80004

/** get string of error code */
LIBXSMM_API const char* libxsmm_dnn_get_error(libxsmm_dnn_err_t code);
LIBXSMM_API size_t libxsmm_dnn_typesize(libxsmm_dnn_datatype datatype);
LIBXSMM_API size_t libxsmm_dnn_get_simd_width(libxsmm_dnn_datatype datatype);

/** Create a layer handle (non-NULL if successful), and pre-build all JIT-code versions. */
LIBXSMM_API libxsmm_dnn_layer* libxsmm_dnn_create_conv_layer(libxsmm_dnn_conv_desc conv_desc, libxsmm_dnn_err_t* status);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_destroy_conv_layer(const libxsmm_dnn_layer* handle);

/** get layout description of buffers and filters from handle */
LIBXSMM_API libxsmm_dnn_tensor_datalayout* libxsmm_dnn_create_tensor_datalayout(const libxsmm_dnn_layer* handle, const libxsmm_dnn_tensor_type type, libxsmm_dnn_err_t* status);
LIBXSMM_API libxsmm_dnn_tensor_datalayout* libxsmm_dnn_duplicate_tensor_datalayout(const libxsmm_dnn_tensor_datalayout* layout, libxsmm_dnn_err_t* status);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_destroy_tensor_datalayout(libxsmm_dnn_tensor_datalayout* layout);
LIBXSMM_API unsigned int libxsmm_dnn_compare_tensor_datalayout(const libxsmm_dnn_tensor_datalayout* layout_a, const libxsmm_dnn_tensor_datalayout* layout_b, libxsmm_dnn_err_t* status);
LIBXSMM_API unsigned int libxsmm_dnn_get_tensor_size(const libxsmm_dnn_tensor_datalayout* layout, libxsmm_dnn_err_t* status);
LIBXSMM_API unsigned int libxsmm_dnn_get_tensor_elements(const libxsmm_dnn_tensor_datalayout* layout, libxsmm_dnn_err_t* status);

/** Create and manage buffers, filters and bias (non-NULL if successful) */
LIBXSMM_API libxsmm_dnn_tensor* libxsmm_dnn_link_tensor(const libxsmm_dnn_tensor_datalayout* layout, const void* data, libxsmm_dnn_err_t* status);
LIBXSMM_API libxsmm_dnn_tensor* libxsmm_dnn_link_qtensor(const libxsmm_dnn_tensor_datalayout* layout, const void* data, const unsigned char exp, libxsmm_dnn_err_t* status);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_set_tensor_data_ptr(libxsmm_dnn_tensor* tensor, const void* data);
LIBXSMM_API void* libxsmm_dnn_get_tensor_data_ptr(const libxsmm_dnn_tensor* tensor, libxsmm_dnn_err_t* status);
LIBXSMM_API unsigned char libxsmm_dnn_get_qtensor_scf(const libxsmm_dnn_tensor* tensor, libxsmm_dnn_err_t* status);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_set_qtensor_scf(libxsmm_dnn_tensor* tensor, const unsigned char scf);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_destroy_tensor(const libxsmm_dnn_tensor* tensor);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_zero_tensor(const libxsmm_dnn_tensor* tensor);

/**
 * Copy-in/out from a plain format such [N][C][H][W] or [N][H][W][C]
 */
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_copyin_tensor(const libxsmm_dnn_tensor* tensor, const void* data, const libxsmm_dnn_tensor_format in_format);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_copyout_tensor(const libxsmm_dnn_tensor* tensor, void* data, const libxsmm_dnn_tensor_format out_format);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_trans_reg_filter(const libxsmm_dnn_layer* handle);

/** scratch pad management */
LIBXSMM_API size_t libxsmm_dnn_get_scratch_size(const libxsmm_dnn_layer* handle, const libxsmm_dnn_compute_kind kind, libxsmm_dnn_err_t* status);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_bind_scratch(libxsmm_dnn_layer* handle, const libxsmm_dnn_compute_kind kind, const void* scratch);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_release_scratch(libxsmm_dnn_layer* handle, const libxsmm_dnn_compute_kind kind);

/** Bind/Release buffers, filters and bias to layer operation */
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_bind_tensor(libxsmm_dnn_layer* handle, const libxsmm_dnn_tensor* tensor, const libxsmm_dnn_tensor_type type);
LIBXSMM_API libxsmm_dnn_tensor* libxsmm_dnn_get_tensor(libxsmm_dnn_layer* handle, const libxsmm_dnn_tensor_type type, libxsmm_dnn_err_t* status);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_release_tensor(libxsmm_dnn_layer* handle, const libxsmm_dnn_tensor_type type);

/** Run the layer identified by the handle; may use threads internally. */
LIBXSMM_API void libxsmm_dnn_execute(libxsmm_dnn_layer* handle, libxsmm_dnn_compute_kind kind);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_execute_st(libxsmm_dnn_layer* handle, libxsmm_dnn_compute_kind kind,
  /*unsigned*/int start_thread, /*unsigned*/int tid);

/** some helper functions for framework integration */
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_transpose_filter(libxsmm_dnn_layer* handle, const libxsmm_dnn_tensor_type type);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_reduce_wu_filters(libxsmm_dnn_layer* handle, const libxsmm_dnn_tensor_type type);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_get_codegen_success(libxsmm_dnn_layer* handle, libxsmm_dnn_compute_kind kind);
LIBXSMM_API libxsmm_dnn_err_t libxsmm_dnn_get_parallel_tasks(libxsmm_dnn_layer* handle, libxsmm_dnn_compute_kind kind, unsigned int* num_tasks);

/** some quantization helper functions,
    @TODO need to be integrated better for all different ways of quantizations */
LIBXSMM_API void libxsmm_dnn_quantize( float* in_buffer, short* out_buffer, int length, unsigned char add_shift, unsigned char* scf, int round_mode );
LIBXSMM_API void libxsmm_dnn_quantize_act( float* in_buffer, short* out_buffer, unsigned int N, unsigned int C, unsigned int H, unsigned int W, unsigned int cblk_f32, unsigned int cblk_i16, unsigned int lp_blk, unsigned char add_shift, unsigned char* scf, int round_mode );
LIBXSMM_API void libxsmm_dnn_quantize_fil( float* in_buffer, short* out_buffer, unsigned int K, unsigned int C, unsigned int R, unsigned int S, unsigned int cblk_f32, unsigned int cblk_i16, unsigned int kblk_f32, unsigned int kblk_i16, unsigned int lp_blk, unsigned char add_shift, unsigned char* scf, int round_mode );
LIBXSMM_API void libxsmm_dnn_dequantize( short* in_buffer, float* out_buffer, int length, unsigned char scf );

/** some BF16<->FP32 conversion functions
    @TODO we need to find a final place for those */
LIBXSMM_API void libxsmm_truncate_convert_f32_bf16(const float* in, libxsmm_bfloat16* out, unsigned int length);
LIBXSMM_API void libxsmm_rnaz_convert_fp32_bfp16(const float* in, libxsmm_bfloat16* out, unsigned int len);
LIBXSMM_API void libxsmm_rne_convert_fp32_bfp16(const float* in, libxsmm_bfloat16* out, unsigned int len);
LIBXSMM_API void libxsmm_convert_bf16_f32(const libxsmm_bfloat16* in, float* out, unsigned int length);

#endif /*LIBXSMM_DNN_H*/

