#!/bin/sh

# this script succeeds if compilation fails
# it is used to ensure the compiler will fail for bad cases

RED='\e[1;31m'
GREEN='\e[1;32m'
YELLOW='\e[1;33m'
BLUE='\e[1;34m'
MAGENTA='\e[1;35m'
CYAN='\e[1;36m'
END='\e[0m'

for file in tests/passing/*; do
    ./mcc $file
    if [ $? -ne 0 ]; then
        echo "$file: ${RED}Compilation failed, success was expected${END}"
        return 1
    fi
done

for file in tests/failing/*; do
    ./mcc $file
    if [ $? -eq 0 ]; then
        echo "$file: ${RED}expected failure, but compilation succeeded${END}"
        return 1
    fi
done

echo "${GREEN}All tests passed!${END}"
