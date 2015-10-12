#!/bin/bash

GIT_TP_LOC=~/git/tp-2015-2c-elclan/
PLANIFICADOR=Planificador
CPU=Cpu
ADMIN_MEMORIA=Admin-Memoria
ADMIN_SWAP=Admin-Swap

if [ $1 = clean ]; then

#Compile all and then clean output folders

cd $GIT_TP_LOC/$PLANIFICADOR/Debug/
make all
mv $PLANIFICADOR ..
cd ..
rm -r Debug

cd $GIT_TP_LOC/$CPU/Debug/
make all
mv $CPU ..
cd ..
rm -r Debug

cd $GIT_TP_LOC/$ADMIN_MEMORIA/Debug/
make all
mv $ADMIN_MEMORIA ..
cd ..
rm -r Debug

cd $GIT_TP_LOC/$ADMIN_SWAP/Debug/
make all
mv $ADMIN_SWAP ..
cd ..
rm -r Debug

else

#Only compile all without cleaning compiled folders

cd $GIT_TP_LOC/$PLANIFICADOR/Debug/
make all
mv $PLANIFICADOR ..

cd $GIT_TP_LOC/$CPU/Debug/
make all
mv $CPU ..

cd $GIT_TP_LOC/$ADMIN_MEMORIA/Debug/
make all
mv $ADMIN_MEMORIA ..

cd $GIT_TP_LOC/$ADMIN_SWAP/Debug/
make all
mv $ADMIN_SWAP ..

fi

clear

exit 