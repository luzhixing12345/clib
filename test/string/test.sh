#!/bin/bash

source "../common.sh"

./split_str || { ERR "Error: split_str test failed"; exit 1; }
INFO "split_str test passed"