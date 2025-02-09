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
/* Kunal Banerjee (Intel Corp.)
******************************************************************************/
#include <libxsmm.h>
#include <libxsmm_intrinsics_x86.h>

#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(push,target(LIBXSMM_OFFLOAD_TARGET))
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#if defined(_OPENMP)
# include <omp.h>
#endif
#if defined(LIBXSMM_OFFLOAD_TARGET)
# pragma offload_attribute(pop)
#endif

#define CHKERR_LIBXSMM_DNN(A) if ( A != LIBXSMM_DNN_SUCCESS ) fprintf(stderr, "%s\n", libxsmm_dnn_get_error(A) );


LIBXSMM_INLINE void zero_buf(float* buf, size_t size) {
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < (int)size; ++i) {
    buf[i] = 0.0f;
  }
}


LIBXSMM_INLINE void matrix_add(int size, float *a, float *b, float *c)
{
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < size; i++) {
    c[i] = a[i] + b[i];
  }
}


LIBXSMM_INLINE void matrix_eltwise_mult(int size, float *a, float *b, float *c)
{
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < size; i++) {
    c[i] = a[i] * b[i];
  }
}


LIBXSMM_INLINE void matrix_sigmoid(int size, float *src, float *dst)
{
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < size; i++) {
    const float exp_value = (float)exp((double) -src[i]);
    dst[i] = 1 / (1 + exp_value);
  }
}


LIBXSMM_INLINE void matrix_tanh(int size, float *src, float *dst)
{
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < size; i++) {
    dst[i] = (float)tanh((double)src[i]);
  }
}


LIBXSMM_INLINE void matrix_relu(int size, float *src, float *dst)
{
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < size; i++) {
    dst[i] = (src[i] > 0.0f) ? src[i] : 0.0f;
  }
}


LIBXSMM_INLINE void matrix_sigmoid_inverse(int size, float *src, float *dst)
{
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < size; i++) {
    const float exp_value = (float)exp((double) -src[i]);
    const float sig_exp = 1 / (1 + exp_value);
    dst[i] = (1.0f - sig_exp)*sig_exp;
  }
}


LIBXSMM_INLINE void matrix_tanh_inverse(int size, float *src, float *dst)
{
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < size; i++) {
    const float tanh_value = (float)tanh((double)src[i]);
    dst[i] = 1.0f - (tanh_value * tanh_value);
  }
}


LIBXSMM_INLINE void matrix_relu_inverse(int size, float *src, float *dst)
{
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < size; i++) {
    dst[i] = (float)(src[i] > 0.0f ? 1.0f : 0.0f);
  }
}


LIBXSMM_INLINE void matrix_transpose(int rows, int cols, float *src, float *dst)
{
  libxsmm_otrans_omp(dst, src, sizeof(float), cols, rows, cols/*ldi*/, rows/*ldo*/);
}


LIBXSMM_INLINE void matrix_copy(int size, float *src, float *dst)
{
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < size; i++) {
    dst[i] = src[i];
  }
}

LIBXSMM_INLINE void matrix_copy_NC_to_NCNC(float *src, float *dst, int T, int N, int C, int bn, int bc)
{
  int t, n1, n2, c1, c2;
  int nBlocks = N/bn;
  int cBlocks = C/bc;
  LIBXSMM_VLA_DECL(3, float, real_src, src, N, C);
  LIBXSMM_VLA_DECL(5, float, real_dst, dst, nBlocks, cBlocks, bn, bc);

  for (t = 0; t < T; t++) {
#if defined(_OPENMP)
# pragma omp parallel for
#endif
    for (n1 = 0; n1 < nBlocks; n1++) {
      for (c1 = 0; c1 < cBlocks; c1++) {
        for (n2 = 0; n2 < bn; n2++) {
          for (c2 = 0; c2 < bc; c2++) {
            LIBXSMM_VLA_ACCESS(5, real_dst, t, n1, c1, n2, c2, nBlocks, cBlocks, bn, bc) =
              LIBXSMM_VLA_ACCESS(3, real_src, t, n1*bn+n2, c1*bc+c2, N, C);
          }
        }
      }
    }
  }
}

LIBXSMM_INLINE void matrix_copy_NCNC_to_NC(float *src, float *dst, int T, int N, int C, int bn, int bc)
{
  int t, n1, n2, c1, c2;
  int nBlocks = N/bn;
  int cBlocks = C/bc;
  LIBXSMM_VLA_DECL(3, float, real_dst, dst, N, C);
  LIBXSMM_VLA_DECL(5, float, real_src, src, nBlocks, cBlocks, bn, bc);

  for (t = 0; t < T; t++) {
#if defined(_OPENMP)
# pragma omp parallel for
#endif
    for (n1 = 0; n1 < nBlocks; n1++) {
      for (c1 = 0; c1 < cBlocks; c1++) {
        for (n2 = 0; n2 < bn; n2++) {
          for (c2 = 0; c2 < bc; c2++) {
            LIBXSMM_VLA_ACCESS(3, real_dst, t, n1*bn+n2, c1*bc+c2, N, C) =
              LIBXSMM_VLA_ACCESS(5, real_src, t, n1, c1, n2, c2, nBlocks, cBlocks, bn, bc);
          }
        }
      }
    }
  }
}

LIBXSMM_INLINE void matrix_copy_CK_to_KCCK(float *src, float *dst, int C, int K, int bc, int bk)
{
  int k1, k2, c1, c2;
  int kBlocks = K/bk;
  int cBlocks = C/bc;
  LIBXSMM_VLA_DECL(2, float, real_src, src, K);
  LIBXSMM_VLA_DECL(4, float, real_dst, dst, cBlocks, bc, bk);

#if defined(_OPENMP)
# pragma omp parallel for private(k1)
#endif
  for (k1 = 0; k1 < kBlocks; k1++) {
    for (c1 = 0; c1 < cBlocks; c1++) {
      for (c2 = 0; c2 < bc; c2++) {
        for (k2 = 0; k2 < bk; k2++) {
          LIBXSMM_VLA_ACCESS(4, real_dst, k1, c1, c2, k2, cBlocks, bc, bk) =
            LIBXSMM_VLA_ACCESS(2, real_src, c1*bc+c2, k1*bk+k2, K);
        }
      }
    }
  }
}

LIBXSMM_INLINE void matrix_copy_CK_to_CKKC(float *src, float *dst, int C, int K, int bc, int bk)
{
  int k1, k2, c1, c2;
  int kBlocks = K/bk;
  int cBlocks = C/bc;
  LIBXSMM_VLA_DECL(2, float, real_src, src, K);
  LIBXSMM_VLA_DECL(4, float, real_dst, dst, kBlocks, bk, bc);

#if defined(_OPENMP)
# pragma omp parallel for private(c1)
#endif
  for (c1 = 0; c1 < cBlocks; c1++) {
    for (k1 = 0; k1 < kBlocks; k1++) {
      for (k2 = 0; k2 < bk; k2++) {
        for (c2 = 0; c2 < bc; c2++) {
          LIBXSMM_VLA_ACCESS(4, real_dst, c1, k1, k2, c2, kBlocks, bk, bc) =
            LIBXSMM_VLA_ACCESS(2, real_src, c1*bc+c2, k1*bk+k2, K);
        }
      }
    }
  }
}

LIBXSMM_INLINE void matrix_complement(int size, float *src, float *dst)
{
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < size; i++) {
    dst[i] = 1.0f - src[i];
  }
}


LIBXSMM_INLINE void matrix_complement_square(int size, float *src, float *dst)
{
  int i;
#if defined(_OPENMP)
# pragma omp parallel for private(i)
#endif
  for (i = 0; i < size; i++) {
    dst[i] = 1.0f - (src[i] * src[i]);
  }
}


LIBXSMM_INLINE void rnn_copyin(int N, int C, int bn, int bc, float *src, float *dst)
{
  LIBXSMM_VLA_DECL(2, float, real_src, src, C);
  LIBXSMM_VLA_DECL(4, float, real_dst, dst, C/bc, bn, bc);
  int in, ic, jn, jc;

  for (in = 0; in < N; in += bn) {
    for (ic = 0; ic < C; ic += bc) {
      for (jn = 0; jn < bn; jn++) {
        for (jc = 0; jc < bc; jc++) {
          LIBXSMM_VLA_ACCESS(4, real_dst, in/bn, ic/bc, jn, jc, C/bc, bn, bc) =
            LIBXSMM_VLA_ACCESS(2, real_src, in + jn, ic + jc, C);
        }
      }
    }
  }
}


LIBXSMM_INLINE void rnn_copyout(int N, int C, int bn, int bc, float *src, float *dst)
{
  LIBXSMM_VLA_DECL(4, float, real_src, src, C/bc, bn, bc);
  LIBXSMM_VLA_DECL(2, float, real_dst, dst, C);
  int in, ic, jn, jc;

  for (in = 0; in < N; in += bn) {
    for (ic = 0; ic < C; ic += bc) {
      for (jn = 0; jn < bn; jn++) {
        for (jc = 0; jc < bc; jc++) {
          LIBXSMM_VLA_ACCESS(2, real_dst, in + jn, ic + jc, C) =
            LIBXSMM_VLA_ACCESS(4, real_src, in/bn, ic/bc, jn, jc, C/bc, bn, bc);
        }
      }
    }
  }
}


int main(int argc, char* argv[])
{
  /* Arrays related to FWD pass */
  float *wgold, *xgoldt, *ugold, *hpgold, *hgoldt, *z1gold, *z2gold, *zgoldt, *bgold, *bmgold;
  float *w, *wt, *xt, *u, *ut, *hp, *ht, *htest, *h_nc_buf, *b;
  /* Arrays related to BWD and UPD pass */
  float *djdhgoldt, *deltagoldt, *djdugold, *djdwgold, *djdxgoldt, *djdbgold;
  float *zigold, *di1gold, *di2gold, *ugoldTp, *wgoldTp, *hgoldTp, *xgoldTp;
  float *djdht, *djdu, *djdw, *djdxt, *djdb, *djdxtestt, *djdwtest, *djdutest;

  const char transa = 'N', transb = 'N'; /* no transposes */
  const float alpha = 1, beta = 1, beta0 = 0;
  void *scratch, *internalstate;
  size_t scratch_size = 0, internalstate_size = 0;

  int iters = 10; /* repetitions of benchmark */
  int pass = 0;   /* pass: 0--FWD, 1--BWD, 2--UPD, 3--BWD+UPD */
  int nonlin = 2; /* nonlin=1 denotes ReLU, 2 denotes sigmoid, 3 denotes tanh */
  int N = 128;    /* size of mini-batch */
  int C = 512;    /* number of inputs */
  int K = 256;    /* number of outputs */
  int t = 4;      /* number of time steps (> 1) */
  int bk = 64;
  int bn = 64;
  int bc = 64;

  const char *const env_check = getenv("CHECK");
  const double check = LIBXSMM_ABS(0 == env_check ? 1/*enable by default*/ : atof(env_check));

#if defined(_OPENMP)
  int nThreads = omp_get_max_threads(); /* number of threads */
#else
  int nThreads = 1; /* number of threads */
#endif

  unsigned long long l_start, l_end;
  double l_total = 0.0;
  double flops = 0.0, tempflops = 0.0;
  const double tflops = 12; /* transcendental flops */
  int i, j, it;

  libxsmm_dnn_rnncell_desc rnncell_desc;
  libxsmm_dnn_rnncell* libxsmm_handle;
  libxsmm_dnn_tensor* libxsmm_input;
  libxsmm_dnn_tensor* libxsmm_hidden_state_prev;
  libxsmm_dnn_tensor* libxsmm_weight;
  libxsmm_dnn_tensor* libxsmm_recur_weight;
  libxsmm_dnn_tensor* libxsmm_weight_t;
  libxsmm_dnn_tensor* libxsmm_recur_weight_t;
  libxsmm_dnn_tensor* libxsmm_bias;
  libxsmm_dnn_tensor* libxsmm_hidden_state;
  libxsmm_dnn_tensor* libxsmm_dinput;
  libxsmm_dnn_tensor* libxsmm_dweight;
  libxsmm_dnn_tensor* libxsmm_drecur_weight;
  libxsmm_dnn_tensor* libxsmm_dbias;
  libxsmm_dnn_tensor* libxsmm_dhidden_state;

  libxsmm_dnn_tensor_datalayout* libxsmm_layout;
  libxsmm_dnn_err_t status;
  libxsmm_dnn_err_t global_status = LIBXSMM_DNN_SUCCESS;

  libxsmm_matdiff_info norms_fwd, norms_bwd, norms_upd_w, norms_upd_u, norms_upd_b, diff;
  libxsmm_matdiff_clear(&norms_fwd);
  libxsmm_matdiff_clear(&norms_bwd);
  libxsmm_matdiff_clear(&norms_upd_w);
  libxsmm_matdiff_clear(&norms_upd_u);
  libxsmm_matdiff_clear(&norms_upd_b);
  libxsmm_matdiff_clear(&diff);

  if (argc > 1 && !strncmp(argv[1], "-h", 3)) {
    printf("\nUsage: ./rnndriver [reps] [pass: 0--FWD, 1--BWD, 2--UPD, 3--BWD+UPD] [nonlin: 1--ReLU, 2--sigmoid, 3--tanh] [N] [C] [K] [time_steps > 0]\n\n");
    return 0;
  }
  libxsmm_rng_set_seed(1);

  /* reading new values from cli */
  i = 1;
  if (argc > i) iters = atoi(argv[i++]);
  if (argc > i) pass  = atoi(argv[i++]);
  if (argc > i) nonlin= atoi(argv[i++]);
  if (argc > i) N     = atoi(argv[i++]);
  if (argc > i) C     = atoi(argv[i++]);
  if (argc > i) K     = atoi(argv[i++]);
  if (argc > i) t     = atoi(argv[i++]);
  if (argc > i) bn     = atoi(argv[i++]);
  if (argc > i) bk     = atoi(argv[i++]);
  if (argc > i) bc     = atoi(argv[i++]);

  if (t <= 0) {
    printf("time_steps %d should be greater than 0\n\n", t);
    return 0;
  }
  if (!(pass == 0 || pass == 1 || pass == 2 || pass == 3)) {
    printf("Unknown pass: %d, valid arguments for pass = {0(FWD), 1(BWD), 2(UPD), 3(BWD+UPD)\n\n", pass);
    return 0;
  }
  if (nonlin != 1 && nonlin != 2 && nonlin != 3) {
    printf("Unsupported non-linear function used [1--ReLU, 2--sigmoid, 3--tanh]\n\n");
    return 0;
  }

#if defined(__SSE3__)
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
  _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
  _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
#endif

  /* print some summary */
  printf("##########################################\n");
  printf("#          Setting Up (Common)           #\n");
  printf("##########################################\n");
  printf("PARAMS: N:%d  C:%d  K:%d  T:%d\n", N, C, K, t);
  printf("PARAMS: ITERS:%d", iters); if (LIBXSMM_FEQ(0, check)) printf("  Threads:%d\n", nThreads); else printf("\n");
  printf("SIZE Weight (MB): %10.2f MiB\n", (double)(C*K*sizeof(float))/(1024.0*1024.0) );
  printf("SIZE Input (MB): %10.2f MiB\n", (double)(N*C*sizeof(float))/(1024.0*1024.0) );
  printf("SIZE Hidden State: %10.2f MiB\n", (double)(K*N*sizeof(float))/(1024.0*1024.0) );

  /* allocate data */
  xgoldt = (float*)libxsmm_aligned_malloc(N*C*t*sizeof(float), 2097152);
  hpgold = (float*)libxsmm_aligned_malloc(K*N*sizeof(float), 2097152);
  wgold  = (float*)libxsmm_aligned_malloc(C*K*sizeof(float), 2097152);
  ugold  = (float*)libxsmm_aligned_malloc(K*K*sizeof(float), 2097152);
  bgold  = (float*)libxsmm_aligned_malloc(K*sizeof(float), 2097152);
  hgoldt = (float*)libxsmm_aligned_malloc(K*N*t*sizeof(float), 2097152);
  h_nc_buf = (float*)libxsmm_aligned_malloc(K*N*t*sizeof(float), 2097152);
  zgoldt = (float*)libxsmm_aligned_malloc(K*N*t*sizeof(float), 2097152);
  bmgold = (float*)libxsmm_aligned_malloc(K*N*sizeof(float), 2097152);
  z1gold = (float*)libxsmm_aligned_malloc(K*N*sizeof(float), 2097152);
  z2gold = (float*)libxsmm_aligned_malloc(K*N*sizeof(float), 2097152);
  djdxgoldt  = (float*)libxsmm_aligned_malloc(N*C*t*sizeof(float), 2097152);
  djdwgold   = (float*)libxsmm_aligned_malloc(C*K*sizeof(float), 2097152);
  djdugold   = (float*)libxsmm_aligned_malloc(K*K*sizeof(float), 2097152);
  djdbgold   = (float*)libxsmm_aligned_malloc(K*sizeof(float), 2097152);
  djdhgoldt  = (float*)libxsmm_aligned_malloc(K*N*t*sizeof(float), 2097152);
  deltagoldt = (float*)libxsmm_aligned_malloc(K*N*t*sizeof(float), 2097152);
  zigold     = (float*)libxsmm_aligned_malloc(K*N*sizeof(float), 2097152);
  di1gold    = (float*)libxsmm_aligned_malloc(K*N*sizeof(float), 2097152);
  di2gold    = (float*)libxsmm_aligned_malloc(K*N*sizeof(float), 2097152);
  xgoldTp    = (float*)libxsmm_aligned_malloc(N*C*sizeof(float), 2097152);
  wgoldTp    = (float*)libxsmm_aligned_malloc(C*K*sizeof(float), 2097152);
  ugoldTp    = (float*)libxsmm_aligned_malloc(K*K*sizeof(float), 2097152);
  hgoldTp    = (float*)libxsmm_aligned_malloc(K*N*sizeof(float), 2097152);
  xt     = (float*)libxsmm_aligned_malloc(N*C*t*sizeof(float), 2097152);
  hp     = (float*)libxsmm_aligned_malloc(K*N*sizeof(float), 2097152);
  w      = (float*)libxsmm_aligned_malloc(C*K*sizeof(float), 2097152);
  wt     = (float*)libxsmm_aligned_malloc(C*K*sizeof(float), 2097152);
  u      = (float*)libxsmm_aligned_malloc(K*K*sizeof(float), 2097152);
  ut     = (float*)libxsmm_aligned_malloc(K*K*sizeof(float), 2097152);
  ht     = (float*)libxsmm_aligned_malloc(K*N*t*sizeof(float), 2097152);
  b      = (float*)libxsmm_aligned_malloc(K*sizeof(float), 2097152);
  djdxt  = (float*)libxsmm_aligned_malloc(N*C*t*sizeof(float), 2097152);
  djdw   = (float*)libxsmm_aligned_malloc(C*K*sizeof(float), 2097152);
  djdu   = (float*)libxsmm_aligned_malloc(K*K*sizeof(float), 2097152);
  djdb   = (float*)libxsmm_aligned_malloc(K*sizeof(float), 2097152);
  djdht  = (float*)libxsmm_aligned_malloc(K*N*t*sizeof(float), 2097152);
  htest  = (float*)libxsmm_aligned_malloc(K*N*sizeof(float), 2097152);
  djdxtestt = (float*)libxsmm_aligned_malloc(N*C*t*sizeof(float), 2097152);
  djdwtest  = (float*)libxsmm_aligned_malloc(C*K*sizeof(float), 2097152);
  djdutest  = (float*)libxsmm_aligned_malloc(K*K*sizeof(float), 2097152);
  LIBXSMM_VLA_DECL(2, float, xgold, xgoldt, N*C);
  LIBXSMM_VLA_DECL(2, float, hgold, hgoldt, K*N);
  LIBXSMM_VLA_DECL(2, float, h_nc, h_nc_buf, K*N);
  LIBXSMM_VLA_DECL(2, float, zgold, zgoldt, K*N);
  LIBXSMM_VLA_DECL(2, float, djdxgold, djdxgoldt, N*C);
  LIBXSMM_VLA_DECL(2, float, djdhgold, djdhgoldt, K*N);
  LIBXSMM_VLA_DECL(2, float, deltagold, deltagoldt, K*N);
  /*LIBXSMM_VLA_DECL(2, float, djdh, djdht, K*N);*/
  /*LIBXSMM_VLA_DECL(2, float, hb, ht, K*N);*/
  /*LIBXSMM_VLA_DECL(2, float, djdx, djdxt, N*C);*/

  /* initialize data */
  /* All data in gold is considered to be in column-major format */
  for (it = 0; it < t; ++it) {
    LIBXSMM_MATINIT_OMP(float, 24, &LIBXSMM_VLA_ACCESS(2, xgold, it, 0, N*C), N, C, N, 1.0);
  }
  LIBXSMM_MATINIT_OMP(float, 24, hpgold,N, K, N, 1.0);
  LIBXSMM_MATINIT_OMP(float, 42, wgold, C, K, C, 1.0);
  LIBXSMM_MATINIT_OMP(float, 42, ugold, K, K, K, 1.0);
  LIBXSMM_MATINIT_OMP(float, 42, bgold, 1, K, 1, 1.0);
  for (j = 0; j < N; j++) {
    matrix_copy(K, bgold, &(bmgold[j*K]));
  }
  zero_buf(hgoldt, K*N*t);
  zero_buf(zgoldt, K*N*t);
  zero_buf(z1gold, K*N);
  zero_buf(z2gold, K*N);
  for (it = 0; it < t; ++it) {
    LIBXSMM_MATINIT_OMP(float, 24, &LIBXSMM_VLA_ACCESS(2, djdhgold, it, 0, K*N), N, K, N, 1.0);
  }
  zero_buf(djdxgoldt, N*C*t);
  zero_buf(djdwgold, C*K);
  zero_buf(djdugold, K*K);
  zero_buf(djdbgold, K);
  zero_buf(deltagoldt, K*N*t);
  zero_buf(zigold, K*N);
  zero_buf(di1gold, K*N);
  zero_buf(di2gold, K*N);
  zero_buf(xgoldTp, N*C);
  zero_buf(ugoldTp, K*K);
  zero_buf(wgoldTp, C*K);
  zero_buf(hgoldTp, K*N);

  /* first touch LIBXSMM */
  zero_buf(xt, N*C*t);
  zero_buf(hp, K*N);
  zero_buf(w,  C*K);
  zero_buf(u,  K*K);
  zero_buf(wt, C*K);
  zero_buf(ut, K*K);
  zero_buf(b,  K);
  zero_buf(ht, K*N*t);
  zero_buf(djdxt,N*C*t);
  zero_buf(djdw, C*K);
  zero_buf(djdu, K*K);
  zero_buf(djdb, K);
  zero_buf(djdht, K*N*t);
  /*LIBXSMM_VLA_DECL(2, float, x, xt, N*C);*/
  /*LIBXSMM_VLA_DECL(2, float, h, ht, K*N);*/

  if (LIBXSMM_NEQ(0, check)) {
    printf("##########################################\n");
    printf("#         Computing Reference ...        #\n");
    printf("##########################################\n");
    for (i = 0; i < t; ++i) {
      LIBXSMM_XBLAS_SYMBOL(float)(&transa, &transb, &K, &N, &C, &alpha, wgold, &K, &LIBXSMM_VLA_ACCESS(2, xgold, i, 0, N*C), &C, &beta0, z1gold, &K);
      if (0 == i) {
        LIBXSMM_XBLAS_SYMBOL(float)(&transa, &transb, &K, &N, &K, &alpha, ugold, &K, hpgold, &K, &beta0, z2gold, &K);
      } else {
        LIBXSMM_XBLAS_SYMBOL(float)(&transa, &transb, &K, &N, &K, &alpha, ugold, &K, &LIBXSMM_VLA_ACCESS(2, hgold, i-1, 0, K*N), &K, &beta0, z2gold, &K);
      }
      matrix_add(K*N, z1gold, z2gold, &LIBXSMM_VLA_ACCESS(2, zgold, i, 0, K*N));
      matrix_add(K*N, &LIBXSMM_VLA_ACCESS(2, zgold, i, 0, K*N), bmgold, &LIBXSMM_VLA_ACCESS(2, zgold, i, 0, K*N));
      if (1 == nonlin) {
        matrix_relu(K*N, &LIBXSMM_VLA_ACCESS(2, zgold, i, 0, K*N), &LIBXSMM_VLA_ACCESS(2, hgold, i, 0, K*N));
      } else if (2 == nonlin) {
        matrix_sigmoid(K*N, &LIBXSMM_VLA_ACCESS(2, zgold, i, 0, K*N), &LIBXSMM_VLA_ACCESS(2, hgold, i, 0, K*N));
      } else {
        matrix_tanh(K*N, &LIBXSMM_VLA_ACCESS(2, zgold, i, 0, K*N), &LIBXSMM_VLA_ACCESS(2, hgold, i, 0, K*N));
      }
    }
    /* Conceptually, delta iterates over 0 ... t-1, whereas, djdh and z iterates over 1 ... t */
    /* Hence these have identical array indices */
    if (1 == nonlin) {
      matrix_relu_inverse(K*N, &LIBXSMM_VLA_ACCESS(2, zgold, t-1, 0, K*N), zigold);
    } else if (2 == nonlin) {
      matrix_sigmoid_inverse(K*N, &LIBXSMM_VLA_ACCESS(2, zgold, t-1, 0, K*N), zigold);
    } else {
      matrix_tanh_inverse(K*N, &LIBXSMM_VLA_ACCESS(2, zgold, t-1, 0, K*N), zigold);
    }
    matrix_eltwise_mult(K*N, zigold, &LIBXSMM_VLA_ACCESS(2, djdhgold, t-1, 0, K*N), &LIBXSMM_VLA_ACCESS(2, deltagold, t-1, 0, K*N));
    matrix_transpose(K, K, ugold, ugoldTp);
    for (i = t-2; i >= 0; --i) {
      if (1 == nonlin) {
        matrix_relu_inverse(K*N, &LIBXSMM_VLA_ACCESS(2, zgold, i, 0, K*N), zigold);
      } else if (2 == nonlin) {
        matrix_sigmoid_inverse(K*N, &LIBXSMM_VLA_ACCESS(2, zgold, i, 0, K*N), zigold);
      } else {
        matrix_tanh_inverse(K*N, &LIBXSMM_VLA_ACCESS(2, zgold, i, 0, K*N), zigold);
      }
      LIBXSMM_XBLAS_SYMBOL(float)(&transa, &transb, &K, &N, &K, &alpha, ugoldTp, &K, &LIBXSMM_VLA_ACCESS(2, deltagold, i+1, 0, K*N), &K, &beta0, di1gold, &K);
      matrix_add(K*N, &LIBXSMM_VLA_ACCESS(2, djdhgold, i, 0, K*N), di1gold, di2gold);
      matrix_eltwise_mult(K*N, zigold, di2gold, &LIBXSMM_VLA_ACCESS(2, deltagold, i, 0, K*N));
    }
    if (pass == 1 || pass == 3) {
      matrix_transpose(C, K, wgold, wgoldTp);
      for (i = 0; i < t; ++i) {
        LIBXSMM_XBLAS_SYMBOL(float)(&transa, &transb, &C, &N, &K, &alpha, wgoldTp, &C, &LIBXSMM_VLA_ACCESS(2, deltagold, i, 0, K*N), &K, &beta0, &LIBXSMM_VLA_ACCESS(2, djdxgold, i, 0, N*C), &C);
      }
    }
    if (pass == 2 || pass == 3) {
      for (i = 0; i < t; ++i) {
        if (0 == i) {
          matrix_transpose(N, K, hpgold, hgoldTp);
        } else {
          matrix_transpose(N, K, &LIBXSMM_VLA_ACCESS(2, hgold, i-1, 0, K*N), hgoldTp);
        }
        LIBXSMM_XBLAS_SYMBOL(float)(&transa, &transb, &K, &K, &N, &alpha, &LIBXSMM_VLA_ACCESS(2, deltagold, i, 0, K*N), &K, hgoldTp, &N, &beta, djdugold, &K);
        matrix_transpose(N, C, &LIBXSMM_VLA_ACCESS(2, xgold, i, 0, N*C), xgoldTp);
        LIBXSMM_XBLAS_SYMBOL(float)(&transa, &transb, &K, &C, &N, &alpha, &LIBXSMM_VLA_ACCESS(2, deltagold, i, 0, K*N), &K, xgoldTp, &N, &beta, djdwgold, &K);
        for (j = 0; j < K*N; j++) {
          djdbgold[j%K] += LIBXSMM_VLA_ACCESS(2, deltagold, i, j, K*N);
        }
      }
    }
    printf("##########################################\n");
    printf("#      Computing Reference ... done      #\n");
    printf("##########################################\n");
  }

  if (1 /* format == 'A' || format == 'L' */) {
    printf("\n");
    printf("##########################################\n");
    printf("#      Setting Up  (custom-Storage)      #\n");
    printf("##########################################\n");

    /* setup LIBXSMM handle */
    rnncell_desc.threads = nThreads;
    rnncell_desc.N = N;
    rnncell_desc.C = C;
    rnncell_desc.K = K;
    rnncell_desc.bn = bn;
    rnncell_desc.bk = bk;
    rnncell_desc.bc = bc;
    rnncell_desc.max_T = t;

    if ( nonlin == 1 ) {
      rnncell_desc.cell_type = LIBXSMM_DNN_RNNCELL_RNN_RELU;
    } else if ( nonlin == 2 ) {
      rnncell_desc.cell_type = LIBXSMM_DNN_RNNCELL_RNN_SIGMOID;
    } else if ( nonlin == 3 ) {
      rnncell_desc.cell_type = LIBXSMM_DNN_RNNCELL_RNN_TANH;
    } else {
      /* should not happen */
    }
    rnncell_desc.datatype_in = LIBXSMM_DNN_DATATYPE_F32;
    rnncell_desc.datatype_out = LIBXSMM_DNN_DATATYPE_F32;
    rnncell_desc.buffer_format = LIBXSMM_DNN_TENSOR_FORMAT_NCPACKED;
    rnncell_desc.filter_format = LIBXSMM_DNN_TENSOR_FORMAT_CKPACKED;

    libxsmm_handle = libxsmm_dnn_create_rnncell( rnncell_desc, &status );
    CHKERR_LIBXSMM_DNN( status );

    /* setup LIBXSMM buffers and filter */
    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_INPUT, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_input = libxsmm_dnn_link_tensor( libxsmm_layout, xt, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_HIDDEN_STATE_PREV, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_hidden_state_prev = libxsmm_dnn_link_tensor( libxsmm_layout, hp, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_WEIGHT, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_weight = libxsmm_dnn_link_tensor( libxsmm_layout, w, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_WEIGHT_TRANS, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_weight_t = libxsmm_dnn_link_tensor( libxsmm_layout, wt, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_RECUR_WEIGHT, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_recur_weight = libxsmm_dnn_link_tensor( libxsmm_layout, u, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_RECUR_WEIGHT_TRANS, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_recur_weight_t = libxsmm_dnn_link_tensor( libxsmm_layout, ut, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_BIAS, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_bias = libxsmm_dnn_link_tensor( libxsmm_layout, b, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_HIDDEN_STATE, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_hidden_state = libxsmm_dnn_link_tensor( libxsmm_layout, ht, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_GRADIENT_INPUT, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dinput = libxsmm_dnn_link_tensor( libxsmm_layout, djdxt, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_GRADIENT_WEIGHT, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dweight = libxsmm_dnn_link_tensor( libxsmm_layout, djdw, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_GRADIENT_RECUR_WEIGHT, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_drecur_weight = libxsmm_dnn_link_tensor( libxsmm_layout, djdu, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_GRADIENT_BIAS, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dbias = libxsmm_dnn_link_tensor( libxsmm_layout, djdb, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    libxsmm_layout = libxsmm_dnn_rnncell_create_tensor_datalayout( libxsmm_handle, LIBXSMM_DNN_RNN_GRADIENT_HIDDEN_STATE, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dhidden_state = libxsmm_dnn_link_tensor( libxsmm_layout, djdht, &status ); CHKERR_LIBXSMM_DNN( status );
    libxsmm_dnn_destroy_tensor_datalayout( libxsmm_layout );

    /* copy in data to LIBXSMM format */
    /*matrix_copy( t*N*C, xgoldt, xt );
      matrix_copy( K*N, hpgold, hp );
      matrix_copy( C*K, wgold, w );
      matrix_copy( K*K, ugold, u );*/
    matrix_copy( K, bgold, b );
    matrix_copy_NC_to_NCNC(xgoldt, xt, t, N, C, bn, bc);
    matrix_copy_NC_to_NCNC(hpgold, hp, 1, N, K, bn, bk);
    matrix_copy_CK_to_KCCK(wgold, w,  C, K, bc, bk);
    matrix_copy_CK_to_KCCK(ugold, u,  K, K, bk, bk);
    matrix_copy_CK_to_CKKC(wgold, wt, C, K, bc, bk);
    matrix_copy_CK_to_CKKC(ugold, ut, K, K, bk, bk);

    /* rnn_copyin(K, K, bm, bm, ugold, u); */
    /* rnn_copyin(C, K, bk, bm, wgold, w); */
    matrix_copy( t*K*N, djdhgoldt, djdht );

    /* bind buffers and filter to handle */
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_input, LIBXSMM_DNN_RNN_REGULAR_INPUT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_hidden_state_prev, LIBXSMM_DNN_RNN_REGULAR_HIDDEN_STATE_PREV ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_weight, LIBXSMM_DNN_RNN_REGULAR_WEIGHT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_weight_t, LIBXSMM_DNN_RNN_REGULAR_WEIGHT_TRANS ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_recur_weight, LIBXSMM_DNN_RNN_REGULAR_RECUR_WEIGHT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_recur_weight_t, LIBXSMM_DNN_RNN_REGULAR_RECUR_WEIGHT_TRANS ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_recur_weight, LIBXSMM_DNN_RNN_REGULAR_RECUR_WEIGHT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_bias, LIBXSMM_DNN_RNN_REGULAR_BIAS ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_hidden_state, LIBXSMM_DNN_RNN_REGULAR_HIDDEN_STATE ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_dinput, LIBXSMM_DNN_RNN_GRADIENT_INPUT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_dweight, LIBXSMM_DNN_RNN_GRADIENT_WEIGHT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_drecur_weight, LIBXSMM_DNN_RNN_GRADIENT_RECUR_WEIGHT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_dbias, LIBXSMM_DNN_RNN_GRADIENT_BIAS ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_tensor( libxsmm_handle, libxsmm_dhidden_state, LIBXSMM_DNN_RNN_GRADIENT_HIDDEN_STATE ) );

    /* let's allocate and bind scratch */
    if (pass == 0) {
      scratch_size = libxsmm_dnn_rnncell_get_scratch_size( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_FWD, &status );
      CHKERR_LIBXSMM_DNN( status );
      scratch = libxsmm_aligned_malloc( scratch_size, 2097152 );
      CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_scratch( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_FWD, scratch ) );
    } else {
      scratch_size = libxsmm_dnn_rnncell_get_scratch_size( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_ALL, &status );
      CHKERR_LIBXSMM_DNN( status );
      scratch = libxsmm_aligned_malloc( scratch_size, 2097152 );
      CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_scratch( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_ALL, scratch ) );
    }
    zero_buf( (float*)scratch, scratch_size/4 );

    /* let's allocate and bind internalstate */
    if (pass == 0) {
      internalstate_size = libxsmm_dnn_rnncell_get_internalstate_size( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_FWD, &status );
      CHKERR_LIBXSMM_DNN( status );
      internalstate = libxsmm_aligned_malloc( internalstate_size, 2097152 );
      CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_internalstate( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_FWD, internalstate ) );
    } else {
      internalstate_size = libxsmm_dnn_rnncell_get_internalstate_size( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_ALL, &status );
      CHKERR_LIBXSMM_DNN( status );
      internalstate = libxsmm_aligned_malloc( internalstate_size, 2097152 );
      CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_bind_internalstate( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_ALL, internalstate ) );
    }
    zero_buf( (float*)internalstate, internalstate_size/4 );

    if ((pass == 0) && LIBXSMM_NEQ(0, check)) {
      printf("##########################################\n");
      printf("#   Correctness - FWD (custom-Storage)   #\n");
      printf("##########################################\n");
      /* run LIBXSMM RNN */
#if defined(_OPENMP)
#     pragma omp parallel
#endif
      {
#if defined(_OPENMP)
        const int tid = omp_get_thread_num();
#else
        const int tid = 0;
#endif
        CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_execute_st( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_FWD, 0, tid ) );
      }

      /* Copy out LIBXSMM result to NC format for correctness checking */
      matrix_copy_NCNC_to_NC(ht, h_nc_buf, t, N, K, bn, bk);
      matrix_copy( N*K, &LIBXSMM_VLA_ACCESS(2, h_nc, t-1, 0, K*N), htest );

      /* compare */
      libxsmm_matdiff(&norms_fwd, LIBXSMM_DATATYPE_F32, K*N, 1, &LIBXSMM_VLA_ACCESS(2, hgold, t-1, 0, K*N), htest, 0, 0);
      printf("L1 reference  : %.25g\n", norms_fwd.l1_ref);
      printf("L1 test       : %.25g\n", norms_fwd.l1_tst);
      printf("L2 abs.error  : %.24f\n", norms_fwd.l2_abs);
      printf("L2 rel.error  : %.24f\n", norms_fwd.l2_rel);
      printf("Linf abs.error: %.24f\n", norms_fwd.linf_abs);
      printf("Linf rel.error: %.24f\n", norms_fwd.linf_rel);
      printf("Check-norm    : %.24f\n", norms_fwd.normf_rel);
      libxsmm_matdiff_reduce(&diff, &norms_fwd);
    } else {
      /* We need to always run FWD pass once to populate zt, ht */
#if defined(_OPENMP)
#     pragma omp parallel
#endif
      {
#if defined(_OPENMP)
        const int tid = omp_get_thread_num();
#else
        const int tid = 0;
#endif
        CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_execute_st( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_FWD, 0, tid ) );
      }
    }

    if ( (pass == 1) && LIBXSMM_NEQ(0, check) ) {
      printf("##########################################\n");
      printf("#   Correctness - BWD (custom-Storage)   #\n");
      printf("##########################################\n");
      /* run LIBXSMM RNN */
#if defined(_OPENMP)
#     pragma omp parallel
#endif
      {
#if defined(_OPENMP)
        const int tid = omp_get_thread_num();
#else
        const int tid = 0;
#endif
        CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_execute_st( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_BWD, 0, tid ) );
      }

      /* copy out data */
      /*
         LIBXSMM_VLA_DECL(2, float, djdxtest, djdxtestt, N*C);
         for (i = 0; i < t; ++i) {
         rnn_copyout(n, k, bn, bk, &LIBXSMM_VLA_ACCESS(2, djdx, i, 0, N*C), &LIBXSMM_VLA_ACCESS(2, djdxtest, i, 0, N*C));
         }
         */
      matrix_copy(N*C*t, djdxt, djdxtestt);

      /* compare */
      libxsmm_matdiff(&norms_bwd, LIBXSMM_DATATYPE_F32, N*C*t, 1, djdxgoldt, djdxtestt, 0, 0);
      printf("L1 reference  : %.25g\n", norms_bwd.l1_ref);
      printf("L1 test       : %.25g\n", norms_bwd.l1_tst);
      printf("L2 abs.error  : %.24f\n", norms_bwd.l2_abs);
      printf("L2 rel.error  : %.24f\n", norms_bwd.l2_rel);
      printf("Linf abs.error: %.24f\n", norms_bwd.linf_abs);
      printf("Linf rel.error: %.24f\n", norms_bwd.linf_rel);
      printf("Check-norm    : %.24f\n", norms_bwd.normf_rel);
      libxsmm_matdiff_reduce(&diff, &norms_bwd);
    }

    if ( (pass == 2) && LIBXSMM_NEQ(0, check) ) {
      printf("##########################################\n");
      printf("#   Correctness - UPD (custom-Storage)   #\n");
      printf("##########################################\n");
      /* run LIBXSMM RNN */
#if defined(_OPENMP)
#     pragma omp parallel
#endif
      {
#if defined(_OPENMP)
        const int tid = omp_get_thread_num();
#else
        const int tid = 0;
#endif
        CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_execute_st( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_UPD, 0, tid ) );
      }

      /* copy out data */
      /*
         rnn_copyout(k, m, bk, bm, djdw, djdwtest);
         rnn_copyout(m, m, bm, bm, djdu, djdutest);
         */
      matrix_copy(C*K, djdw, djdwtest);
      matrix_copy(K*K, djdu, djdutest);

      /* compare */
      libxsmm_matdiff(&norms_upd_w, LIBXSMM_DATATYPE_F32, C*K, 1, djdwgold, djdwtest, 0, 0);
      printf("Delta weight\n");
      printf("L1 reference  : %.25g\n", norms_upd_w.l1_ref);
      printf("L1 test       : %.25g\n", norms_upd_w.l1_tst);
      printf("L2 abs.error  : %.24f\n", norms_upd_w.l2_abs);
      printf("L2 rel.error  : %.24f\n", norms_upd_w.l2_rel);
      printf("Linf abs.error: %.24f\n", norms_upd_w.linf_abs);
      printf("Linf rel.error: %.24f\n", norms_upd_w.linf_rel);
      printf("Check-norm    : %.24f\n", norms_upd_w.normf_rel);
      libxsmm_matdiff_reduce(&diff, &norms_upd_w);

      libxsmm_matdiff(&norms_upd_u, LIBXSMM_DATATYPE_F32, K*K, 1, djdugold, djdutest, 0, 0);
      printf("Delta recurrent weight\n");
      printf("L1 reference  : %.25g\n", norms_upd_u.l1_ref);
      printf("L1 test       : %.25g\n", norms_upd_u.l1_tst);
      printf("L2 abs.error  : %.24f\n", norms_upd_u.l2_abs);
      printf("L2 rel.error  : %.24f\n", norms_upd_u.l2_rel);
      printf("Linf abs.error: %.24f\n", norms_upd_u.linf_abs);
      printf("Linf rel.error: %.24f\n", norms_upd_u.linf_rel);
      printf("Check-norm    : %.24f\n", norms_upd_u.normf_rel);
      libxsmm_matdiff_reduce(&diff, &norms_upd_u);

      libxsmm_matdiff(&norms_upd_b, LIBXSMM_DATATYPE_F32, K, 1, djdbgold, djdb, 0, 0);
      printf("Delta bias\n");
      printf("L1 reference  : %.25g\n", norms_upd_b.l1_ref);
      printf("L1 test       : %.25g\n", norms_upd_b.l1_tst);
      printf("L2 abs.error  : %.24f\n", norms_upd_b.l2_abs);
      printf("L2 rel.error  : %.24f\n", norms_upd_b.l2_rel);
      printf("Linf abs.error: %.24f\n", norms_upd_b.linf_abs);
      printf("Linf rel.error: %.24f\n", norms_upd_b.linf_rel);
      printf("Check-norm    : %.24f\n", norms_upd_b.normf_rel);
      libxsmm_matdiff_reduce(&diff, &norms_upd_b);
    }

    if ( (pass == 3) && LIBXSMM_NEQ(0, check) ) {
      printf("##########################################\n");
      printf("# Correctness - BWD+UPD (custom-Storage) #\n");
      printf("##########################################\n");
      /* run LIBXSMM RNN */
#if defined(_OPENMP)
#     pragma omp parallel
#endif
      {
#if defined(_OPENMP)
        const int tid = omp_get_thread_num();
#else
        const int tid = 0;
#endif
        CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_execute_st( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_BWDUPD, 0, tid ) );
      }

      /* copy out data */
      /*
         LIBXSMM_VLA_DECL(2, float, djdxtest, djdxtestt, N*C);
         for (i = 0; i < t; ++i) {
         rnn_copyout(N, C, bN, bC, &LIBXSMM_VLA_ACCESS(2, djdx, i, 0, N*C), &LIBXSMM_VLA_ACCESS(2, djdxtest, i, 0, N*C));
         }
         rnn_copyout(C, K, bC, bK, djdw, djdwtest);
         rnn_copyout(K, K, bK, bK, djdu, djdutest);
         */
      matrix_copy(N*C*t, djdxt, djdxtestt);
      matrix_copy(C*K, djdw, djdwtest);
      matrix_copy(K*K, djdu, djdutest);

      /* compare */
      libxsmm_matdiff(&norms_bwd, LIBXSMM_DATATYPE_F32, N*C*t, 1, djdxgoldt, djdxtestt, 0, 0);
      printf("Delta input\n");
      printf("L1 reference  : %.25g\n", norms_bwd.l1_ref);
      printf("L1 test       : %.25g\n", norms_bwd.l1_tst);
      printf("L2 abs.error  : %.24f\n", norms_bwd.l2_abs);
      printf("L2 rel.error  : %.24f\n", norms_bwd.l2_rel);
      printf("Linf abs.error: %.24f\n", norms_bwd.linf_abs);
      printf("Linf rel.error: %.24f\n", norms_bwd.linf_rel);
      printf("Check-norm    : %.24f\n", norms_bwd.normf_rel);
      libxsmm_matdiff_reduce(&diff, &norms_bwd);

      libxsmm_matdiff(&norms_upd_w, LIBXSMM_DATATYPE_F32, C*K, 1, djdwgold, djdwtest, 0, 0);
      printf("Delta weight\n");
      printf("L1 reference  : %.25g\n", norms_upd_w.l1_ref);
      printf("L1 test       : %.25g\n", norms_upd_w.l1_tst);
      printf("L2 abs.error  : %.24f\n", norms_upd_w.l2_abs);
      printf("L2 rel.error  : %.24f\n", norms_upd_w.l2_rel);
      printf("Linf abs.error: %.24f\n", norms_upd_w.linf_abs);
      printf("Linf rel.error: %.24f\n", norms_upd_w.linf_rel);
      printf("Check-norm    : %.24f\n", norms_upd_w.normf_rel);
      libxsmm_matdiff_reduce(&diff, &norms_upd_w);

      libxsmm_matdiff(&norms_upd_u, LIBXSMM_DATATYPE_F32, K*K, 1, djdugold, djdutest, 0, 0);
      printf("Delta recurrent weight\n");
      printf("L1 reference  : %.25g\n", norms_upd_u.l1_ref);
      printf("L1 test       : %.25g\n", norms_upd_u.l1_tst);
      printf("L2 abs.error  : %.24f\n", norms_upd_u.l2_abs);
      printf("L2 rel.error  : %.24f\n", norms_upd_u.l2_rel);
      printf("Linf abs.error: %.24f\n", norms_upd_u.linf_abs);
      printf("Linf rel.error: %.24f\n", norms_upd_u.linf_rel);
      printf("Check-norm    : %.24f\n", norms_upd_u.normf_rel);
      libxsmm_matdiff_reduce(&diff, &norms_upd_u);

      libxsmm_matdiff(&norms_upd_b, LIBXSMM_DATATYPE_F32, K, 1, djdbgold, djdb, 0, 0);
      printf("Delta bias\n");
      printf("L1 reference  : %.25g\n", norms_upd_b.l1_ref);
      printf("L1 test       : %.25g\n", norms_upd_b.l1_tst);
      printf("L2 abs.error  : %.24f\n", norms_upd_b.l2_abs);
      printf("L2 rel.error  : %.24f\n", norms_upd_b.l2_rel);
      printf("Linf abs.error: %.24f\n", norms_upd_b.linf_abs);
      printf("Linf rel.error: %.24f\n", norms_upd_b.linf_rel);
      printf("Check-norm    : %.24f\n", norms_upd_b.normf_rel);
      libxsmm_matdiff_reduce(&diff, &norms_upd_b);
    }

    if ( pass == 0 ) {
      printf("##########################################\n");
      printf("#   Performance - FWD (custom-Storage)   #\n");
      printf("##########################################\n");
      /* run LIBXSMM RNN for performance */
      l_start = libxsmm_timer_tick();

#if defined(_OPENMP)
#     pragma omp parallel private(i)
#endif
      {
#if defined(_OPENMP)
        const int tid = omp_get_thread_num();
#else
        const int tid = 0;
#endif
        for (i = 0; i < iters; ++i) {
          libxsmm_dnn_rnncell_execute_st( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_FWD, 0, tid );
        }
      }
      l_end = libxsmm_timer_tick();
      l_total = libxsmm_timer_duration(l_start, l_end);
      flops = ((2.0 * K*N*C) + (2.0 * K*N*K) + (K*N) + (tflops * K*N)) * (double)t * (double)iters;

      printf("GFLOP  = %.5g\n", flops*1e-9/(double)iters);
      printf("fp time = %.5g\n", ((double)(l_total/iters)));
      printf("GFLOPS  = %.5g\n", (flops*1e-9)/l_total);

      printf("PERFDUMP,FP,%s,%i,%i,%i,%i,%i,%.5g,%.5g\n", LIBXSMM_VERSION, nThreads, N, C, K, t, ((double)(l_total/iters)), (flops*1e-9)/l_total);
    }

    if ( pass == 1 ) {
      printf("##########################################\n");
      printf("#   Performance - BWD (custom-Storage)   #\n");
      printf("##########################################\n");
      /* run LIBXSMM RNN for performance */
      l_start = libxsmm_timer_tick();

#if defined(_OPENMP)
#     pragma omp parallel private(i)
#endif
      {
#if defined(_OPENMP)
        const int tid = omp_get_thread_num();
#else
        const int tid = 0;
#endif
        for (i = 0; i < iters; ++i) {
          libxsmm_dnn_rnncell_execute_st( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_BWD, 0, tid );
        }
      }
      l_end = libxsmm_timer_tick();
      l_total = libxsmm_timer_duration(l_start, l_end);
      flops = K*K; /* U^T */
      flops += (2.0 * K*N*K); /* U^T * delta */
      flops += (K*N); /* dJdh + (U^T * delta) */
      flops += (tflops * K*N); /* sigma'(Z) */
      flops += (K*N); /* sigma'(Z) * (dJdh + (U^T * delta)) */
      flops *= t; /* for t time steps */
      tempflops = C*K; /* W^T */
      tempflops += (2.0 * K*N*C); /* W^T * delta */
      tempflops *= t; /* for t time steps of input */
      flops += tempflops;
      flops *= iters;

      printf("GFLOP  = %.5g\n", flops*1e-9/(double)iters);
      printf("bp time = %.5g\n", ((double)(l_total/iters)));
      printf("GFLOPS  = %.5g\n", (flops*1e-9)/l_total);

      printf("PERFDUMP,BP,%s,%i,%i,%i,%i,%i,%.5g,%.5g\n", LIBXSMM_VERSION, nThreads, N, C, K, t, ((double)(l_total/iters)), (flops*1e-9)/l_total);
    }

    if ( pass == 2 ) {
      printf("##########################################\n");
      printf("#   Performance - UPD (custom-Storage)   #\n");
      printf("##########################################\n");
      /* run LIBXSMM RNN for performance */
      l_start = libxsmm_timer_tick();

#if defined(_OPENMP)
#     pragma omp parallel private(i)
#endif
      {
#if defined(_OPENMP)
        const int tid = omp_get_thread_num();
#else
        const int tid = 0;
#endif
        for (i = 0; i < iters; ++i) {
          libxsmm_dnn_rnncell_execute_st( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_UPD, 0, tid );
        }
      }
      l_end = libxsmm_timer_tick();
      l_total = libxsmm_timer_duration(l_start, l_end);
      flops = K*K; /* U^T */
      flops += (2.0 * K*N*K); /* U^T * delta */
      flops += (K*N); /* dJdh + (U^T * delta) */
      flops += (tflops * K*N); /* sigma'(Z) */
      flops += (K*N); /* sigma'(Z) * (dJdh + (U^T * delta)) */
      flops *= t; /* for t time steps */
      tempflops = K*N; /* h^T */
      tempflops += (2.0 * K*N*K); /* delta * h^T */
      tempflops *= t; /* for t time steps */
      tempflops += (K*K * (t-1)); /* for summation of dJdU */
      flops += tempflops;
      tempflops = N*C; /* x^T */
      tempflops += (2.0 * K*N*C); /* delta * x^T */
      tempflops *= t; /* for t time steps */
      tempflops += (C*K * (t-1)); /* for summation of dJdW */
      flops += tempflops;
      flops *= iters;

      printf("GFLOP  = %.5g\n", flops*1e-9/(double)iters);
      printf("wu time = %.5g\n", ((double)(l_total/iters)));
      printf("GFLOPS  = %.5g\n", (flops*1e-9)/l_total);

      printf("PERFDUMP,WU,%s,%i,%i,%i,%i,%i,%.5g,%.5g\n", LIBXSMM_VERSION, nThreads, N, C, K, t, ((double)(l_total/iters)), (flops*1e-9)/l_total);
    }

    if ( pass == 3 ) {
      printf("##########################################\n");
      printf("# Performance - BWD+UPD (custom-Storage) #\n");
      printf("##########################################\n");
      /* run LIBXSMM RNN for performance */
      l_start = libxsmm_timer_tick();

#if defined(_OPENMP)
#     pragma omp parallel private(i)
#endif
      {
#if defined(_OPENMP)
        const int tid = omp_get_thread_num();
#else
        const int tid = 0;
#endif
        for (i = 0; i < iters; ++i) {
          libxsmm_dnn_rnncell_execute_st( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_BWDUPD, 0, tid );
        }
      }
      l_end = libxsmm_timer_tick();
      l_total = libxsmm_timer_duration(l_start, l_end);
      flops = K*K; /* U^T */
      flops += (2.0 * K*N*K); /* U^T * delta */
      flops += (K*N); /* dJdh + (U^T * delta) */
      flops += (tflops * K*N); /* sigma'(Z) */
      flops += (K*N); /* sigma'(Z) * (dJdh + (U^T * delta)) */
      flops *= t; /* for t time steps */
      tempflops = K*N; /* h^T */
      tempflops += (2.0 * K*N*K); /* delta * h^T */
      tempflops *= t; /* for t time steps */
      tempflops += (K*K * (t-1)); /* for summation of dJdU */
      flops += tempflops;
      tempflops = N*C; /* x^T */
      tempflops += (2.0 * K*N*C); /* delta * x^T */
      tempflops *= t; /* for t time steps */
      tempflops += (C*K * (t-1)); /* for summation of dJdW */
      flops += tempflops;
      tempflops = C*K; /* W^T */
      tempflops += (2.0 * K*N*C); /* W^T * delta */
      tempflops *= t; /* for t time steps of input */
      flops += tempflops;
      flops *= iters;

      printf("GFLOP  = %.5g\n", flops*1e-9/(double)iters);
      printf("bp+wu time = %.5g\n", ((double)(l_total/iters)));
      printf("GFLOPS  = %.5g\n", (flops*1e-9)/l_total);

      printf("PERFDUMP,BP+WU,%s,%i,%i,%i,%i,%i,%.5g,%.5g\n", LIBXSMM_VERSION, nThreads, N, C, K, t, ((double)(l_total/iters)), (flops*1e-9)/l_total);
    }

    /* clean-up */
    if (pass == 0) {
      CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_scratch( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_FWD ) );
      CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_internalstate( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_FWD ) );
    } else {
      CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_scratch( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_ALL ) );
      CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_internalstate( libxsmm_handle, LIBXSMM_DNN_COMPUTE_KIND_ALL ) );
    }
    libxsmm_free(scratch);
    libxsmm_free(internalstate);
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_tensor( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_INPUT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_tensor( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_HIDDEN_STATE_PREV ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_tensor( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_WEIGHT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_tensor( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_RECUR_WEIGHT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_tensor( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_BIAS ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_tensor( libxsmm_handle, LIBXSMM_DNN_RNN_REGULAR_HIDDEN_STATE ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_tensor( libxsmm_handle, LIBXSMM_DNN_RNN_GRADIENT_INPUT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_tensor( libxsmm_handle, LIBXSMM_DNN_RNN_GRADIENT_WEIGHT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_tensor( libxsmm_handle, LIBXSMM_DNN_RNN_GRADIENT_RECUR_WEIGHT ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_tensor( libxsmm_handle, LIBXSMM_DNN_RNN_GRADIENT_BIAS ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_rnncell_release_tensor( libxsmm_handle, LIBXSMM_DNN_RNN_GRADIENT_HIDDEN_STATE ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_tensor( libxsmm_input ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_tensor( libxsmm_hidden_state_prev ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_tensor( libxsmm_weight ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_tensor( libxsmm_recur_weight ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_tensor( libxsmm_bias ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_tensor( libxsmm_hidden_state ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_tensor( libxsmm_dinput ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_tensor( libxsmm_dweight ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_tensor( libxsmm_drecur_weight ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_tensor( libxsmm_dbias ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_tensor( libxsmm_dhidden_state ) );
    CHKERR_LIBXSMM_DNN( libxsmm_dnn_destroy_rnncell( libxsmm_handle ) );
  }

  /* deallocate data */
  libxsmm_free(xgoldt);
  libxsmm_free(hpgold);
  libxsmm_free(wgold);
  libxsmm_free(ugold);
  libxsmm_free(bgold);
  libxsmm_free(hgoldt);
  libxsmm_free(zgoldt);
  libxsmm_free(bmgold);
  libxsmm_free(z1gold);
  libxsmm_free(z2gold);
  libxsmm_free(djdxgoldt);
  libxsmm_free(djdwgold);
  libxsmm_free(djdugold);
  libxsmm_free(djdbgold);
  libxsmm_free(djdhgoldt);
  libxsmm_free(deltagoldt);
  libxsmm_free(zigold);
  libxsmm_free(di1gold);
  libxsmm_free(di2gold);
  libxsmm_free(xgoldTp);
  libxsmm_free(wgoldTp);
  libxsmm_free(ugoldTp);
  libxsmm_free(hgoldTp);
  libxsmm_free(xt);
  libxsmm_free(hp);
  libxsmm_free(w);
  libxsmm_free(u);
  libxsmm_free(b);
  libxsmm_free(ht);
  libxsmm_free(djdxt);
  libxsmm_free(djdw);
  libxsmm_free(djdu);
  libxsmm_free(djdb);
  libxsmm_free(djdht);
  libxsmm_free(htest);
  libxsmm_free(djdxtestt);
  libxsmm_free(djdwtest);
  libxsmm_free(djdutest);

  { const char *const env_check_scale = getenv("CHECK_SCALE");
    const double check_scale = LIBXSMM_ABS(0 == env_check_scale ? 1.0 : atof(env_check_scale));
    if (LIBXSMM_NEQ(0, check) && (check < 100.0 * check_scale * diff.normf_rel) && (global_status == LIBXSMM_DNN_SUCCESS)) {
      fprintf(stderr, "FAILED with an error of %f%%!\n", 100.0 * diff.normf_rel);
      exit(EXIT_FAILURE);
    }
  }

  /* some empty lines at the end */
  printf("\n\n\n");

  return EXIT_SUCCESS;
}

