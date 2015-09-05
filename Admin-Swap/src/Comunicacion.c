/*
 * Comunicacion.c
 *
 *  Created on: 30/08/2015
 *      Author: federico
 */

#include "Comunicacion.h"
#include <stdlib.h>
#include <stdio.h>

t_pagina* deserializar_pedido(char* pedido_serializado){
		t_pagina* pagina_solicitada = malloc(sizeof(t_pagina));

		int32_t offset = 0;

		memcpy(&(pagina_solicitada->PID), pedido_serializado+offset, sizeof(int32_t));

		offset += sizeof(int32_t);
		memcpy(&(pagina_solicitada->nro_pagina), pedido_serializado+offset, sizeof(int32_t));

		offset += sizeof(int32_t);
		memcpy(&(pagina_solicitada->tamanio_contenido), pedido_serializado+offset, sizeof(int32_t));

		offset += sizeof(int32_t);
		memcpy(pagina_solicitada->contenido, pedido_serializado+offset, pagina_solicitada->tamanio_contenido);

		return pagina_solicitada;
	return NULL;
}

char* serializarTexto (char* texto) {

	int32_t tamanio=strlen(texto);
	char* serializado=malloc(sizeof(int32_t)+tamanio);
	u_int32_t offset = 0;
	u_int32_t size_to_send;

	size_to_send =  sizeof(u_int32_t);
	memcpy(serializado + offset, &(tamanio), size_to_send);
	offset += size_to_send;

	size_to_send =  tamanio;
	memcpy(serializado + offset,texto, size_to_send);
	offset += size_to_send;

	return serializado;

}

