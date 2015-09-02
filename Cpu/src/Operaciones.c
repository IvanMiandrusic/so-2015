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

int32_t quantum;

void* thread_Cpu(void* id){
	int32_t thread_id = (void*) id;

	sock_t* socket_Planificador=create_client_socket(arch->ip_planificador,arch->puerto_planificador);
	int32_t resultado = connect_to_server(socket_Planificador);

	//enviarle al planificador NUEVA_CPU y su id;

	header_t* header = create_header(NUEVA_CPU, sizeof(int32_t));
	int32_t resul_header =_send_header (socket_Planificador, header);
	int32_t resul_id = _send_bytes(socket_Planificador, &thread_id,sizeof(int32_t));

	if (resul_id == 0)
				{
					log_info(loggerError,"Se perdio la conexion con el Planificador");
					return NULL;
				}


	sock_t* socket_Memoria=create_client_socket(arch->ip_memoria,arch->puerto_memoria);
	resultado=connect_to_server(socket_Memoria);

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

		if (resul_Mensaje_Recibido == 0)
						{
							log_info(loggerError,"Se perdio la conexion con el Planificador");

							finalizar = 0;

							break;

						}

		tipo_Cod_Operacion (cod_Operacion,socket_Planificador);

	}

	//uso la funcion sacar digito para ver lo que me mando el planificador,
	//un switch (digito) segun el "numero" ver que hace, case 1 envio_quantum... etc
	//termina el switch y termina el while, y ponele que cierro los sockets
	log_info(loggerInfo, "CPU_ID:%d->Finaliza sus tareas, hilo concluido", thread_id);
	return 0;

}

void tipo_Cod_Operacion (cod_Operacion,socket_Planificador){ //Switch
	switch (cod_Operacion)
				{
				case ENVIO_QUANTUM:{
						int32_t recibido = _receive_bytes(socket_Planificador, &quantum, sizeof(int32_t));
						log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d QUANTUM: %d",cod_Operacion, quantum);
						if (recibido == ERROR_OPERATION)return;
						break;
									}

				case ENVIO_PCB:{
					log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d PCB",cod_Operacion);
					//TODO ver de donde sale el header
					char* pedido_serializado = malloc(get_message_size(header));
					int32_t recibido = _receive_bytes(socket_Planificador, pedido_serializado, get_message_size(header));
					if(recibido == ERROR_OPERATION) return;

					PCB* pcb = deserializo_PCB (pedido_serializado);

					ejecutar_Instruccion (pcb);

					break;
								}

				case FINALIZAR_PROCESO:{
					log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d Finalizar Proceso",cod_Operacion);
					break;
					}

				}
}


void ejecutar_Instrucciones (PCB* pcb){
	if (quantum == 0) ejecutar_FIFO(pcb);
	if (quantum > 0) ejecutar_RR(pcb);
										}

//TODO hacer funciones FIFO y RR


void mAnsisOp_iniciar(int32_t cantDePaginas){
    //mandar al admin de memoria que se inició un proceso de N paginas

	//TODO agregar id como parametro en el parser

	header_t* header_A_Memoria = _create_header(INICIAR, 2 * sizeof(int32_t));

	int32_t enviado_Header = _send_header(socket_Memoria, header_A_Memoria, sizeof(header_t));

	int32_t enviado_Id_Proceso = send_bytes(socket_Memoria,&PID, sizeof (int32_t));
	int32_t enviado_CantPaginas = send_bytes(socket_Memoria,&cantDePaginas, sizeof (int32_t));

	if(enviado_CantPaginas == ERROR_OPERATION) return;

	int32_t resultado;

	int32_t resultado_Mensaje = _receive_bytes(socket_Memoria, &resultado, sizeof(int32_t));
	if(resultado_Mensaje == ERROR_OPERATION) return;

//TODO enviar resultado al planificador

}

void mAnsisOp_leer(int numDePagina){
	//se debe leer de la memoria la pagina N

	//falta el PID y el socket_Memoria
		header_t* header_A_Memoria = _create_header(LEER, 2 * sizeof(int32_t));

		int32_t enviado_Header = _send_header(socket_Memoria, header_A_Memoria, sizeof(header_t));

		int32_t enviado_Id_Proceso = send_bytes(socket_Memoria,&PID, sizeof (int32_t));
		int32_t enviado_numDePagina = send_bytes(socket_Memoria,&numDePagina, sizeof (int32_t));



		int32_t longPagina;
		int32_t recibi_longPagina = _receive_bytes(socket_Memoria, &longPagina, get_message_size(header));
		if(recibi_longPagina == ERROR_OPERATION) return;

		char* contenido_pagina = malloc(longPagina);
		int32_t recibido = _receive_bytes(socket_Memoria, contenido_pagina, longPagina);
		if(recibido == ERROR_OPERATION) return;


		log_info(loggerInfo,"mProc %d - Pagina %d leida: %s ",PID, numDePagina, contenido_pagina);
}

void mAnsisOp_escribir(int32_t numDePagina, char* texto){
	//se debe escribir en memoria el texto en la pagina N

	int32_t tamanio = strlen (texto);
	header_t* header_A_Memoria = _create_header(ESCRIBIR, 3 * sizeof(int32_t)+tamanio);

	int32_t enviado_Header = _send_header(socket_Memoria, header_A_Memoria, sizeof(header_t));


	int32_t enviado_Id_Proceso = send_bytes(socket_Memoria,&PID, sizeof (int32_t));
	int32_t enviado_numDePagina = send_bytes(socket_Memoria,&numDePagina, sizeof (int32_t));
	int32_t enviado_tamanio_texto = send_bytes(socket_Memoria,&tamanio, sizeof (int32_t));
	int32_t enviado_texto = send_bytes(socket_Memoria,texto, tamanio);




}

void mAnsisOp_IO(int32_t tiempo){
	//decirle al planificador que se haga una i/o de cierto tiempo
	//solo ver si se envio al planificador


}

void mAnsisOp_finalizar(){
//decirle a la memoria que se elimine la tabla asociada


}
