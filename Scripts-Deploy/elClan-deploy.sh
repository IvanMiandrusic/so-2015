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

# Clone Git Repository
#cd $HOME
#git clone https://github.com/sisoputnfrba/tp-2015-2c-elclan.git

GIT_TP_LOC=~/git/tp-2015-2c-elclan/
PLANIFICADOR=Planificador
CPU=Cpu
ADMIN_MEMORIA=Admin-Memoria
ADMIN_SWAP=Admin-Swap
SCRIPTS=Scripts-Deploy

# Check if commons are installed
LIB_OK=$(ldconfig -p | grep libcommons.so)

echo Checking for so-commons-library: $LIB_OK

if [ -z "$LIB_OK" ]; then
  echo "so-commons-library IS NOT INSTALLED, PROCEEDING TO SETTING IT UP"
  /bin/sh ./commons-install.sh

else
	echo "so-commons-library IS ALREADY INSTALLED"
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
	cd $GIT_TP_LOC/$PLANIFICADOR/Debug/
	make all
	mv $PLANIFICADOR ..
	cd ..
	rm -r Debug
	echo "PLANIFICADOR COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x planificador.sh
  	/bin/sh ./planificador.sh
;;
2 )
	cd $GIT_TP_LOC/$ADMIN_SWAP/Debug/
	make all
	mv $ADMIN_SWAP ..
	cd ..
	rm -r Debug
	echo "ADMIN_SWAP COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x swap.sh
  	/bin/sh ./swap.sh
;;
3 )
	cd $GIT_TP_LOC/$ADMIN_MEMORIA/Debug/
	make all
	mv $ADMIN_MEMORIA ..
	cd ..
	rm -r Debug
	echo "ADMIN_MEMORIA COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x memoria.sh
 	/bin/sh ./memoria.sh
;;
4)
	cd $GIT_TP_LOC/$CPU/Debug/
	make all
	mv $CPU ..
	cd ..
	rm -r Debug
	echo "CPU COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x cpu.sh
	/bin/sh ./cpu.sh
;;
esac

else

case $PROCESS_ID in
1 )
	cd $GIT_TP_LOC/$PLANIFICADOR/Debug/
	make all
	mv $PLANIFICADOR ..
	cd ..
	echo "PLANIFICADOR COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x planificador.sh
  	/bin/sh ./planificador.sh
;;
2 )
	cd $GIT_TP_LOC/$ADMIN_SWAP/Debug/
	make all
	mv $ADMIN_SWAP ..
	cd ..
	echo "ADMIN_SWAP COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x swap.sh
  	/bin/sh ./swap.sh
;;
3 )
	cd $GIT_TP_LOC/$ADMIN_MEMORIA/Debug/
	make all
	mv $ADMIN_MEMORIA ..
	cd ..
	echo "ADMIN_MEMORIA COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x memoria.sh
 	/bin/sh ./memoria.sh
;;
4)
	cd $GIT_TP_LOC/$CPU/Debug/
	make all
	mv $CPU ..
	cd ..
	echo "CPU COMPILATION FINISHED"
	cd $GIT_TP_LOC/$SCRIPTS/
	chmod +x cpu.sh
	/bin/sh ./cpu.sh
;;
esac

fi

exit 0