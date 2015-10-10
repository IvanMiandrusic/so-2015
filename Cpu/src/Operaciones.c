#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include "Cpu.h"
#include "Operaciones.h"

extern ProcesoCPU* arch;
extern t_log* loggerInfo;
extern t_log* loggerError;
extern t_log* loggerDebug;
extern t_list* socketsCPU;
extern int32_t* tiempoInicial;
extern int32_t* tiempoFinal;
extern int32_t* tiempoAcumulado;
extern int32_t* estado;

#define TRUE 1
#define FALSE 0
int32_t quantum;

void* thread_Use(void* thread_id){
	int32_t id = (void*)thread_id;
	tiempoAcumulado[id]=0;
	tiempoInicial[id]=0;
	tiempoFinal[id]=0;
	int32_t toAnterior;
	int32_t tfAnterior;
	int32_t porcentaje;
	sock_t* socketPlanificador=getSocketPlanificador(id);
	int32_t finalizar=1;
	while(finalizar){finalizar=obtengoSegundos();}
	while(TRUE){
		if(estado[id]==1){					//si esta ejecutando
			tiempoAcumulado[id]+=60-tiempoInicial[id];
		}

		porcentaje=tiempoAcumulado[id]*10/6;
		if(tiempoFinal[id]==tfAnterior && tiempoInicial[id]==toAnterior){
			if(estado[id]==1) {				//si en un minuto no cambio y esta ejecutando
				porcentaje=100;
				tiempoAcumulado[id]=60;
			}
			if(estado[id]==0) {				//si en un minuto no cambio y no esta ejecutando
				porcentaje=0;
				tiempoAcumulado[id]=0;
			}
		}
		log_debug(loggerDebug,ANSI_COLOR_BOLDGREEN"CPU:%d El porcentaje de uso es: %d%%(%d/60)" ANSI_COLOR_RESET,id, porcentaje, tiempoAcumulado[id]);
		tiempoAcumulado[id]=0;
		header_t* header_uso_cpu = _create_header(UTILIZACION_CPU, 2*sizeof(int32_t));
		int32_t enviado = _send_header (socketPlanificador, header_uso_cpu);
		if(enviado == ERROR_OPERATION) exit(1);
		enviado = _send_bytes(socketPlanificador,&id,sizeof(int32_t));
		if(enviado == ERROR_OPERATION) exit(1);
		enviado = _send_bytes(socketPlanificador,&porcentaje,sizeof(int32_t));
		if(enviado == ERROR_OPERATION) exit(1);
		free(header_uso_cpu);
		toAnterior=tiempoInicial[id];
		tfAnterior=tiempoFinal[id];
		sleep(60);
	}
	return NULL;
}

void* thread_Cpu(void* id){

	int32_t thread_id = (void*) id;
	sem_wait(&sem_mutex);
	sock_t* socket_Planificador=create_client_socket(arch->ip_planificador,arch->puerto_planificador);
	int32_t resultado = connect_to_server(socket_Planificador);
	sem_post(&sem_mutex);
	if(resultado == ERROR_OPERATION) {
		log_error(loggerError, ANSI_COLOR_RED "CPU: %d - Error al conectar al planificador" ANSI_COLOR_RESET, thread_id);
		return NULL;
	}

	//enviarle al planificador NUEVA_CPU y su id;
	header_t* header_nueva_cpu = _create_header(NUEVA_CPU, sizeof(int32_t));
	int32_t enviado =_send_header (socket_Planificador, header_nueva_cpu);

	if (enviado !=SUCCESS_OPERATION)
	{
		log_error(loggerError, ANSI_COLOR_RED "Se perdio la conexion con el Planificador de la cpu: %d" ANSI_COLOR_RESET, thread_id);
		return NULL;
	}
	enviado = _send_bytes(socket_Planificador, &thread_id,sizeof(int32_t));
	if (enviado !=SUCCESS_OPERATION)
	{
		log_error(loggerError, ANSI_COLOR_RED "CPU: %d - Se perdio la conexion con el Planificador" ANSI_COLOR_RESET, thread_id);
		return NULL;
	}
	log_debug(loggerDebug, "Conectado con el planificador la cpu: %d", thread_id);
	free(header_nueva_cpu);

	sock_t* socket_Memoria=create_client_socket(arch->ip_memoria,arch->puerto_memoria);
	if(connect_to_server(socket_Memoria)!=SUCCESS_OPERATION){
		log_error(loggerError, ANSI_COLOR_RED "CPU: %d - No se puedo conectar con la memoria, se aborta el proceso" ANSI_COLOR_RESET, thread_id);
		return NULL;
	}
	log_debug(loggerDebug, "Conectado con la memoria cpu:%d", thread_id);

	pthread_t CPUuse;
	int32_t resultado_uso = pthread_create(&CPUuse, NULL, thread_Use, (void*) thread_id );
	if (resultado_uso != 0) {
		log_error(loggerError, ANSI_COLOR_RED "Error al crear el hilo de uso de CPU número:%d"ANSI_COLOR_RESET, id);
		return NULL;
	}

	/*Me guardo en mi list de sockets los fd*/
	t_sockets* sockets=list_get(socketsCPU, thread_id);
	sockets->socketPlanificador->fd = socket_Planificador->fd;
	sockets->socketMemoria->fd = socket_Memoria->fd;

	int32_t finalizar = 1;
	int32_t resul_Mensaje_Recibido;
	header_t* header_planificador = _create_empty_header() ;
	while (finalizar == 1)
	{
		resul_Mensaje_Recibido = _receive_header(socket_Planificador, header_planificador);
		if (resul_Mensaje_Recibido !=SUCCESS_OPERATION )
		{
			log_error(loggerError, ANSI_COLOR_RED "CPU_ID:%d - Se perdio la conexion con el Planificador" ANSI_COLOR_RESET, thread_id);
			finalizar = 0;
			break;
		}
		tipo_Cod_Operacion (thread_id,header_planificador);
	}
	log_info(loggerInfo, ANSI_COLOR_BOLDBLUE "CPU_ID:%d->Finaliza sus tareas, hilo concluido" ANSI_COLOR_RESET, thread_id);
	free(header_planificador);
	return NULL;
}

void tipo_Cod_Operacion (int32_t id, header_t* header){

	sock_t* socketPlanificador=getSocketPlanificador(id);
	switch (get_operation_code(header))
	{
	case ENVIO_QUANTUM:{
		int32_t recibido = _receive_bytes(socketPlanificador, &quantum, sizeof(int32_t));
		log_debug(loggerDebug,"CPU:%d recibio de Planificador codOperacion %d QUANTUM: %d",id, get_operation_code(header), quantum);
		if (recibido == ERROR_OPERATION)return;
		break;
	}
	case ENVIO_PCB:{
		log_debug(loggerDebug,"CPU: %d recibio de Planificador codOperacion %d PCB",id, get_operation_code(header));
		char* pedido_serializado = malloc(get_message_size(header));
		int32_t recibido = _receive_bytes(socketPlanificador, pedido_serializado, get_message_size(header));
		if(recibido == ERROR_OPERATION) {
			log_debug(loggerDebug, "Error al recibir el PCB del planificador");
			return;
		}
		log_debug(loggerDebug, "Recibio el PCB correctamente");
		PCB* pcb = deserializarPCB (pedido_serializado);
		free(pedido_serializado);
		int32_t inicial;
		int32_t final;
		inicial=obtengoSegundos();
		tiempoInicial[id]=inicial;
		estado[id]=1;
		ejecutar(id, pcb);
		final=obtengoSegundos();
		tiempoFinal[id]=final;
		if(final>inicial){
			tiempoAcumulado[id]+=(final-inicial);
		}else if(final<inicial){
			tiempoAcumulado[id]+=final;
		}
		estado[id]=0;
		break;
	}

	}
}

void ejecutar(int32_t id, PCB* pcb){
	log_info(loggerInfo, "PCB recibido- Contexto: PID: %d, ruta:%s, PC:%d, Quantum:%d", pcb->PID, pcb->ruta_archivo, pcb->siguienteInstruccion, quantum);
	int32_t ultimoQuantum=0;
	char cadena[100];
	char* log_acciones=string_new();
	FILE* prog = fopen(pcb->ruta_archivo, "r");
	if (prog==NULL)
	{
		string_append_with_format(&log_acciones, "No se pudo encontrar la ruta del archivo del proceso con id: %d", pcb->PID);
		log_error (loggerError, ANSI_COLOR_RED "CPU: %d - Error al abrir la ruta del archivo del proceso:%d"ANSI_COLOR_RESET, id, pcb->PID);
		fclose(prog);
		enviar_Header_ID_Retardo_PCB_Texto (RESULTADO_ERROR,id,pcb,log_acciones,0);
		return;
	}
	int32_t finalizado=FALSE;
	while(finalizado == FALSE){
		fseek(prog, pcb->siguienteInstruccion, SEEK_SET);
		if(fgets(cadena, 100, prog) != NULL)
		{
			t_respuesta* respuesta=analizadorLinea(id,pcb,cadena);
			sleep(arch->retardo);
			string_append(&log_acciones, respuesta->texto);
			pcb->siguienteInstruccion=ftell(prog);
			log_debug(loggerDebug, "Analice y ejecute una linea, la proxima tiene PC en:%d", pcb->siguienteInstruccion);
			if(respuesta->id==FINALIZAR){
				enviar_Header_ID_Retardo_PCB_Texto (RESULTADO_OK,id,pcb,log_acciones,0);
				ultimoQuantum = quantum;
				free(log_acciones);
				free(respuesta);
				fclose(prog);
				return ;
			}
			if(respuesta->id==ENTRADASALIDA){
				enviar_Header_ID_Retardo_PCB_Texto (SOLICITUD_IO,id,pcb,log_acciones,respuesta->retardo);
				free(log_acciones);
				free(respuesta);
				fclose(prog);
				return ;
			}
			if(respuesta->id==ERROR){
				enviar_Header_ID_Retardo_PCB_Texto (RESULTADO_ERROR,id,pcb,log_acciones,0);
				free(log_acciones);
				free(respuesta);
				fclose(prog);
				return ;
			}
		} else finalizado = TRUE;
		if (quantum > 0){
					ultimoQuantum ++;
					if (ultimoQuantum > quantum)finalizado = TRUE;
			}
	}
	log_info(loggerInfo, "Rafaga terminada del PID: %d", pcb->PID);
	free(log_acciones);
	fclose(prog);
	enviar_Header_ID_Retardo_PCB_Texto (TERMINO_RAFAGA,id,pcb,log_acciones,0);
}

void enviar_Header_ID_Retardo_PCB_Texto (int32_t Cod_Operacion,int32_t id,PCB* pcb,char* texto, int32_t retardo){

		sock_t* socketPlanificador=getSocketPlanificador(id);
		int32_t tamanio_texto = string_length(texto);
		int32_t tamanio_pcb = obtener_tamanio_pcb(pcb);
		header_t* header;

		if (Cod_Operacion == SOLICITUD_IO){ //Si es IO hay que enviar el retardo y cambia el tamaño de envio
		header = _create_header(Cod_Operacion, 4* sizeof(int32_t)+ tamanio_pcb+tamanio_texto);
		}else{
		header = _create_header(Cod_Operacion, 3* sizeof(int32_t)+ tamanio_pcb+tamanio_texto);
		}

		int32_t enviado = _send_header(socketPlanificador, header);
		if(enviado == ERROR_OPERATION) return;
		enviado = _send_bytes(socketPlanificador,&id,sizeof(int32_t));
		if(enviado == ERROR_OPERATION) return;

		if (Cod_Operacion == SOLICITUD_IO){
			enviado = _send_bytes(socketPlanificador,&retardo,sizeof(int32_t));
			if(enviado == ERROR_OPERATION) return;
		}

		char* pcb_serializado=serializarPCB(pcb);

		enviado = _send_bytes(socketPlanificador,&tamanio_pcb,sizeof(int32_t));
		if(enviado == ERROR_OPERATION) return;
		enviado = _send_bytes(socketPlanificador,pcb_serializado,tamanio_pcb);
		if(enviado == ERROR_OPERATION) return;
		enviado = _send_bytes(socketPlanificador,&tamanio_texto,sizeof(int32_t));
		if(enviado == ERROR_OPERATION) return;
		enviado = _send_bytes(socketPlanificador,texto,tamanio_texto);
		if(enviado == ERROR_OPERATION) return;
		free(header);
		free(pcb);
}

t_respuesta* mAnsisOp_iniciar(int32_t id, PCB* pcb, int32_t cantDePaginas){

	sock_t* socketMemoria=getSocketMemoria(id);
	log_debug(loggerDebug, "CPU: %d - Se procedera a iniciar el proceso:%d", id,  pcb->PID);

	/** Envio header a la memoria con INICIAR **/
	header_t* header_A_Memoria = _create_header(M_INICIAR, 2 * sizeof(int32_t));

	log_debug(loggerDebug, "Envie el header INICIAR con %d bytes",header_A_Memoria->size_message);

	int32_t enviado = _send_header(socketMemoria, header_A_Memoria);
	if(enviado == ERROR_OPERATION) return NULL;
	enviado = _send_bytes(socketMemoria,&pcb->PID, sizeof (int32_t));
	if(enviado == ERROR_OPERATION) return NULL;
	enviado = _send_bytes(socketMemoria,&cantDePaginas, sizeof (int32_t));
	if(enviado == ERROR_OPERATION) return NULL;
	log_debug(loggerDebug, "Envie el id %d, con %d paginas",pcb->PID, cantDePaginas);
	free(header_A_Memoria);

	/** Recibo header de la memoria con el resultado (OK o ERROR) **/
	header_t* header_de_memoria = _create_empty_header();

	int32_t recibido = _receive_header(socketMemoria, header_de_memoria);
	if(recibido == ERROR_OPERATION) return NULL;
	t_respuesta* response= malloc(sizeof(t_respuesta));
	if(get_operation_code(header_de_memoria) == 2) {
		response->id=ERROR;
		response->texto=string_new();
		response->retardo = 0;
		string_append_with_format(&response->texto, "mProc %d - Fallo ",pcb->PID);
		log_error(loggerError,ANSI_COLOR_RED "CPU: %d - mProc %d - Fallo" ANSI_COLOR_RESET,id, pcb->PID);
	}else {
		response->id=INICIAR;
		response->texto=string_new();
		response->retardo = 0;
		string_append_with_format(&response->texto, "mProc %d - Iniciado ",pcb->PID);
		log_info(loggerInfo,ANSI_COLOR_YELLOW "CPU: %d - mProc %d - Iniciado " ANSI_COLOR_RESET,id, pcb->PID);
	}

	free(header_de_memoria);
	return response;
}

t_respuesta* mAnsisOp_leer(int32_t id,PCB* pcb,int32_t numDePagina){

	sock_t* socketMemoria=getSocketMemoria(id);

	log_debug(loggerDebug, "CPU: %d - Se procedera a leer del proceso:%d, la pagina:%d",id, pcb->PID, numDePagina);

	/** Envio header a la memoria con LEER **/
	header_t* header_A_Memoria = _create_header(M_LEER, 3 * sizeof(int32_t));

	int32_t enviado = _send_header(socketMemoria, header_A_Memoria);
	if(enviado == ERROR_OPERATION) return NULL;
	enviado = _send_bytes(socketMemoria,&pcb->PID, sizeof (int32_t));
	if(enviado == ERROR_OPERATION) return NULL;
	enviado = _send_bytes(socketMemoria,&numDePagina, sizeof (int32_t));
	if(enviado == ERROR_OPERATION) return NULL;
	int32_t tamanio=0;
	enviado = _send_bytes(socketMemoria,&tamanio, sizeof (int32_t));
	if(enviado == ERROR_OPERATION) return NULL;
	log_debug(loggerDebug, "Envie el id %d,el numero de pagina %d, tamanio:%d",pcb->PID, numDePagina, tamanio);
	free(header_A_Memoria);
	/** Recibo header de la memoria con el codigo CONTENIDO_PAGINA o NULL **/
	header_t* header_de_memoria = _create_empty_header();
	int32_t recibido = _receive_header(socketMemoria, header_de_memoria);
	int32_t longPagina;
	int32_t recibi_longPagina = _receive_bytes(socketMemoria, &longPagina, sizeof(int32_t));
	if(recibi_longPagina == ERROR_OPERATION) return NULL;

	char* contenido_pagina = malloc(longPagina);
	recibido = _receive_bytes(socketMemoria, contenido_pagina, longPagina);
	contenido_pagina[longPagina]='\0';

	if(recibido == ERROR_OPERATION) return NULL;
	t_respuesta* response= malloc(sizeof(t_respuesta));
	if (contenido_pagina != NULL){
	response->id=LEER;
	response->texto=string_new();
	response->retardo = 0;
	string_append_with_format(&response->texto, "mProc %d - Pagina %d leida: %s ",pcb->PID, numDePagina, contenido_pagina);
	log_info(loggerInfo,ANSI_COLOR_GREEN "CPU: %d - mProc %d - Pagina %d leida: %s "ANSI_COLOR_RESET,id, pcb->PID, numDePagina, contenido_pagina);
	}
	else{
		response->id=ERROR;
		response->texto=string_new();
		response->retardo = 0;
		string_append_with_format(&response->texto, "Error en el mProc %d - Pagina %d no leida: %s ",pcb->PID, numDePagina, contenido_pagina);
		log_error(loggerError, ANSI_COLOR_RED "CPU:%d - Error en el mProc %d - Pagina %d NO leida: %s " ANSI_COLOR_RESET,id, pcb->PID, numDePagina, contenido_pagina);
	}
	free(contenido_pagina);
	free(header_de_memoria);
	return response;
}

t_respuesta* mAnsisOp_escribir(int32_t id,PCB* pcb, int32_t numDePagina, char* texto){
	sock_t* socketMemoria=getSocketMemoria(id);
	log_debug(loggerDebug, "CPU:%d - Se procedera a escribir del proceso:%d, la pagina: %d, %d bytes", id, pcb->PID, numDePagina, string_length(texto));
	/** Envio header a la memoria con ESCRIBIR **/
	int32_t tamanio = string_length(texto);
	header_t* header_A_Memoria = _create_header(M_ESCRIBIR, 3 * sizeof(int32_t)+tamanio);

	int32_t enviado = _send_header(socketMemoria, header_A_Memoria);
	if(enviado == ERROR_OPERATION) return NULL;
	enviado = _send_bytes(socketMemoria,&pcb->PID,sizeof (int32_t));
	if(enviado == ERROR_OPERATION) return NULL;
	enviado = _send_bytes(socketMemoria,&numDePagina, sizeof (int32_t));
	if(enviado == ERROR_OPERATION) return NULL;
	enviado = _send_bytes(socketMemoria,&tamanio, sizeof (int32_t));
	if(enviado == ERROR_OPERATION) return NULL;
	enviado = _send_bytes(socketMemoria,texto, tamanio);
	if(enviado == ERROR_OPERATION) return NULL;
	log_debug(loggerDebug, "Envie el id %d,el numero de pagina %d,y el texto %s",pcb->PID, numDePagina, texto);
	free(header_A_Memoria);
	/** Recibo header de la memoria con el resultado (OK o ERROR) **/
	header_t* header_de_memoria = _create_empty_header();
	int32_t recibido = _receive_header(socketMemoria, header_de_memoria);
	if(recibido == ERROR_OPERATION) return NULL;

	t_respuesta* response= malloc(sizeof(t_respuesta));
	if (header_de_memoria->cod_op == OK){
		response->id=ESCRIBIR;
		response->texto=string_new();
		response->retardo = 0;
		string_append_with_format(&response->texto, "mProc %d - Pagina %d escrita: %s ", pcb->PID,numDePagina, texto);
		log_info(loggerInfo, ANSI_COLOR_CYAN "CPU: %d - mProc %d - Pagina %d escrita: %s " ANSI_COLOR_RESET,id, pcb->PID,numDePagina, texto);
	} else {
		response->id=ERROR;
		response->texto=string_new();
		response->retardo = 0;
		string_append_with_format(&response->texto, "Error en el mProc %d - Pagina %d NO escrita: %s ", pcb->PID,numDePagina, texto);
		log_error(loggerError, ANSI_COLOR_RED "CPU: %d - Error en el mProc %d - Pagina %d NO escrita: %s "ANSI_COLOR_RESET, id, pcb->PID,numDePagina, texto);
	}
	free(header_de_memoria);
	free(texto);
	return response;
}

t_respuesta* mAnsisOp_IO(int32_t id, PCB* pcb,int32_t tiempo){

	log_debug(loggerDebug, "CPU: %d - Se procedera a enviar el proceso:%d a IO con retardo: %d",id, pcb->PID, tiempo);

	t_respuesta* response= malloc(sizeof(t_respuesta));
	response->id=ENTRADASALIDA;
	response->texto=string_new();
	response->retardo = tiempo;
	string_append_with_format(&response->texto, "mProc %d en entrada-salida de tiempo %d ", id,tiempo);
	log_info(loggerInfo, ANSI_COLOR_BLUE "CPU:%d - mProc %d en entrada-salida de tiempo %d " ANSI_COLOR_RESET, id,tiempo);
	return response;
}

t_respuesta* mAnsisOp_finalizar(int32_t id, PCB* pcb){

	sock_t* socketMemoria=getSocketMemoria(id);

	log_debug(loggerDebug, "CPU:%d - Se procedera a finalizar el proceso:%d",id, pcb->PID);

	header_t* header_A_Memoria = _create_header(M_FINALIZAR, sizeof(int32_t));

	int32_t enviado = _send_header(socketMemoria,header_A_Memoria);
	if(enviado == ERROR_OPERATION) return NULL;
	enviado = _send_bytes(socketMemoria,&pcb->PID, sizeof (int32_t));
	if(enviado == ERROR_OPERATION) return NULL;
	log_debug(loggerDebug, "Envie el id %d de finalizar",pcb->PID);
	free(header_A_Memoria);

	header_t* respuestaMem=_create_empty_header();
	int32_t recibido = _receive_header(socketMemoria,respuestaMem);
	if(recibido == ERROR_OPERATION) return NULL;

	t_respuesta* response= malloc(sizeof(t_respuesta));
	if (get_operation_code(respuestaMem) ==OK){
	response->id=FINALIZAR;
	response->texto=string_new();
	response->retardo = 0;
	string_append_with_format(&response->texto, "mProc %d Finalizado ", pcb->PID);
	log_info(loggerInfo, ANSI_COLOR_BOLDMAGENTA "CPU:%d - mProc %d Finalizado "ANSI_COLOR_RESET, id, pcb->PID);
	}else{
		response->id=ERROR;
		response->texto=string_new();
		response->retardo = 0;
		string_append_with_format(&response->texto, "Error al finalizar el mProc %d ", pcb->PID);
		log_error(loggerError, ANSI_COLOR_RED "CPU: %d - Error al finalizar el mProc %d " ANSI_COLOR_RESET,id, pcb->PID);
	}
	free(respuestaMem);
	return response;
}


//Analizador de linea
t_respuesta* analizadorLinea(int32_t id,PCB* pcb, char* const instruccion){

	char* linea= strdup(instruccion);
	switch (analizar_operacion_asociada(linea)){
	case INICIAR:{
		int cantDePaginas=buscarPrimerParametro(strstr(instruccion," ")+1);
		free(linea);
		return mAnsisOp_iniciar(id,pcb, cantDePaginas);
		break;
		}
	case FINALIZAR:{
		free(linea);
		return mAnsisOp_finalizar(id, pcb);
		break;
		}
	case ENTRADASALIDA:{
		int tiempo=buscarPrimerParametro(strstr(instruccion," ")+1);
		free(linea);
		return mAnsisOp_IO(id,pcb,tiempo);
		break;
		}
	case LEER:{
		int numDePagina=buscarPrimerParametro(strstr(instruccion," ")+1);
		free(linea);
		return mAnsisOp_leer(id,pcb, numDePagina);
		break;
		}
	case ESCRIBIR:{
		structEscribir parametros=buscarAmbosParametros(strstr(instruccion," ")+1);
		free(linea);
		return mAnsisOp_escribir(id,pcb,parametros.pagina, parametros.texto);
		break;
		}
	default:
		printf("Linea no válida\n");
		free(linea);
		return NULL;
		break;
	}
	return NULL;
}
sock_t* getSocketPlanificador(int32_t id){
	t_sockets* sock= list_get(socketsCPU, id);
	return sock->socketPlanificador;
}

sock_t* getSocketMemoria(int32_t id){
	t_sockets* sock= list_get(socketsCPU, id);
	return sock->socketMemoria;
}

