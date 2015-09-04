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
#include "Comunicacion.h"
#include <pthread.h>

/** TAD para PLANIFICADOR **/
ProcesoPlanificador* arch;
/** Loggers **/
t_log* loggerInfo;
t_log* loggerError;
t_log* loggerDebug;
/** TADs **/
t_list* colaListos;
t_list* colaBlock;
t_list* colaExec;
t_list* colaFinalizados;
t_list* colaCPUs;
/** ID para los PCB **/
int32_t idParaPCB = 0;
/** SOCKET SERVIDOR **/
sock_t* socketServidor;


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
	unPCB->estado=LISTO;
	unPCB->tamanio_ruta_archivo = string_length(rutaArchivo);
	unPCB->ruta_archivo = malloc(unPCB->tamanio_ruta_archivo);
	strcpy(unPCB->ruta_archivo, rutaArchivo);
	return unPCB;

}

CPU_t* generarCPU(int32_t ID, sock_t* socketCPU){
	CPU_t* unaCPU = malloc(sizeof(CPU_t));
	unaCPU->ID=ID;
	unaCPU->socketCPU= malloc(sizeof(sock_t));
	unaCPU->socketCPU= socketCPU;
	return unaCPU;
}

void creoEstructurasDeManejo(){
	colaListos=list_create();
	colaBlock=list_create();
	colaExec=list_create();
	colaFinalizados=list_create();
	colaCPUs=list_create();

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

/*Este metodo podria ser generico para ambos algoritmos, lo que varia seria el manejo de la "cola de listos"*/
void administrarPath(char* filePath){

	idParaPCB++;
	PCB* unPCB = generarPCB(idParaPCB, filePath);
	list_add(colaListos, unPCB);
	printf("este es el pcb %s y su id = %d \n", unPCB->ruta_archivo,unPCB->PID); //Cambiar a logger
	return;

}

//Funcion del hilo servidor de conexiones
void servidor_conexiones() {

	connection_pool_server_listener(socketServidor, procesarPedido);
}

void procesarPedido(sock_t* socketCPU, header_t* header){

	int32_t recibido;
	int32_t cpu_id;

	//todo: tratar los errores


	switch(get_operation_code(header)){

	case NUEVA_CPU: {

			recibido = _receive_bytes(socketCPU, &(cpu_id), get_message_size(header));
			if(recibido == ERROR_OPERATION) return;
			CPU_t* nuevaCPU = generarCPU(cpu_id ,socketCPU);
			list_add(colaCPUs, nuevaCPU);
			break;
	}

	case TERMINO_RAFAGA: {

			break;
	}

	case INSTRUCCION_IO: {




		break;
	}

	case RESULTADO_ERROR :{

		break;
	}

	case RESULTADO_OK: {

		break;
	}

	case RESPUESTA_UTILIZACION_CPU: {

		break;
	}

	default: { break;}
	}

}

//Funcion que saca el tamaño de un PCB para enviar
int32_t obtener_tamanio_pcb(PCB* pcb) {
	return 4*sizeof(int32_t) + pcb->tamanio_ruta_archivo;
}


void asignarPCBaCPU(){         //todo: (Ceckear) toma el primer pcb de la lista, serializarlo y enviarlo a la cpu

	if(list_size(colaCPUs) > 0){
	PCB* pcbAEnviar = list_get(colaListos, 0);
	printf("agarre el primer pcb: id= %d, arch= %s", pcbAEnviar->PID, pcbAEnviar->ruta_archivo); 
	char* paquete = serializarPCB(pcbAEnviar); 
	int32_t tamanio_pcb = obtener_tamanio_pcb(pcbAEnviar);
	enviarPCB(paquete, tamanio_pcb);
	list_remove(colaListos, 0);
	printf("el size de PCBs es: %d", list_size(colaListos));
	log_info(loggerInfo, "El PCB se envio satisfactoriamente");

	}else{
		printf("No hay CPUs Disponibles que asignar \n");
		log_error(loggerError, "No hay una CPU disponible");

	//todo: tratar el caso en que no hay CPUs disponibles pero si hay PCBs, corta la ejecucion? para mi deberia
	//retornar a la consola.
	}

}


void enviarPCB(char* paquete_serializado, int32_t tamanio_pcb){    // todo: cual es el tema del "warning"?? sera porq falta algun malloc?? probe pero no
									// lo descubri

	sock_t* socketCPU = list_get(colaCPUs, 0); //?????????? FIX
	header_t* headerPCB = _create_header(ENVIO_PCB, sizeof(int32_t));
	int32_t enviado = _send_header(socketCPU, headerPCB);
	if(enviado == ERROR_OPERATION){
		return;
		free(headerPCB); //Esto abajo de un return nunca va a ejecutar
	}
	//todo: (Checkear) enviar el PCB serializado
	enviado = _send_bytes(socketCPU,paquete_serializado, sizeof(PCB));
	if(enviado == ERROR_OPERATION){
		log_error(loggerError, "Fallo en el envio de PCB");
		free(headerPCB);
		return;

	}
	
	//Una opcion mas facil SOLO cuando tenemos mensajes serializados (OJO: prestar atencion como calcular tamaño envio)
	enviado = send_msg(socketCPU, ENVIO_PCB, paquete_serializado, tamanio_pcb); //Te crea el header internamente
	if(enviado == ERROR_OPERATION){
		log_error(loggerError, "Fallo en el envio de PCB");
		return;
	}
	
	list_remove(colaCPUs, 0);
	free(headerPCB);

}



//----------------------------------// MAIN //------------------------------------------//


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


	/** Creacion del socket servidor **/
	socketServidor = create_server_socket(arch->puerto_escucha);

	int32_t result = listen_connections(socketServidor);

	if(result == ERROR_OPERATION){
		log_error(loggerError, "Error al escuchar conexiones");
		exit(EXIT_FAILURE);
	}


	/** Creo hilos CONSOLA y SERVIDOR DE CONEXIONES **/

	pthread_t server_thread;
	int32_t resultado = pthread_create(&server_thread, NULL, servidor_conexiones, NULL);
	if (resultado != 0) {
		log_error(loggerError,"Error al crear el hilo del servidor de conexiones");
		abort();
	}else{
		log_info(loggerInfo, "Se creo exitosamente el hilo de servidor de conexiones");
	}

	pthread_t consola_thread;
	resultado = pthread_create(&server_thread, NULL, consola_planificador, NULL);
	if (resultado != 0) {
		log_error(loggerError,"Error al crear el hilo de la consola");
		exit(EXIT_FAILURE);
	}else{
		log_info(loggerInfo, "Se creo exitosamente el hilo de la consola");
	}

	pthread_join(server_thread, NULL); //espero a que el hilo termine su ejecución */
	pthread_join(consola_thread, NULL);

	clean();

	return EXIT_SUCCESS;
}


