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
	socketPlanificador[thread_id]=socket_Planificador;
	//enviarle al planificador NUEVA_CPU y su id;

	header_t* header = create_header(NUEVA_CPU, sizeof(int32_t));
	int32_t resul_header =_send_header (socket_Planificador, header);
	int32_t resul_id = _send_bytes(socket_Planificador, &thread_id,sizeof(int32_t));

	if (resul_id !=SUCCESS_OPERATION)
				{
					log_error(loggerError,"Se perdio la conexion con el Planificador");
					exit(1);
				}


	sock_t* socket_Memoria=create_client_socket(arch->ip_memoria,arch->puerto_memoria);
	if(connect_to_server(socket_Memoria)!=SUCCESS_OPERATION){
		log_error(loggerError, "No se puedo conectar con la memoria, se aborta el proceso");
		exit(1);
	}
	socketMemoria[thread_id]=socket_Memoria;
    //por el momento no se le envia nada
	//while finalizar==false recv operacion del planificador

	int32_t finalizar = 1;
	int32_t resul_Mensaje_Recibido;
	int32_t cod_Operacion;
	header_t* header_t = _create_empty_header() ;

	while (finalizar == 1)
	{
		resul_Mensaje_Recibido = _receive_header(socket_Planificador, header_t);

		cod_Operacion = get_operation_code(header_t);

		if (resul_Mensaje_Recibido !=SUCCESS_OPERATION )
						{
							log_error(loggerError,"Se perdio la conexion con el Planificador");
							finalizar = 0;
							break;
						}
		tipo_Cod_Operacion (thread_id,cod_Operacion);

	}

	//uso la funcion sacar digito para ver lo que me mando el planificador,
	//un switch (digito) segun el "numero" ver que hace, case 1 envio_quantum... etc
	//termina el switch y termina el while, y ponele que cierro los sockets
	log_info(loggerInfo, "CPU_ID:%d->Finaliza sus tareas, hilo concluido", thread_id);
	return 0;

}

void tipo_Cod_Operacion (id,cod_Operacion){
	switch (cod_Operacion)
				{
				case ENVIO_QUANTUM:{
						int32_t recibido = _receive_bytes(socketPlanificador[id], &quantum, sizeof(int32_t));
						log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d QUANTUM: %d",cod_Operacion, quantum);
						if (recibido == ERROR_OPERATION)return;
						break;
									}

				case ENVIO_PCB:{
					log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d PCB",cod_Operacion);
					//TODO ver de donde sale el header
					char* pedido_serializado = malloc(get_message_size(header));
					int32_t recibido = _receive_bytes(socketPlanificador[id], pedido_serializado, get_message_size(header));
					if(recibido == ERROR_OPERATION) return;

					PCB* pcb = deserializarPCB (pedido_serializado);

					ejecutar_Instrucciones (id, pcb);

					break;
								}

				case FINALIZAR_PROCESO:{
					log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d Finalizar Proceso",cod_Operacion);
					break;
					}

				}
}


void ejecutar_Instrucciones (int32_t id, PCB* pcb){
	if (quantum == 0) ejecutar_FIFO(id, pcb);
	if (quantum > 0) ejecutar_RR(id, pcb);

										}


//TODO hacer funciones FIFO y RR y //actualizar program counter donde corresponda

PCB* deserializarPCB(serializado)
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


	header_t* header_A_Memoria = _create_header(INICIAR, 2 * sizeof(int32_t));

	int32_t enviado_Header = _send_header(socketMemoria[id], header_A_Memoria, sizeof(header_t));

	int32_t enviado_Id_Proceso = send_bytes(socketMemoria[id],&id, sizeof (int32_t));
	int32_t enviado_CantPaginas = send_bytes(socketMemoria[id],&cantDePaginas, sizeof (int32_t));

	if(enviado_CantPaginas == ERROR_OPERATION) return;

	int32_t resultado;

	int32_t resultado_Mensaje = _receive_bytes(socketMemoria[id], &resultado, sizeof(int32_t));
		if(resultado_Mensaje == ERROR_OPERATION)
			log_error(loggerError,"mProc %d -Fallo",id);
		return;

	log_info(loggerInfo,"mProc %d -Iniciado",id);
	int32_t enviar_Resultado = _send_bytes(socketMemoria[id],&resultado_Mensaje,sizeof (int32_t)); //Esta bien?


}

void mAnsisOp_leer(int32_t id,int32_t numDePagina){
	//se debe leer de la memoria la pagina N

		header_t* header_A_Memoria = _create_header(LEER, 2 * sizeof(int32_t));

		int32_t enviado_Header = _send_header(socketMemoria[id], header_A_Memoria, sizeof(header_t));

		int32_t enviado_Id_Proceso = send_bytes(socketMemoria[id],&id, sizeof (int32_t));
		int32_t enviado_numDePagina = send_bytes(socketMemoria[id],&numDePagina, sizeof (int32_t));



		int32_t longPagina;
		int32_t recibi_longPagina = _receive_bytes(socketMemoria[id], &longPagina, get_message_size(header));
		if(recibi_longPagina == ERROR_OPERATION) return;

		char* contenido_pagina = malloc(longPagina);
		int32_t recibido = _receive_bytes(socketMemoria[id], contenido_pagina, longPagina);
		if(recibido == ERROR_OPERATION) return;

		int32_t enviar_Resultado = _send_bytes(socketMemoria[id],&recibido,sizeof (int32_t)); //Esta bien?

		log_info(loggerInfo,"mProc %d - Pagina %d leida: %s ",id, numDePagina, contenido_pagina);
}

void mAnsisOp_escribir(int32_t id, int32_t numDePagina, char* texto){
	//se debe escribir en memoria el texto en la pagina N

	int32_t tamanio = strlen (texto);
	header_t* header_A_Memoria = _create_header(ESCRIBIR, 3 * sizeof(int32_t)+tamanio);

	int32_t enviado_Header = _send_header(socketMemoria[id], header_A_Memoria, sizeof(header_t));

	int32_t enviado_Id_Proceso = send_bytes(socketMemoria[id],&id, sizeof (int32_t));
	int32_t enviado_numDePagina = send_bytes(socketMemoria[id],&numDePagina, sizeof (int32_t));
	int32_t enviado_tamanio_texto = send_bytes(socketMemoria[id],&tamanio, sizeof (int32_t));
	int32_t enviado_texto = send_bytes(socketMemoria[id],texto, tamanio);

	int32_t longTexto;
	int32_t recibi_longTexto = _receive_bytes(socketMemoria[id],&longTexto, get_message_size(header));
		if(recibi_longTexto == ERROR_OPERATION) return;

	log_info(loggerInfo, "mProc %d - Pagina %d escrita: %s", id,numDePagina, texto);

	int32_t enviar_Resultado = _send_bytes(socketMemoria[id],&recibi_longTexto,sizeof (int32_t)); //Esta bien?



}

void mAnsisOp_IO(int32_t id, int32_t tiempo){
	//decirle al planificador que se haga una i/o de cierto tiempo
	//solo ver si se envio al planificador

	//TODO aca envio la NUEVA_IO pero no esta en comunicacion....
	//TODO como se libera la CPU?
	header_t* header_A_Planificador = _create_header(NUEVA_IO,2*sizeof(int32_t));

	int32_t enviado_Header = _send_header(socketPlanificador[id],header_A_Planificador,sizeof(header_t));

	int32_t enviado_Id_Proceso = _send_bytes(socketPlanificador[id],&id,sizeof(int32_t));
	int32_t enviado_Tiempo = _send_bytes(socketPlanificador[id],&tiempo,sizeof(int32_t));

	int32_t resultado;
	int32_t recibi_Resultado = _receive_bytes(socketPlanificador[id],&resultado,sizeof (int32_t));
			if(recibi_Resultado == ERROR_OPERATION) return;

		log_info(loggerInfo, "mProc %d en entrada-salida de tiempo %d", id,tiempo);

		int32_t enviar_Resultado = _send_bytes(socketMemoria[id],&recibi_Resultado,sizeof (int32_t)); //Esta bien?



}

void mAnsisOp_finalizar(int32_t id){
//decirle a la memoria que se elimine la tabla asociada

	header_t* header_A_Memoria = _create_header(FINALIZAR, sizeof(int32_t));

	int32_t enviado_Header = _send_header(socketMemoria[id],header_A_Memoria, sizeof (header_t));
	int32_t enviado_Id_Proceso = _send_bytes(socketMemoria[id],&id, sizeof (int32_t));

	int32_t resultado;
	int32_t resultado_Mensaje = _receive_bytes(socketMemoria[id],&resultado, sizeof (int32_t)); //aca recibo un que?
	if(resultado_Mensaje == ERROR_OPERATION) return;

	log_info(loggerInfo, "mProc %d finalizado", id);

	int32_t enviar_Resultado = _send_bytes(socketMemoria[id],&resultado_Mensaje,sizeof(int32_t));


}
