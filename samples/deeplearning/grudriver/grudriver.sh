#!/bin/sh
#############################################################################
# Copyright (c) 2015-2019, Intel Corporation                                #
# All rights reserved.                                                      #
#                                                                           #
# Redistribution and use in source and binary forms, with or without        #
# modification, are permitted provided that the following conditions        #
# are met:                                                                  #
# 1. Redistributions of source code must retain the above copyright         #
#    notice, this list of conditions and the following disclaimer.          #
# 2. Redistributions in binary form must reproduce the above copyright      #
#    notice, this list of conditions and the following disclaimer in the    #
#    documentation and/or other materials provided with the distribution.   #
# 3. Neither the name of the copyright holder nor the names of its          #
#    contributors may be used to endorse or promote products derived        #
#    from this software without specific prior written permission.          #
#                                                                           #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         #
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR     #
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT      #
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    #
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED  #
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR    #
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    #
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      #
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        #
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              #
#############################################################################
# Hans Pabst (Intel Corp.), Kunal Banerjee (Intel Corp.)
#############################################################################

HERE=$(cd $(dirname $0); pwd -P)
NAME=$(basename $0 .sh)
ECHO=$(command -v echo)
GREP=$(command -v grep)
ENV=$(command -v env)

if [ "Windows_NT" = "${OS}" ]; then
  # Cygwin's "env" does not set PATH ("Files/Black: No such file or directory")
  export PATH=${PATH}:${HERE}/../../lib:/usr/x86_64-w64-mingw32/sys-root/mingw/bin
  # Cygwin's ldd hangs with dyn. linked executables or certain shared libraries
  LDD=$(command -v cygcheck)
  EXE=.exe
else
  if [ "" != "$(command -v ldd)" ]; then
    LDD=ldd
  elif [ "" != "$(command -v otool)" ]; then
    LDD="otool -L"
  else
    LDD=${ECHO}
  fi
fi

MICINFO=$(command -v micinfo)
if [ "" != "${MICINFO}" ]; then
  MICCORES=$(${MICINFO} 2>/dev/null | sed -n "0,/[[:space:]]\+Total No of Active Cores :[[:space:]]\+\([0-9]\+\)/s//\1/p")
fi
if [ "" = "${MICCORES}" ]; then
  MICCORES=61
fi
MICTPERC=3

if [ "-mic" != "$1" ]; then
  if [ "" != "$(${LDD} ${HERE}/${NAME}${EXE} 2>/dev/null | ${GREP} libiomp5\.)" ]; then
    ${ENV} LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${HERE}/../../lib \
      DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${HERE}/../../lib \
      KMP_AFFINITY=compact,granularity=fine,1 \
      MIC_KMP_AFFINITY=compact,granularity=fine \
      MIC_KMP_HW_SUBSET=$((MICCORES-1))c${MICTPERC}t \
      MIC_ENV_PREFIX=MIC \
      OFFLOAD_INIT=on_start \
    ${TOOL_COMMAND} ${HERE}/${NAME}${EXE} "$@"
  else
    ${ENV} LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${HERE}/../../lib \
      DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${HERE}/../../lib \
      OMP_PROC_BIND=TRUE \
    ${TOOL_COMMAND} ${HERE}/${NAME}${EXE} "$@"
  fi
else
  shift
  ${ENV} \
    SINK_LD_LIBRARY_PATH=${SINK_LD_LIBRARY_PATH}:${MIC_LD_LIBRARY_PATH}:${HERE}/../../lib \
  micnativeloadex \
    ${HERE}/${NAME}${EXE} -a "$*" \
    -e "KMP_AFFINITY=compact,granularity=fine" \
    -e "MIC_KMP_HW_SUBSET=$((MICCORES-1))${MICTPERC}t"
fi

ITERS=100
CHKVAL=1

export OMP_NUM_THREADS=28
export KMP_AFFINITY=granularity=fine,compact,1,0


echo "GRU FWD"
CHECK=${CHKVAL} ./grudriver  ${ITERS}  0  168   256   256  50
wait
CHECK=${CHKVAL} ./grudriver  ${ITERS}  0  168   512   512  50
wait
CHECK=${CHKVAL} ./grudriver  ${ITERS}  0  168  1024  1024  50
wait
CHECK=${CHKVAL} ./grudriver  ${ITERS}  0  168  2048  2048  50
wait
CHECK=${CHKVAL} ./grudriver  ${ITERS}  0  168  4096  4096  50
wait

echo "GRU BWD+UPD"
CHECK=${CHKVAL} ./grudriver  ${ITERS}  3  168   256   256  50
wait
CHECK=${CHKVAL} ./grudriver  ${ITERS}  3  168   512   512  50
wait
CHECK=${CHKVAL} ./grudriver  ${ITERS}  3  168  1024  1024  50
wait
CHECK=${CHKVAL} ./grudriver  ${ITERS}  3  168  2048  2048  50
wait
CHECK=${CHKVAL} ./grudriver  ${ITERS}  3  168  4096  4096  50
wait

echo "GRU performance done"
echo ""
