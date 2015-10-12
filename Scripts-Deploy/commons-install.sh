#!bin/bash

#Clone commons library

cd $HOME

git clone https://github.com/sisoputnfrba/so-commons-library.git

GIT_LOC=~/git/

cd $GIT_LOC/so-commons-library

#Install library

sudo make install

exit 0