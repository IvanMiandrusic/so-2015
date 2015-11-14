#include "Comunicacion.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/string.h>

//Aca irian todas las funciones de serializar/deserializar los envios
char* serializarPCB(PCB* pcb)

{
	char* pcbserializado = malloc(4 * sizeof(u_int32_t)+strlen(pcb->ruta_archivo));

	u_int32_t offset = 0;
	u_int32_t size_to_send;										//El orden de serializado es el siguiente:

	size_to_send =  sizeof(u_int32_t);
	memcpy(pcbserializado + offset, &(pcb->PID), size_to_send);
	offset += size_to_send;

	int32_t tamanio = string_length(pcb->ruta_archivo);

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
	pcb->ruta_archivo = malloc(tamanio+1);
	memcpy(pcb->ruta_archivo, serializado + offset, tamanio);
	pcb->ruta_archivo[tamanio] = '\0';

	offset += tamanio;

	memcpy(&(pcb->estado), serializado + offset, size_entero);

	offset += size_entero;

	memcpy(&(pcb->siguienteInstruccion), serializado + offset, size_entero);

	offset += size_entero;
	return pcb;

}

int32_t obtener_tamanio_pcb(PCB* pcb) {
	return 4*sizeof(int32_t) + string_length(pcb->ruta_archivo);
}

int32_t obtengoSegundos(){
	char* tiempo= temporal_get_string_time();
	char* segundos=malloc(3);
	segundos[0]=tiempo[6];
	segundos[1]=tiempo[7];
	segundos[2]='\0';
	int32_t valor=atoi(segundos);
	free(tiempo);
	free(segundos);
	return valor;
}



