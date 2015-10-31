#!/bin/sh

#Clone commons library
GIT_TP_LOC=~/git/tp-2015-2c-elclan/

cd $GIT_TP_LOC

git clone https://github.com/sisoputnfrba/so-commons-library.git

cd so-commons-library/

#Install library

sudo make install

exit 0