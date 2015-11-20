#!/bin/bash

#CONFIGURE  LD_LIBRARY_PATH

clear
cd /home/utnso/
echo "export LD_LIBRARY_PATH=/home/utnso/tp-2015-2c-elclan/lib" >  /home/utnso/shared_library

cd /home/utnso/tp-2015-2c-elclan/Scripts-Deploy

chmod +x commons-setup.sh
./commons-setup.sh

cd /home/utnso/tp-2015-2c-elclan/lib
make all

clear

exit 0

