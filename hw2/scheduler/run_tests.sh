#!/bin/bash

make

./run_rr_tests.sh
./run_stcf_tests.sh
./run_stride_tests.sh