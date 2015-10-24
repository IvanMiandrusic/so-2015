#!/bin/sh
GIT_LOC=~/git/
GIT_SL_LOC=~/git/c-sockets-library

# Clone Git Repository
cd $GIT_LOC
git clone https://github.com/fedecatinello/c-sockets-library.git

cd $GIT_SL_LOC

make all

sudo nano /etc/bash.bashrc

ldconfig -n $GIT_SL_LOC
