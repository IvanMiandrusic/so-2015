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

	//sock_t* socket_Planificador=create_client_socket(arch->ip_planificador,arch->puerto_planificador);
	//int32_t resultado=connect_to_server(socket_Planificador);

	//sock_t* socket_Memoria=create_client_socket(arch->ip_memoria,arch->puerto_memoria);
	//int32_t resultado=connect_to_server(socket_Memoria);

	log_info(loggerInfo, "CPU_ID:%d->Finaliza sus tareas, hilo concluido", thread_id);
	return 0;

}


void* iniciar (int32_t paginas){
    //mandar al admin de memoria que se inició un proceso de N paginas

	return 0;
}

void* leer (int32_t pagina){
	//se debe leer de la memoria la pagina N

	return 0;
}

void* escribir (int32_t pagina, char* texto){
	//se debe escribir en memoria el texto en la pagina N

	return 0;
}

void* entrada_salida (int32_t tiempo){
	//decirle al planificador que se haga una i/o de cierto tiempo

	return 0;
}

void* finalizar (){
//decirle a la memoria que se elimine la tabla asociada

	return 0;
}
