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

void deserializar_pedido(char* pedidoserializado){
	//todo: hacer la deserializacion y fijar bien el TIPO que deserializa
}

char* serializarPCB(PCB* unPCB){

	char* serializado = malloc(3*sizeof(int32_t)+strlen(unPCB->ruta_archivo));

	int32_t offset = 0;
	int32_t size_to_send;

		size_to_send = sizeof(int32_t);
		memcpy(serializado + offset, &(unPCB->PID), size_to_send);
		offset += size_to_send;


		size_to_send = strlen(unPCB->ruta_archivo);
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

