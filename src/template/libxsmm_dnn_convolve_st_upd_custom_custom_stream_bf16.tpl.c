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
/* Evangelos Georganas (Intel Corp.)
******************************************************************************/

/* computing first logical thread */
const int ltid = tid-start_thread;

/* FIXME assignments here */
int BLOCKSIFM = handle->blocksifm;
int BLOCKSOFM = handle->blocksofm;
int OFWP = handle->ofwp+handle->output_lp_padding;
const int adjust_offset = (handle->desc.C == 3 || handle->reduce_weights == 0) ? 2 : 1;

/* Auxiliary integer variables   */
int ofm1, ifm1, i, j;

/* transpose, copy and reduce work-related variables  */
const int reduce_work = (handle->ifmblock != 3)
  ? (BLOCKSOFM*BLOCKSIFM*handle->desc.R*handle->desc.S*(handle->ifmblock_hp/2))
  : (BLOCKSOFM*BLOCKSIFM*handle->desc.R*handle->desc.S*handle->ifmblock);
const int reduce_chunksize = (reduce_work % handle->desc.threads == 0) ? (reduce_work / handle->desc.threads) : (reduce_work / handle->desc.threads) + 1;
const int reduce_thr_begin = (ltid * reduce_chunksize < reduce_work) ? (ltid * reduce_chunksize) : reduce_work;
const int reduce_thr_end = ((ltid + 1) * reduce_chunksize < reduce_work) ? ((ltid + 1) * reduce_chunksize) : reduce_work;

/* Pointer related variables for output and weight */
int pixels_lp = handle->fm_lp_block;
element_output_type *const out = ((element_output_type*)handle->grad_output->data) + (handle->desc.pad_h_out * handle->ofwp + handle->desc.pad_w_out) * handle->ofmblock_lp * handle->fm_lp_block;
LIBXSMM_VLA_DECL(6, element_output_type, tr_output, (element_output_type*)handle->scratch2 , BLOCKSOFM, handle->ofhp, OFWP/pixels_lp, handle->ofmblock, pixels_lp);
LIBXSMM_VLA_DECL(6, element_output_type, output, out, handle->blocksofm_lp, handle->ofhp, handle->ofwp, handle->ofmblock_lp, handle->fm_lp_block);
#if 0
LIBXSMM_VLA_DECL(6, element_filter_type, weight, (element_filter_type*)handle->grad_filter->data, BLOCKSIFM, handle->desc.R, handle->desc.S, handle->ifmblock_hp, handle->ofmblock);
#endif
/*element_filter_type* weight_ptr = (element_filter_type*)handle->grad_filter->data;*/
float *weight_ptr = ((float*) handle->scratch2) + (handle->desc.N * handle->blocksofm * handle->ofmblock * (handle->ofhp+2*handle->desc.pad_h) * (handle->ofwp+8+2*handle->desc.pad_w))/2;

float* reduction_weight_ptr = ((float*)handle->scratch4) + (handle->weight_copies * BLOCKSOFM * BLOCKSIFM * handle->desc.R*handle->desc.S*handle->ifmblock_hp*handle->ofmblock);
LIBXSMM_VLA_DECL(3, libxsmm_bfloat16, reduction_weight, (libxsmm_bfloat16*)reduction_weight_ptr, handle->weight_copies, handle->ofmblock);

int padded_h = (handle->padding_flag == 1) ? handle->ifhp + 2 * handle->desc.pad_h : handle->ifhp;
int padded_w = (handle->padding_flag == 1) ? handle->ifwp + 2 * handle->desc.pad_w : handle->ifwp;
int ifwp_extended = (handle->resize_input == 1 ? (handle->ifwp_resized + handle->qfma_input_pad) : (padded_w + handle->qfma_input_pad));
int dst_ifhp = (handle->resize_input == 1 ? handle->ifhp_resized : handle->ifhp);

LIBXSMM_VLA_DECL(6, element_input_type, input_nopad, (element_input_type*)handle->reg_input->data, handle->blocksifm_lp, handle->ifhp, handle->ifwp, handle->ifmblock, handle->fm_lp_block);
LIBXSMM_VLA_DECL(5, element_input_type, tr_input_padded, (element_input_type*)handle->scratch5, BLOCKSIFM, padded_h, handle->ifmblock_hp, ifwp_extended);
LIBXSMM_VLA_DECL(5, element_input_type, tr_input_nopad, (element_input_type*)handle->scratch3, BLOCKSIFM, dst_ifhp, handle->ifmblock_hp, ifwp_extended);

/* Stream related variables  */
int *stream = handle->compute_upd_indices_ptrs[ltid];
int instr, offset_i, offset_o, offset_w, pi, po, pw, pc;

/* Base pointers  */
element_input_type *input_base;
float *weight_base;
element_output_type *output_base;
element_input_type *input_zero;

/* Kernel related variables  */
libxsmm_convfunction kernel = (handle->trans_ofw_ifm == 0 ) ? (libxsmm_convfunction)handle->code_upd[0].xconv.sconv : (libxsmm_convfunction)handle->code_upd[1].xconv.sconv;

LIBXSMM_ALIGNED(float scale_factor, 64);
LIBXSMM_ALIGNED(float vnni_scratch[32], 64);
#if 0
LIBXSMM_ALIGNED(float *max_vals, 64);
#if defined(LIBXSMM_INTRINSICS_AVX512) /*__AVX512F__*/
__m512 max_abs = _mm512_setzero_ps();
#else /* won't happen as this code only runs on AVX512 platforms */
LIBXSMM_ASSERT(0);
#endif
#endif
/* lazy barrier init */
libxsmm_barrier_init(handle->barrier, ltid);
/* Initialize base pointers */
if (handle->padding_flag == 1) {
  input_base = &LIBXSMM_VLA_ACCESS(5, tr_input_padded, 0, 0, 0, 0, 0, BLOCKSIFM, padded_h, handle->ifmblock_hp, ifwp_extended);
  input_zero = &LIBXSMM_VLA_ACCESS(5, tr_input_padded, ltid, 0, 0, 0, 0, BLOCKSIFM, padded_h, handle->ifmblock_hp, ifwp_extended);
  memset( input_zero, 0, BLOCKSIFM * padded_h * ifwp_extended * handle->ifmblock_hp * sizeof(element_input_type) );
} else {
  input_base = &LIBXSMM_VLA_ACCESS(5, tr_input_nopad, 0, 0, 0, 0, 0, BLOCKSIFM, dst_ifhp, handle->ifmblock_hp, ifwp_extended);
}

if (handle->padding_flag == 1) {
  int img = ltid, ij, ifm2, ii, lp;
  for (ifm1 = 0; ifm1 < handle->blocksifm_lp; ++ifm1) {
    for (ij = 0; ij < handle->ifhp; ++ij) {
      for (ii = 0; ii < handle->ifwp; ++ii) {
        for (ifm2 = 0; ifm2 < handle->ifmblock; ++ifm2) {
          for (lp = 0; lp < handle->fm_lp_block; ++lp) {
            LIBXSMM_VLA_ACCESS(5, tr_input_padded, img, ifm1, ij+handle->desc.pad_h, ifm2*handle->fm_lp_block+lp, ii+handle->desc.pad_w, BLOCKSIFM, padded_h, handle->ifmblock_hp, ifwp_extended) =
              LIBXSMM_VLA_ACCESS(6, input_nopad, img, ifm1, ij, ii, ifm2, lp, handle->blocksifm_lp, handle->ifhp, handle->ifwp, handle->ifmblock, handle->fm_lp_block);
          }
        }
      }
    }
  }
#   include "transpose_lp_output.tpl.c"
} else {
  if (handle->resize_input == 0) {
    libxsmm_dnn_inout_transpose_lp(ltid, handle);
  } else {
    libxsmm_dnn_inout_transpose_resize_lp(ltid, handle);
  }
}

libxsmm_barrier_wait(handle->barrier, ltid);

if (handle->reduce_weights) {
  if (handle->desc.C ==3) {
    weight_base = ((float*)handle->scratch4) + (ltid * BLOCKSOFM * BLOCKSIFM * handle->desc.R*handle->desc.S*handle->ifmblock*handle->ofmblock);
    /* We DO USE private weights, initialize them to zero...  */
#if defined(LIBXSMM_INTRINSICS_AVX512)
    { const __m512 zero_reg = _mm512_setzero_ps();
      for (i=0; i<reduce_work; i++) {
        _mm512_store_ps( ((float*) weight_base) + i * 16, zero_reg);
      }
    }
#else
#endif
  } else {
    weight_base = (float*) &LIBXSMM_VLA_ACCESS(3, reduction_weight, 0, ltid/(handle->desc.threads/handle->weight_copies), 0, handle->weight_copies, handle->ofmblock);
  }
} else {
  weight_base = weight_ptr;
  /* Initialize accumulation scratch to zero...  */
#if defined(LIBXSMM_INTRINSICS_AVX512)
  { const __m512 zero_reg = _mm512_setzero_ps();
    const int zero_work = (BLOCKSOFM*BLOCKSIFM*handle->desc.R*handle->desc.S*handle->ifmblock_hp);
    const int zero_chunksize = (zero_work % handle->desc.threads == 0) ? (zero_work / handle->desc.threads) : (zero_work / handle->desc.threads) + 1;
    const int zero_thr_begin = (ltid * zero_chunksize < zero_work) ? (ltid * zero_chunksize) : zero_work;
    const int zero_thr_end = ((ltid + 1) * zero_chunksize < zero_work) ? ((ltid + 1) * zero_chunksize) : zero_work;
    for ( j = zero_thr_begin; j < zero_thr_end; j++ ) {
      float *fp32_weight_ptr = ((float*) weight_ptr) + j * 16;
      _mm512_store_ps(fp32_weight_ptr, zero_reg);
    }
  }
#else
#endif
}
libxsmm_barrier_wait(handle->barrier, ltid);

if (handle->trans_ofw_ifm == 1) {
  if (handle->padding_flag == 1) {
    input_base = &LIBXSMM_VLA_ACCESS(5, tr_input_padded, 0, 0, 0, 0, 0, BLOCKSIFM, padded_h, handle->ifmblock_hp, ifwp_extended);
  } else {
    input_base = &LIBXSMM_VLA_ACCESS(5, tr_input_nopad, 0, 0, 0, 0, 0, BLOCKSIFM, dst_ifhp, handle->ifmblock_hp, ifwp_extended);
  }
} else {
  if (handle->avoid_input_trans == 1) {
    LIBXSMM_VLA_DECL(6, element_input_type, lp_input, (element_input_type*)handle->reg_input->data, BLOCKSIFM, handle->ifhp, handle->ifwp/pixels_lp, handle->ifmblock_hp, pixels_lp);
    input_base = &LIBXSMM_VLA_ACCESS(6, lp_input, 0, 0, 0, 0, 0, 0, handle->blocksifm, handle->ifhp, handle->ifwp/pixels_lp, handle->ifmblock_hp, pixels_lp);
  } else {
    LIBXSMM_VLA_DECL(6, element_input_type, lp_input, (element_input_type*)handle->scratch3, handle->blocksifm, handle->ifhp, handle->ifwp/pixels_lp, handle->ifmblock_hp, pixels_lp);
    input_base = &LIBXSMM_VLA_ACCESS(6, lp_input, 0, 0, 0, 0, 0, 0, handle->blocksifm, handle->ifhp, handle->ifwp/pixels_lp, handle->ifmblock_hp, pixels_lp);
  }
}

if (handle->avoid_output_trans) {
  element_output_type *const grad_out = ((element_output_type*)handle->grad_output->data) + (handle->desc.pad_h_out * handle->ofwp + handle->desc.pad_w_out) * handle->ofmblock_lp * handle->fm_lp_block;
  LIBXSMM_VLA_DECL(6, element_output_type, lp_output, grad_out, BLOCKSOFM, handle->ofhp, handle->ofwp/pixels_lp, handle->ofmblock, pixels_lp);
  output_base = &LIBXSMM_VLA_ACCESS(6, lp_output, 0, 0, 0, 0, 0, 0, handle->blocksofm, handle->ofhp, handle->ofwp/pixels_lp, handle->ofmblock, pixels_lp);
} else {
  LIBXSMM_VLA_DECL(6, element_output_type, scratch_out, (element_output_type*)handle->scratch2 , BLOCKSOFM, handle->ofhp, OFWP/pixels_lp, handle->ofmblock, pixels_lp);
  output_base = &LIBXSMM_VLA_ACCESS(6, scratch_out, 0, 0, 0, 0, 0, 0, BLOCKSOFM, handle->ofhp, OFWP/pixels_lp, handle->ofmblock, pixels_lp);
}

i = 0;
instr = handle->n_entries_upd[ltid];

if (handle->use_lp_kernel == 1) {
  scale_factor = libxsmm_sexp2(-1.f*((float)(handle->reg_input->scf + handle->grad_output->scf)));
}

for (pc = 0; pc < instr; pc++) {
  offset_i = stream[i];
  offset_w = stream[i+1] * adjust_offset;
  offset_o = stream[i+2];
  pi = stream[i+3];
  pw = stream[i+4];
  po = stream[i+5];
  kernel( input_base + offset_i, (float*)((libxsmm_bfloat16*)weight_base + offset_w), output_base + offset_o, input_base + pi, weight_base + pw, output_base + po, &scale_factor, &vnni_scratch[0]);
  i+=3;
}

if (handle->desc.C == 3) {
  /* Iterate over my private fp32 copy and convert it to BF16 (such that the reduction can happen also in bf16) */
  libxsmm_bfloat16 *dst_weight_base = (libxsmm_bfloat16 *) (((float*)handle->scratch4) + (ltid * BLOCKSOFM * BLOCKSIFM * handle->desc.R*handle->desc.S*handle->ifmblock*handle->ofmblock));
  weight_base = ((float*)handle->scratch4) + (ltid * BLOCKSOFM * BLOCKSIFM * handle->desc.R*handle->desc.S*handle->ifmblock*handle->ofmblock);
#if defined(LIBXSMM_INTRINSICS_AVX512)
#define _mm512_roundbf16rne(A) LIBXSMM_INTRINSICS_MM512_ROUNDNE_BF16(A)
#define _mm512_storecvtrne_fp32_bf16(A,B)  _mm256_stream_si256((__m256i*)(A),_mm512_cvtepi32_epi16(_mm512_srai_epi32(_mm512_roundbf16rne((B)),16)))
#define _mm512_storecvttrunc_fp32_bf16(A,B)  _mm256_stream_si256((__m256i*)(A),_mm512_cvtepi32_epi16(_mm512_srai_epi32(_mm512_castps_si512(B),16)))
  if ( handle->f32_bf16_cvt_rne ) {
    for (i=0; i<reduce_work; i++) {
      _mm512_storecvtrne_fp32_bf16( ((libxsmm_bfloat16*) dst_weight_base) + i * 16, LIBXSMM_INTRINSICS_MM512_LOAD_PS(((float*) weight_base) + i * 16 ));
    }
  } else {
    for (i=0; i<reduce_work; i++) {
      _mm512_storecvttrunc_fp32_bf16( ((libxsmm_bfloat16*) dst_weight_base) + i * 16, LIBXSMM_INTRINSICS_MM512_LOAD_PS(((float*) weight_base) + i * 16 ));
    }
  }
#undef _mm512_roundbf16rne
#undef _mm512_storecvtrne_fp32_bf16
#undef _mm512_storecvttrunc_fp32_bf16
#else
#endif
}

libxsmm_barrier_wait(handle->barrier, ltid);

#define _mm512_loadcvt_bf16_fp32(A)   _mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepi16_epi32(_mm256_loadu_si256((__m256i*)(A))),16))
if (handle->reduce_weights) {
#if defined(LIBXSMM_INTRINSICS_AVX512)
  if (handle->desc.C == 3) {
    const int total_filter_size = reduce_work * handle->ofmblock;
    libxsmm_bfloat16 *dst_weight_ptr = (libxsmm_bfloat16*)handle->grad_filter->data;
    if ( handle->f32_bf16_cvt_rne ) {
      __m512i vnaninf = _mm512_set1_epi32( 0x7f800000 );
      __m512i vrneadd = _mm512_set1_epi32( 0x00007fff );
      __m512i vfixup = _mm512_set1_epi32( 0x00000001 );
      __m512i vfixupmask = _mm512_set1_epi32( 0x00010000 );
      for ( j = reduce_thr_begin; j < reduce_thr_end; j++) {
        __m512 sum_weight = _mm512_setzero_ps();
        for ( i = 0; i < handle->desc.threads; i++ ) {
          const libxsmm_bfloat16 *const remote_weight_ptr = ((libxsmm_bfloat16*)handle->scratch4) + (i*2*total_filter_size);
          const __m512 remote_weight = _mm512_loadcvt_bf16_fp32(remote_weight_ptr + j*16);
          sum_weight = _mm512_add_ps( remote_weight, sum_weight);
        }
        { /* open new scope for additional variable declarations (C89) */
          __m512i vfp32     = _mm512_castps_si512( sum_weight);
          __m512i vfp32nan  = _mm512_and_epi32( vfp32, vnaninf );
          __m512i vfp32fixup  = _mm512_and_epi32( vfp32, vfixupmask );
          __mmask16 rnemask = _mm512_cmp_epi32_mask( vfp32nan, vnaninf, _MM_CMPINT_NE );
          __mmask16 fixupmask = _mm512_cmp_epi32_mask( vfp32fixup, vfixupmask, _MM_CMPINT_EQ );
          __m512i vrnd = _mm512_mask_add_epi32( vrneadd , fixupmask, vrneadd, vfixup );
          __m512i vfp32rne  = _mm512_mask_add_epi32( vfp32, rnemask, vfp32, vrnd );
          __m512i vbfp16_32 = _mm512_srai_epi32( vfp32rne, 16 );
          __m256i vbfp16    = _mm512_cvtepi32_epi16( vbfp16_32 );
          _mm256_storeu_si256( (__m256i*)( ((libxsmm_bfloat16*)dst_weight_ptr)+j*16), vbfp16 );
        }
      }
    } else {
      for ( j = reduce_thr_begin; j < reduce_thr_end; j++) {
        __m512 sum_weight = _mm512_setzero_ps();
        __m256i vbfp16;
        for ( i = 0; i < handle->desc.threads; i++ ) {
          const libxsmm_bfloat16 *const remote_weight_ptr = ((libxsmm_bfloat16*)handle->scratch4) + (i*2*total_filter_size);
          const __m512 remote_weight = _mm512_loadcvt_bf16_fp32(remote_weight_ptr + j*16);
          sum_weight = _mm512_add_ps( remote_weight, sum_weight);
        }
        vbfp16 = _mm512_cvtepi32_epi16(_mm512_srai_epi32( _mm512_castps_si512( sum_weight ), 16));
        _mm256_storeu_si256( (__m256i*)( ((libxsmm_bfloat16*)dst_weight_ptr)+j*16), vbfp16 );
      }
    }
  } else {
    if ( handle->f32_bf16_cvt_rne ) {
      __m512i vnaninf = _mm512_set1_epi32( 0x7f800000 );
      __m512i vrneadd = _mm512_set1_epi32( 0x00007fff );
      __m512i vfixup = _mm512_set1_epi32( 0x00000001 );
      __m512i vfixupmask = _mm512_set1_epi32( 0x00010000 );
      for ( j = 2*reduce_thr_begin; j < 2*reduce_thr_end; j+=2 ) {
        __m512 weight_sum_lo = _mm512_setzero_ps();
        __m512 weight_sum_hi = _mm512_setzero_ps();
        __m512i pair_fms;
        for ( i = 0; i < handle->weight_copies; i++ ) {
          weight_sum_lo = _mm512_add_ps(weight_sum_lo, _mm512_loadcvt_bf16_fp32(&LIBXSMM_VLA_ACCESS(3, reduction_weight, j, i, 0, handle->weight_copies, 16)));
        }
        for ( i = 0; i < handle->weight_copies; i++ ) {
          weight_sum_hi = _mm512_add_ps(weight_sum_hi, _mm512_loadcvt_bf16_fp32(&LIBXSMM_VLA_ACCESS(3, reduction_weight, j+1, i, 0, handle->weight_copies, 16)));
        }
        { /* open new scope for additional variable declarations (C89) */
          __m512i vfp32     = _mm512_castps_si512( weight_sum_lo);
          __m512i vfp32nan  = _mm512_and_epi32( vfp32, vnaninf );
          __m512i vfp32fixup  = _mm512_and_epi32( vfp32, vfixupmask );
          __mmask16 rnemask = _mm512_cmp_epi32_mask( vfp32nan, vnaninf, _MM_CMPINT_NE );
          __mmask16 fixupmask = _mm512_cmp_epi32_mask( vfp32fixup, vfixupmask, _MM_CMPINT_EQ );
          __m512i vrnd = _mm512_mask_add_epi32( vrneadd , fixupmask, vrneadd, vfixup );
          __m512i vfp32rne  = _mm512_mask_add_epi32( vfp32, rnemask, vfp32, vrnd );
          __m512i vbfp16_32_lo = _mm512_srli_epi32( vfp32rne, 16 );
          __m512i vbfp16_32_hi;

          vfp32     = _mm512_castps_si512( weight_sum_hi);
          vfp32nan  = _mm512_and_epi32( vfp32, vnaninf );
          vfp32fixup  = _mm512_and_epi32( vfp32, vfixupmask );
          rnemask = _mm512_cmp_epi32_mask( vfp32nan, vnaninf, _MM_CMPINT_NE );
          fixupmask = _mm512_cmp_epi32_mask( vfp32fixup, vfixupmask, _MM_CMPINT_EQ );
          vrnd = _mm512_mask_add_epi32( vrneadd , fixupmask, vrneadd, vfixup );
          vfp32rne  = _mm512_mask_add_epi32( vfp32, rnemask, vfp32, vrnd );
          vbfp16_32_hi = _mm512_srli_epi32( vfp32rne, 16 );
          vbfp16_32_hi = _mm512_slli_epi32(vbfp16_32_hi, 16);
          pair_fms = _mm512_or_epi32(vbfp16_32_lo, vbfp16_32_hi);
          _mm512_store_epi32( ((libxsmm_bfloat16*) handle->grad_filter->data) + j * 16, pair_fms);
        }
      }
    } else {
      for ( j = 2*reduce_thr_begin; j < 2*reduce_thr_end; j+=2 ) {
        __m512 weight_sum_lo = _mm512_setzero_ps();
        __m512 weight_sum_hi = _mm512_setzero_ps();
        __m512i fm0, fm1, pair_fms;
        for ( i = 0; i < handle->weight_copies; i++ ) {
          weight_sum_lo = _mm512_add_ps(weight_sum_lo, _mm512_loadcvt_bf16_fp32(&LIBXSMM_VLA_ACCESS(3, reduction_weight, j, i, 0, handle->weight_copies, 16)));
        }
        for ( i = 0; i < handle->weight_copies; i++ ) {
          weight_sum_hi = _mm512_add_ps(weight_sum_hi, _mm512_loadcvt_bf16_fp32(&LIBXSMM_VLA_ACCESS(3, reduction_weight, j+1, i, 0, handle->weight_copies, 16)));
        }
        fm0 = _mm512_castps_si512(weight_sum_lo);
        fm1 = _mm512_castps_si512(weight_sum_hi);
        fm0 = _mm512_srli_epi32(fm0, 16);
        fm1 = _mm512_srli_epi32(fm1, 16);
        fm1 = _mm512_slli_epi32(fm1, 16);
        pair_fms = _mm512_or_epi32(fm0, fm1);
        _mm512_store_epi32( ((libxsmm_bfloat16*) handle->grad_filter->data) + j * 16, pair_fms);
      }
    }
#else
#endif
  }
} else {
#if defined(LIBXSMM_INTRINSICS_AVX512)
  /* If there is no reduction, then just convert the result ...*/
  const int transform_work = BLOCKSOFM*BLOCKSIFM*handle->desc.R*handle->desc.S*8;
  const int transform_chunksize = (transform_work % handle->desc.threads == 0) ? (transform_work / handle->desc.threads) : (transform_work / handle->desc.threads) + 1;
  const int transform_thr_begin = (ltid * transform_chunksize < transform_work) ? (ltid * transform_chunksize) : transform_work;
  const int transform_thr_end = ((ltid + 1) * transform_chunksize < transform_work) ? ((ltid + 1) * transform_chunksize) : transform_work;
  if ( handle->f32_bf16_cvt_rne ) {
    __m512i vnaninf = _mm512_set1_epi32( 0x7f800000 );
    __m512i vrneadd = _mm512_set1_epi32( 0x00007fff );
    __m512i vfixup = _mm512_set1_epi32( 0x00000001 );
    __m512i vfixupmask = _mm512_set1_epi32( 0x00010000 );
    for ( j = 2*transform_thr_begin; j < 2*transform_thr_end; j+=2 ) {
      libxsmm_bfloat16 *bf16_weight_ptr = ((libxsmm_bfloat16*) handle->grad_filter->data) + j * 16;
      float *fp32_weight_ptr = ((float*) weight_ptr) + j * 16;
      __m512i vfp32     = _mm512_castps_si512( LIBXSMM_INTRINSICS_MM512_LOAD_PS((float*)fp32_weight_ptr));
      __m512i vfp32nan  = _mm512_and_epi32( vfp32, vnaninf );
      __m512i vfp32fixup  = _mm512_and_epi32( vfp32, vfixupmask );
      __mmask16 rnemask = _mm512_cmp_epi32_mask( vfp32nan, vnaninf, _MM_CMPINT_NE );
      __mmask16 fixupmask = _mm512_cmp_epi32_mask( vfp32fixup, vfixupmask, _MM_CMPINT_EQ );
      __m512i vrnd = _mm512_mask_add_epi32( vrneadd , fixupmask, vrneadd, vfixup );
      __m512i vfp32rne  = _mm512_mask_add_epi32( vfp32, rnemask, vfp32, vrnd );
      __m512i vbfp16_32_lo = _mm512_srli_epi32( vfp32rne, 16 );
      __m512i vbfp16_32_hi, pair_fms;
      vfp32     = _mm512_castps_si512( LIBXSMM_INTRINSICS_MM512_LOAD_PS((float*)fp32_weight_ptr + 16));
      vfp32nan  = _mm512_and_epi32( vfp32, vnaninf );
      vfp32fixup  = _mm512_and_epi32( vfp32, vfixupmask );
      rnemask = _mm512_cmp_epi32_mask( vfp32nan, vnaninf, _MM_CMPINT_NE );
      fixupmask = _mm512_cmp_epi32_mask( vfp32fixup, vfixupmask, _MM_CMPINT_EQ );
      vrnd = _mm512_mask_add_epi32( vrneadd , fixupmask, vrneadd, vfixup );
      vfp32rne  = _mm512_mask_add_epi32( vfp32, rnemask, vfp32, vrnd );
      vbfp16_32_hi = _mm512_srli_epi32( vfp32rne, 16 );
      vbfp16_32_hi = _mm512_slli_epi32(vbfp16_32_hi, 16);
      pair_fms = _mm512_or_epi32(vbfp16_32_lo, vbfp16_32_hi);
      _mm512_store_epi32( ((libxsmm_bfloat16*) bf16_weight_ptr), pair_fms);
    }
  } else {
    for ( j = 2*transform_thr_begin; j < 2*transform_thr_end; j+=2 ) {
      libxsmm_bfloat16 *bf16_weight_ptr = ((libxsmm_bfloat16*) handle->grad_filter->data) + j * 16;
      float *fp32_weight_ptr = ((float*) weight_ptr) + j * 16;
      __m512i fm0 = _mm512_castps_si512(LIBXSMM_INTRINSICS_MM512_LOAD_PS((float*)fp32_weight_ptr));
      __m512i fm1 = _mm512_castps_si512(LIBXSMM_INTRINSICS_MM512_LOAD_PS((float*)fp32_weight_ptr + 16));
      __m512i pair_fms;
      fm0 = _mm512_srli_epi32 (fm0, 16);
      fm1 = _mm512_srli_epi32 (fm1, 16);
      fm1 = _mm512_slli_epi32 (fm1, 16);
      pair_fms = _mm512_or_epi32(fm0, fm1);
      _mm512_store_epi32( ((libxsmm_bfloat16*) bf16_weight_ptr), pair_fms);
    }
  }
#else
#endif
}

#undef _mm512_loadcvt_bf16_fp32
libxsmm_barrier_wait(handle->barrier, ltid);

