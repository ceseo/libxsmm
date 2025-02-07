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
/* Alexander Heinecke, Evangelos Georganas (Intel Corp.)
******************************************************************************/
#include "generator_gemm_avx512_fsdbcst.h"
#include "generator_gemm_avx512_microkernel_fsdbcst.h"
#include "generator_gemm_common.h"
#include "generator_x86_instructions.h"
#include "generator_common.h"
#include "libxsmm_main.h"

#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(push,target(LIBXSMM_OFFLOAD_TARGET))
#endif
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(pop)
#endif

LIBXSMM_API_INLINE
void libxsmm_generator_gemm_avx512_kernel_fsdbcst_initialize_mask( libxsmm_generated_code*            io_generated_code,
                                                                   const libxsmm_gp_reg_mapping*      i_gp_reg_mapping,
                                                                   const libxsmm_micro_kernel_config* i_micro_kernel_config,
                                                                   const libxsmm_gemm_descriptor*     i_xgemm_desc,
                                                                   unsigned int                       i_m_done ) {
  unsigned int l_mask;

  /* init full mask */
  if ( LIBXSMM_GEMM_PRECISION_F64 == LIBXSMM_GETENUM_INP( i_xgemm_desc->datatype )  ) {
    l_mask = 0xff;
  } else {
    l_mask = 0xffff;
  }
  /* shift right by "inverse" remainder */
  l_mask = l_mask >> ( i_micro_kernel_config->vector_length - (i_xgemm_desc->m - i_m_done) );

  /* move mask to GP register */
  libxsmm_x86_instruction_alu_imm( io_generated_code,
                               i_micro_kernel_config->alu_mov_instruction,
                               i_gp_reg_mapping->gp_reg_help_5,
                               l_mask );

  if ( i_micro_kernel_config->instruction_set >= LIBXSMM_X86_AVX512 ) {
    libxsmm_x86_instruction_mask_move( io_generated_code,
                                       LIBXSMM_X86_INSTR_KMOVW,
                                       i_gp_reg_mapping->gp_reg_help_5,
                                       LIBXSMM_X86_AVX512_MASK );
  } else {
    /* shouldn't happen */
    LIBXSMM_HANDLE_ERROR( io_generated_code, LIBXSMM_ERR_ARCH );
    return;
  }
}

LIBXSMM_API_INLINE
void libxsmm_generator_gemm_avx512_kernel_fsdbcst_mloop( libxsmm_generated_code*            io_generated_code,
                                                         libxsmm_loop_label_tracker*        io_loop_label_tracker,
                                                         const libxsmm_gp_reg_mapping*      i_gp_reg_mapping,
                                                         const libxsmm_micro_kernel_config* i_micro_kernel_config,
                                                         const libxsmm_gemm_descriptor*     i_xgemm_desc,
                                                         const char*                        i_arch,
                                                         unsigned int                       i_n_blocking ) {
  /* set function pointers for AVX512 kernels */
  unsigned int (*l_generator_microkernel_kloop)( libxsmm_generated_code*, libxsmm_loop_label_tracker*, const libxsmm_gp_reg_mapping*, const libxsmm_micro_kernel_config*,
                                                 const libxsmm_gemm_descriptor*, const char*, unsigned int ) = 0;
  void (*l_generator_load)( libxsmm_generated_code*, const libxsmm_gp_reg_mapping*, const libxsmm_micro_kernel_config*,
                            const libxsmm_gemm_descriptor*, const unsigned int, const unsigned int ) = 0;
  void (*l_generator_store)( libxsmm_generated_code*, const libxsmm_gp_reg_mapping*, const libxsmm_micro_kernel_config*,
                            const libxsmm_gemm_descriptor*, const unsigned int, const unsigned int ) = 0;
  unsigned int l_k_unrolled;
  unsigned int l_m_done;

  if ( i_micro_kernel_config->instruction_set >= LIBXSMM_X86_AVX512 ) {
    l_generator_microkernel_kloop = libxsmm_generator_gemm_avx512_kernel_fsdbcst_kloop;
    l_generator_load = libxsmm_generator_gemm_load_C;
    l_generator_store = libxsmm_generator_gemm_store_C;
  } else {
    /* shouldn't happen */
    LIBXSMM_HANDLE_ERROR( io_generated_code, LIBXSMM_ERR_ARCH );
    return;
  }

  /* we proceed as much as we can in vector length steps, remainder is handled using masking */
  assert(0 < i_micro_kernel_config->vector_length);
  l_m_done = (i_xgemm_desc->m / i_micro_kernel_config->vector_length) * i_micro_kernel_config->vector_length;

  /* multiples of vector_length in M */
  if (l_m_done > 0) {
    libxsmm_generator_gemm_header_mloop( io_generated_code, io_loop_label_tracker, i_gp_reg_mapping, i_micro_kernel_config, i_micro_kernel_config->vector_length );
    l_generator_load( io_generated_code, i_gp_reg_mapping, i_micro_kernel_config,
        i_xgemm_desc, i_micro_kernel_config->vector_length, i_n_blocking );

    if (i_xgemm_desc->flags & LIBXSMM_GEMM_FLAG_BATCH_REDUCE) {
      libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_mloop);
      libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_nloop);
      /* This is the reduce loop  */
      libxsmm_generator_gemm_header_reduceloop( io_generated_code, io_loop_label_tracker, i_gp_reg_mapping, i_micro_kernel_config );
      libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_a);
      libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_b);
      /* load to reg_a the proper array based on the reduce loop index  */
      libxsmm_x86_instruction_alu_mem( io_generated_code,
          i_micro_kernel_config->alu_mov_instruction,
          i_gp_reg_mapping->gp_reg_a,
          i_gp_reg_mapping->gp_reg_reduce_loop, 8,
          0,
          i_gp_reg_mapping->gp_reg_a,
          0 );
      /* load to reg_b the proper array based on the reduce loop index  */
      libxsmm_x86_instruction_alu_mem( io_generated_code,
          i_micro_kernel_config->alu_mov_instruction,
          i_gp_reg_mapping->gp_reg_b,
          i_gp_reg_mapping->gp_reg_reduce_loop, 8,
          0,
          i_gp_reg_mapping->gp_reg_b,
          0 );
    }

    /* i_n_blocking is greater 18  -> push prefetch gpr for more register space */
    if ( i_n_blocking > 18 ) {
      libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_c );
    }

    l_k_unrolled = l_generator_microkernel_kloop( io_generated_code,
        io_loop_label_tracker,
        i_gp_reg_mapping,
        i_micro_kernel_config,
        i_xgemm_desc,
        i_arch,
        i_n_blocking );

    if (0 != io_generated_code->last_error) {
      return; /* propagate error */
    }

    /* i_n_blocking is greater 18  -> push prefetch gpr for more register space */
    if ( i_n_blocking > 18 ) {
      libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_c );
    }

    if (i_xgemm_desc->flags & LIBXSMM_GEMM_FLAG_BATCH_REDUCE) {
      /* Pop address of B_array to help_0 and store proper address of B   */
      libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_help_0);
      libxsmm_x86_instruction_alu_mem( io_generated_code,
          i_micro_kernel_config->alu_mov_instruction,
          i_gp_reg_mapping->gp_reg_help_0,
          i_gp_reg_mapping->gp_reg_reduce_loop, 8,
          0,
          i_gp_reg_mapping->gp_reg_b,
          1 );
      libxsmm_x86_instruction_alu_reg( io_generated_code, i_micro_kernel_config->alu_mov_instruction, i_gp_reg_mapping->gp_reg_help_0, i_gp_reg_mapping->gp_reg_b);
      libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_help_0);
      libxsmm_x86_instruction_alu_mem( io_generated_code,
          i_micro_kernel_config->alu_mov_instruction,
          i_gp_reg_mapping->gp_reg_help_0,
          i_gp_reg_mapping->gp_reg_reduce_loop, 8,
          0,
          i_gp_reg_mapping->gp_reg_a,
          1 );
      libxsmm_x86_instruction_alu_reg( io_generated_code, i_micro_kernel_config->alu_mov_instruction, i_gp_reg_mapping->gp_reg_help_0, i_gp_reg_mapping->gp_reg_a);
      libxsmm_generator_gemm_footer_reduceloop( io_generated_code, io_loop_label_tracker, i_gp_reg_mapping, i_micro_kernel_config, i_xgemm_desc);
      libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_nloop);
      libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_mloop);
    }

    l_generator_store( io_generated_code, i_gp_reg_mapping, i_micro_kernel_config,
        i_xgemm_desc, i_micro_kernel_config->vector_length, i_n_blocking  );
    libxsmm_generator_gemm_footer_mloop( io_generated_code, io_loop_label_tracker, i_gp_reg_mapping, i_micro_kernel_config, i_xgemm_desc,
        i_micro_kernel_config->vector_length, l_m_done, l_k_unrolled );
  }

  /* Remainder Handling using Masking, we are using M loop counter register as GP register for the mask */
  if ( l_m_done != (unsigned int)i_xgemm_desc->m ) {
    /* request masking support, @TODO performance penalty here, as a new object is created */
    libxsmm_micro_kernel_config l_micro_kernel_config_mask;
    libxsmm_generator_gemm_init_micro_kernel_config_fullvector( &l_micro_kernel_config_mask, i_xgemm_desc, i_arch, 1 );

    /* initialize k1 register */
    libxsmm_generator_gemm_avx512_kernel_fsdbcst_initialize_mask (  io_generated_code,
                                                                    i_gp_reg_mapping,
                                                                   &l_micro_kernel_config_mask,
                                                                    i_xgemm_desc,
                                                                    l_m_done );

    /* run masked micro kernel */
    l_generator_load( io_generated_code, i_gp_reg_mapping, &l_micro_kernel_config_mask,
        i_xgemm_desc, l_micro_kernel_config_mask.vector_length, i_n_blocking );

    if (i_xgemm_desc->flags & LIBXSMM_GEMM_FLAG_BATCH_REDUCE) {
      libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_mloop);
      libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_nloop);
      /* This is the reduce loop  */
      libxsmm_generator_gemm_header_reduceloop( io_generated_code, io_loop_label_tracker, i_gp_reg_mapping, i_micro_kernel_config );
      libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_a);
      libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_b);
      /* load to reg_a the proper array based on the reduce loop index  */
      libxsmm_x86_instruction_alu_mem( io_generated_code,
          i_micro_kernel_config->alu_mov_instruction,
          i_gp_reg_mapping->gp_reg_a,
          i_gp_reg_mapping->gp_reg_reduce_loop, 8,
          0,
          i_gp_reg_mapping->gp_reg_a,
          0 );
      /* load to reg_b the proper array based on the reduce loop index  */
      libxsmm_x86_instruction_alu_mem( io_generated_code,
          i_micro_kernel_config->alu_mov_instruction,
          i_gp_reg_mapping->gp_reg_b,
          i_gp_reg_mapping->gp_reg_reduce_loop, 8,
          0,
          i_gp_reg_mapping->gp_reg_b,
          0 );
    }

    /* i_n_blocking is greater 18  -> push prefetch gpr for more register space */
    if ( i_n_blocking > 18 ) {
      libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_c );
    }

    l_k_unrolled = l_generator_microkernel_kloop( io_generated_code,
        io_loop_label_tracker,
        i_gp_reg_mapping,
        &l_micro_kernel_config_mask,
        i_xgemm_desc,
        i_arch,
        i_n_blocking );

    /* i_n_blocking is greater 18  -> push prefetch gpr for more register space */
    if ( i_n_blocking > 18 ) {
      libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_c );
    }

    if (i_xgemm_desc->flags & LIBXSMM_GEMM_FLAG_BATCH_REDUCE) {
      /* Pop address of B_array to help_0 and store proper address of B   */
      libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_help_0);
      libxsmm_x86_instruction_alu_mem( io_generated_code,
          i_micro_kernel_config->alu_mov_instruction,
          i_gp_reg_mapping->gp_reg_help_0,
          i_gp_reg_mapping->gp_reg_reduce_loop, 8,
          0,
          i_gp_reg_mapping->gp_reg_b,
          1 );
      libxsmm_x86_instruction_alu_reg( io_generated_code, i_micro_kernel_config->alu_mov_instruction, i_gp_reg_mapping->gp_reg_help_0, i_gp_reg_mapping->gp_reg_b);
      libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_help_0);
      libxsmm_x86_instruction_alu_mem( io_generated_code,
          i_micro_kernel_config->alu_mov_instruction,
          i_gp_reg_mapping->gp_reg_help_0,
          i_gp_reg_mapping->gp_reg_reduce_loop, 8,
          0,
          i_gp_reg_mapping->gp_reg_a,
          1 );
      libxsmm_x86_instruction_alu_reg( io_generated_code, i_micro_kernel_config->alu_mov_instruction, i_gp_reg_mapping->gp_reg_help_0, i_gp_reg_mapping->gp_reg_a);
      libxsmm_generator_gemm_footer_reduceloop( io_generated_code, io_loop_label_tracker, i_gp_reg_mapping, i_micro_kernel_config, i_xgemm_desc);
      libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_nloop );
      libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_mloop );
    }

    l_generator_store( io_generated_code, i_gp_reg_mapping, &l_micro_kernel_config_mask,
        i_xgemm_desc, l_micro_kernel_config_mask.vector_length, i_n_blocking  );

    /* adjust pointers as we don't have a m-loop body */
    /* C */
    if ( LIBXSMM_GEMM_PRECISION_BF16 == LIBXSMM_GETENUM_OUT( i_xgemm_desc->datatype ) ) {
      libxsmm_x86_instruction_alu_imm( io_generated_code,
          l_micro_kernel_config_mask.alu_add_instruction,
          i_gp_reg_mapping->gp_reg_c,
          (i_xgemm_desc->m - l_m_done) * l_micro_kernel_config_mask.datatype_size/2 );
    } else {
      libxsmm_x86_instruction_alu_imm( io_generated_code,
          l_micro_kernel_config_mask.alu_add_instruction,
          i_gp_reg_mapping->gp_reg_c,
          (i_xgemm_desc->m - l_m_done) * l_micro_kernel_config_mask.datatype_size );
    }
    /* A */
    if (l_k_unrolled == 0) {
      if (i_xgemm_desc->flags & LIBXSMM_GEMM_FLAG_BATCH_REDUCE) {
        /* Adjust all A ptrs... */
        libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_help_0 );
        libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_reduce_loop );
        libxsmm_generator_gemm_header_reduceloop( io_generated_code, io_loop_label_tracker, i_gp_reg_mapping, i_micro_kernel_config );
        libxsmm_x86_instruction_alu_mem( io_generated_code,
            i_micro_kernel_config->alu_mov_instruction,
            i_gp_reg_mapping->gp_reg_a,
            i_gp_reg_mapping->gp_reg_reduce_loop, 8,
            0,
            i_gp_reg_mapping->gp_reg_help_0,
            0 );

        libxsmm_x86_instruction_alu_imm( io_generated_code,
            l_micro_kernel_config_mask.alu_sub_instruction,
            i_gp_reg_mapping->gp_reg_help_0,
            (i_xgemm_desc->k * l_micro_kernel_config_mask.datatype_size * i_xgemm_desc->lda) - ((i_xgemm_desc->m - l_m_done) * l_micro_kernel_config_mask.datatype_size) );

        libxsmm_x86_instruction_alu_mem( io_generated_code,
            i_micro_kernel_config->alu_mov_instruction,
            i_gp_reg_mapping->gp_reg_a,
            i_gp_reg_mapping->gp_reg_reduce_loop, 8,
            0,
            i_gp_reg_mapping->gp_reg_help_0,
            1 );
        libxsmm_generator_gemm_footer_reduceloop( io_generated_code, io_loop_label_tracker, i_gp_reg_mapping, i_micro_kernel_config, i_xgemm_desc);
        libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_reduce_loop );
        libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_help_0 );
      } else {
        libxsmm_x86_instruction_alu_imm( io_generated_code,
            l_micro_kernel_config_mask.alu_sub_instruction,
            i_gp_reg_mapping->gp_reg_a,
            (i_xgemm_desc->k * l_micro_kernel_config_mask.datatype_size * i_xgemm_desc->lda) - ((i_xgemm_desc->m - l_m_done) * l_micro_kernel_config_mask.datatype_size) );
      }
    } else {
      if (i_xgemm_desc->flags & LIBXSMM_GEMM_FLAG_BATCH_REDUCE) {
        /* Adjust all A ptrs... */
        libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_help_0 );
        libxsmm_x86_instruction_push_reg( io_generated_code, i_gp_reg_mapping->gp_reg_reduce_loop );
        libxsmm_generator_gemm_header_reduceloop( io_generated_code, io_loop_label_tracker, i_gp_reg_mapping, i_micro_kernel_config );
        libxsmm_x86_instruction_alu_mem( io_generated_code,
            i_micro_kernel_config->alu_mov_instruction,
            i_gp_reg_mapping->gp_reg_a,
            i_gp_reg_mapping->gp_reg_reduce_loop, 8,
            0,
            i_gp_reg_mapping->gp_reg_help_0,
            0 );

        libxsmm_x86_instruction_alu_imm( io_generated_code,
            l_micro_kernel_config_mask.alu_add_instruction,
            i_gp_reg_mapping->gp_reg_help_0,
            ((i_xgemm_desc->m - l_m_done) * l_micro_kernel_config_mask.datatype_size) );

        libxsmm_x86_instruction_alu_mem( io_generated_code,
            i_micro_kernel_config->alu_mov_instruction,
            i_gp_reg_mapping->gp_reg_a,
            i_gp_reg_mapping->gp_reg_reduce_loop, 8,
            0,
            i_gp_reg_mapping->gp_reg_help_0,
            1 );
        libxsmm_generator_gemm_footer_reduceloop( io_generated_code, io_loop_label_tracker, i_gp_reg_mapping, i_micro_kernel_config, i_xgemm_desc);
        libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_reduce_loop );
        libxsmm_x86_instruction_pop_reg( io_generated_code, i_gp_reg_mapping->gp_reg_help_0 );
      } else {
        libxsmm_x86_instruction_alu_imm( io_generated_code,
            l_micro_kernel_config_mask.alu_add_instruction,
            i_gp_reg_mapping->gp_reg_a,
            ((i_xgemm_desc->m - l_m_done) * l_micro_kernel_config_mask.datatype_size) );
      }
    }
  }
}

LIBXSMM_API_INTERN
unsigned int libxsmm_generator_gemm_avx512_fsdbcst_get_max_n_blocking( const libxsmm_gemm_descriptor* i_xgemm_desc,
                                                                       const char*                    i_arch ) {
  /* handle KNM qmadd */
  if ( (strcmp(i_arch, "knm") == 0) && (LIBXSMM_GEMM_PRECISION_F32 == LIBXSMM_GETENUM_INP( i_xgemm_desc->datatype )) ) {
    return 28;
  }

  /* handle int16 on SKX */
  if ( (strcmp(i_arch, "skx") == 0) && (LIBXSMM_GEMM_PRECISION_I16 == LIBXSMM_GETENUM_INP( i_xgemm_desc->datatype )) ) {
    return 28;
  }

  /* handle bf16 */
  if ( /*(strcmp(i_arch, "skx") == 0) &&*/ (LIBXSMM_GEMM_PRECISION_BF16 == LIBXSMM_GETENUM_INP( i_xgemm_desc->datatype )) ) {
    return 28;
  }

  return 30;
}

LIBXSMM_API_INTERN
void libxsmm_generator_gemm_avx512_kernel_fsdbcst( libxsmm_generated_code*        io_generated_code,
                                                   const libxsmm_gemm_descriptor* i_xgemm_desc,
                                                   const char*                    i_arch ) {
  libxsmm_micro_kernel_config l_micro_kernel_config;
  libxsmm_loop_label_tracker l_loop_label_tracker;
  libxsmm_gp_reg_mapping l_gp_reg_mapping;

  unsigned int l_max_n_rb_block = libxsmm_generator_gemm_avx512_fsdbcst_get_max_n_blocking( i_xgemm_desc, i_arch );
  unsigned int l_number_of_chunks = 1+((i_xgemm_desc->n-1)/l_max_n_rb_block);
  unsigned int l_modulo = i_xgemm_desc->n%l_number_of_chunks;
  unsigned int l_n2 = i_xgemm_desc->n/l_number_of_chunks;
  unsigned int l_n1 = l_n2 + 1;
  unsigned int l_N2 = 0;
  unsigned int l_N1 = 0;
  unsigned int l_chunk = 0;

  /* define gp register mapping */
  libxsmm_reset_x86_gp_reg_mapping( &l_gp_reg_mapping );
#if defined(_WIN32) || defined(__CYGWIN__)
  l_gp_reg_mapping.gp_reg_a = LIBXSMM_X86_GP_REG_RCX;
  l_gp_reg_mapping.gp_reg_b = LIBXSMM_X86_GP_REG_RDX;
  l_gp_reg_mapping.gp_reg_c = LIBXSMM_X86_GP_REG_R8;
  l_gp_reg_mapping.gp_reg_a_prefetch = LIBXSMM_X86_GP_REG_R9;
  /* TODO: full support for Windows calling convention */
  l_gp_reg_mapping.gp_reg_b_prefetch = LIBXSMM_X86_GP_REG_RDI;
  l_gp_reg_mapping.gp_reg_c_prefetch = LIBXSMM_X86_GP_REG_RSI;
#else /* match calling convention on Linux */
  l_gp_reg_mapping.gp_reg_a = LIBXSMM_X86_GP_REG_RDI;
  l_gp_reg_mapping.gp_reg_b = LIBXSMM_X86_GP_REG_RSI;
  l_gp_reg_mapping.gp_reg_c = LIBXSMM_X86_GP_REG_RDX;
  l_gp_reg_mapping.gp_reg_a_prefetch = LIBXSMM_X86_GP_REG_RCX;
  l_gp_reg_mapping.gp_reg_b_prefetch = LIBXSMM_X86_GP_REG_R8;
  l_gp_reg_mapping.gp_reg_c_prefetch = LIBXSMM_X86_GP_REG_R9;
  /* If we are generating the batchreduce kernel, then we rename the registers  */
  if (i_xgemm_desc->flags & LIBXSMM_GEMM_FLAG_BATCH_REDUCE) {
    l_gp_reg_mapping.gp_reg_a = LIBXSMM_X86_GP_REG_RDI;
    l_gp_reg_mapping.gp_reg_b = LIBXSMM_X86_GP_REG_RSI;
    l_gp_reg_mapping.gp_reg_c = LIBXSMM_X86_GP_REG_RDX;
    l_gp_reg_mapping.gp_reg_reduce_count = LIBXSMM_X86_GP_REG_RCX;
    l_gp_reg_mapping.gp_reg_a_prefetch = LIBXSMM_X86_GP_REG_R8;
    l_gp_reg_mapping.gp_reg_b_prefetch = LIBXSMM_X86_GP_REG_R9;
    l_gp_reg_mapping.gp_reg_reduce_loop = LIBXSMM_X86_GP_REG_R12;
    l_gp_reg_mapping.gp_reg_help_0 = LIBXSMM_X86_GP_REG_R13;
  }
#endif
  l_gp_reg_mapping.gp_reg_mloop = LIBXSMM_X86_GP_REG_R12;
  l_gp_reg_mapping.gp_reg_nloop = LIBXSMM_X86_GP_REG_R13;
  l_gp_reg_mapping.gp_reg_kloop = LIBXSMM_X86_GP_REG_R14;
  l_gp_reg_mapping.gp_reg_help_0 = LIBXSMM_X86_GP_REG_R15; /* masking */
  l_gp_reg_mapping.gp_reg_help_1 = LIBXSMM_X86_GP_REG_RAX; /* B stride helper */
  l_gp_reg_mapping.gp_reg_help_2 = LIBXSMM_X86_GP_REG_RBX; /* B stride helper */
  l_gp_reg_mapping.gp_reg_help_3 = LIBXSMM_X86_GP_REG_R11;  /* B stride helper */
  l_gp_reg_mapping.gp_reg_help_4 = LIBXSMM_X86_GP_REG_R10; /* B stride helper */
  l_gp_reg_mapping.gp_reg_help_5 = LIBXSMM_X86_GP_REG_R9; /* B stride helper */

  /* define loop_label_tracker */
  libxsmm_reset_loop_label_tracker( &l_loop_label_tracker );

  /* define the micro kernel code gen properties */
  libxsmm_generator_gemm_init_micro_kernel_config_fullvector( &l_micro_kernel_config, i_xgemm_desc, i_arch, 0 );

  if (l_n1 > l_max_n_rb_block) l_n1 = l_max_n_rb_block; /* this just the case if i_xgemm_desc->n/l_number_of_chunks has no remainder */
  for (l_chunk = 0; l_chunk < l_number_of_chunks; l_chunk++) {
    if (l_chunk < l_modulo) {
      l_N1 += l_n1;
    } else {
      l_N2 += l_n2;
    }
  }

  /* printf("N splitting of DP AVX512 Kernel: %i %i %i %i\n", l_N1, l_N2, l_n1, l_n2); */

  /* open asm */
  libxsmm_x86_instruction_open_stream( io_generated_code, &l_gp_reg_mapping, i_arch, i_xgemm_desc->prefetch );

  /* Load the actual batch-reduce trip count */
  if (i_xgemm_desc->flags & LIBXSMM_GEMM_FLAG_BATCH_REDUCE) {
    libxsmm_x86_instruction_alu_mem( io_generated_code,
        l_micro_kernel_config.alu_mov_instruction,
        l_gp_reg_mapping.gp_reg_reduce_count,
        LIBXSMM_X86_GP_REG_UNDEF, 0,
        0,
        l_gp_reg_mapping.gp_reg_reduce_count,
        0 );
  }

  if (l_number_of_chunks == 1) {
    libxsmm_generator_gemm_avx512_kernel_fsdbcst_mloop( io_generated_code, &l_loop_label_tracker, &l_gp_reg_mapping, &l_micro_kernel_config,
        i_xgemm_desc, i_arch, i_xgemm_desc->n);
  } else {
    if ((l_N2 > 0) && (l_N1 > 0)) {
      libxsmm_generator_gemm_header_nloop( io_generated_code, &l_loop_label_tracker, &l_gp_reg_mapping, &l_micro_kernel_config, l_n1 );
      libxsmm_generator_gemm_avx512_kernel_fsdbcst_mloop( io_generated_code, &l_loop_label_tracker, &l_gp_reg_mapping, &l_micro_kernel_config,
          i_xgemm_desc, i_arch, l_n1);
      libxsmm_generator_gemm_footer_nloop( io_generated_code, &l_loop_label_tracker, &l_gp_reg_mapping, &l_micro_kernel_config, i_xgemm_desc, l_n1, l_N1 );

      libxsmm_generator_gemm_header_nloop( io_generated_code, &l_loop_label_tracker, &l_gp_reg_mapping, &l_micro_kernel_config, l_n2 );
      libxsmm_generator_gemm_avx512_kernel_fsdbcst_mloop( io_generated_code, &l_loop_label_tracker, &l_gp_reg_mapping, &l_micro_kernel_config,
          i_xgemm_desc, i_arch, l_n2);
      libxsmm_generator_gemm_footer_nloop( io_generated_code, &l_loop_label_tracker, &l_gp_reg_mapping, &l_micro_kernel_config, i_xgemm_desc, l_n2, i_xgemm_desc->n );
    } else if ((l_N2 > 0) && (l_N1 == 0)) {
      libxsmm_generator_gemm_header_nloop( io_generated_code, &l_loop_label_tracker, &l_gp_reg_mapping, &l_micro_kernel_config, l_n2 );
      libxsmm_generator_gemm_avx512_kernel_fsdbcst_mloop( io_generated_code, &l_loop_label_tracker, &l_gp_reg_mapping, &l_micro_kernel_config,
          i_xgemm_desc, i_arch, l_n2);
      libxsmm_generator_gemm_footer_nloop( io_generated_code, &l_loop_label_tracker, &l_gp_reg_mapping, &l_micro_kernel_config, i_xgemm_desc, l_n2, i_xgemm_desc->n );
    } else {}
  }

  /* close asm */
  libxsmm_x86_instruction_close_stream( io_generated_code, &l_gp_reg_mapping, i_arch, i_xgemm_desc->prefetch );
}

