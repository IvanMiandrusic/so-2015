/*Source file */

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

void crear_estructura_config(char* path) {
	t_config* archConfig = config_create(path);
	arch = malloc(sizeof(ProcesoMemoria));
	arch->puerto_escucha = config_get_int_value(archConfig, "PUERTO_ESCUCHA");
	arch->ip_swap = config_get_string_value(archConfig, "IP_SWAP");
	arch->puerto_swap = config_get_int_value(archConfig, "PUERTO_SWAP");
	arch->maximo_marcos = config_get_int_value(archConfig, "MAXIMO_MARCOS_POR_PROCESO");
	arch->cantidad_marcos = config_get_int_value(archConfig, "CANTIDAD_MARCOS");
	arch->tamanio_marco = config_get_int_value(archConfig, "TAMANIO_MARCO");
	arch->entradas_tlb = config_get_int_value(archConfig, "ENTRADAS_TLB");
	arch->tlb_habilitada = config_get_string_value(archConfig, "TLB_HABILITADA");
	arch->retardo = config_get_int_value(archConfig, "RETARDO_MEMORIA");
	arch->algoritmo_reemplazo = config_get_string_value(archConfig, "ALGORITMO_REEMPLAZO");
}

/** Funcion que es llamada cuando pierdo la conexion con el swap **/
void swap_shutdown() {
	log_error(loggerError, ANSI_COLOR_BOLDRED "Se perdio la conexion con el proceso SWAP" ANSI_COLOR_RESET);
	ifProcessDie();
}

/* Función que es llamada cuando ctrl+c */
void ifProcessDie() {
	log_info(loggerInfo, ANSI_COLOR_BOLDBLUE "Se dara de baja el proceso MEMORIA"ANSI_COLOR_RESET);
	limpiar_estructuras_memoria();
	exit(EXIT_FAILURE);
}

/** Señales **/

void ifSigusr1() {
	log_info(loggerInfo, ANSI_COLOR_BLUE "Sigusr1: Señal de TLB Flush recibida"ANSI_COLOR_RESET);
	TLB_flush();
	log_info(loggerInfo, ANSI_COLOR_BLUE "TLB Flush realizado"ANSI_COLOR_RESET);

	/** Mostrar entradas TLB dsps de flush **/

	void mostrar_entradas(void* parametro) {
		TLB* entrada = (TLB*) parametro;
		log_debug(loggerDebug, "Entrada: pid %d, pagina %d, marco %d, presente %d, "
				"modificada %d, tiempoReferecencia %d", entrada->PID, entrada->pagina, entrada->marco,
				entrada->presente, entrada->modificada, entrada->tiempo_referencia);
	}

	sem_wait(&sem_mutex_tlb);
	list_iterate(TLB_tabla, mostrar_entradas);
	sem_post(&sem_mutex_tlb);
}


void ifSigusr2() {
	log_info(loggerInfo, ANSI_COLOR_BLUE "Sigurs2: Señal de limpieza de memoria recibida"ANSI_COLOR_RESET);
	limpiar_MP();
	log_info(loggerInfo, ANSI_COLOR_BLUE "Limpieza de memoria realizada"ANSI_COLOR_RESET);
}

void ifSigpoll() {
	log_info(loggerInfo, ANSI_COLOR_BLUE "Sigpoll: Señal de Dump recibida"ANSI_COLOR_RESET);
	dump();
	log_info(loggerInfo, ANSI_COLOR_BLUE "Dump Realizado"ANSI_COLOR_RESET);
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
			free(texto);
		}
		abort();
	}
	log_info(loggerInfo, ANSI_COLOR_GREEN "Dump realizado con exito" ANSI_COLOR_RESET);
}

void limpiar_MP() {

	void clean_by_PID(void* parametro) {
		t_paginas_proceso* paginas_PID = (t_paginas_proceso*) parametro;

		void clean_from_MP(void* parametro) {

			TPagina* entrada = (TPagina*) parametro;

			/** Sacarle la presencia a la entrada de pagina**/
			entrada->presente = 0;

			/** Coloco el marco como vacio de la entrada de pagina **/
			frames[entrada->marco] = 0;

			/** Si esta modificada, escribir en swap **/
			if(entrada->modificada == 1)
				escribir_pagina_modificada_en_swap(paginas_PID->PID, entrada);
			entrada->bitUso=0;
			entrada->marco = 0;
			entrada->modificada = 0;
			entrada->tiempo_referencia = 0;
		}

		list_iterate(paginas_PID->paginas, clean_from_MP);
	}

	sem_wait(&sem_mutex_tabla_paginas);
	list_iterate(tabla_Paginas, clean_by_PID);
	sem_post(&sem_mutex_tabla_paginas);
	log_info(loggerInfo, ANSI_COLOR_BOLDGREEN "Memoria limpiada con exito" ANSI_COLOR_RESET);

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
	metricas_create();

}

void limpiar_estructuras_memoria(){

	TLB_destroy();
	tabla_paginas_destroy();
	MP_destroy();
	frames_destroy();
	metricas_destroy();
	sem_destroy(&sem_mutex_tlb);
	sem_destroy(&sem_mutex_tabla_paginas);
	log_destroy(loggerDebug);
	log_destroy(loggerError);
	log_destroy(loggerInfo);
	clean_socket(socketServidorCpus);
	clean_socket(socketSwap);
	free(arch);

}


/** MAIN **/
int main(int argc, char** argv) {

	if(argc!=2) {
		printf(ANSI_COLOR_BOLDRED "Cantidad erronea de parametros. Este proceso recibe un parametro\n" ANSI_COLOR_RESET);
		return EXIT_FAILURE;
	}

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

	/** Chequeamos si la conexion se rompe del lado swap **/
	if (signal(SIGPIPE, swap_shutdown) == SIG_ERR) log_error(loggerError, ANSI_COLOR_RED "Error con la señal SIGPIPE" ANSI_COLOR_RESET);

	/*Se genera el struct con los datos del archivo de config.- */
	char* path = argv[1];
	crear_estructura_config(path);

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

void thread_request(void* arg) {

	t_atencion_pedido* pedido = (t_atencion_pedido*) arg;
	sock_t* socketCpu = _create_socket_from_fd(pedido->cpu_fd);
	header_t* headerCpu = _create_header(pedido->operacion, pedido->tam_msj);
	switch (get_operation_code(headerCpu)) {
	case INICIAR: {
		iniciar_proceso(socketCpu);
		break;
	}
	case LEER: {
		readOrWrite(LEER, socketCpu, headerCpu);
		break;
	}
	case ESCRIBIR: {
		readOrWrite(ESCRIBIR, socketCpu, headerCpu);
		break;
	}
	case FINALIZAR: {
		finalizarPid(socketCpu);
		break;
	}
	default: {
		log_error(loggerError, ANSI_COLOR_RED "Desde la cpu recibo un codigo de operacion erroneo" ANSI_COLOR_RESET);
		break;
	}
	}
	free(headerCpu);

}

void procesar_pedido(sock_t* socketCpu, header_t* header) {

	t_atencion_pedido* pedido = malloc(sizeof(t_atencion_pedido));
	pedido->cpu_fd = socketCpu->fd;
	pedido->operacion = header->cod_op;
	pedido->tam_msj = header->size_message;
	thread_request(pedido);


}


/*Funciones referentes a finalizar proceso */
void finalizarPid(sock_t* socketCpu){

	int32_t PID;

	int32_t recibido = _receive_bytes(socketCpu, &(PID), sizeof(int32_t));
	if (recibido == ERROR_OPERATION || recibido == SOCKET_CLOSED) {
		enviar_resultado_cpu(ERROR, socketCpu);
		if(recibido == SOCKET_CLOSED) swap_shutdown();
	}

	t_list* paginasConPresencia=obtengoPaginasConPresencia(obtener_tabla_paginas_by_PID(PID));
	int asd;
	for (asd=0;asd<list_size(paginasConPresencia);asd++){
		TPagina* pagina=list_get(paginasConPresencia, asd);
		printf(ANSI_COLOR_BOLDBLUE"\n\nNro: %d, marco:%d, tiempo:%d, bitUso:%d, modificada:%d \n\n"ANSI_COLOR_RESET, pagina->pagina, pagina->marco, pagina->tiempo_referencia,pagina->bitUso,pagina->modificada);
	}

	dump();

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
	if (recibido == ERROR_OPERATION || recibido == SOCKET_CLOSED) {
		enviar_resultado_cpu(ERROR, socketCpu);
		if(recibido == SOCKET_CLOSED) swap_shutdown();
	}

	if (resultado_operacion == RESULTADO_ERROR) {

		log_debug(loggerDebug, "El swap informa que no pudo eliminar el pid:%d", PID);

		header_t* headerMemoria = _create_header(ERROR, 0);
		int32_t enviado = _send_header(socketCpu, headerMemoria);
		if(enviado == ERROR_OPERATION) return;
		free(headerMemoria);

	} else if (resultado_operacion == RESULTADO_OK) {

		log_debug(loggerDebug, "El swap informa que pudo eliminar el pid:%d", PID);
		limpiar_Informacion_PID(PID);
		header_t* headerMemoria = _create_header(OK, 0);
		int32_t enviado = _send_header(socketCpu, headerMemoria);
		if(enviado == ERROR_OPERATION) return;
		free(headerMemoria);
		log_info(loggerInfo, "Se ha borrado de memoria el proceso: %d op:%d", PID, OK);

	}

}

int32_t limpiar_Informacion_PID(int32_t PID){

	/** Logeo metricas **/
	t_metricas* metrica_PID = obtener_metrica_PID(PID);
	log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "mProc %d - cantidad de fallos de pagina %d - cantidad de paginas accedidas %d" ANSI_COLOR_RESET,
			PID, metrica_PID->page_fault, metrica_PID->accesos_memoria);

	if(TLB_habilitada())TLB_clean_by_PID(PID);

	return tabla_paginas_clean(PID);

}

/*Aca finalizan las funciones referentes a finalizar proceso*/

/*Funciones referentes a iniciar proceso */

void iniciar_proceso(sock_t* socketCpu) {

	int32_t recibido;
	int32_t enviado;
	int32_t resultado_operacion;
	t_pedido_cpu* pedido_cpu = malloc(sizeof(t_pedido_cpu));

	recibido = _receive_bytes(socketCpu, &(pedido_cpu->pid), sizeof(int32_t));
	if (recibido == ERROR_OPERATION || recibido == SOCKET_CLOSED) {
		enviar_resultado_cpu(ERROR, socketCpu);
		if(recibido == SOCKET_CLOSED) swap_shutdown();
	}

	recibido = _receive_bytes(socketCpu, &(pedido_cpu->cantidad_paginas), sizeof(int32_t));
	if (recibido == ERROR_OPERATION || recibido == SOCKET_CLOSED) {
		enviar_resultado_cpu(ERROR, socketCpu);
		if(recibido == SOCKET_CLOSED) swap_shutdown();
	}

	log_debug(loggerDebug, "Recibi el pcb %d, con %d paginas" ,pedido_cpu->pid, pedido_cpu->cantidad_paginas);

	//Envio al swap para que reserve espacio
	header_t* headerSwap = _create_header(RESERVAR_ESPACIO,	2 * sizeof(int32_t));
	enviado = _send_header(socketSwap, headerSwap);
	if (enviado == ERROR_OPERATION) return;
	free(headerSwap);

	enviado = _send_bytes(socketSwap, &(pedido_cpu->pid), sizeof(int32_t));
	if (enviado == ERROR_OPERATION) return;

	enviado = _send_bytes(socketSwap, &(pedido_cpu->cantidad_paginas), sizeof(int32_t));
	if (enviado == ERROR_OPERATION) return;

	header_t* headerNuevo=_create_empty_header();
	recibido=_receive_header(socketSwap,headerNuevo);

	resultado_operacion=get_operation_code(headerNuevo);
	free(headerNuevo);

	if (resultado_operacion == RESULTADO_ERROR || recibido == ERROR_OPERATION || recibido == SOCKET_CLOSED) {

		enviar_resultado_cpu(ERROR, socketCpu);
		//log_debug(loggerDebug,ANSI_COLOR_RED"El swap informa que no se pudo asignar" ANSI_COLOR_RESET);
		log_error(loggerError, "Error de creacion del mProc: %d", pedido_cpu->pid);
		if(recibido == SOCKET_CLOSED) swap_shutdown();

	} else if (resultado_operacion == RESULTADO_OK) {

		//Creo la tabla de paginas del PID dado
		//log_debug(loggerDebug,"El swap informa que se pudo asignar");
		crear_tabla_pagina_PID(pedido_cpu->pid, pedido_cpu->cantidad_paginas);
		header_t* headerCpu = _create_header(OK, 0);
		enviado = _send_header(socketCpu, headerCpu);
		free(headerCpu);
		log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "mProc: %d creado, con %d paginas" ANSI_COLOR_RESET, pedido_cpu->pid, pedido_cpu->cantidad_paginas);
	}

	free(pedido_cpu);
}
/*Aca finalizan las funciones referentes a iniciar proceso*/

/*Funciones referentes a leer o escribir pagina*/

void readOrWrite(int32_t cod_Operacion, sock_t* socketCpu, header_t* header){

	char* pedido_serializado = malloc(get_message_size(header));

	int32_t recibido = _receive_bytes(socketCpu, pedido_serializado, get_message_size(header));
	if(recibido == ERROR_OPERATION || recibido == SOCKET_CLOSED) {
		enviar_resultado_cpu(ERROR, socketCpu);
		if(recibido == SOCKET_CLOSED) swap_shutdown();
	}

	t_pagina* pagina_pedida = deserializar_pedido(pedido_serializado);
	if(cod_Operacion==LEER)log_info(loggerInfo, "Debo leer la pagina:%d, del proceso: %d", pagina_pedida->nro_pagina, pagina_pedida->PID);
	if(cod_Operacion==ESCRIBIR)log_info(loggerInfo, "Se tiene que escribir del proceso:%d la pagina:%d con:%s", pagina_pedida->PID, pagina_pedida->nro_pagina, pagina_pedida->contenido);
	t_resultado_busqueda resultado=buscar_pagina(cod_Operacion, pagina_pedida);
	free(pedido_serializado);
	int32_t enviado;
	if (resultado==NOT_FOUND) {

		/** Finalizo el proceso con ERROR **/
		finalizar_proceso_error(socketCpu, pagina_pedida->PID);

		/** Informo a la CPU que el proceso tuvo un ERROR **/
		enviar_resultado_cpu(ERROR, socketCpu);

		free(pagina_pedida);
		if(cod_Operacion==LEER) log_debug(loggerDebug,ANSI_COLOR_RED "No se pudo leer la pagina" ANSI_COLOR_RESET);
		if(cod_Operacion==ESCRIBIR) log_debug(loggerDebug,ANSI_COLOR_RED "No se pudo escribir la pagina" ANSI_COLOR_RESET);

	}else {
		if(cod_Operacion==LEER){
			header_t* headerCpu = _create_header(OK, sizeof(int32_t)+pagina_pedida->tamanio_contenido);
			enviado = _send_header(socketCpu, headerCpu);
			if(enviado == ERROR_OPERATION) return;
			int32_t tamanio=pagina_pedida->tamanio_contenido;
			enviado = _send_bytes(socketCpu, &tamanio, sizeof(int32_t));
			if(enviado == ERROR_OPERATION) return;
			enviado = _send_bytes(socketCpu, pagina_pedida->contenido, tamanio);
			if(enviado == ERROR_OPERATION) return;
			free(headerCpu);
			free(pagina_pedida->contenido);
			free(pagina_pedida);
			log_debug(loggerDebug,"Se leyo la pagina correctamente");
		}else{
			header_t* headerCpu = _create_header(OK, 0);
			enviado = _send_header(socketCpu, headerCpu);
			if(enviado == ERROR_OPERATION) return;
			free(headerCpu);
			free(pagina_pedida->contenido);
			free(pagina_pedida);
			log_debug(loggerDebug, "Se escribio la pagina correctamente");
		}
	}

}

t_resultado_busqueda buscar_pagina(int32_t codOperacion, t_pagina* pagina_solicitada) {

	/** Las paginas tienen el mismo tamaño del marco (como maximo) **/
	if(codOperacion==LEER)pagina_solicitada->contenido = malloc(arch->tamanio_marco);

	/** Realizo el retardo antes de buscar en estructuras administrativas **/
	sleep(arch->retardo);

	return TLB_buscar_pagina(codOperacion, pagina_solicitada);

}

void finalizar_proceso_error(sock_t* socketCpu, int32_t PID) {

	int32_t recibido;

	header_t* headerSwap = _create_header(BORRAR_ESPACIO, 1 * sizeof(int32_t));
	int32_t enviado = _send_header(socketSwap, headerSwap);
	if (enviado == ERROR_OPERATION) return;

	free(headerSwap);

	enviado = _send_bytes(socketSwap, &(PID), sizeof(int32_t));
	if (enviado == ERROR_OPERATION)	return;

	//log_debug(loggerDebug, "Envie al swap para finalizar el proceso:%d", PID);

	header_t* headerNuevo=_create_empty_header();
	recibido = _receive_header(socketSwap,headerNuevo);
	if(recibido == ERROR_OPERATION || recibido == SOCKET_CLOSED) {
		enviar_resultado_cpu(ERROR, socketCpu);
		if(recibido == SOCKET_CLOSED) swap_shutdown();
	}
	int32_t resultado_operacion = get_operation_code(headerNuevo);
	if (resultado_operacion == RESULTADO_ERROR) return;

	free(headerNuevo);
	/** Libero el espacio ocupado en memoria por el pid finalizado **/
	limpiar_Informacion_PID(PID);

}

/*Aca finalizan las funciones referentes a leer o escribir pagina*/


void enviar_resultado_cpu(int32_t resultado, sock_t* socket_cpu) {

	header_t* headerCpu = _create_header(resultado, 0);
	int32_t enviado = _send_header(socket_cpu, headerCpu);
	if(enviado == ERROR_OPERATION) return;

	free(headerCpu);
}
