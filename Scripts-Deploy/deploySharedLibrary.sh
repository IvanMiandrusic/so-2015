#!/bin/sh

# Clone Git Repository
#cd ~
#git clone https://github.com/sisoputnfrba/tp-2015-2c-elclan.git

GIT_LIB_LOC=~/tp-2015-2c-elclan/lib/

cd $GIT_LIB_LOC

echo "LD_LIBRARY_PATH=~/tp-2015-2c-elclan/lib/" >> shared_library

source shared_library
