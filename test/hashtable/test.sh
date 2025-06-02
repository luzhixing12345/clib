#!/bin/bash

source "../common.sh"

run_and_match "./base" "test_basic_operations PASSED
Iterate: key = 4, value = 4
Iterate: key = 1, value = 1
Iterate: key = 2, value = 2
Iterate: key = 5, value = 5
Iterate: key = 3, value = 3
test_iteration PASSED
test_large_dataset PASSED
test_custom_struct PASSED
All tests PASSED!"