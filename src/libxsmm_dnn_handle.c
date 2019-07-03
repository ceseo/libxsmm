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
/* Alexander Heinecke, Hans Pabst, Rajkishore Barik,
 * Ankush Mandal, Evangelos Georganas (Intel Corp.)
******************************************************************************/
#include "libxsmm_dnn_handle.h"
#include "libxsmm_main.h"
#include <libxsmm.h>
#include "libxsmm_dnn_setup.h"

#if !defined(LIBXSMM_DNN_HANDLE_DEBUG) && 0
# define LIBXSMM_DNN_HANDLE_DEBUG
#endif

#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(push,target(LIBXSMM_OFFLOAD_TARGET))
#endif
#include <stdlib.h>
#include <string.h>
#if defined(LIBXSMM_DNN_HANDLE_DEBUG)
# include <stdio.h>
#endif
#if defined(_OPENMP)
# include <omp.h>
#endif
#if !defined(NDEBUG)
# include <stdio.h>
#endif
#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(pop)
#endif


LIBXSMM_API_INTERN libxsmm_dnn_err_t libxsmm_dnn_internal_create_conv_handle_direct( libxsmm_dnn_layer* handle ) {
  libxsmm_dnn_err_t status = LIBXSMM_DNN_SUCCESS;
  LIBXSMM_ASSERT(0 != handle);

  /* we only support physical paddind in these days */
  /* @TODO: add logical padding support */
  if ( ( handle->desc.pad_h != handle->desc.pad_h_in )  ||
       ( handle->desc.pad_w != handle->desc.pad_w_in )  ||
       ( handle->desc.pad_h != handle->desc.pad_h_out ) ||
       ( handle->desc.pad_w != handle->desc.pad_w_out )    ) {
    status = LIBXSMM_DNN_ERR_INVALID_PADDING;
    free( handle );
    handle = 0;
    return status;
  }

  status = libxsmm_dnn_setup_generic(handle);

  {
    {
      const size_t padded_h = ((size_t)2 * handle->desc.pad_h) + handle->desc.H, padded_w = ((size_t)2 * handle->desc.pad_w) + handle->desc.W;
      const size_t size5_tensor = padded_h * padded_w * handle->ifmblock * libxsmm_dnn_typesize(handle->datatype_in);
      const size_t size5 = LIBXSMM_UP2(size5_tensor, LIBXSMM_CACHELINE) * handle->desc.threads;
      if (handle->max_scratch5_size < size5) handle->max_scratch5_size = size5;
      handle->scratch5 = 0;
    }
    {
      const size_t size6a = (size_t)handle->ofmblock * handle->ofw * handle->ofh * sizeof(float);
      const size_t size6b = (size_t)handle->ifmblock * handle->fm_lp_block *  handle->desc.W * handle->desc.H * sizeof(float);
      const size_t size6 = ( size6a > size6b ) ? size6a : size6b;
      handle->scratch6_size = LIBXSMM_MAX(LIBXSMM_UP2(size6, LIBXSMM_CACHELINE) * handle->desc.threads, handle->scratch6_size);
    }
    {
      const size_t output_typesize = libxsmm_dnn_typesize(handle->datatype_out);
      const size_t size6_tensor = (size_t)handle->ofhp * handle->ofwp * handle->ofmblock * output_typesize;
      const size_t size6 = LIBXSMM_UP2(size6_tensor, LIBXSMM_CACHELINE) * handle->desc.threads;
      if (handle->scratch6_size < size6) handle->scratch6_size = size6;
    }
    handle->scratch6 = 0;
    {
      /* FIXME: currently filter data-type is always smaller/equal output type */
      const size_t filter_typesize = libxsmm_dnn_typesize(handle->datatype_out);
      const size_t size7 = (size_t)handle->desc.R * handle->desc.S * handle->desc.C * handle->desc.K * filter_typesize + handle->ifmblock * handle->ofmblock * sizeof(float);
      handle->scratch7_size = LIBXSMM_UP2(size7, LIBXSMM_CACHELINE) * LIBXSMM_MAX(handle->desc.threads, handle->desc.N);
    }
  }
  return status;
}


LIBXSMM_API_INTERN libxsmm_dnn_err_t libxsmm_dnn_internal_free_structs_code_conv_handle( const libxsmm_dnn_layer* handle ) {
  libxsmm_dnn_err_t status = LIBXSMM_DNN_SUCCESS;
  LIBXSMM_UNUSED( handle );
  return status;
}

