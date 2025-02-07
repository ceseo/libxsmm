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
#include <string>
#include "Node.hpp"
#include "Engine.hpp"
#include "Params.hpp"
#include "Tensor.hpp"
#include "proto/gxm.pb.h"

using namespace std;
using namespace gxm;

class AccuracyParams : public NNParams
{
  public:
    AccuracyParams(void) {}
    virtual ~AccuracyParams(void) {}

    void set_axis(int axis) { axis_ = axis; }
    int get_axis() { return axis_; }

    void set_top_k(int top_k) { top_k_ = top_k; }
    int get_top_k() { return top_k_; }

  protected:
    int axis_, top_k_;
};

static MLParams* parseAccuracyParams(NodeParameter* np)
{
  AccuracyParams *p = new AccuracyParams();
  AccuracyParameter ap = np->accuracy_param();

  // Set name of node
  assert(!np->name().empty());
  p->set_node_name(np->name());

  //Set node type (Convolution, FullyConnected, etc)
  assert(!np->type().empty());
  p->set_node_type(np->type());

  //Set tensor names
  //Set tensor names
  for(int i=0; i<np->bottom_size(); i++)
  {
    assert(!np->bottom(i).empty());
    p->set_bottom_names(np->bottom(i));
  }

  //Set Mode for the node
  assert((np->mode() == TRAIN) || (np->mode() == TEST));
  p->set_mode(np->mode());

  int axis = ap.axis();
  p->set_axis(axis);

  int top_k = ap.top_k();
  p->set_top_k(top_k);

  return p;
}

class AccuracyNode : public NNNode
{
  public:

    AccuracyNode(AccuracyParams* p, MLEngine* e);

    virtual ~AccuracyNode(void) {}

  protected:
    void forwardPropagate();

    vector<Tensor*> tenBot_;
    vector<TensorBuf*> tenBotData_;
    string node_name_, node_type_;
    Shape ts_;
    int top_k_, train_batch_count_, test_batch_count_;
    double avg_train_acc_, avg_test_acc_;
    MLEngine *eptr_;
#if 1
    vector<float> max_val;
    vector<int> max_id;
    vector<std::pair<float, int> > bot_data_vec;
#endif
    void shape_setzero(Shape* s)
    {
      for(int i=0; i<MAX_DIMS; i++)
        s->dims[i] = 0;
    }
};
