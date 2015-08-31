/*
 ============================================================================
 Name        : Cpu.c
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
#include "libsocket.h"
#include "Colores.h"
#include "Cpu.h"
#include "Operaciones.h"

/* VARIABLES GLOBALES (Definir acá o en el Header)*/
ProcesoCPU* arch;
t_log* loggerInfo;
t_log* loggerError;
t_log* loggerDebug;


ProcesoCPU* crear_estructura_config(char* path)
{
    t_config* archConfig = config_create(path);
    ProcesoCPU* config = malloc(sizeof(ProcesoCPU));
    config->ip_planificador = config_get_string_value(archConfig, "IP_PLANIFICADOR");
    config->puerto_planificador = config_get_int_value(archConfig, "PUERTO_PLANIFICADOR");
    config->ip_memoria = config_get_string_value(archConfig, "IP_MEMORIA");
    config->puerto_memoria = config_get_int_value(archConfig, "PUERTO_MEMORIA");
    config->cantidad_hilos = config_get_int_value(archConfig, "CANTIDAD_HILOS");
    config->retardo = config_get_int_value(archConfig, "RETARDO");
    return config;
}

/* Función que es llamada cuando ctrl+c */
void ifProcessDie(){
		//Queda a cargo del programador la implementación de la función
	exit(1);
}

/*Función donde se inicializan los semaforos */

void inicializoSemaforos(){

	/* Abajo, una inicialización ejemplo, sem_init(&semaforo, flags, valor) con su validacion
	int32_t semMutex = sem_init(&sem_mutex,0,1);
	if(semMutex==-1)log_error(loggerError,"No pudo crearse el semaforo Mutex"); */
}

/*Se crea un archivo de log donde se registra to-do */

void crearArchivoDeLog() {
	char* pathLog = "Cpu.log";
	char* archLog = "CPU";
	loggerInfo = log_create(pathLog, archLog, 1, LOG_LEVEL_INFO);
	loggerError = log_create(pathLog, archLog, 1, LOG_LEVEL_ERROR);
	loggerDebug = log_create(pathLog, archLog, 1, LOG_LEVEL_DEBUG);
}


/*Main.- Queda a criterio del programador definir si requiere parametros para la invocación */
int main(void) {

	/*En el header Colores, se adjunta un ejemplo de uso de los colores por consola*/

	/*Tratamiento del ctrl+c en el proceso */
	if(signal(SIGINT, ifProcessDie) == SIG_ERR ) log_error(loggerError,"Error con la señal SIGINT");


	/*Se genera el struct con los datos del archivo de config.- */
	char* path = "Cpu.config";
	arch = crear_estructura_config(path);

	/*Se genera el archivo de log, to-do lo que sale por pantalla */
	crearArchivoDeLog();
	log_info(loggerInfo, "Se cargaron correctamente los parametros de configuración");


	/*Se inicializan todos los semaforos necesarios */
	inicializoSemaforos();



	/*Creo los hilos necesarios para la ejecución*/
	int32_t i;
	int32_t cantidad_hilos=arch->cantidad_hilos;
    pthread_t CPUthreads[cantidad_hilos];

    for(i=0;i<cantidad_hilos;i++){

    	int resultado = pthread_create(&CPUthreads[i], NULL, thread_Cpu, (void*) i );
    	if (resultado != 0) {
    		log_error(loggerError,"Error al crear el hilo de ejecucion CPU número: %d", i);
    		abort();
    	}else{log_info(loggerInfo, "Se creo exitosamente el hilo de ejecucion CPU número: %d", i);}

    }
	for(i=0;i<cantidad_hilos;i++){ // espera a que terminen los hilos para terminar el proceso
		pthread_join(CPUthreads[i],NULL);
	}

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

