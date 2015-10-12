#!/bin/bash

# Check if Git is installed

PKG_OK=$(dpkg-query -W --showformat='${Status}\n' git|grep "install ok installed")

echo Checking for git: $PKG_OK

if [ "" == "$PKG_OK" ]; then
  echo "NO git IS INSTALLED. SETTING UP GIT."
  sudo apt-get --force-yes --yes install git
fi

# Clone Git Repository

cd $HOME

git clone https://github.com/sisoputnfrba/tp-2015-2c-elclan.git

GIT_LOC=~/git/tp-2015-2c-elclan/

#Install commons libraries

/bin/bash ./commons-install.sh

#Compile all processes (give permissions)
chmod +x compile-all.sh

/bin/bash ./compile-all.sh clean

#Execute all processes

clear

echo "¿QUE PROCESO DESEA LEVANTAR?"

echo "1- PLANIFICADOR"
echo "2- ADMIN-SWAP"
echo "3- ADMIN-MEMORIA"
echo "4- CPU"

read PROCESS_ID

echo "PROCEED TO INSTALL SELECTED PROCESS"

case $PROCESS_ID in
1 )
	chmod +x planificador.sh
  	/bin/bash ./planificador.sh
;;
2 )
	chmod +x swap.sh
  	/bin/bash ./swap.sh
;;
3 )
	chmod +x memoria.sh
 	/bin/bash ./memoria.sh
;;
4)
	chmod +x cpu.sh
	/bin/bash ./cpu.sh
;;
esac

exit

echo "DEPLOY & CONFIGURACION SUCCESFULLY COMPLETED"

exit 0



