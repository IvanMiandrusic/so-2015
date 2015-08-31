/*
 ============================================================================
 Name        : Planificador.c
 Author      : Fiorillo Diego
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
/*Source file */

/* Include para las librerias */
#include <stdio.h>
#include <stdlib.h>
#include "Colores.h"
#include "Planificador.h"
#include "Consola.h"

/* DEFINICION DE VARIABLES GLOBALES */
ProcesoPlanificador* arch;
t_log* loggerInfo;
t_log* loggerError;
t_log* loggerDebug;
t_list* colaListos;
t_list* colaBlock;
t_list* colaExec;
t_list* colaFinalizados;

ProcesoPlanificador* crear_estructura_config(char* path)
{
    t_config* archConfig = config_create(path);
    ProcesoPlanificador* config = malloc(sizeof(ProcesoPlanificador));
    config->puerto_escucha = config_get_int_value(archConfig, "PUERTO_ESCUCHA");
    config->algoritmo = config_get_string_value(archConfig, "ALGORITMO_PLANIFICACION");
    config->quantum = config_get_int_value(archConfig, "QUANTUM");
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
	int32_t semMutex = sem_init(&sem_mutex,1,0);
	if(semMutex==-1)log_error(loggerError,"No pudo crearse el semaforo Mutex"); */
}

/*Se crea un archivo de log donde se registra to-do */

void crearArchivoDeLog() {
	char* pathLog = "Planificador.log";
	char* archLog = "Planificador";
	loggerInfo = log_create(pathLog, archLog, 0, LOG_LEVEL_INFO);
	loggerError = log_create(pathLog, archLog, 0, LOG_LEVEL_ERROR);
	loggerDebug = log_create(pathLog, archLog, 0, LOG_LEVEL_DEBUG);
}

PCB* generarPCB(int32_t PID, char* rutaArchivo){
	PCB* unPCB= malloc(sizeof(PCB));
	unPCB->PID=PID;
	unPCB->ruta_archivo=rutaArchivo;
	return unPCB;
}

void creoEstructurasDeManejo(){
	colaListos=list_create();
	colaBlock=list_create();
	colaExec=list_create();
	colaFinalizados=list_create();

}

void clean(){
	list_destroy_and_destroy_elements(colaListos, free);
	list_destroy_and_destroy_elements(colaExec, free);
	list_destroy_and_destroy_elements(colaBlock, free);
	list_destroy_and_destroy_elements(colaFinalizados, free);
	log_destroy(loggerInfo);
	log_destroy(loggerError);
	log_destroy(loggerDebug);
}

/*Main.- Queda a criterio del programador definir si requiere parametros para la invocación */
int main(void) {

	/*Tratamiento del ctrl+c en el proceso */
	if(signal(SIGINT, ifProcessDie) == SIG_ERR ) log_error(loggerError,"Error con la señal SIGINT");


	/*Se genera el struct con los datos del archivo de config.- */
	char* path = "Planificador.config";
	arch = crear_estructura_config(path);

	/*Se genera el archivo de log, to-do lo que sale por pantalla */
	crearArchivoDeLog();


	/*Se inicializan todos los semaforos necesarios */
	inicializoSemaforos();

	/*Se inicializan todos los elementos de gestión necesarios */
	creoEstructurasDeManejo();


	sock_t* socketServidor = create_server_socket(arch->puerto_escucha);
	listen_connections(socketServidor);



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
	admin_consola();
	clean();
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
