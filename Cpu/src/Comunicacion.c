/*
 * Comunicacion.c
 *
 *  Created on: 30/08/2015
 *      Author: federico
 */

#include "Comunicacion.h"
#include <stdlib.h>
#include <stdio.h>

//Aca irian todas las funciones de serializar/deserializar los envios
char* serializarPCB(PCB* pcb)

{
	char* pcbserializado = malloc(3 * sizeof(u_int32_t)+strlen(pcb->ruta_archivo));

	u_int32_t offset = 0;
	u_int32_t size_to_send;										//El orden de serializado es el siguiente:

	size_to_send =  sizeof(u_int32_t);
	memcpy(pcbserializado + offset, &(pcb->PID), size_to_send);
	offset += size_to_send;

	int32_t tamanio=strlen(pcb->ruta_archivo);

	size_to_send =  sizeof(u_int32_t);
	memcpy(pcbserializado + offset, &(tamanio), size_to_send);
	offset += size_to_send;

	size_to_send =  tamanio	;
	memcpy(pcbserializado + offset,pcb->ruta_archivo , size_to_send);
	offset += size_to_send;


	size_to_send =  sizeof(u_int32_t);
	memcpy(pcbserializado + offset, &(pcb->estado), size_to_send);
	offset += size_to_send;


	size_to_send =  sizeof(u_int32_t);
	memcpy(pcbserializado + offset, &(pcb->siguienteInstruccion), size_to_send);
	offset += size_to_send;


	return pcbserializado;
}
