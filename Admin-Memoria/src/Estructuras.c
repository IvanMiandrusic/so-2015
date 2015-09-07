/*
 * Estructuras.c
 *
 *  Created on: 05/09/2015
 *      Author: federico
 */

#include <stdio.h>
#include <stdlib.h>
#include "Estructuras.h"
#include "Admin-Memoria.h"


/** Variables globales **/
t_list* TLB_tabla;
char* mem_principal;
t_list* tabla_Paginas;

void MP_crear() {

	mem_principal = malloc((arch->cantidad_marcos) * (arch->tamanio_marco));
}

void TLB_crear() {

	if (string_equals_ignore_case((arch->tlb_habilitada), "si")) {
		TLB_tabla = list_create();
		TLB_init();
	}
}

void TLB_init() {

	int i;
	for (i = 0; i < arch->entradas_tlb; i++) {

		TLB* nuevaEntrada = malloc(sizeof(TLB));
		nuevaEntrada->PID = 0;
		nuevaEntrada->pagina = 0;
		nuevaEntrada->modificada = 0;
		nuevaEntrada->presente = 0;
		nuevaEntrada->marco = 0;

		sem_wait(&sem_mutex_tlb);
		list_add(TLB_tabla, nuevaEntrada);
		sem_post(&sem_mutex_tlb);
	}
}

void TLB_flush() {

	void limpiar_entradas(TLB* entrada) {
		//todo si modificada es 1, le mandas al swap escribir(PID, pagina, texto)
		//o hacerlo con la tabla de paginas
		entrada->PID = 0;
		entrada->marco = 0;
		entrada->modificada = 0;
		entrada->pagina = 0;
		entrada->presente = 0;
	}

	sem_wait(&sem_mutex_tlb);
	list_iterate(TLB_tabla, limpiar_entradas);
	sem_post(&sem_mutex_tlb);
}

void crear_tabla_pagina_PID(int32_t processID, int32_t cantidad_paginas) {

	t_paginas_proceso* nueva_entrada_proceso = malloc(
			sizeof(t_paginas_proceso));
	nueva_entrada_proceso->PID = processID;
	nueva_entrada_proceso->paginas = list_create();

	int i;
	for (i = 0; i < cantidad_paginas; i++) {

		TPagina* nuevaEntrada = malloc(sizeof(TPagina));
		nuevaEntrada->marco = 0;
		nuevaEntrada->pagina = i;
		nuevaEntrada->modificada = 0;
		nuevaEntrada->presente = 0;

		list_add(nueva_entrada_proceso->paginas, nuevaEntrada);
	}

	sem_wait(&sem_mutex_tabla_paginas);
	list_add(tabla_Paginas, nueva_entrada_proceso);
	sem_post(&sem_mutex_tabla_paginas);

}

bool TLB_habilitada() {

	return string_equals_ignore_case(arch->tlb_habilitada, "SI");
}

t_resultado_busqueda TLB_buscar_pagina(t_pagina* pagina, char** contenido_pagina) {

	bool find_by_PID_page(TLB* entrada) {

		return (entrada->PID == pagina->PID) && (entrada->pagina == pagina->nro_pagina);
	}
	TLB* entrada_pagina=NULL;
	if(TLB_habilitada()) {
		sem_wait(&sem_mutex_tlb);
		entrada_pagina = list_find(TLB_tabla, find_by_PID_page);
		sem_post(&sem_mutex_tlb);
	}
	if(entrada_pagina == NULL) {
		log_debug(loggerDebug, "No se encontro la pagina en la TLB, se busca en la tabla de paginas");
		return buscar_pagina_tabla_paginas(pagina, contenido_pagina); //todo porque con & ?
	}
	else{
		int32_t offset=entrada_pagina->marco*arch->tamanio_marco;
		memcpy(contenido_pagina, mem_principal+offset, arch->tamanio_marco);
		return FOUND;
		//Todo Traer el contenido de MP
	}

}

t_resultado_busqueda buscar_pagina_tabla_paginas(t_pagina* pagina, char** contenido) {

	bool obtenerTabPagina(t_paginas_proceso* entrada){
		return entrada->PID == pagina->PID;
	}

	t_paginas_proceso* tablaPagina=list_find(tabla_Paginas, obtenerTabPagina);
	if(tablaPagina!=NULL){
		bool obtenerMarco_Pagina(TPagina* entradaBuscada){
				return entradaBuscada->pagina== pagina->nro_pagina;
			}

		TPagina* entradaFound = list_find(tablaPagina->paginas, obtenerMarco_Pagina);
		int32_t offset=(entradaFound->marco)*(arch->tamanio_marco);
		memcpy(contenido, mem_principal+offset, arch->tamanio_marco); 		//todo va con &?
		return FOUND;
	}else {
		log_debug(loggerDebug, "No se encontro en la tabla de paginas, se pide al swap");
		return pedidoPagina_Swap(pagina, contenido);
	}

	//Todo buscar pagina en la tabla de paginas del PID
}

t_resultado_busqueda pedidoPagina_Swap(t_pagina* pagina, char** contenido) {

		int32_t enviado;
		int32_t recibido;
		//Envio al swap para pedir la pagina
		header_t* headerSwap = _create_header(LEER_PAGINA,2 * sizeof(int32_t));
		enviado = _send_header(socketSwap, headerSwap);
		if (enviado == ERROR_OPERATION) return NOT_FOUND;
		free(headerSwap);

		enviado = _send_bytes(socketSwap, &(pagina->PID), sizeof(int32_t));
		if (enviado == ERROR_OPERATION) return NOT_FOUND;

		enviado = _send_bytes(socketSwap, &(pagina->nro_pagina), sizeof(int32_t));
		if (enviado == ERROR_OPERATION) return NOT_FOUND;

		//todo recibir del swap la pagina y guardarla en contenido
		//recibido = _receive_bytes(socketSwap, &(resultado_operacion), sizeof(int32_t));
		//if (recibido == ERROR_OPERATION) return NOT_FOUND;
		//if (la encontre) return FOUND
		return NOT_FOUND; //depende si encuentra

}
