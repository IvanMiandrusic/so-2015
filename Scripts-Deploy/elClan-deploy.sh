#!/bin/sh

# Check if Git is installed
PKG_OK=$(dpkg-query -W --showformat='${Status}\n' git|grep "install ok installed")

echo Checking for git: $PKG_OK

if [ -z "$PKG_OK" ]; then
  echo "NO git IS INSTALLED. SETTING UP GIT."
  sudo apt-get --force-yes --yes install git

else
	echo "git IS ALREADY INSTALLED"
fi

echo "Compiling all processes..."

GIT_TP_LOC=~/tp-2015-2c-elclan
PLANIFICADOR=Planificador
CPU=Cpu
ADMIN_MEMORIA=Admin-Memoria
ADMIN_SWAP=Admin-Swap
SCRIPTS=Scripts-Deploy

# Check if commons are installed
if [ ! test -d "so-commons-library" ]; then
  echo "so-commons-library IS NOT INSTALLED, PROCEEDING TO SETTING IT UP"
  /bin/sh ./commons-setup.sh

else
	echo "so-commons-library IS ALREADY INSTALLED"
fi

#Check if LD_LIBRARY_PATH is not set or is empty
echo "La variable de entorno LD_LIBRARY_PATH esta en: ${LD_LIBRARY_PATH:?La variable de entorno LD_LIBRARY_PATH no esta seteada}"

#Check if LD_LIBRARY_PATH is set but has wrong value
CORRECT_PATH=/home/utnso/tp-2015-2c-elclan/lib

if [ "$LD_LIBRARY_PATH" != "$CORRECT_PATH" ]
then 
	echo "La variable de entorno LD_LIBRARY_PATH no esta seteada correctamente"
	exit 1
fi

#Compile all and then clean output folders
echo "¿QUE PROCESO DESEA LEVANTAR?"

echo "1- PLANIFICADOR"
echo "2- ADMIN-SWAP"
echo "3- ADMIN-MEMORIA"
echo "4- CPU"

read PROCESS_ID

echo "PROCEED TO EXECUTE SELECTED PROCESS"

if [ $1 = clean ]; then

case $PROCESS_ID in
1 )
	cd $GIT_TP_LOC/$PLANIFICADOR
	make all
	echo "PLANIFICADOR COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x planificador.sh
  	/bin/sh ./planificador.sh
;;
2 )
	cd $GIT_TP_LOC/$ADMIN_SWAP
	make all
	echo "ADMIN_SWAP COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x swap.sh
  	/bin/sh ./swap.sh
;;
3 )
	cd $GIT_TP_LOC/$ADMIN_MEMORIA
	make all
	echo "ADMIN_MEMORIA COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x memoria.sh
 	/bin/sh ./memoria.sh
;;
4)
	cd $GIT_TP_LOC/$CPU
	make all
	echo "CPU COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x cpu.sh
	/bin/sh ./cpu.sh
;;
esac

else

case $PROCESS_ID in
1 )
	cd $GIT_TP_LOC/$PLANIFICADOR
	make all
	echo "PLANIFICADOR COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x planificador.sh
  	/bin/sh ./planificador.sh
;;
2 )
	cd $GIT_TP_LOC/$ADMIN_SWAP
	make all
	echo "ADMIN_SWAP COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x swap.sh
  	/bin/sh ./swap.sh
;;
3 )
	cd $GIT_TP_LOC/$ADMIN_MEMORIA
	make all
	echo "ADMIN_MEMORIA COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x memoria.sh
 	/bin/sh ./memoria.sh
;;
4)
	cd $GIT_TP_LOC/$CPU
	make all
	echo "CPU COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x cpu.sh
	/bin/sh ./cpu.sh
;;
esac

fi

exit 0
