/*
 ============================================================================
 Name        : Admin-Swap.c
 Author      : Fiorillo Diego
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/*Source file */

/*Include para las librerias */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <commons/config.h>
#include <commons/log.h>
#include <semaphore.h>
#include <commons/collections/list.h>
//#include "Socket.h"
#include "Colores.h"
#include "Admin-Swap.h"

/* VARIABLES GLOBALES (Definir acá o en el Header)*/
ProcesoSwap* arch;
t_log* loggerInfo;
t_log* loggerError;
t_log* loggerDebug;
t_list* espacioLibre;
t_list* espacioOcupado;
sem_t sem_mutex_libre;
sem_t sem_mutex_ocupado;
int32_t paginasLibres;

ProcesoSwap* crear_estructura_config(char* path)
{
    t_config* archConfig = config_create(path);
    ProcesoSwap* config = malloc(sizeof(ProcesoSwap));
    config->puerto_escucha = config_get_int_value(archConfig, "PUERTO_ESCUCHA");
    config->nombre_swap = config_get_string_value(archConfig, "NOMBRE_SWAP");
    config->cantidad_paginas = config_get_int_value(archConfig, "CANTIDAD_PAGINAS");
    config->tamanio_pagina= config_get_int_value(archConfig, "TAMANIO_PAGINA");
    config->retardo= config_get_int_value(archConfig, "RETARDO_COMPACTACION");
    return config;
}

/* Función que es llamada cuando ctrl+c */
void ifProcessDie(){
		//Queda a cargo del programador la implementación de la función
	exit(1);
}

/*Función donde se inicializan los semaforos */

void inicializoSemaforos(){

	//Abajo, una inicialización ejemplo, sem_init(&semaforo, flags, valor) con su validacion
	int32_t semMutexLibre = sem_init(&sem_mutex_libre,0,1);
	if(semMutexLibre==-1)log_error(loggerError,"No pudo crearse el semaforo Mutex");

	int32_t semMutexOcupado = sem_init(&sem_mutex_ocupado,0,1);
	if(semMutexOcupado==-1)log_error(loggerError,"No pudo crearse el semaforo Mutex");

}

/*Se crea un archivo de log donde se registra to-do */

void crearArchivoDeLog() {
	char* pathLog = "LogDelAdministradorSwap";
	char* archLog = "SWAP-Admin";
	loggerInfo = log_create(pathLog, archLog, 1, LOG_LEVEL_INFO);
	loggerError = log_create(pathLog, archLog, 1, LOG_LEVEL_ERROR);
	loggerDebug = log_create(pathLog, archLog, 1, LOG_LEVEL_DEBUG);
}

/*Se crean las estructuras necesarias para manejar el Swap*/
void creoEstructuraSwap(){
	espacioLibre=list_create();
	espacioOcupado=list_create();
	char* comando=string_new();
	string_append_with_format(&comando, "dd if=/dev/zero of=%s bs=%d count=%d", arch->nombre_swap, arch->tamanio_pagina, arch->cantidad_paginas);
	system(comando);
	free(comando);
	NodoLibre* nodo= malloc(sizeof(NodoLibre));
	nodo->comienzo=0;
	nodo->paginas=arch->cantidad_paginas;
	sem_wait(&sem_mutex_libre);
	list_add(espacioLibre, nodo);		//todo validacion
	sem_post(&sem_mutex_libre);
	paginasLibres=arch->cantidad_paginas;

}

void compactar(){

	NodoOcupado* primerNodo=list_get(espacioOcupado, 0);
	if(primerNodo->comienzo!=0){
		primerNodo->comienzo=0;
		list_replace(espacioOcupado, 0, primerNodo);
	}
	int valor= list_size(espacioOcupado);
	int i;

	for(i=1; i<list_size(espacioOcupado); i++){
		NodoOcupado* nodo= list_get(espacioOcupado, i);
		NodoOcupado* anterior= list_get(espacioOcupado, i-1);
		bool chequeo=(nodo->comienzo==anterior->comienzo+anterior->paginas);
		if(!chequeo){
			nodo->comienzo=anterior->comienzo+anterior->paginas;
			list_replace(espacioOcupado, i, nodo);
		}
		//fin for
	}
	NodoLibre* nodoLibre=malloc(sizeof(NodoLibre));
	NodoOcupado* nodo=list_get(espacioOcupado, i-1);
	mostrar(nodo);
	int nuevoComienzo=(nodo->comienzo+nodo->paginas);
	nodoLibre->comienzo = nuevoComienzo;
	nodoLibre->paginas = paginasLibres - nuevoComienzo;
	list_clean_and_destroy_elements(espacioLibre, free);
	list_add(espacioLibre, nodoLibre);
}

/*Main.- Queda a criterio del programador definir si requiere parametros para la invocación */
int main(void) {

	/*En el header Colores, se adjunta un ejemplo de uso de los colores por consola*/

	/*Tratamiento del ctrl+c en el proceso */
	if(signal(SIGINT, ifProcessDie) == SIG_ERR ) log_error(loggerError,"Error con la señal SIGINT");


	/*Se genera el struct con los datos del archivo de config.- */
	char* path = "Admin-Swap.config";
    arch = crear_estructura_config(path);

	/*Se genera el archivo de log, to-do lo que sale por pantalla */
	crearArchivoDeLog();


	/*Se inicializan todos los semaforos necesarios */
	inicializoSemaforos();

	/*Se inicializan todos los semaforos necesarios */
	creoEstructuraSwap();

//		sock_t* socketServidor=create_server_socket(arch->puerto_escucha);
//		int32_t resultado=listen_connections(socketServidor);

	/*Sintaxis para la creacion de Hilos

	    pthread_create(&NombreThread, NULL, thread_funcion, (void*) parametros);
	    recordando que: solo se puede pasar UN PARAMETRO:
	    NULL: no necesita parametros.
	    unParametro: se le pasa solo uno y luego se castea.
	   ¿Mas parametros?: un struct.

	    pthread_t NombreThread; //declaro el hilo


	   int resultado = pthread_create(&NombreThread, NULL, thread_funcion, (void*) parametros);
	 	   		if (resultado != 0) {
	 	   			log_error(loggerError,"Error al crear el hilo de );
	 	   			abort();
	 	   		}else{
	 	   			log_info(loggerInfo, "Se creo exitosamente el hilo de ");
	 	   		}

	pthread_join(NombreThread, NULL); //espero a que el hilo termine su ejecución */


	return EXIT_SUCCESS;
}

/*	Algo de commons y manejo de arrays:
 * de las commons obtengo la hora actual, y la separo en horas, minutos y segundos:
 *
 * 	char* tiempo= temporal_get_string_time();
	char* hora=malloc(2);
	hora[0]=tiempo[0];
	hora[1]=tiempo[1];
	char* minutos=malloc(2);
	minutos[0]=tiempo[3];
	minutos[1]=tiempo[4];
	minutos[2]='\0';
	char* segundos=malloc(2);
	segundos[0]=tiempo[6];
	segundos[1]=tiempo[7];
	segundos[2]='\0';
	log_info(loggerInfo, ANSI_COLOR_BLUE "Time: %s" ANSI_COLOR_RESET, tiempo);
	log_info(loggerInfo, ANSI_COLOR_BLUE "hora: %s" ANSI_COLOR_RESET, hora);
	log_info(loggerInfo, ANSI_COLOR_BLUE "minutos: %s" ANSI_COLOR_RESET, minutos);
	log_info(loggerInfo, ANSI_COLOR_BLUE "segundos: %s" ANSI_COLOR_RESET, segundos);
 *
 */

