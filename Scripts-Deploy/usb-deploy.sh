#!/bin/sh

SHARED_LIBRARY_LOC=~/tp-2015-2c-elclan/lib

#Ask for pendrive name
echo "INGRESE LA UBICACION DEL DISPOSITIVO"

read USB_PATH

sudo mkdir /media/usb

#Build usb path
USB_LOC=/media/usb

sudo mount $USB_PATH $USB_LOC

cd $USB_LOC

#Copy needed folders into home directory
cp tp-2015-2c-elclan ~

cp so-commons-library ~

#Install commons library in system
cd so-commons-library

sudo make all

#Install elClan shared library
cd $SHARED_LIBRARY_LOC

make all

#Return to hom directory
cd ~
