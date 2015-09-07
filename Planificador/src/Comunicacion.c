/*
 * Comunicacion.c
 *
 *  Created on: 30/08/2015
 *      Author: federico
 */

#include <stdlib.h>
#include <stdio.h>
#include "Comunicacion.h"

//Aca irian todas las funciones de serializar/deserializar los envios

PCB* deserializarPCB(char* pedidoserializado){

	PCB* pcb = malloc(sizeof(PCB));
	int32_t offset = 0;
	int32_t size = sizeof(int32_t);

	memcpy(&(pcb->PID), pedidoserializado + offset, size);

	offset+= size;
	int32_t tamanio;
	memcpy(&tamanio, pedidoserializado + offset, size);

	offset+= size;
	pcb->ruta_archivo = malloc(tamanio);
	memcpy(pcb->ruta_archivo, pedidoserializado + offset, tamanio);
	pcb->ruta_archivo[tamanio] = '\0';

	offset += tamanio;

	memcpy(&(pcb->estado), pedidoserializado + offset, size);

	offset += size;

	memcpy(&(pcb->siguienteInstruccion), pedidoserializado + offset, size);

	offset += size;
	return pcb;

	//todo: hacer la deserializacion y fijar bien el TIPO que deserializa
}

char* serializarPCB(PCB* unPCB){

	char* serializado = malloc(4*sizeof(int32_t)+strlen(unPCB->ruta_archivo));

	int32_t offset = 0;
	int32_t size_to_send;

		size_to_send = sizeof(int32_t);
		memcpy(serializado + offset, &(unPCB->PID), size_to_send);
		offset += size_to_send;

		int32_t tamanio=strlen(unPCB->ruta_archivo);

		size_to_send =  sizeof(u_int32_t);
		memcpy(serializado + offset, &(tamanio), size_to_send);
		offset += size_to_send;

		size_to_send = tamanio;
		memcpy(serializado + offset, unPCB->ruta_archivo, size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(int32_t);
		memcpy(serializado + offset, &(unPCB->estado), size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(int32_t);
		memcpy(serializado + offset, &(unPCB->siguienteInstruccion), size_to_send);
		offset += size_to_send;


		return serializado;

}

