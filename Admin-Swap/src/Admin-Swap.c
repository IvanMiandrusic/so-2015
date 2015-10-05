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
#include "Colores.h"
#include "Admin-Swap.h"

/* VARIABLES GLOBALES */
ProcesoSwap* arch;
t_log* loggerInfo;
t_log* loggerError;
t_log* loggerDebug;
t_list* espacioLibre;
t_list* espacioOcupado;
t_list* metricas;
sem_t sem_mutex_libre;
sem_t sem_mutex_ocupado;
int32_t totalPaginas;
sock_t* socketServidor;

ProcesoSwap* crear_estructura_config(char* path)
{
    t_config* archConfig = config_create(path);
    ProcesoSwap* config = malloc(sizeof(ProcesoSwap));
    config->puerto_escucha = config_get_int_value(archConfig, "PUERTO_ESCUCHA");
    config->nombre_swap = config_get_string_value(archConfig, "NOMBRE_SWAP");
    config->cantidad_paginas = config_get_int_value(archConfig, "CANTIDAD_PAGINAS");
    config->tamanio_pagina= config_get_int_value(archConfig, "TAMANIO_PAGINA");
    config->retardo= config_get_int_value(archConfig, "RETARDO_SWAP");
    config->retardo_comp= config_get_int_value(archConfig, "RETARDO_COMPACTACION");
    config_destroy(archConfig);
    return config;
}

/* Función que es llamada cuando ctrl+c */
void ifProcessDie(){
		log_error(loggerError, ANSI_COLOR_BLUE "Se da por finalizado el proceso Swap"ANSI_COLOR_RESET);
		exit(1);
}

/*Función donde se inicializan los semaforos */

void inicializoSemaforos(){

	int32_t semMutexLibre = sem_init(&sem_mutex_libre,0,1);
	if(semMutexLibre==-1)log_error(loggerError, ANSI_COLOR_RED"No pudo crearse el semaforo Mutex del espacio Libre"ANSI_COLOR_RESET);

	int32_t semMutexOcupado = sem_init(&sem_mutex_ocupado,0,1);
	if(semMutexOcupado==-1)log_error(loggerError, ANSI_COLOR_RED"No pudo crearse el semaforo Mutex del espacio Ocupado"ANSI_COLOR_RESET);

}

/*Se crea un archivo de log donde se registra to-do */

void crearArchivoDeLog() {
	char* pathLog = "AdministradorSwap.log";
	char* archLog = "SWAP-Admin";
	loggerInfo = log_create(pathLog, archLog, 1, LOG_LEVEL_INFO);
	loggerError = log_create(pathLog, archLog, 1, LOG_LEVEL_ERROR);
	loggerDebug = log_create(pathLog, archLog, 1, LOG_LEVEL_DEBUG);
}

/*Se crean las estructuras necesarias para manejar el Swap*/
void creoEstructuraSwap(){

	espacioLibre=list_create();
	espacioOcupado=list_create();
	metricas=list_create();

	char* comando=string_new();
	string_append_with_format(&comando, "dd if=/dev/zero of=%s bs=%d count=%d", arch->nombre_swap, arch->tamanio_pagina, arch->cantidad_paginas);

	execv(NULL,comando);
	free(comando);
	log_info(loggerInfo, ANSI_COLOR_BOLDGREEN"Archivo de Swap creado con exito" ANSI_COLOR_RESET);
	NodoLibre* nodo= malloc(sizeof(NodoLibre));
	nodo->comienzo=0;
	nodo->paginas=arch->cantidad_paginas;

	sem_wait(&sem_mutex_libre);
	list_add(espacioLibre, nodo);
	sem_post(&sem_mutex_libre);

	totalPaginas=arch->cantidad_paginas;

}

void leeryEscribir(NodoOcupado* nodoViejo,int32_t comienzo){

	int i;
	for(i=nodoViejo->comienzo;i<nodoViejo->paginas+nodoViejo->comienzo;i++){

		t_pagina* pagina=malloc(sizeof(t_pagina));
		pagina->PID=nodoViejo->PID;
		pagina->nro_pagina=i;
		pagina->contenido=NULL;
		leer_pagina(pagina);
		pagina->nro_pagina=comienzo;
		escribir_pagina(pagina);
		comienzo++;

	}

}

int32_t compactar(){

	sem_wait(&sem_mutex_ocupado);

	if (list_is_empty(espacioOcupado)){
		log_error(loggerError, ANSI_COLOR_RED "No se puede compactar una memoria vacía"ANSI_COLOR_RESET);
		return -1;
	}
	if(list_is_empty(espacioLibre)){
		log_error(loggerError, ANSI_COLOR_RED "No se puede compactar una memoria completamente llena"ANSI_COLOR_RESET);
		return -1;
	}

	log_info(loggerInfo, ANSI_COLOR_GREEN"Compactacion iniciada por fragmentacion externa"ANSI_COLOR_RESET);

	NodoOcupado* primerNodo=list_get(espacioOcupado, 0);
	if(primerNodo->comienzo!=0){
		log_debug(loggerDebug, "Debo mover el primer elemento que no arranca en 0");
		leeryEscribir(primerNodo,0);
		primerNodo->comienzo=0;
	}

	int32_t i;

	for(i=1; i<list_size(espacioOcupado); i++){
		NodoOcupado* nodo= list_get(espacioOcupado, i);
		NodoOcupado* anterior= list_get(espacioOcupado, i-1);
		bool chequeo=(nodo->comienzo==anterior->comienzo+anterior->paginas);
		if(!chequeo){
			leeryEscribir(nodo,anterior->comienzo+anterior->paginas);
			nodo->comienzo=anterior->comienzo+anterior->paginas;
		}
		//fin for
	}
	NodoOcupado* nodo=list_get(espacioOcupado, i-1);
	int32_t nuevoComienzo=(nodo->comienzo+nodo->paginas);
	if(nuevoComienzo==totalPaginas){
		list_clean_and_destroy_elements(espacioLibre, free);
	}else{
	NodoLibre* nodoLibre=malloc(sizeof(NodoLibre));
	nodoLibre->comienzo = nuevoComienzo;
	nodoLibre->paginas = totalPaginas - nuevoComienzo;
	log_debug(loggerDebug, "Creo un hueco con comienzo: %d, offset:%d", nuevoComienzo, nodoLibre->paginas);
	sem_wait(&sem_mutex_libre);
	list_clean_and_destroy_elements(espacioLibre, free);
	list_add(espacioLibre, nodoLibre);
	sem_post(&sem_mutex_libre);
	}
	graficoCompactar();
	sem_post(&sem_mutex_ocupado);
	return 1;
}

int32_t calcularEspacioLibre(){
	int32_t espacio = 0;
	int i;
	for(i=0; i<list_size(espacioLibre); i++){
		NodoLibre* nodo= list_get(espacioLibre, i);
		espacio+=nodo->paginas;
	}
	log_debug(loggerDebug, "El espacio libre es %d", espacio);
	return espacio;
}

void graficoCompactar(){
	printf(ANSI_COLOR_BOLDYELLOW "Procesando, por favor espere......\n" ANSI_COLOR_RESET);
	printf("Procesando, por favor espere......\n");
	sleep(arch->retardo_comp/3);
	printf("░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
		   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░\n"
		   "░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
		   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n");
	printf("\n... Creando estructuras necesarias para la compactación.....\n");
	sleep(arch->retardo_comp/3);
	printf("▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
	   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
	   "░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
	   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n");
	sleep(arch->retardo_comp/3);
	printf("\n... Guardando estructuras necesarias para la compactación\n");

	printf("▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
	   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
	   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
	   "░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n");

	log_info(loggerInfo, ANSI_COLOR_GREEN"Compactacion finalizada por fragmentacion externa"ANSI_COLOR_RESET);
}

int main(void) {

	/*Tratamiento del ctrl+c en el proceso */
	if(signal(SIGINT, ifProcessDie) == SIG_ERR ) log_error(loggerError, ANSI_COLOR_RED"Error con la señal SIGINT"ANSI_COLOR_RESET);


	/*Se genera el struct con los datos del archivo de config.- */
	char* path = "../Admin-Swap.config";
    arch = crear_estructura_config(path);

	/*Se genera el archivo de log, to-do lo que sale por pantalla */
	crearArchivoDeLog();


	/*Se inicializan todos los semaforos necesarios */
	inicializoSemaforos();

	/*Se inicializan todos los semaforos necesarios */
	creoEstructuraSwap();

	socketServidor = create_server_socket(arch->puerto_escucha);

	int32_t resultado = listen_connections(socketServidor);

	if(resultado == ERROR_OPERATION) {
		log_error(loggerError, ANSI_COLOR_RED "Error al escuchar nuevas conexiones"ANSI_COLOR_RESET);
		exit(EXIT_FAILURE);
	}

	sock_t* socketCliente = accept_connection(socketServidor);

	log_debug(loggerDebug, "Memoria conectada");

	recibir_operaciones_memoria(socketCliente);

	limpiar_estructuras_swap();

	return EXIT_SUCCESS;
}

void limpiar_estructuras_swap() {

	log_destroy(loggerDebug);
	log_destroy(loggerError);
	log_destroy(loggerInfo);
	list_destroy_and_destroy_elements(metricas, free);
	list_destroy_and_destroy_elements(espacioLibre, free);
	list_destroy_and_destroy_elements(espacioOcupado, free);
	sem_destroy(&sem_mutex_libre);
	sem_destroy(&sem_mutex_ocupado);
	free(arch);
	clean_socket(socketServidor);
}

void recibir_operaciones_memoria(sock_t* socketMemoria){

	int32_t finalizar = true;
	int32_t recibido;
	char* pedido_serializado;
	header_t* header = _create_empty_header();

	while(finalizar){

		recibido = _receive_header(socketMemoria, header);

		if(recibido == ERROR_OPERATION) return;
		if(recibido == SOCKET_CLOSED) {
			socketMemoria = accept_connection(socketServidor);
			return;
		}

		switch(get_operation_code(header)) {

		case LEER_PAGINA: {
			pedido_serializado = malloc(get_message_size(header));
			recibido = _receive_bytes(socketMemoria, pedido_serializado, get_message_size(header));
			if(recibido == ERROR_OPERATION) return;
			t_pagina* pagina_pedida = deserializar_pedido(pedido_serializado);
			leer_pagina(pagina_pedida);
			sleep(arch->retardo);
			header_t* headerMemoria = _create_header(RESULTADO_OK, sizeof(int32_t)+pagina_pedida->tamanio_contenido);
			int32_t enviado = _send_header(socketMemoria, headerMemoria);
			enviado=_send_bytes(socketMemoria, &pagina_pedida->tamanio_contenido, sizeof(int32_t));
			enviado=_send_bytes(socketMemoria, pagina_pedida->contenido, pagina_pedida->tamanio_contenido);
			log_debug(loggerDebug, "Se leyeron :%d bytes", pagina_pedida->tamanio_contenido);
			if(enviado == ERROR_OPERATION) return;
			free(headerMemoria);
			break;
		}

		case ESCRIBIR_PAGINA: {
			pedido_serializado = malloc(get_message_size(header));
			recibido = _receive_bytes(socketMemoria, pedido_serializado, get_message_size(header));
			if(recibido == ERROR_OPERATION) return;
			t_pagina* pagina_pedida = deserializar_pedido(pedido_serializado);
			log_debug(loggerDebug, "Me llega a escribir pid: %d, pagina:%d, tam:%d, contenido:%s", pagina_pedida->PID, pagina_pedida->nro_pagina, pagina_pedida->tamanio_contenido, pagina_pedida->contenido);
			int32_t resultado= escribir_pagina(pagina_pedida);
			sleep(arch->retardo);
			if (resultado == RESULTADO_ERROR) {
				header_t* headerMemoria = _create_header(RESULTADO_ERROR, 0);
				int32_t enviado = _send_header(socketMemoria, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				free(headerMemoria);
				return;
			} else if (resultado == RESULTADO_OK) {
				header_t* headerMemoria = _create_header(RESULTADO_OK, 0);
				int32_t enviado = _send_header(socketMemoria, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				free(headerMemoria);
				}
			break;
		}

		case RESERVAR_ESPACIO: {
			t_pedido_memoria* pedido_memoria = malloc(sizeof(t_pedido_memoria));
			recibido = _receive_bytes(socketMemoria, &(pedido_memoria->pid), sizeof(int32_t));
			if (recibido == ERROR_OPERATION)return;

			recibido = _receive_bytes(socketMemoria, &(pedido_memoria->cantidad_paginas),
					sizeof(int32_t));
			if (recibido == ERROR_OPERATION)return;

			log_debug(loggerDebug, "Recibi este pedido: %d y cant paginas %d", pedido_memoria->pid, pedido_memoria->cantidad_paginas);

			int32_t operacionValida= reservarEspacio(pedido_memoria);
			log_debug (loggerDebug, "Reservo espacio con resultado: %d", operacionValida);
			sleep(arch->retardo);
			if (operacionValida == RESULTADO_ERROR) {
				header_t* headerMemoria = _create_header(RESULTADO_ERROR, 0);
				int32_t enviado = _send_header(socketMemoria, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				log_debug(loggerDebug, "No se reservo el espacio solicitado");
				free(headerMemoria);

			} else if (operacionValida == RESULTADO_OK) {
				header_t* headerMemoria = _create_header(RESULTADO_OK, 0);
				int32_t enviado = _send_header(socketMemoria, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				log_info(loggerInfo, ANSI_COLOR_GREEN"Se reservo el espacio solicitado" ANSI_COLOR_RESET);
				free(headerMemoria);
			}
			break;
		}

		case BORRAR_ESPACIO: {
			int32_t PID;
			recibido = _receive_bytes(socketMemoria, &(PID), sizeof(int32_t));
			if (recibido == ERROR_OPERATION)return;
			int32_t operacionValida= borrarEspacio(PID);
			sleep(arch->retardo);
			if (operacionValida == RESULTADO_ERROR) {
				header_t* headerMemoria = _create_header(RESULTADO_ERROR, 0);
				int32_t enviado = _send_header(socketMemoria, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				log_debug(loggerDebug, "Finalizado erroneo");
				free(headerMemoria);
				return;
			} else if (operacionValida == RESULTADO_OK) {
				header_t* headerMemoria = _create_header(RESULTADO_OK, 0);
				int32_t enviado = _send_header(socketMemoria, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				log_debug(loggerDebug, "Finalizado correcto, se envia:%d", RESULTADO_OK);
				free(headerMemoria);
			}
			break;
		}

		default: {
				log_error(loggerError, ANSI_COLOR_RED "Se recibió un codigo de operación no valido"ANSI_COLOR_RESET);
		}

		}

	}
}

int32_t reservarEspacio(t_pedido_memoria* pedido_pid){
	int32_t libre=calcularEspacioLibre();
	if(libre < pedido_pid->cantidad_paginas)return RESULTADO_ERROR;
	bool tieneEspacio(NodoLibre* nodo)
		{
				return nodo->paginas>=pedido_pid->cantidad_paginas;
		}
	sem_wait(&sem_mutex_libre);
	NodoLibre* nodoLibre=list_find(espacioLibre, tieneEspacio);
	sem_post(&sem_mutex_libre);

	if(nodoLibre==NULL){
		log_debug(loggerDebug, "Se debe compactar, no hay espacio libre");
		int32_t resultado=compactar();
		if (resultado==-1) return RESULTADO_ERROR;
		nodoLibre=list_get(espacioLibre, 0);
	}
	if(nodoLibre==NULL)return RESULTADO_ERROR;

	NodoOcupado* nodoNuevo=malloc(sizeof(NodoOcupado));
	nodoNuevo->PID=pedido_pid->pid;
	nodoNuevo->comienzo=nodoLibre->comienzo;
	nodoNuevo->paginas=pedido_pid->cantidad_paginas;

	sem_wait(&sem_mutex_ocupado);
	list_add(espacioOcupado, nodoNuevo);
	sem_post(&sem_mutex_ocupado);
	int32_t nuevoComienzo=nodoNuevo->comienzo+nodoNuevo->paginas;
	int32_t nuevoCantPag=nodoLibre->paginas-nodoNuevo->paginas;

	bool findNodo(NodoLibre* nodoABorrar)
	{
		return nodoABorrar->comienzo == nodoLibre->comienzo && nodoABorrar->paginas == nodoLibre->paginas;
	}

	if(nuevoComienzo==arch->cantidad_paginas && nuevoCantPag==0){
	sem_wait(&sem_mutex_libre);
	list_remove_and_destroy_by_condition(espacioLibre, findNodo, free);
	sem_post(&sem_mutex_libre);
	}else{
		nodoLibre->comienzo=nuevoComienzo;
		nodoLibre->paginas= nuevoCantPag;
		log_debug(loggerDebug, "Creo un hueco, comienzo:%d, cantpags:%d",  nodoLibre->comienzo, nodoLibre->paginas);
	}
	log_debug(loggerDebug, "Creo un proceso: %d, comienzo:%d, cantpags:%d", nodoNuevo->PID, nodoNuevo->comienzo, nodoNuevo->paginas);

	t_metrica* metrica=malloc(sizeof(t_metrica));
	metrica->PID=pedido_pid->pid;
	metrica->lecturas=0;
	metrica->escrituras=0;
	list_add(metricas, metrica);
	return RESULTADO_OK;
}

int32_t borrarEspacio(int32_t PID){

	bool find_by_PID(NodoOcupado* nodo)
			{
				return nodo->PID==PID;
			}

	NodoOcupado* procesoRemovido = list_remove_by_condition(espacioOcupado, find_by_PID);
	if (procesoRemovido==NULL) return RESULTADO_ERROR;
	bool find_by_ConditionInitial(NodoLibre* espacioAnterior)
				{
					return espacioAnterior->comienzo+espacioAnterior->paginas==procesoRemovido->comienzo;
				}

	NodoLibre* nodoAnterior=list_find(espacioLibre, find_by_ConditionInitial);

	bool find_by_ConditionFinal(NodoLibre* espacioPosterior)
					{
						return espacioPosterior->comienzo==procesoRemovido->comienzo+procesoRemovido->paginas;
					}

	NodoLibre* nodoPosterior=list_find(espacioLibre, find_by_ConditionFinal);
	NodoLibre* nuevoNodo=malloc(sizeof(NodoLibre));

	if(nodoAnterior!=NULL && nodoPosterior!=NULL){
		nuevoNodo->comienzo=nodoAnterior->comienzo;
		nuevoNodo->paginas=nodoAnterior->paginas+procesoRemovido->paginas+nodoPosterior->paginas;
		list_remove_and_destroy_by_condition(espacioLibre, find_by_ConditionInitial,free);
		list_remove_and_destroy_by_condition(espacioLibre, find_by_ConditionFinal,free);
	}else if(nodoAnterior!=NULL && nodoPosterior==NULL){
		nuevoNodo->comienzo=nodoAnterior->comienzo;
		nuevoNodo->paginas=nodoAnterior->paginas+procesoRemovido->paginas;
		list_remove_and_destroy_by_condition(espacioLibre, find_by_ConditionInitial,free);
	}else if(nodoAnterior==NULL && nodoPosterior!=NULL){
		nuevoNodo->comienzo=procesoRemovido->comienzo;
		nuevoNodo->paginas=nodoPosterior->paginas+procesoRemovido->paginas;
		list_remove_and_destroy_by_condition(espacioLibre, find_by_ConditionFinal,free);
	}else {
		nuevoNodo->comienzo=procesoRemovido->comienzo;
		nuevoNodo->paginas=procesoRemovido->paginas;
	}
	sem_wait(&sem_mutex_libre);
	list_add(espacioLibre,nuevoNodo);
	sem_post(&sem_mutex_libre);
	log_info(loggerInfo, ANSI_COLOR_GREEN"Proceso mProc liberado: PID: %d byte inicial: %d tamaño: %d"ANSI_COLOR_RESET,procesoRemovido->PID,
				(procesoRemovido->comienzo)*(arch->tamanio_pagina),(procesoRemovido->paginas)*(arch->tamanio_pagina));

	bool findById(t_metrica* unaMetrica)
		{
			return unaMetrica->PID==procesoRemovido->PID;
		}
	t_metrica* metrica=list_remove_by_condition(metricas, findById);
	log_info(loggerInfo, ANSI_COLOR_GREEN "Proceso mProc: %d tuvo %d lecturas y %d escrituras"ANSI_COLOR_RESET, procesoRemovido->PID, metrica->lecturas, metrica->escrituras);
	free(procesoRemovido);
	free(metrica);
	return RESULTADO_OK;
}

FILE* abrirArchivoConTPagina(t_pagina* pagina_pedida){

	bool find_by_PID(NodoOcupado* nodo)
		{
			return nodo->PID==pagina_pedida->PID;
		}
	NodoOcupado* nodoProceso=list_find(espacioOcupado, find_by_PID);


	FILE* espacioDeDatos = fopen(arch->nombre_swap,"r+");
	if (espacioDeDatos==NULL) {
		log_error(loggerError, ANSI_COLOR_RED "No se puede abrir el archivo de Swap para escribir"ANSI_COLOR_RESET);
		return NULL;
	}
	int32_t bloque= nodoProceso->comienzo+pagina_pedida->nro_pagina;
	if(fseek(espacioDeDatos, bloque*arch->tamanio_pagina,SEEK_SET)!=0){
		log_error(loggerError, ANSI_COLOR_RED "No se puede ubicar la pagina a escribir"ANSI_COLOR_RESET);
		return NULL;
	}
	return espacioDeDatos;
}

void leer_pagina(t_pagina* pagina_pedida){
	log_debug(loggerDebug, "leer pid:%d, pag:%d", pagina_pedida->PID, pagina_pedida->nro_pagina);
	char* texto=malloc(arch->tamanio_pagina);
	FILE* espacioDeDatos=abrirArchivoConTPagina(pagina_pedida);
	log_debug(loggerDebug, "Se abre el espacio de datos");
	int32_t posicion=ftell(espacioDeDatos);
	log_debug(loggerDebug, "La posicion es:%d", posicion);
	pagina_pedida->contenido=malloc(arch->tamanio_pagina);
	int32_t resultado=fread(pagina_pedida->contenido, sizeof(char),arch->tamanio_pagina, espacioDeDatos);
	log_debug(loggerDebug, "Se lee el espacio de datos");
	fclose(espacioDeDatos);
	if (resultado==arch->tamanio_pagina){
		pagina_pedida->tamanio_contenido=strlen(pagina_pedida->contenido);
		log_info(loggerInfo, ANSI_COLOR_GREEN"Lectura: PID: %d byte inicial: %d tamaño: %d contenido: %s"ANSI_COLOR_RESET,pagina_pedida->PID,
				posicion, pagina_pedida->tamanio_contenido,pagina_pedida->contenido);
		agregarMetrica(LEER_PAGINA, pagina_pedida->PID);
		return ;
	}
	return ;

}

void agregarMetrica(int32_t codigo, int32_t PID){

	bool findById(t_metrica* unaMetrica)
	{
		return unaMetrica->PID==PID;
	}
	t_metrica* metrica=list_find(metricas, findById);
	if(codigo==LEER_PAGINA) (metrica->lecturas)++;
	if(codigo==ESCRIBIR_PAGINA) (metrica->escrituras)++;

}
int32_t escribir_pagina(t_pagina* pagina_pedida){

	FILE* espacioDeDatos=abrirArchivoConTPagina(pagina_pedida);
	int32_t posicion = ftell(espacioDeDatos);
	log_debug(loggerDebug, "Lo posiciono en:%d", posicion);
	int32_t resultado=fwrite(pagina_pedida->contenido, 1, strlen(pagina_pedida->contenido), espacioDeDatos);
	log_debug(loggerDebug, "Escribo:%d bytes", resultado);

	int i;
	int32_t resultado2=0;
	for(i=0;i<((arch->tamanio_pagina)-(strlen(pagina_pedida->contenido)));i++){
		resultado2+=fwrite("\0", 1,1, espacioDeDatos);
	}
	fclose(espacioDeDatos);
	log_debug(loggerDebug, "Cierro archivo");
	if (resultado+resultado2==arch->tamanio_pagina){
		log_info(loggerInfo, ANSI_COLOR_GREEN"Escritura: PID: %d byte inicial: %d tamaño: %d contenido: %s"ANSI_COLOR_RESET,pagina_pedida->PID,
				posicion, strlen(pagina_pedida->contenido),pagina_pedida->contenido);
		agregarMetrica(ESCRIBIR_PAGINA, pagina_pedida->PID);
		return RESULTADO_OK;
	}

	return RESULTADO_ERROR;
}






