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
    config->retardo= config_get_int_value(archConfig, "RETARDO_COMPACTACION");
    return config;
}

/* Función que es llamada cuando ctrl+c */
void ifProcessDie(){
		log_error(loggerError, "Se da por finalizado el proceso Swap");
	exit(1);
}

/*Función donde se inicializan los semaforos */

void inicializoSemaforos(){

	int32_t semMutexLibre = sem_init(&sem_mutex_libre,0,1);
	if(semMutexLibre==-1)log_error(loggerError,"No pudo crearse el semaforo Mutex del espacio Libre");

	int32_t semMutexOcupado = sem_init(&sem_mutex_ocupado,0,1);
	if(semMutexOcupado==-1)log_error(loggerError,"No pudo crearse el semaforo Mutex del espacio Ocupado");

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

	char* comando=string_new();
	string_append_with_format(&comando, "dd if=/dev/zero of=%s bs=%d count=%d", arch->nombre_swap, arch->tamanio_pagina, arch->cantidad_paginas);

	system(comando);
	free(comando);

	NodoLibre* nodo= malloc(sizeof(NodoLibre));
	nodo->comienzo=0;
	nodo->paginas=arch->cantidad_paginas;

	sem_wait(&sem_mutex_libre);
	list_add(espacioLibre, nodo);
	sem_post(&sem_mutex_libre);

	totalPaginas=arch->cantidad_paginas;

}

int32_t compactar(){

	if (list_is_empty(espacioOcupado)){
		log_error(loggerError, "No se puede compactar una memoria vacía");
		return -1;
	}
	if(list_is_empty(espacioLibre)){
		log_error(loggerError, "No se puede compactar una memoria completamente llena");
		return -1;
	}
	NodoOcupado* primerNodo=list_get(espacioOcupado, 0);
	if(primerNodo->comienzo!=0){
		primerNodo->comienzo=0;
		list_replace_and_destroy_element(espacioOcupado, 0, primerNodo, free);
		//cambiar en el archivo con leer y escribir TODO
	}

	int32_t i;

	for(i=1; i<list_size(espacioOcupado); i++){
		NodoOcupado* nodo= list_get(espacioOcupado, i);
		NodoOcupado* anterior= list_get(espacioOcupado, i-1);
		bool chequeo=(nodo->comienzo==anterior->comienzo+anterior->paginas);
		if(!chequeo){
			nodo->comienzo=anterior->comienzo+anterior->paginas;
			list_replace_and_destroy_element(espacioOcupado, i, nodo, free);
			//TODO cambiar en el archivo swap con leer y escribir
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
	list_clean_and_destroy_elements(espacioLibre, free);
	list_add(espacioLibre, nodoLibre);
}
	graficoCompactar();
	return 1;
}

int32_t calcularEspacioLibre(){
	int32_t espacio;
	int i;
	for(i=0; i<list_size(espacioLibre); i++){
		NodoLibre* nodo= list_get(espacioLibre, i);
		espacio+=nodo->paginas;
	}
	return espacio;
}

void graficoCompactar(){
	printf("Procesando, por favor espere......\n");
	sleep(arch->retardo/3);
	printf("░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
		   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░\n"
		   "░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
		   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n");
	printf("\n... Creando estructuras necesarias para la compactación.....\n");
	sleep(arch->retardo/3);
printf("▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
	   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
	   "░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
	   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n");
sleep(arch->retardo/3);
printf("\n... Guardando estructuras necesarias para la compactación\n");

printf("▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
	   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n"
	   "▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n"
	   "░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n");

printf("Swap compactado\n");
}

int main(void) {

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

	socketServidor = create_server_socket(arch->puerto_escucha);

	int32_t resultado = listen_connections(socketServidor);

	if(resultado == ERROR_OPERATION) {
		log_error(loggerError, "Error al escuchar nuevas conexiones");
		exit(EXIT_FAILURE);
	}

	sock_t* socketCliente = accept_connection(socketServidor);

	recibir_operaciones_memoria(socketCliente);

	//Todo ver que sigue

	return EXIT_SUCCESS;
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

			char* serializado= serializarTexto(leer_pagina(pagina_pedida));
			//enviar serializado
			break;
		}

		case ESCRIBIR_PAGINA: {
			pedido_serializado = malloc(get_message_size(header));
			recibido = _receive_bytes(socketMemoria, pedido_serializado, get_message_size(header));
			if(recibido == ERROR_OPERATION) return;
			t_pagina* pagina_pedida = deserializar_pedido(pedido_serializado);
			int32_t resultado= escribir_pagina(pagina_pedida);

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
			int32_t operacionValida= reservarEspacio(pedido_memoria);

			if (operacionValida == RESULTADO_ERROR) {
				header_t* headerMemoria = _create_header(RESULTADO_ERROR, 0);
				int32_t enviado = _send_header(socketMemoria, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				free(headerMemoria);
				return;
			} else if (operacionValida == RESULTADO_OK) {
				header_t* headerMemoria = _create_header(RESULTADO_OK, 0);
				int32_t enviado = _send_header(socketMemoria, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				free(headerMemoria);
			}
			break;
		}

		case BORRAR_ESPACIO: {
			int32_t PID;
			recibido = _receive_bytes(socketMemoria, &(PID), sizeof(int32_t));
			if (recibido == ERROR_OPERATION)return;
			int32_t operacionValida= borrarEspacio(PID);

			if (operacionValida == RESULTADO_ERROR) {
				header_t* headerMemoria = _create_header(RESULTADO_ERROR, 0);
				int32_t enviado = _send_header(socketMemoria, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				free(headerMemoria);
				return;
			} else if (operacionValida == RESULTADO_OK) {
				header_t* headerMemoria = _create_header(RESULTADO_OK, 0);
				int32_t enviado = _send_header(socketMemoria, headerMemoria);
				if(enviado == ERROR_OPERATION) return;
				free(headerMemoria);
			}
			break;
		}

		default: {
				log_error(loggerError, "Se recibió un codigo de operación no valido");
		}

		}

	}
}

int32_t reservarEspacio(t_pedido_memoria* pedido_pid){
	//todo falta saber si es bestfit-firstfit-worstfit o solo asignación contigua
	int32_t libre=calcularEspacioLibre();
	if(libre <= pedido_pid->cantidad_paginas)return RESULTADO_ERROR;
	//asignarBF
	//asignarFF
	//asignarWF
	return RESULTADO_OK;
}

int32_t borrarEspacio(int32_t PID){

	bool find_by_PID(NodoOcupado* nodo)
			{
				return nodo->PID==PID;
			}

	NodoOcupado* procesoRemovido = list_remove_by_condition(espacioLibre, find_by_PID);

	bool find_by_ConditionInitial(NodoLibre* espacioAnterior)
				{
					return espacioAnterior->comienzo+espacioAnterior->paginas==procesoRemovido->comienzo;
				}

	NodoLibre* nodoAnterior=list_find(espacioLibre, find_by_ConditionInitial);

	bool find_by_ConditionFinal(NodoLibre* espacioPosterior)
					{
						return espacioPosterior->comienzo==procesoRemovido->comienzo+procesoRemovido->comienzo;
					}

	NodoLibre* nodoPosterior=list_find(espacioLibre, find_by_ConditionFinal);
	NodoLibre* nuevoNodo=malloc(sizeof(NodoLibre));

	if(nodoAnterior!=NULL && nodoPosterior!=NULL){
		nuevoNodo->comienzo=nodoAnterior->comienzo;
		nuevoNodo->paginas=nodoAnterior->paginas+procesoRemovido->paginas+nodoPosterior->paginas;
	}else if(nodoAnterior!=NULL && nodoPosterior==NULL){
		nuevoNodo->comienzo=nodoAnterior->comienzo;
		nuevoNodo->paginas=nodoAnterior->paginas+procesoRemovido->paginas;
	}else if(nodoAnterior==NULL && nodoPosterior!=NULL){
		nuevoNodo->comienzo=procesoRemovido->comienzo;
		nuevoNodo->paginas=nodoPosterior->paginas+procesoRemovido->paginas;
	}else {
		nuevoNodo->comienzo=procesoRemovido->comienzo;
		nuevoNodo->paginas=procesoRemovido->paginas;
	}
	free(procesoRemovido);
	return RESULTADO_OK;
}


char* leer_pagina(t_pagina* pagina_pedida){

	char* texto;

	return texto;
	//Todo
}

int32_t escribir_pagina(t_pagina* pagina_pedida){
	bool find_by_PID(NodoOcupado* nodo)
	{
		return nodo->PID==pagina_pedida->PID;
	}
	NodoOcupado* nodoProceso=list_find(espacioOcupado, find_by_PID);
	//ver la necesidad de completarlo con ceros TODO
	FILE* espacioDeDatos = fopen(arch->nombre_swap,"wr+");
	if (espacioDeDatos==NULL) {
		log_error(loggerError, "No se puede abrir el archivo de Swap para escribir");
		return ERROR_OPERATION;
	}
	int32_t bloque= nodoProceso->comienzo+pagina_pedida->nro_pagina;
	if(fseek(espacioDeDatos, bloque*arch->tamanio_pagina,SEEK_SET)!=0){
		log_error(loggerError, "No se puede ubicar la pagina a escribir");
		return ERROR_OPERATION;
	}
	//todo ver fwrite y que devuelve
	int32_t resultado=fwrite(pagina_pedida->contenido, strlen(pagina_pedida->contenido), 1, espacioDeDatos);
	if (resultado<0) return ERROR_OPERATION;
	if (resultado==0) return SUCCESS_OPERATION;

	return 0;
}


