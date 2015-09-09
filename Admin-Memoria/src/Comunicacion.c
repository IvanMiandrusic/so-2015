/*
 * Comunicacion.c
 *
 *  Created on: 30/08/2015
 *      Author: federico
 */

#include "Comunicacion.h"
#include <stdlib.h>
#include <stdio.h>

int32_t obtener_tamanio_pagina(t_pagina* pagina) {
	return 3*sizeof(int32_t) + pagina->tamanio_contenido;
}

char* serializar_pedido(t_pagina* pagina){

	char* pedido_serializado = malloc(obtener_tamanio_pagina(pagina));

	int32_t offset = 0;
	int32_t size_to_send;

	size_to_send = sizeof(int32_t);
	memcpy(pedido_serializado + offset, &(pagina->PID), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(int32_t);
	memcpy(pedido_serializado + offset, &(pagina->nro_pagina), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(int32_t);
	memcpy(pedido_serializado + offset, &(pagina->tamanio_contenido), size_to_send);
	offset += size_to_send;

	size_to_send = pagina->tamanio_contenido;
	memcpy(pedido_serializado + offset, pagina->contenido, size_to_send);

	return pedido_serializado;
}

t_pagina* deserializar_pedido(char* pedido_serializado){

	t_pagina* pagina_solicitada = malloc(sizeof(t_pagina));

	int32_t offset = 0;

	memcpy(&(pagina_solicitada->PID), pedido_serializado+offset, sizeof(int32_t));

	offset += sizeof(int32_t);
	memcpy(&(pagina_solicitada->nro_pagina), pedido_serializado+offset, sizeof(int32_t));

	offset += sizeof(int32_t);
	memcpy(&(pagina_solicitada->tamanio_contenido), pedido_serializado+offset, sizeof(int32_t));

	pagina_solicitada->contenido=malloc(pagina_solicitada->tamanio_contenido);
	offset += sizeof(int32_t);
	memcpy(pagina_solicitada->contenido, pedido_serializado+offset, pagina_solicitada->tamanio_contenido);
	pagina_solicitada->contenido[pagina_solicitada->tamanio_contenido]= '\0';

	return pagina_solicitada;
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

