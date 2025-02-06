#!/bin/bash

source "../common.sh"

./parseline || { ERR "Error: args test failed"; exit 1; }
INFO "args test passed"