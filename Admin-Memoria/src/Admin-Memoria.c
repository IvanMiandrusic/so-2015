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

/* VARIABLES GLOBALES */
ProcesoMemoria* arch;
t_log* loggerInfo;
t_log* loggerError;
t_log* loggerDebug;
t_list* tabla_TLB;
char* mem_principal;
t_list* tabla_Paginas;
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

	exit(1);
}

void ifSigusr1() {
	sem_wait(&sem_mutex_tlb);
	TLB_flush();
	sem_post(&sem_mutex_tlb);
}


void ifSigusr2() {
	//todo actualizar bit presencia en tabla de paginas
}

void ifSigpoll() {
//todo dump
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

void TLB_init() {

	int i;
	for (i = 0; i < arch->entradas_tlb; i++) {

		TLB* nuevaEntrada = malloc(sizeof(TLB));
		nuevaEntrada->PID = 0;
		nuevaEntrada->pagina = 0;
		nuevaEntrada->modificada = 0;
		nuevaEntrada->presente = 0;
		nuevaEntrada->marco = 0;

		sem_wait(&sem_mutex_tlb);
		list_add(tabla_TLB, nuevaEntrada);
		sem_post(&sem_mutex_tlb);
	}
}

void TLB_flush() {

	void limpiar_entradas(TLB* entrada) {
		//todo si modificada es 1, le mandas al swap escribir(PID, pagina, texto)
		//o hacerlo con la tabla de paginas
		entrada->PID = 0;
		entrada->marco = 0;
		entrada->modificada = 0;
		entrada->pagina = 0;
		entrada->presente = 0;
	}

	sem_wait(&sem_mutex_tlb);
	list_iterate(tabla_TLB, limpiar_entradas);
	sem_post(&sem_mutex_tlb);
}

void crear_tabla_pagina_PID(int32_t processID, int32_t cantidad_paginas) {

	t_paginas_proceso* nueva_entrada_proceso = malloc(
			sizeof(t_paginas_proceso));
	nueva_entrada_proceso->PID = processID;
	nueva_entrada_proceso->paginas = list_create();

	int i;
	for (i = 0; i < cantidad_paginas; i++) {

		TPagina* nuevaEntrada = malloc(sizeof(TPagina));
		nuevaEntrada->marco = 0;
		nuevaEntrada->pagina = i;
		nuevaEntrada->modificada = 0;
		nuevaEntrada->presente = 0;

		list_add(nueva_entrada_proceso->paginas, nuevaEntrada);
	}

	sem_wait(&sem_mutex_tabla_paginas);
	list_add(tabla_Paginas, nueva_entrada_proceso);
	sem_post(&sem_mutex_tabla_paginas);

}

void creoEstructurasDeManejo() {
	if (string_equals_ignore_case((arch->tlb_habilitada), "si")) {
		tabla_TLB = list_create();
		TLB_init();
	}
	mem_principal = malloc((arch->cantidad_marcos) * (arch->tamanio_marco));
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
	char* path = "Admin-Memoria.config";
	arch = crear_estructura_config(path);

	/*Se genera el archivo de log, to-do lo que sale por pantalla */
	crearArchivoDeLog();

	/*Se inicializan todos los semaforos necesarios */
	inicializoSemaforos();

	creoEstructurasDeManejo();

	socketSwap = create_client_socket(arch->ip_swap, arch->puerto_swap);
	int32_t resultado = connect_to_server(socketSwap);
	if(resultado == ERROR_OPERATION) {
		log_error(loggerError, "Error al conectar al swap");
		return EXIT_FAILURE;
	}

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
		char* pedido_serializado = malloc(get_message_size(header));
		int32_t recibido = _receive_bytes(socketCpu, pedido_serializado, get_message_size(header));
		if(recibido == ERROR_OPERATION) return;
		t_pagina* pagina_pedida = deserializar_pedido(pedido_serializado);
		char* contenido_pagina = buscar_pagina(pagina_pedida);
		//Todo, buscar en TLB, buscar en TPags y sino ->Swap
		//char* serializado= serializarTexto(leer_pagina(pagina_pedida));
		//enviar serializado

		break;
	}
	case ESCRIBIR: {

		break;
	}
	case FINALIZAR: {
		int32_t PID;
		int32_t recibido = _receive_bytes(socketCpu, &(PID), sizeof(int32_t));
		if (recibido == ERROR_OPERATION)return;
		header_t* headerSwap = _create_header(BORRAR_ESPACIO, 1 * sizeof(int32_t));
		int32_t enviado = _send_header(socketSwap, headerSwap);
		if (enviado == ERROR_OPERATION)return;
		free(headerSwap);
		enviado = _send_bytes(socketSwap, &(pedido_cpu->pid), sizeof(int32_t));
		if (enviado == ERROR_OPERATION)	return;

		int32_t resultado_operacion;
		recibido = _receive_bytes(socketSwap, &(resultado_operacion),sizeof(int32_t));
		if (recibido == ERROR_OPERATION)return;
		//Todo si no se puede borrar en el swap que pasa?
		int32_t operacionValida= borrarEspacio(PID);

		if (operacionValida == RESULTADO_ERROR) {
			header_t* headerMemoria = _create_header(RESULTADO_ERROR, 0);
			int32_t enviado = _send_header(socketCpu, headerMemoria);
			if(enviado == ERROR_OPERATION) return;
			free(headerMemoria);
			return;
		} else if (operacionValida == RESULTADO_OK) {
			header_t* headerMemoria = _create_header(RESULTADO_OK, 0);
			int32_t enviado = _send_header(socketCpu, headerMemoria);
			if(enviado == ERROR_OPERATION) return;
			free(headerMemoria);
		}
		break;
	}
	default: {

		break;
	}
	}
}

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

char* buscar_pagina(t_pagina* pagina_pedida) {

	bool find_by_PID_page(TLB* entrada) {

		return (entrada->PID == pagina_pedida->PID) && (entrada->pagina == pagina_pedida->nro_pagina);
	}

	sem_wait(&sem_mutex_tlb);
	TLB* entrada_pagina = list_find(tabla_TLB, find_by_PID_page);
	sem_post(&sem_mutex_tlb);

	if(entrada_pagina == NULL); //Todo ir a buscar a tabla de paginas

	else{
		//Traer el contenido de MP
	}

	return NULL;
}

int32_t borrarEspacio(int32_t PID){
	//todo
	return RESULTADO_OK;
}

