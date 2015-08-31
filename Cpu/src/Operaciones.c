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
	sock_t* socket_Memoria=create_client_socket(arch->ip_memoria,arch->puerto_memoria);
	resultado=connect_to_server(socket_Memoria);
    //por el momento no se le envia nada
	//while finalizar==false recv operacion del planificador
	//uso la funcion sacar digito para ver lo que me mando el planificador,
	//un switch (digito) segun el "numero" ver que hace, case 1 envio_quantum... etc
	//termina el switch y termina el while, y ponele que cierro los sockets
	log_info(loggerInfo, "CPU_ID:%d->Finaliza sus tareas, hilo concluido", thread_id);
	return 0;

}


void mAnsisOp_iniciar(int32_t cantDePaginas){
    //mandar al admin de memoria que se inició un proceso de N paginas


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
