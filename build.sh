#!/bin/bash

set -e  # exit on fail
cd src
make 
cp mcc ..
cd ..
