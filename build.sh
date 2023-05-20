#!/bin/bash

set -e  # exit on fail
cd src
make -B
cp mcc rt32.o rt64.o ..
cd ..
