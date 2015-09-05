/*
 * Operaciones.c
 *
 *  Created on: 28/8/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include "Cpu.h"
#include "Operaciones.h"
extern ProcesoCPU* arch;
extern t_log* loggerInfo;
extern t_log* loggerError;
extern t_log* loggerDebug;
extern sock_t* socketPlanificador;
extern sock_t* socketMemoria;

int32_t quantum;

void* thread_Cpu(void* id){

	int32_t thread_id = (void*) id;

	sock_t* socket_Planificador=create_client_socket(arch->ip_planificador,arch->puerto_planificador);
	int32_t resultado = connect_to_server(socket_Planificador);
	if(resultado == ERROR_OPERATION) {
		log_error(loggerError, "Error al conectar al planificador");
		return NULL;
	}

	socketPlanificador[thread_id] = *socket_Planificador;

	//enviarle al planificador NUEVA_CPU y su id;
	header_t* header_nueva_cpu = _create_header(NUEVA_CPU, sizeof(int32_t));
	int32_t enviado =_send_header (socket_Planificador, header_nueva_cpu);

	if (enviado !=SUCCESS_OPERATION)
	{
		log_error(loggerError,"Se perdio la conexion con el Planificador");
		exit(EXIT_FAILURE);
	}

	enviado = _send_bytes(socket_Planificador, &thread_id,sizeof(int32_t));

	if (enviado !=SUCCESS_OPERATION)
	{
		log_error(loggerError,"Se perdio la conexion con el Planificador");
		exit(EXIT_FAILURE);
	}

	free(header_nueva_cpu);


	sock_t* socket_Memoria=create_client_socket(arch->ip_memoria,arch->puerto_memoria);
	if(connect_to_server(socket_Memoria)!=SUCCESS_OPERATION){
		log_error(loggerError, "No se puedo conectar con la memoria, se aborta el proceso");
		//exit(1);
	}

	//Todo socketMemoria[thread_id] = *socket_Memoria;
    //por el momento no se le envia nada
	//while finalizar==false recv operacion del planificador

	int32_t finalizar = 1;
	int32_t resul_Mensaje_Recibido;
	header_t* header_memoria = _create_empty_header() ;

	while (finalizar == 1)
	{
		resul_Mensaje_Recibido = _receive_header(socket_Planificador, header_memoria);

		if (resul_Mensaje_Recibido !=SUCCESS_OPERATION )
		{
			log_error(loggerError,"Se perdio la conexion con el Planificador");
			finalizar = 0;
			break;
		}
		tipo_Cod_Operacion (thread_id,header_memoria);

	}

	//uso la funcion sacar digito para ver lo que me mando el planificador,
	//un switch (digito) segun el "numero" ver que hace, case 1 envio_quantum... etc
	//termina el switch y termina el while, y ponele que cierro los sockets
	log_info(loggerInfo, "CPU_ID:%d->Finaliza sus tareas, hilo concluido", thread_id);

	return NULL;

}

void tipo_Cod_Operacion (int32_t id, header_t* header){

	switch (get_operation_code(header))
	{
	case ENVIO_QUANTUM:{
		int32_t recibido = _receive_bytes(&(socketPlanificador[id]), &quantum, sizeof(int32_t));
		log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d QUANTUM: %d",get_operation_code(header), quantum);
		if (recibido == ERROR_OPERATION)return;
		break;
	}

	case ENVIO_PCB:{
		log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d PCB",get_operation_code(header));
		//TODO ver de donde sale el header
		char* pedido_serializado = malloc(get_message_size(header));
		int32_t recibido = _receive_bytes(&(socketPlanificador[id]), pedido_serializado, get_message_size(header));
		if(recibido == ERROR_OPERATION) return;

		PCB* pcb = deserializarPCB (pedido_serializado);

		ejecutar_Instrucciones (id, pcb);

		break;
	}

	case FINALIZAR_PROCESO:{
		log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d Finalizar Proceso",get_operation_code(header));
		break;
	}

	}
}


void ejecutar_Instrucciones (int32_t id, PCB* pcb){
//  Todo esto estaria tirando error
//	if (quantum == 0) ejecutar_FIFO(id, pcb);
//	if (quantum > 0) ejecutar_RR(id, pcb);

}

void ejecutarFIFO(int32_t id, PCB* pcb){
	bool finalizar=false;
	FILE *f = fopen(pcb->ruta_archivo, "r");
	if (f==NULL)
	{
		log_error (loggerError, "Error al abrir fichero.txt");
		return;
	}
	char cadena[100];
	while(finalizar==false){

		if(fgets(cadena, 100, f) != NULL)
		{

			analizadorLinea(id, cadena);
		}
		/*todo, arreglar para que se devuelva un struct int-char, de modo de appendear la respuesta y el int
		para saber que ejecuto ultimo de modo de cortar en un finalizar o io.
		actualizar pcb, con ftells(f);
		*/
		}
}

void ejecutarRR(int32_t id, PCB* pcb){
	int32_t ultimoQuantum;
	FILE *f = fopen(pcb->ruta_archivo, "r");
	if (f==NULL)
	{
		log_error (loggerError, "Error al abrir fichero.txt");
		return;
	}
	char cadena[100];
	while(ultimoQuantum<=quantum){
		if(fgets(cadena, 100, f) != NULL)
		{

			analizadorLinea(id, cadena);
			/*todo, arreglar para que se devuelva un struct int-char, de modo de appendear la respuesta y el int
					para saber que ejecuto ultimo de modo de cortar en un finalizar o io.
					actualizar pcb, con ftells(f);
			 */
		}
		ultimoQuantum++;
	}

}

//TODO hacer funciones FIFO y RR y //actualizar program counter donde corresponda

PCB* deserializarPCB(char* serializado)
{
	PCB* pcb = malloc(sizeof(PCB));
	int offset = 0;
	int size_entero = sizeof(u_int32_t);

	memcpy(&(pcb->PID), serializado + offset, size_entero);

	offset += size_entero;
	int32_t tamanio;
	memcpy(&tamanio, serializado + offset, size_entero);

	offset += size_entero;

	memcpy(pcb->ruta_archivo, serializado + offset, tamanio);

	offset += tamanio;

	memcpy(&(pcb->estado), serializado + offset, size_entero);

	offset += size_entero;

	memcpy(&(pcb->siguienteInstruccion), serializado + offset, size_entero);

	offset += size_entero;
	return pcb;

}

void mAnsisOp_iniciar(int32_t id, int32_t cantDePaginas){
    //mandar al admin de memoria que se inició un proceso de N paginas

	/** Envio header a la memoria con INICIAR **/
	header_t* header_A_Memoria = _create_header(INICIAR, 2 * sizeof(int32_t));

	int32_t enviado = _send_header(&(socketMemoria[id]), header_A_Memoria);
	if(enviado == ERROR_OPERATION) return;

	enviado = _send_bytes(&(socketMemoria[id]),&id, sizeof (int32_t)); //Todo es PID o CPU_ID
	if(enviado == ERROR_OPERATION) return;

	enviado = _send_bytes(&(socketMemoria[id]),&cantDePaginas, sizeof (int32_t));
	if(enviado == ERROR_OPERATION) return;

	free(header_A_Memoria);


	/** Recibo header de la memoria con el resultado (OK o ERROR) **/
	header_t* header_de_memoria = _create_empty_header();

	int32_t recibido = _receive_header(&(socketMemoria[id]), header_de_memoria);

	if(get_operation_code(header_de_memoria) == ERROR) {
		log_error(loggerError,"mProc %d -Fallo",id); //Todo es el PID o CPU_ID ?
		enviado = enviarResultadoAlPlanificador(RESULTADO_ERROR, &(socketPlanificador[id]));
		if(enviado == ERROR_OPERATION) {
			log_error(loggerError, "Error al enviar el aviso al planificador");
			exit(EXIT_FAILURE);
		}
	}
	else {
		log_info(loggerInfo,"mProc %d -Iniciado",id);//Todo es el PID o CPU_ID ?
		enviarResultadoAlPlanificador(RESULTADO_OK, &(socketPlanificador[id]));
		if(enviado == ERROR_OPERATION) {
			log_error(loggerError, "Error al enviar el aviso al planificador");
			exit(EXIT_FAILURE);
		}
	}

	free(header_de_memoria);

}

int32_t enviarResultadoAlPlanificador(int32_t codigo_resultado, sock_t* socket_planificador) {

	/** Envio header al planificador con codigo de resultado (solo envia el codigo)**/
	header_t* header_planificador = _create_header(codigo_resultado, 0);

	int32_t enviado = _send_header(socket_planificador, header_planificador);

	free(header_planificador);

	if(enviado == ERROR_OPERATION) return ERROR_OPERATION;

	return SUCCESS_OPERATION;

}

void mAnsisOp_leer(int32_t id,int32_t numDePagina){
	//se debe leer de la memoria la pagina N

	/** Envio header a la memoria con LEER **/
	header_t* header_A_Memoria = _create_header(LEER, 2 * sizeof(int32_t));

	int32_t enviado = _send_header(&(socketMemoria[id]), header_A_Memoria);
	if(enviado == ERROR_OPERATION) return;

	enviado = _send_bytes(&(socketMemoria[id]),&id, sizeof (int32_t)); //Todo es el PID o CPU_ID ?
	if(enviado == ERROR_OPERATION) return;

	enviado = _send_bytes(&(socketMemoria[id]),&numDePagina, sizeof (int32_t));
	if(enviado == ERROR_OPERATION) return;


	/** Recibo header de la memoria con el codigo CONTENIDO_PAGINA o NULL **/
	header_t* header_de_memoria = _create_empty_header();

	int32_t recibido = _receive_header(&(socketMemoria[id]), header_de_memoria);

	int32_t longPagina;

	int32_t recibi_longPagina = _receive_bytes(&(socketMemoria[id]), &longPagina, sizeof(int32_t));
	if(recibi_longPagina == ERROR_OPERATION) return;

	char* contenido_pagina = malloc(longPagina);
	recibido = _receive_bytes(&(socketMemoria[id]), contenido_pagina, longPagina);
	if(recibido == ERROR_OPERATION) return;

	//Todo Ver de ir acumulando resultados para enviar al planificador
	int32_t enviar_Resultado = _send_bytes(&(socketMemoria[id]),&recibido,sizeof (int32_t)); //Esta bien?

	log_info(loggerInfo,"mProc %d - Pagina %d leida: %s ",id, numDePagina, contenido_pagina);
}

void mAnsisOp_escribir(int32_t id, int32_t numDePagina, char* texto){
	//se debe escribir en memoria el texto en la pagina N

	/** Envio header a la memoria con ESCRIBIR **/
	int32_t tamanio = strlen(texto);
	header_t* header_A_Memoria = _create_header(ESCRIBIR, 3 * sizeof(int32_t)+tamanio);

	int32_t enviado_Header = _send_header(&(socketMemoria[id]), header_A_Memoria);

	int32_t enviado_Id_Proceso = _send_bytes(&(socketMemoria[id]),&id, sizeof (int32_t));
	int32_t enviado_numDePagina = _send_bytes(&(socketMemoria[id]),&numDePagina, sizeof (int32_t));
	int32_t enviado_tamanio_texto = _send_bytes(&(socketMemoria[id]),&tamanio, sizeof (int32_t));
	int32_t enviado_texto = _send_bytes(&(socketMemoria[id]),texto, tamanio);


	/** Recibo header de la memoria con el resultado (OK o ERROR) **/
	header_t* header_de_memoria = _create_empty_header();

	int32_t recibido = _receive_header(&(socketMemoria[id]), header_de_memoria);

	//Todo ver si conviene recibirlo
	int32_t longTexto;
	int32_t recibi_longTexto = _receive_bytes(&(socketMemoria[id]),&longTexto, get_message_size(header_de_memoria));
		if(recibi_longTexto == ERROR_OPERATION) return;

	log_info(loggerInfo, "mProc %d - Pagina %d escrita: %s", id,numDePagina, texto);

	int32_t enviar_Resultado = _send_bytes(&(socketMemoria[id]),&recibi_longTexto,sizeof (int32_t)); //Esta bien?



}

void mAnsisOp_IO(int32_t id, int32_t tiempo){
	//decirle al planificador que se haga una i/o de cierto tiempo
	//solo ver si se envio al planificador

	//TODO como se libera la CPU?
	header_t* header_A_Planificador = _create_header(TERMINO_IO,2*sizeof(int32_t));

	int32_t enviado_Header = _send_header(&(socketPlanificador[id]),header_A_Planificador);

	int32_t enviado_Id_Proceso = _send_bytes(&(socketPlanificador[id]),&id,sizeof(int32_t));
	int32_t enviado_Tiempo = _send_bytes(&(socketPlanificador[id]),&tiempo,sizeof(int32_t));

	int32_t resultado;
	int32_t recibi_Resultado = _receive_bytes(&(socketPlanificador[id]),&resultado,sizeof (int32_t));

	if(recibi_Resultado == ERROR_OPERATION) return;

	log_info(loggerInfo, "mProc %d en entrada-salida de tiempo %d", id,tiempo);

	int32_t enviar_Resultado = _send_bytes(&(socketMemoria[id]),&recibi_Resultado,sizeof (int32_t)); //Esta bien?



}

void mAnsisOp_finalizar(int32_t id){
//decirle a la memoria que se elimine la tabla asociada

	header_t* header_A_Memoria = _create_header(FINALIZAR, sizeof(int32_t));

	int32_t enviado_Header = _send_header(&(socketMemoria[id]),header_A_Memoria);
	int32_t enviado_Id_Proceso = _send_bytes(&(socketMemoria[id]),&id, sizeof (int32_t));

	int32_t resultado;
	int32_t resultado_Mensaje = _receive_bytes(&(socketMemoria[id]),&resultado, sizeof (int32_t)); //aca recibo un que?
	if(resultado_Mensaje == ERROR_OPERATION) return;

	log_info(loggerInfo, "mProc %d finalizado", id);

	int32_t enviar_Resultado = _send_bytes(&(socketMemoria[id]),&resultado_Mensaje,sizeof(int32_t));


}
