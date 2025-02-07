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
/* Sasikanth Avancha, Dhiraj Kalamkar (Intel Corp.)
******************************************************************************/


#pragma once

#include <omp.h>
#include <assert.h>
#include <sys/time.h>
#include "common.hpp"
#include "check.hpp"
#include "Tensor.hpp"

typedef struct {
  string node_name;
  int nInput, nOutput;
  int batch_size;
  int iHeight, iWidth;
  int oHeight, oWidth;
  int kh, kw;
  bool bias_term;
  int in_data_type, out_data_type;
  int algType;
  int num_threads;
} FCImplParams;

class FCImpl
{
  protected:
    FCImplParams* gp;
    int engine;
    TensorLayoutType bot_layout_type, top_layout_type, gbot_layout_type;
    void *bot_layout=NULL, *top_layout=NULL, *gbot_layout=NULL;
    int top_compute_engine=-1;
    int bot_compute_engine=-1;
    string nname;
    TensorBuf* scratchp;

  public:
    FCImpl(FCImplParams* gp_, int engine_): gp(gp_), engine(engine_) {}

    void set_top_compute_engine(int e) { top_compute_engine = e;}
    void set_bot_compute_engine(int e) { bot_compute_engine = e;}
    void set_node_name(string s) { nname = s; }
    void set_scratch_buffer(TensorBuf* sb) { scratchp = sb; }

    virtual void forwardPropagate(TensorBuf *inp, TensorBuf* weightp, TensorBuf* biasp, TensorBuf *outp, int tid) = 0;
    virtual void backPropagate(TensorBuf *deloutp, TensorBuf* weightp, TensorBuf *delinp, int tid) = 0;
    virtual void weightUpdate(TensorBuf *deloutp, TensorBuf *inp, TensorBuf *delweightp, TensorBuf *delbiasp, int tid) = 0;

    virtual void forwardPropagate(TensorBuf *inp, TensorBuf* weightp, TensorBuf* biasp, TensorBuf *outp)
    {
      switch(engine)
      {
        case XSMM:
          forwardPropagate(inp, weightp, biasp, outp, 0);
          break;
      }
    }

    virtual void backPropagate(TensorBuf *deloutp, TensorBuf *weightp, TensorBuf *delinp)
    {
      switch(engine)
      {
        case XSMM:
          backPropagate(deloutp, weightp, delinp, 0);
          break;
      }
    }

    virtual void weightUpdate(TensorBuf *deloutp, TensorBuf *inp, TensorBuf *delweightp, TensorBuf *delbiasp)
    {
      switch(engine)
      {
        case XSMM:
          weightUpdate(deloutp, inp, delweightp, delbiasp, 0);
          break;
      }
    }
};
