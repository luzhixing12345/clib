#!/bin/bash

source "../common.sh"

./base || { ERR "Error: base test failed"; exit 1; }
INFO "base test passed"