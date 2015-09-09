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

ProcesoMemoria* crear_estructura_config(char* path) {
	t_config* archConfig = config_create(path);
	ProcesoMemoria* config = malloc(sizeof(ProcesoMemoria));
	config->puerto_escucha = config_get_int_value(archConfig, "PUERTO_ESCUCHA");
	config->ip_swap = config_get_string_value(archConfig, "IP_SWAP");
	config->puerto_swap = config_get_int_value(archConfig, "PUERTO_SWAP");
	config->maximo_marcos = config_get_int_value(archConfig,
			"MAXIMO_MARCOS_POR_PROCESO");
	config->cantidad_marcos = config_get_int_value(archConfig,
			"CANTIDAD_MARCOS");
	config->tamanio_marco = config_get_int_value(archConfig, "TAMANIO_MARCO");
	config->entradas_tlb = config_get_int_value(archConfig, "ENTRADAS_TLB");
	config->tlb_habilitada = config_get_string_value(archConfig,
			"TLB_HABILITADA");
	config->retardo = config_get_int_value(archConfig, "RETARDO_MEMORIA");
	return config;
}

/* Función que es llamada cuando ctrl+c */
void ifProcessDie() {
	log_info(loggerInfo, "Se dara de baja el proceso Memoria");
	exit(1);
}

/** Señales **/

void ifSigusr1() {
	sem_wait(&sem_mutex_tlb);
	TLB_flush();
	sem_post(&sem_mutex_tlb);
}


void ifSigusr2() {
	//todo actualizar bit presencia en tabla de paginas
}

void ifSigpoll() {
	dump();
}

void dump(){
	int pid=fork();

	if(pid==-1){
		log_error(loggerError, "Error al hacer el fork() para dump");
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
}

/*Función donde se inicializan los semaforos */

void inicializoSemaforos() {

	int32_t semMutex = sem_init(&sem_mutex_tlb, 0, 1);
	if (semMutex == -1)
		log_error(loggerError, "No pudo crearse el semaforo Mutex");
}

/*Se crea un archivo de log donde se registra to-do */

void crearArchivoDeLog() {
	char* pathLog = "AdministradorMemoria.log";
	char* archLog = "MEM_Admin";
	loggerInfo = log_create(pathLog, archLog, 1, LOG_LEVEL_INFO);
	loggerError = log_create(pathLog, archLog, 1, LOG_LEVEL_ERROR);
	loggerDebug = log_create(pathLog, archLog, 1, LOG_LEVEL_DEBUG);
}


/*Main.- Queda a criterio del programador definir si requiere parametros para la invocación */
int main(void) {

	/*En el header Colores, se adjunta un ejemplo de uso de los colores por consola*/

	/*Tratamiento del ctrl+c en el proceso */
	if (signal(SIGINT, ifProcessDie) == SIG_ERR)
		log_error(loggerError, "Error con la señal SIGINT");

	if (signal(SIGUSR1, ifSigusr1) == SIG_ERR)
		log_error(loggerError, "Error con la señal SIGUSR1");

	if (signal(SIGUSR2, ifSigusr2) == SIG_ERR)
		log_error(loggerError, "Error con la señal SIGUSR2");

	if (signal(SIGPOLL, ifSigpoll) == SIG_ERR)
			log_error(loggerError, "Error con la señal SIGPOLL");

	/*Se genera el struct con los datos del archivo de config.- */
	char* path = "../Admin-Memoria.config";
	arch = crear_estructura_config(path);

	/*Se genera el archivo de log, to-do lo que sale por pantalla */
	crearArchivoDeLog();

	/*Se inicializan todos los semaforos necesarios */
	inicializoSemaforos();

	TLB_crear();
	MP_crear();

	socketSwap = create_client_socket(arch->ip_swap, arch->puerto_swap);
	int32_t resultado = connect_to_server(socketSwap);
	if(resultado == ERROR_OPERATION) {
		log_error(loggerError, "Error al conectar al swap");
		return EXIT_FAILURE;
	}

	log_debug(loggerDebug, "Conectado al swap");

	socketServidorCpus = create_server_socket(arch->puerto_escucha);
	resultado = listen_connections(socketServidorCpus);
	if(resultado == ERROR_OPERATION) {
		log_error(loggerError, "Error al escuchar conexiones de las cpus");
		return EXIT_FAILURE;
	}

	connection_pool_server_listener(socketServidorCpus, procesar_pedido);

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
		leer_pagina(socketCpu, header);
		break;
		}
	case ESCRIBIR: {
		escribirPagina(socketCpu, header);
		break;
		}
	case FINALIZAR: {
		finalizarPid(socketCpu);
		break;
		}
	default: {
		log_error(loggerError, "Desde la cpu recibo un codigo de operacion erroneo");
		break;
			}
	}
	log_info(loggerInfo,"Pedido procesado");
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

			int32_t resultado_operacion;
			recibido = _receive_bytes(socketSwap, &(resultado_operacion),sizeof(int32_t));
			if (recibido == ERROR_OPERATION) return;

			if (resultado_operacion == RESULTADO_ERROR) {
				log_debug(loggerDebug, "El swap informa que no pudo eliminar el pid:%d", PID);
				header_t* headerMemoria = _create_header(RESULTADO_ERROR, 0);
				int32_t enviado = _send_header(socketCpu, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				free(headerMemoria);
			} else if (resultado_operacion == RESULTADO_OK) {
				log_debug(loggerDebug, "El swap informa que no pudo eliminar el pid:%d", PID);
				limpiar_Informacion_PID(PID);
				header_t* headerMemoria = _create_header(RESULTADO_OK, 0);
				int32_t enviado = _send_header(socketCpu, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				free(headerMemoria);
				log_info(loggerInfo, "Se ha borrado de memoria el proceso: %d", PID);
			}

}

int32_t limpiar_Informacion_PID(int32_t PID){
	void limpiar(TLB* entrada){
		if (entrada->PID==PID){
			entrada->PID=0;
			entrada->marco=0;
			entrada->modificada=0;
			entrada->pagina=0;
			entrada->presente=0;
		}
	}
	list_iterate(TLB_tabla, limpiar);

	bool obtenerTabPagina(t_paginas_proceso* entrada){
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
	if (recibido == ERROR_OPERATION)
		return;

	recibido = _receive_bytes(socketCpu, &(pedido_cpu->cantidad_paginas),
			sizeof(int32_t));
	if (recibido == ERROR_OPERATION)
		return;

	log_debug(loggerDebug, "Recibi el pcb %d, con %d paginas",pedido_cpu->pid, pedido_cpu->cantidad_paginas);

	//Envio al swap para que reserve espacio
	header_t* headerSwap = _create_header(RESERVAR_ESPACIO,
			2 * sizeof(int32_t));
	enviado = _send_header(socketSwap, headerSwap);
	if (enviado == ERROR_OPERATION)
		return;
	free(headerSwap);

	enviado = _send_bytes(socketSwap, &(pedido_cpu->pid), sizeof(int32_t));
	if (enviado == ERROR_OPERATION)
		return;

	enviado = _send_bytes(socketSwap, &(pedido_cpu->cantidad_paginas),
			sizeof(int32_t));
	if (enviado == ERROR_OPERATION)
		return;

	recibido = _receive_bytes(socketSwap, &(resultado_operacion),
			sizeof(int32_t));
	if (recibido == ERROR_OPERATION)
		return;

	if (resultado_operacion == RESULTADO_ERROR) {
		header_t* headerCpu = _create_header(ERROR, 0);
		enviado = _send_header(socketCpu, headerCpu);
		free(headerCpu);
		return;
	} else if (resultado_operacion == RESULTADO_OK) {
		//Creo la tabla de paginas del PID dado
		crear_tabla_pagina_PID(pedido_cpu->pid, pedido_cpu->cantidad_paginas);
		header_t* headerCpu = _create_header(OK, 0);
		enviado = _send_header(socketCpu, headerCpu);
		free(headerCpu);

	}

	free(pedido_cpu);
}
/*Aca finalizan las funciones referentes a iniciar proceso*/

/*Funciones referentes a leer pagina*/

void leer_pagina(sock_t* socketCpu, header_t* header){
	char* pedido_serializado = malloc(get_message_size(header));
	int32_t recibido = _receive_bytes(socketCpu, pedido_serializado, get_message_size(header));
	if(recibido == ERROR_OPERATION) return;
	t_pagina* pagina_pedida = deserializar_pedido(pedido_serializado);
	char* contenido_pagina = buscar_pagina(pagina_pedida);
	//Todo, si contenido es NULL enviar error, else, enviar contenido
	//char* serializado= serializarTexto(leer_pagina(pagina_pedida));
	//enviar serializado
}

char* buscar_pagina(t_pagina* pagina_solicitada) {

	/** Las paginas tienen el mismo tamaño del marco (como maximo) **/
	char* contenido_pagina = malloc(arch->tamanio_marco);

	t_resultado_busqueda resultado = NOT_FOUND;

	resultado = TLB_buscar_pagina(pagina_solicitada, &contenido_pagina);
	if(resultado==NOT_FOUND){
		return NULL;
	}else{
			//TODO correr algoritmo de reemplazo de pags
		return contenido_pagina;
	}
}

/*Aca finalizan las funciones referentes a leer pagina*/

/*Funciones referentes a escribir pagina*/

void escribirPagina(sock_t* socketCpu, header_t* header){
	//TODO recibir t_pagina y texto
	char* pedido_serializado = malloc(get_message_size(header));
	int32_t recibido = _receive_bytes(socketCpu, pedido_serializado, get_message_size(header));
	if(recibido == ERROR_OPERATION) return;
	t_pagina* pagina_pedida = deserializar_pedido(pedido_serializado);
	t_resultado_busqueda resultado = buscar_y_escribir_pagina(pagina_pedida);
	//todo if resultado NOTFOUND devolver error, sino OK

}

t_resultado_busqueda buscar_y_escribir_pagina(t_pagina* pagina_pedida){
	t_resultado_busqueda resultado = NOT_FOUND;
	return resultado = TLB_buscar_pagina_escribir(pagina_pedida);
}

t_resultado_busqueda TLB_buscar_pagina_escribir(t_pagina* pagina) {

	bool find_by_PID_page(TLB* entrada) {

		return (entrada->PID == pagina->PID) && (entrada->pagina == pagina->nro_pagina);
	}
	TLB* entrada_pagina=NULL;
	if(TLB_habilitada()) {
		sem_wait(&sem_mutex_tlb);
		entrada_pagina = list_find(TLB_tabla, find_by_PID_page);
		sem_post(&sem_mutex_tlb);
	}
	if(entrada_pagina == NULL) {
		log_debug(loggerDebug, "No se encontro la pagina en la TLB, se busca en la tabla de paginas");
		return buscar_pagina_a_escribir_tabla_paginas(pagina);
	}
	else{
		int32_t offset=entrada_pagina->marco*arch->tamanio_marco;
		memcpy(mem_principal+offset, pagina->contenido, arch->tamanio_marco);
		entrada_pagina->modificada=1;
		return FOUND;
		}

}

t_resultado_busqueda buscar_pagina_a_escribir_tabla_paginas(t_pagina* pagina){
	bool obtenerTabPagina(t_paginas_proceso* entrada){
			return entrada->PID == pagina->PID;
		}

		t_paginas_proceso* tablaPagina=list_find(tabla_Paginas, obtenerTabPagina);
		if(tablaPagina!=NULL){
			bool obtenerMarco_Pagina(TPagina* entradaBuscada){
					return entradaBuscada->pagina== pagina->nro_pagina;
				}

			TPagina* entradaFound = list_find(tablaPagina->paginas, obtenerMarco_Pagina);
			int32_t offset=(entradaFound->marco)*(arch->tamanio_marco);
			memcpy(mem_principal+offset,pagina->contenido,arch->tamanio_marco); 		//todo va con &?
			entradaFound->modificada=1;
			return FOUND;
		}else {
			log_debug(loggerDebug, "No se encontro en la tabla de paginas, se pide al swap");
			//TODO es necesario que lo pida al swap?
			return FOUND;
		}


	}

/*Aca finalizan las funciones referentes a escribir pagina*/
