#!/bin/sh

#Clone commons library
cd ~

git clone https://github.com/sisoputnfrba/so-commons-library.git

cd so-commons-library/

#Install library

sudo make install

exit 0
