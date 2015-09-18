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
#include "Admin-Memoria.h"

/* VARIABLES GLOBALES */
ProcesoMemoria* arch;
t_log* loggerInfo;
t_log* loggerError;
t_log* loggerDebug;
sem_t sem_mutex_tlb;
sem_t sem_mutex_tabla_paginas;
sock_t* socketServidorCpus;
sock_t* socketSwap;
int32_t* frames;

ProcesoMemoria* crear_estructura_config(char* path) {
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
	config->retardo = config_get_int_value(archConfig, "RETARDO_MEMORIA");
	config->algoritmo_reemplazo = config_get_string_value(archConfig, "ALGORITMO_REEMPLAZO");
	return config;
}

/* Función que es llamada cuando ctrl+c */
void ifProcessDie() {
	log_info(loggerInfo, ANSI_COLOR_BLUE "Se dara de baja el proceso Memoria"ANSI_COLOR_RESET);
	limpiar_estructuras_memoria();
	exit(EXIT_FAILURE);
}

/** Señales **/

void ifSigusr1() {
	sem_wait(&sem_mutex_tlb);
	TLB_flush();
	sem_post(&sem_mutex_tlb);
}


void ifSigusr2() {
	sem_wait(&sem_mutex_tabla_paginas);
	limpiar_MP();
	sem_post(&sem_mutex_tabla_paginas);
}

void ifSigpoll() {
	dump();
}

void dump(){
	int pid=fork();

	if(pid==-1){
		log_error(loggerError, ANSI_COLOR_RED "Error al hacer el fork() para dump" ANSI_COLOR_RESET);
	}

	if(!pid){
		int i;
		int32_t offset;
		for(i=0;i<arch->cantidad_marcos;i++){
			char* texto= malloc(arch->tamanio_marco);
			offset=i*arch->tamanio_marco;
			memcpy(texto, mem_principal+offset, arch->tamanio_marco);
			log_info(loggerInfo, "Marco:%d; contenido:%s", i, texto);
		}
		abort();
	}
	log_error(loggerError, ANSI_COLOR_GREEN "Dump realizado con exito" ANSI_COLOR_RESET);
}

void limpiar_MP() {

	void clean_by_PID(void* parametro) {
		t_paginas_proceso* paginas_PID = (t_paginas_proceso*) parametro;

		void remove_from_MP(void* parametro) {

			TPagina* entrada = (TPagina*) parametro;

			/** Sacarle la presencia a la entrada de pagina**/
			entrada->presente = 0;

			/** Si esta modificada, escribir en swap **/
			if(entrada->modificada == 1) {
				//Todo obtener contenido del marco y escribir en swap

			}

			entrada->marco = 0;
			entrada->modificada = 0;
			entrada->tiempo_referencia = 0;
		}

		list_iterate(paginas_PID->paginas, remove_from_MP);
	}


	list_iterate(tabla_Paginas, clean_by_PID);
}

/*Función donde se inicializan los semaforos */

void inicializoSemaforos() {

	int32_t semMutex = sem_init(&sem_mutex_tlb, 0, 1);
	if (semMutex == -1)log_error(loggerError, ANSI_COLOR_RED "No pudo crearse el semaforo Mutex de la TLB" ANSI_COLOR_RESET);

	int32_t semMutexPaginas = sem_init(&sem_mutex_tabla_paginas, 0, 1);
	if (semMutexPaginas == -1)log_error(loggerError, ANSI_COLOR_RED "No pudo crearse el semaforo Mutex de la tabla de paginas" ANSI_COLOR_RESET);

}

/*Se crea un archivo de log donde se registra to-do */

void crearArchivoDeLog() {
	char* pathLog = "AdministradorMemoria.log";
	char* archLog = "MEM_Admin";
	loggerInfo = log_create(pathLog, archLog, 1, LOG_LEVEL_INFO);
	loggerError = log_create(pathLog, archLog, 1, LOG_LEVEL_ERROR);
	loggerDebug = log_create(pathLog, archLog, 1, LOG_LEVEL_DEBUG);
}

void crear_estructuras_memoria() {

	TLB_create();
	tabla_paginas_create();
	MP_create();
	frames_create();

}

void limpiar_estructuras_memoria(){

	TLB_destroy();
	tabla_paginas_destroy();
	MP_destroy();
	frames_destroy();
}

/*Main.- Queda a criterio del programador definir si requiere parametros para la invocación */
int main(void) {

	/*Se genera el archivo de log, to-do lo que sale por pantalla */
		crearArchivoDeLog();

	/*Tratamiento del ctrl+c en el proceso */
	if (signal(SIGINT, ifProcessDie) == SIG_ERR)
		log_error(loggerError, ANSI_COLOR_RED "Error con la señal SIGINT" ANSI_COLOR_RESET);

	if (signal(SIGUSR1, ifSigusr1) == SIG_ERR)
		log_error(loggerError, ANSI_COLOR_RED "Error con la señal SIGUSR1" ANSI_COLOR_RESET);

	if (signal(SIGUSR2, ifSigusr2) == SIG_ERR)
		log_error(loggerError,ANSI_COLOR_RED "Error con la señal SIGUSR2" ANSI_COLOR_RESET);

	if (signal(SIGPOLL, ifSigpoll) == SIG_ERR)
			log_error(loggerError, ANSI_COLOR_RED "Error con la señal SIGPOLL" ANSI_COLOR_RESET);

	/*Se genera el struct con los datos del archivo de config.- */
	char* path = "../Admin-Memoria.config";
	arch = crear_estructura_config(path);

	/*Se inicializan todos los semaforos necesarios */
	inicializoSemaforos();

	/** Se crearan todas las estructuras de la memoria **/
	crear_estructuras_memoria();

	socketSwap = create_client_socket(arch->ip_swap, arch->puerto_swap);
	int32_t resultado = connect_to_server(socketSwap);
	if(resultado == ERROR_OPERATION) {
		log_error(loggerError, ANSI_COLOR_RED "Error al conectar al swap" ANSI_COLOR_RESET);
		return EXIT_FAILURE;
	}

	log_info(loggerInfo, ANSI_COLOR_CYAN "Conectado al swap" ANSI_COLOR_RESET);

	socketServidorCpus = create_server_socket(arch->puerto_escucha);
	resultado = listen_connections(socketServidorCpus);
	if(resultado == ERROR_OPERATION) {
		log_error(loggerError, ANSI_COLOR_RED "Error al escuchar conexiones de las cpus" ANSI_COLOR_RESET);
		return EXIT_FAILURE;
	}

	connection_pool_server_listener(socketServidorCpus, procesar_pedido);

	/** Se destruiran las estructuras de la memoria **/
	limpiar_estructuras_memoria();

	return EXIT_SUCCESS;
}

void procesar_pedido(sock_t* socketCpu, header_t* header) {

	t_pedido_cpu* pedido_cpu = malloc(sizeof(t_pedido_cpu));
	//Todo casos de envios
	switch (get_operation_code(header)) {
	case INICIAR: {
		iniciar_proceso(socketCpu, pedido_cpu);
		break;
		}
	case LEER: {
		readOrWrite(LEER, socketCpu, header);
		break;
		}
	case ESCRIBIR: {
		readOrWrite(ESCRIBIR, socketCpu, header);
		break;
		}
	case FINALIZAR: {
		finalizarPid(socketCpu);
		break;
		}
	default: {
		log_debug(loggerDebug, "Se recibio el codigo de operacion:%d", get_operation_code(header));
		log_error(loggerError, ANSI_COLOR_RED "Desde la cpu recibo un codigo de operacion erroneo" ANSI_COLOR_RESET);
		break;
			}
	}
	log_debug(loggerDebug, ANSI_COLOR_BOLDCYAN "Pedido procesado" ANSI_COLOR_RESET);
}


/*Funciones referentes a finalizar proceso */
void finalizarPid(sock_t* socketCpu){

			int32_t PID;

			int32_t recibido = _receive_bytes(socketCpu, &(PID), sizeof(int32_t));
			if (recibido == ERROR_OPERATION) return;

			header_t* headerSwap = _create_header(BORRAR_ESPACIO, 1 * sizeof(int32_t));
			int32_t enviado = _send_header(socketSwap, headerSwap);
			if (enviado == ERROR_OPERATION) return;

			free(headerSwap);

			enviado = _send_bytes(socketSwap, &(PID), sizeof(int32_t));
			if (enviado == ERROR_OPERATION)	return;

			log_debug(loggerDebug, "Envie al swap para finalizar el proceso:%d", PID);

			header_t* headerNuevo=_create_empty_header();
			recibido=_receive_header(socketSwap,headerNuevo);
			int32_t resultado_operacion=get_operation_code(headerNuevo);
			if (recibido == ERROR_OPERATION) return;

			log_debug(loggerDebug, "Recibo del swap la operacion: %d", resultado_operacion); //todo, porque recibo 0???
			if (resultado_operacion == RESULTADO_ERROR) {
				log_debug(loggerDebug, "El swap informa que no pudo eliminar el pid:%d", PID);

				header_t* headerMemoria = _create_header(ERROR, 0);
				int32_t enviado = _send_header(socketCpu, headerMemoria);
				free(headerMemoria);
			} else if (resultado_operacion == RESULTADO_OK) {
				log_debug(loggerDebug, "El swap informa que pudo eliminar el pid:%d", PID);
				limpiar_Informacion_PID(PID);
				header_t* headerMemoria = _create_header(OK, 0);
				int32_t enviado = _send_header(socketCpu, headerMemoria);
				free(headerMemoria);
				log_info(loggerInfo, "Se ha borrado de memoria el proceso: %d op:%d", PID, OK);
			}

}

int32_t limpiar_Informacion_PID(int32_t PID){

	void limpiar(void* parametro){
		TLB* entrada = (TLB*) parametro;
		if (entrada->PID==PID){
			entrada->PID=0;
			entrada->marco=0;
			entrada->modificada=0;
			entrada->pagina=0;
			entrada->presente=0;
		}
	}
	list_iterate(TLB_tabla, limpiar);

	bool obtenerTabPagina(void* parametro){
		t_paginas_proceso* entrada = (t_paginas_proceso*) parametro;
		return entrada->PID==PID;
	}

	t_paginas_proceso* tablaPagina=list_find(tabla_Paginas, obtenerTabPagina);

	if(tablaPagina!=NULL){
		list_destroy_and_destroy_elements(tablaPagina->paginas, free);
		list_remove_and_destroy_by_condition(tabla_Paginas, obtenerTabPagina, free);
		return RESULTADO_OK	;
	}else return RESULTADO_ERROR;

}

/*Aca finalizan las funciones referentes a finalizar proceso*/

/*Funciones referentes a iniciar proceso */

void iniciar_proceso(sock_t* socketCpu, t_pedido_cpu* pedido_cpu) {

	int32_t recibido;
	int32_t enviado;
	int32_t resultado_operacion;

	recibido = _receive_bytes(socketCpu, &(pedido_cpu->pid), sizeof(int32_t));
	if (recibido == ERROR_OPERATION) return;

	recibido = _receive_bytes(socketCpu, &(pedido_cpu->cantidad_paginas),
			sizeof(int32_t));
	if (recibido == ERROR_OPERATION) return;

	log_debug(loggerDebug, ANSI_COLOR_YELLOW "Recibi el pcb %d, con %d paginas" ANSI_COLOR_RESET ,pedido_cpu->pid, pedido_cpu->cantidad_paginas);

	//Envio al swap para que reserve espacio
	header_t* headerSwap = _create_header(RESERVAR_ESPACIO,	2 * sizeof(int32_t));
	enviado = _send_header(socketSwap, headerSwap);
	if (enviado == ERROR_OPERATION)
		return;
	free(headerSwap);

	enviado = _send_bytes(socketSwap, &(pedido_cpu->pid), sizeof(int32_t));
	if (enviado == ERROR_OPERATION) return;

	enviado = _send_bytes(socketSwap, &(pedido_cpu->cantidad_paginas),
			sizeof(int32_t));
	if (enviado == ERROR_OPERATION) return;

	header_t* headerNuevo=_create_empty_header();
	recibido=_receive_header(socketSwap,headerNuevo);
	resultado_operacion=get_operation_code(headerNuevo);

	if (resultado_operacion == RESULTADO_ERROR) {
		header_t* headerCpu = _create_header(ERROR, 0);
		enviado = _send_header(socketCpu, headerCpu);
		free(headerCpu);
		log_debug(loggerDebug,ANSI_COLOR_RED"El swap informa que no se pudo asignar" ANSI_COLOR_RESET);
		return;
	} else if (resultado_operacion == RESULTADO_OK) {
		//Creo la tabla de paginas del PID dado
		log_debug(loggerDebug,"El swap informa que se pudo asignar");
		crear_tabla_pagina_PID(pedido_cpu->pid, pedido_cpu->cantidad_paginas);
		header_t* headerCpu = _create_header(OK, 0);
		enviado = _send_header(socketCpu, headerCpu);
		free(headerCpu);
		log_debug(loggerDebug,ANSI_COLOR_GREEN "En la memoria tambien se asigna" ANSI_COLOR_RESET);
	}

	free(pedido_cpu);
}
/*Aca finalizan las funciones referentes a iniciar proceso*/

/*Funciones referentes a leer o escribir pagina*/

void readOrWrite(int32_t cod_Operacion, sock_t* socketCpu, header_t* header){

	char* pedido_serializado = malloc(get_message_size(header));

	int32_t recibido = _receive_bytes(socketCpu, pedido_serializado, get_message_size(header));
	if(recibido == ERROR_OPERATION) return;

	t_pagina* pagina_pedida = deserializar_pedido(pedido_serializado);
	if(cod_Operacion==LEER)log_debug(loggerDebug, "Debo leer la pagina:%d, del proceso: %d", pagina_pedida->nro_pagina, pagina_pedida->PID);
	if(cod_Operacion==ESCRIBIR)log_debug(loggerDebug, "Se tiene que escribir del proceso:%d la pagina:%d con:%s", pagina_pedida->PID, pagina_pedida->nro_pagina, pagina_pedida->contenido);

	t_resultado_busqueda resultado=buscar_pagina(cod_Operacion, pagina_pedida);

	int32_t enviado;
	if (resultado==NOT_FOUND) {
		header_t* headerCpu = _create_header(ERROR, 0);
		enviado = _send_header(socketCpu, headerCpu);
		free(headerCpu);
		if(cod_Operacion==LEER)log_debug(loggerDebug,ANSI_COLOR_RED "No se pudo leer la pagina" ANSI_COLOR_RESET);
		if(cod_Operacion==ESCRIBIR)log_debug(loggerDebug,ANSI_COLOR_RED "No se pudo escribir la pagina" ANSI_COLOR_RESET);
		return;
	}else {
		if(cod_Operacion==LEER){
			header_t* headerCpu = _create_header(OK, 0);
			enviado = _send_header(socketCpu, headerCpu);
			int32_t tamanio=pagina_pedida->tamanio_contenido;
			enviado = _send_bytes(socketCpu, &tamanio, sizeof(int32_t));
			enviado = _send_bytes(socketCpu, pagina_pedida->contenido, tamanio);
			free(headerCpu);
			log_debug(loggerDebug,ANSI_COLOR_GREEN "Se leyo la pagina correctamente" ANSI_COLOR_RESET);
			}else{
				header_t* headerCpu = _create_header(OK, 0);
				enviado = _send_header(socketCpu, headerCpu);
				free(headerCpu);
				log_debug(loggerDebug,ANSI_COLOR_GREEN "Se escribio la pagina correctamente" ANSI_COLOR_RESET);
				 }
			}

}

t_resultado_busqueda buscar_pagina(int32_t codOperacion, t_pagina* pagina_solicitada) {

	/** Las paginas tienen el mismo tamaño del marco (como maximo) **/
	if(codOperacion==LEER)pagina_solicitada->contenido = malloc(arch->tamanio_marco);

	return TLB_buscar_pagina(codOperacion, pagina_solicitada);

}

/*Aca finalizan las funciones referentes a leer o escribir pagina*/

