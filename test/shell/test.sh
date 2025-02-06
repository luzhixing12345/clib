
#!/bin/bash

source "../common.sh"

./args || { ERR "Error: args test failed"; exit 1; }