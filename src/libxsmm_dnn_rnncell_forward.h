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
/* Alexander Heinecke, Evangelos Georganas (Intel Corp.)
******************************************************************************/
#ifndef LIBXSMM_DNN_RNNCELL_FORWARD_H
#define LIBXSMM_DNN_RNNCELL_FORWARD_H

#include <libxsmm_dnn.h>
#include <libxsmm_dnn_rnncell.h>

LIBXSMM_API_INTERN libxsmm_dnn_err_t libxsmm_dnn_rnncell_st_fwd_nc_ck(libxsmm_dnn_rnncell* handle, int start_thread, int tid);
LIBXSMM_API_INTERN libxsmm_dnn_err_t libxsmm_dnn_rnncell_st_fwd_ncnc_kcck(libxsmm_dnn_rnncell* handle, int start_thread, int tid);
LIBXSMM_API_INTERN libxsmm_dnn_err_t libxsmm_dnn_rnncell_st_fwd_nc_kcck(libxsmm_dnn_rnncell* handle, int start_thread, int tid);

#endif /* LIBXSMM_DNN_RNNCELL_FORWARD_H */
