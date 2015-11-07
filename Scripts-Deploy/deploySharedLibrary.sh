#!/bin/sh

# Clone Git Repository
#cd $HOME
#git clone https://github.com/sisoputnfrba/tp-2015-2c-elclan.git

GIT_LOC=~/git/
GIT_LIB_LOC=~/git/tp-2015-2c-elclan/lib/

cd $GIT_LIB_LOC

sudo nano shared_library

# Add   LD_LIBRARY_PATH=~/git/tp-2015-2c-elclan/lib/   to that file

source shared_library
