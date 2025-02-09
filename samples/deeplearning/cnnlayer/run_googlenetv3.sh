#!/bin/bash

UNAME=$(command -v uname)
SORT=$(command -v sort)
GREP=$(command -v grep)
CUT=$(command -v cut)
WC=$(command -v wc)
TR=$(command -v tr)

if [ "" = "${CHECK}" ] || [ "0" = "${CHECK}" ]; then
  if [ "" = "${CHECK_DNN_MB}" ]; then CHECK_DNN_MB=128; fi
  if [ "" = "${CHECK_DNN_ITERS}" ]; then CHECK_DNN_ITERS=1000; fi
else # check
  if [ "" = "${CHECK_DNN_MB}" ]; then CHECK_DNN_MB=32; fi
  if [ "" = "${CHECK_DNN_ITERS}" ]; then CHECK_DNN_ITERS=1; fi
fi

if [ $# -ne 7 ]
then
  echo "Usage: $(basename $0) mb iters numa (1-mcdram/0-DDR) TYPE ('A'-ALL/'F'-FP/'B'-BP/'U'-WU) FORMAT ('A'-ALL/'L'-LIBXSMM/'T'-Tensorflow/'M'-Mixed) padding; using default values; using default values: 128 1000 1 f32 A L 0"
  MB=${CHECK_DNN_MB}
  ITERS=${CHECK_DNN_ITERS}
  NUMA=-1
  BIN=f32
  TYPE=A
  FORMAT=L
  PAD=0
else
  MB=$1
  ITERS=$2
  NUMA=$3
  BIN=$4
  TYPE=$5
  FORMAT=$6
  PAD=$7
fi

if [ "" != "${GREP}" ] && [ "" != "${CUT}" ] && [ "" != "${SORT}" ] && [ "" != "${WC}" ] && [ -e /proc/cpuinfo ]; then
  export NS=$(${GREP} "physical id" /proc/cpuinfo | ${SORT} -u | ${WC} -l | ${TR} -d " ")
  export NC=$((NS*$(${GREP} -m1 "cpu cores" /proc/cpuinfo | ${TR} -d " " | ${CUT} -d: -f2)))
  export NT=$(${GREP} "core id" /proc/cpuinfo | ${WC} -l | ${TR} -d " ")
elif [ "" != "${UNAME}" ] && [ "" != "${CUT}" ] && [ "Darwin" = "$(${UNAME})" ]; then
  export NS=$(sysctl hw.packages | ${CUT} -d: -f2 | tr -d " ")
  export NC=$(sysctl hw.physicalcpu | ${CUT} -d: -f2 | tr -d " ")
  export NT=$(sysctl hw.logicalcpu | ${CUT} -d: -f2 | tr -d " ")
fi
if [ "" != "${NC}" ] && [ "" != "${NT}" ]; then
  export HT=$((NT/(NC)))
else
  export NS=1 NC=1 NT=1 HT=1
fi
if [ "" != "${CUT}" ] && [ "" != "$(command -v numactl)" ]; then
  export NN=$(numactl -H | ${GREP} available: | ${CUT} -d' ' -f2)
else
  export NN=${NS}
fi

CPUFLAGS=$(if [ "" != "${GREP}" ] && [ "" != "${CUT}" ] && [ -e /proc/cpuinfo ]; then ${GREP} -m1 flags /proc/cpuinfo | ${CUT} -d: -f2-; fi)
if [ "" != "$(echo "${CPUFLAGS}" | ${GREP} -o avx512er)" ]; then
  if [ "0" != "$((0>NUMA))" ] && [ "0" != "$((NS<NN))" ]; then
    NUMACTL="numactl --preferred=${NS} ${TOOL_COMMAND}"
  elif [ "0" != "$((0<=NUMA && NUMA<NN))" ]; then
    NUMACTL="numactl --preferred=${NUMA} ${TOOL_COMMAND}"
  elif [ "1" != "${NS}" ]; then
    #NUMACTL="numactl -i all ${TOOL_COMMAND}"
    NUMACTL="${TOOL_COMMAND}"
  fi
else
  NUMACTL="${TOOL_COMMAND}"
fi

if [ "" = "${OMP_NUM_THREADS}" ] || [ "0" = "${OMP_NUM_THREADS}" ]; then
  if [ "" = "${KMP_AFFINITY}" ]; then
    export KMP_AFFINITY=compact,granularity=fine KMP_HW_SUBSET=1T
  fi
  export OMP_NUM_THREADS=$((NC))
fi

if [ "" = "${LIBXSMM_TARGET_HIDDEN}" ] || [ "0" = "${LIBXSMM_TARGET_HIDDEN}" ]; then
  echo "OMP_NUM_THREADS=${OMP_NUM_THREADS} NUMACTL=\"${NUMACTL}\""
  echo
fi

# ./layer_example_${BIN} iters inpWidth inpHeight nImg nIfm nOfm kw kh padw padh stride type
#
if [ "${BIN}" != "f32" ]; then
  true
else
${NUMACTL} ./layer_example_${BIN} ${ITERS}  299 299 ${MB}     3   32 3 3 0 0 2 ${TYPE} ${FORMAT} ${PAD}
fi && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  149 149 ${MB}    32   32 3 3 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  147 147 ${MB}    32   64 3 3 1 1 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  73  73  ${MB}    64   80 3 3 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  73  73  ${MB}    80  192 3 3 0 0 2 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  71  71  ${MB}    80  192 3 3 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}   192   64 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}    64   96 3 3 1 1 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}    96   96 3 3 1 1 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}   192   48 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}    48   64 5 5 2 2 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}   192   32 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}   256   64 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}   256   48 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}   288   64 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}   288   48 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}    96   96 3 3 0 0 2 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  35  35  ${MB}   288  384 3 3 0 0 2 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   768  128 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   128  128 1 7 0 3 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   128  128 7 1 3 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   128  192 7 1 3 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   128  192 1 7 0 3 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   768  192 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   768  160 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   160  160 1 7 0 3 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   160  160 7 1 3 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   160  192 7 1 3 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   160  192 1 7 0 3 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   192  192 1 7 0 3 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   192  192 7 1 3 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   192  192 3 3 0 0 2 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  17  17  ${MB}   192  320 3 3 0 0 2 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}  1280  320 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}  1280  192 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}  1280  448 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}   448  384 3 3 1 1 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}   384  384 1 3 0 1 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}   384  384 3 1 1 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}  1280  384 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}  2048  320 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}  2048  192 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}  2048  448 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}  2048  384 1 1 0 0 1 ${TYPE} ${FORMAT} ${PAD}   && \
${NUMACTL} ./layer_example_${BIN} ${ITERS}  8   8   ${MB}   384  384 1 3 0 1 1 ${TYPE} ${FORMAT} ${PAD}
