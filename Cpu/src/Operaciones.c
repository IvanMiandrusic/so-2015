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


void* thread_Cpu(void* id){
	int32_t thread_id= (void*) id;

	sock_t* socket_Planificador=create_client_socket(arch->ip_planificador,arch->puerto_planificador);
	int32_t resultado=connect_to_server(socket_Planificador);

	//enviarle al planificador NUEVA_CPU y su id;

	header_t* header = create_header(1, sizeof(int32_t));
	_send_header (socket_Planificador, header,sizeof(header_t));
	_send_bytes(socket_Planificador, thread_id,sizeof(int32_t));




	sock_t* socket_Memoria=create_client_socket(arch->ip_memoria,arch->puerto_memoria);
	resultado=connect_to_server(socket_Memoria);

    //por el momento no se le envia nada
	//while finalizar==false recv operacion del planificador

	int32_t finalizar = 1;
	int32_t resul_Mensaje_Recibido;
	int32_t cod_Operacion;

	while (finalizar == 1)
	{
		resul_Mensaje_Recibido = receive_msg(socket_Planificador, header_t);

		cod_Operacion = get_operation_code(header_t);

		evaluar_Mensaje_Recibido(resul_Mensaje_Recibido,"Planificador");

		tipo_Cod_Operacion (cod_Operacion);

	}

	//uso la funcion sacar digito para ver lo que me mando el planificador,
	//un switch (digito) segun el "numero" ver que hace, case 1 envio_quantum... etc
	//termina el switch y termina el while, y ponele que cierro los sockets
	log_info(loggerInfo, "CPU_ID:%d->Finaliza sus tareas, hilo concluido", thread_id);
	return 0;

}

void tipo_Cod_Operacion (cod_Operacion){ //Switch
	switch (cod_Operacion)
				{
				case 1:{
						log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d QUANTUM",cod_Operacion);
					break;
					}

				case 2:{
					log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d PCB",cod_Operacion);
					break;
					}

				case 3:{
					log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d Finalizar Proceso",cod_Operacion);
					break;
					}

				case 4:{
					log_info(loggerInfo,"CPU recibio de Planificador codOperacion %d Pedido de Utilizacion CPU",cod_Operacion);
					break;
					}
				}
}

void evaluar_Mensaje_Recibido(resul_Mensaje_Recibido,de_Quien_Recibi){ //hace falta agregar el finalizar?
	if (resul_Mensaje_Recibido == -1)
			{
				log_error(loggerError,"Error al recibir mensaje del %s",de_Quien_Recibi);
				return ;
			}
}

void evaluar_Mensaje_Enviado(resul_Mensaje_Recibido,a_Quien_Envie){
	if (resul_Mensaje_Recibido == -1)
			{
				log_error(loggerError,"Error al enviar mensaje a %s",a_Quien_Envie);
				return ;
			}
}

void mAnsisOp_iniciar(int32_t cantDePaginas){
    //mandar al admin de memoria que se inició un proceso de N paginas

	//falta enviarle el id del proceso


	header_t* header_A_Memoria = _create_header(INICIAR, 1 * sizeof(int32_t));
	int32_t enviado_Header = _send_header(socket_Memoria, header_A_Memoria, sizeof(header_t)); // de donde saco el socket_Memoria ??
	int32_t enviado_CantPaginas = send_bytes(socket_Memoria,cantDePaginas, sizeof (int32_t));

	evaluar_Mensaje_Enviado(enviado_Header,"Memoria");
	evaluar_Mensaje_Enviado(enviado_CantPaginas,"Memoria");


}

void mAnsisOp_leer(int numDePagina){
	//se debe leer de la memoria la pagina N


}

void mAnsisOp_escribir(int32_t numDePagina, char* texto){
	//se debe escribir en memoria el texto en la pagina N

}

void mAnsisOp_IO(int32_t tiempo){
	//decirle al planificador que se haga una i/o de cierto tiempo



}

void mAnsisOp_finalizar(){
//decirle a la memoria que se elimine la tabla asociada


}
