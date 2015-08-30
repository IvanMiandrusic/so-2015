/*
 ============================================================================
 Name        : Admin-Memoria.c
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
#include <commons/string.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include "libsocket.h"
#include "Colores.h"
#include "Admin-Memoria.h"

/* VARIABLES GLOBALES (Definir acá o en el Header)*/
ProcesoMemoria* arch;
t_log* loggerInfo;
t_log* loggerError;
t_log* loggerDebug;
t_list* tabla_TLB;
char* memPPAL;
t_list* tabla_Paginas;


ProcesoMemoria* crear_estructura_config(char* path)
{
    t_config* archConfig = config_create(path);
    ProcesoMemoria* config = malloc(sizeof(ProcesoMemoria));
    config->puerto_escucha = config_get_int_value(archConfig, "PUERTO_ESCUCHA");
    config->ip_swap = config_get_string_value(archConfig, "IP_SWAP");
    config->puerto_swap = config_get_int_value(archConfig, "PUERTO_SWAP");
    config->maximo_marcos = config_get_int_value(archConfig, "MAXIMO_MARCOS_POR_PROCESO");
    config->cantidad_marcos = config_get_int_value(archConfig, "CANTIDAD_MARCOS");
    config->tamanio_marco = config_get_int_value(archConfig, "TAMANIO_MARCO");
    config->entradas_tlb = config_get_int_value(archConfig, "ENTRADAS_TLB");
    config->tlb_habilitada = config_get_string_value(archConfig, "TLB_HABILITADA");
    config->retardo= config_get_int_value(archConfig, "RETARDO_MEMORIA");
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
	char* pathLog = "AdministradorMemoria.log";
	char* archLog = "MEM_Admin";
	loggerInfo = log_create(pathLog, archLog, 1, LOG_LEVEL_INFO);
	loggerError = log_create(pathLog, archLog, 1, LOG_LEVEL_ERROR);
	loggerDebug = log_create(pathLog, archLog, 1, LOG_LEVEL_DEBUG);
}

void llenoTLB(){			//tambien sirve para limpiar la TLB
	int i;
	for(i=0; i<arch->entradas_tlb;i++){
	TLB* nuevaEntrada= malloc(sizeof(TLB));
	nuevaEntrada->marco=0;
	nuevaEntrada->modificada=0;
	nuevaEntrada->valida=0;					//todo ver campos bien
	list_add(tabla_TLB, nuevaEntrada);		//todo ver sincro
	}
}


void llenoTPag(){
	int i;
	for(i=0; i<arch->cantidad_marcos;i++){
		TPagina* nuevaEntrada= malloc(sizeof(TLB));
		nuevaEntrada->marco=0;
		nuevaEntrada->pagina=0;				//todo ver campos bien
		list_add(tabla_Paginas, nuevaEntrada);		//todo ver sincro
		}
}

void creoEstructurasDeManejo(){
	if(string_equals_ignore_case((arch->tlb_habilitada), "si"))
	{
		tabla_TLB=list_create();
		llenoTLB();
	}
	memPPAL=malloc((arch->cantidad_marcos)*(arch->tamanio_marco));
	llenoTPag();
}

void ifSigurs1(){

}

void ifSigurs2(){

}

/*Main.- Queda a criterio del programador definir si requiere parametros para la invocación */
int main(void) {

	/*En el header Colores, se adjunta un ejemplo de uso de los colores por consola*/

	/*Tratamiento del ctrl+c en el proceso */
	if(signal(SIGINT, ifProcessDie) == SIG_ERR ) log_error(loggerError,"Error con la señal SIGINT");

	if(signal(SIGINT, ifSigurs1) == SIG_ERR ) log_error(loggerError,"Error con la señal SIGURS1");

	if(signal(SIGINT, ifSigurs2) == SIG_ERR ) log_error(loggerError,"Error con la señal SIGURS2");



	/*Se genera el struct con los datos del archivo de config.- */
	char* path = "Admin-Memoria.config";
	arch = crear_estructura_config(path);

	/*Se genera el archivo de log, to-do lo que sale por pantalla */
	crearArchivoDeLog();


	/*Se inicializan todos los semaforos necesarios */
	inicializoSemaforos();

	creoEstructurasDeManejo();

//	sock_t* socketServidor=create_server_socket(arch->puerto_escucha);
//	int32_t resultado=listen_connections(socketServidor);
//	sock_t* socketCliente=create_client_socket(arch->ip_swap,arch->puerto_swap);
//	int32_t resultado=connect_to_server(socketCliente);

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
