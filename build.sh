#!/bin/bash

set -e  # exit on fail
cd src
make -B
cp mcc ..
cd ..
