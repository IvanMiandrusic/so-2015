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

/** FUNCIONES DE LA TLB **/
void TLB_create() {

	if (TLB_habilitada()) {
		TLB_tabla = list_create();
		TLB_init();
	}
}

void TLB_destroy() {

	if (TLB_habilitada()) {
		TLB_flush();
		list_destroy_and_destroy_elements(TLB_tabla, free);
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

	void limpiar_entradas(void* parametro) {
		TLB* entrada = (TLB*) parametro;
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
	log_info(loggerInfo, ANSI_COLOR_GREEN "Se realizo correctamente el TLB Flush" ANSI_COLOR_RESET);
}

bool TLB_habilitada() {

	return string_equals_ignore_case(arch->tlb_habilitada, "SI");
}

void TLB_buscar_pagina(t_pagina* pagina) {

	bool find_by_PID_page(void* parametro) {
		TLB* entrada = (TLB*) parametro;
		return (entrada->PID == pagina->PID) && (entrada->pagina == pagina->nro_pagina) && (entrada->presente==1);
	}

	TLB* entrada_pagina=NULL;

	if(TLB_habilitada()) {
		sem_wait(&sem_mutex_tlb);
		entrada_pagina = list_find(TLB_tabla, find_by_PID_page);
		sem_post(&sem_mutex_tlb);
	}

	if(entrada_pagina == NULL) {
		log_debug(loggerDebug, "No se encontro la pagina en la TLB, se busca en la tabla de paginas");
		buscar_pagina_tabla_paginas(pagina);
	}
	else{
		int32_t offset=entrada_pagina->marco*arch->tamanio_marco;
		memcpy(pagina->contenido, mem_principal+offset, arch->tamanio_marco);
	}

}

/** FUNCIONES DE TABLA DE PAGINAS Y MP **/

void MP_create() {

	mem_principal = malloc((arch->cantidad_marcos) * (arch->tamanio_marco));
}

void MP_destroy() {

	free(mem_principal);
}

void tabla_paginas_create() {

	tabla_Paginas = list_create();
}

void tabla_paginas_destroy() {

	void paginas_destroyer(void* parametro) {
		t_paginas_proceso* entrada = (t_paginas_proceso*) parametro;
		list_destroy_and_destroy_elements(entrada->paginas, free);
	}

	list_iterate(tabla_Paginas, paginas_destroyer);
	list_destroy(tabla_Paginas);
}

void crear_tabla_pagina_PID(int32_t processID, int32_t cantidad_paginas) {


	t_paginas_proceso* nueva_entrada_proceso = malloc(sizeof(t_paginas_proceso));
	nueva_entrada_proceso->PID = processID;
	nueva_entrada_proceso->paginas = list_create();

	int i;
	for (i = 0; i < cantidad_paginas; i++) {

		TPagina* nuevaEntrada = malloc(sizeof(TPagina));
		nuevaEntrada->marco = 0;
		nuevaEntrada->pagina = i;
		nuevaEntrada->modificada = 0;
		nuevaEntrada->presente = 0;
		nuevaEntrada->tiempo_uso = get_actual_time_integer();

		list_add(nueva_entrada_proceso->paginas, nuevaEntrada);
	}
	sem_wait(&sem_mutex_tabla_paginas);
	list_add(tabla_Paginas, nueva_entrada_proceso);
	sem_post(&sem_mutex_tabla_paginas);

	log_debug(loggerDebug,"Se reserva espacio para pid:%d paginas:%d", processID, cantidad_paginas);

}

void buscar_pagina_tabla_paginas(t_pagina* pagina) {

	bool obtenerTabPagina(void* parametro){
		t_paginas_proceso* entrada = (t_paginas_proceso*) parametro;
		return entrada->PID == pagina->PID;
	}

	t_paginas_proceso* tablaPagina=list_find(tabla_Paginas, obtenerTabPagina);

	if(tablaPagina!=NULL){

		bool obtenerMarco_Pagina(void* parametro){
			TPagina* entradaBuscada = (TPagina*) parametro;
			return entradaBuscada->pagina== pagina->nro_pagina && (entradaBuscada->presente==1);
		}

		TPagina* entradaFound = list_find(tablaPagina->paginas, obtenerMarco_Pagina);

		/** Si es LRU me interesa saber en que instante se utiliza la pag en MP **/
		if(string_equals_ignore_case("LRU", arch->algoritmo_reemplazo))
			entradaFound->tiempo_uso = get_actual_time_integer();

		int32_t offset=(entradaFound->marco)*(arch->tamanio_marco);
		memcpy(pagina->contenido, mem_principal+offset, arch->tamanio_marco);
		//Todo aca no falta devolver algo ?
	}
	else {
		log_debug(loggerDebug, "No se encontro en la tabla de paginas, se pide al swap");
		pedidoPagina_Swap(pagina);
		//todo leer;
	}

}

void pedidoPagina_Swap(t_pagina* pagina) {

		int32_t enviado;
		int32_t recibido;

		//Envio al swap para pedir la pagina
		header_t* headerSwap = _create_header(LEER_PAGINA,3 * sizeof(int32_t));
		enviado = _send_header(socketSwap, headerSwap);
		if (enviado == ERROR_OPERATION) return;
		free(headerSwap);

		enviado = _send_bytes(socketSwap, &(pagina->PID), sizeof(int32_t));
		if (enviado == ERROR_OPERATION) return;

		enviado = _send_bytes(socketSwap, &(pagina->nro_pagina), sizeof(int32_t));
		if (enviado == ERROR_OPERATION) return;

		pagina->tamanio_contenido = 0;
		enviado = _send_bytes(socketSwap, &(pagina->tamanio_contenido), sizeof(int32_t));
		if (enviado == ERROR_OPERATION) return;

		header_t* header_resultado_swap = _create_empty_header();
		recibido = _receive_header(socketSwap, header_resultado_swap);
		if(recibido == ERROR_OPERATION) return;

		if(get_operation_code(header_resultado_swap)==RESULTADO_OK) {

			recibido = _receive_bytes(socketSwap, &(pagina->tamanio_contenido), sizeof(int32_t));
			if (enviado == ERROR_OPERATION) return;

			recibido = _receive_bytes(socketSwap, pagina->contenido, sizeof(int32_t));
			if (enviado == ERROR_OPERATION) return;

			log_info(loggerInfo, ANSI_COLOR_BOLDGREEN "Exito al leer pagina en el swap" ANSI_COLOR_RESET);

			/** Se reemplaza la pagina en MP **/
			reemplazar_pagina(pagina);
	}
		else
			log_error(loggerError, ANSI_COLOR_BOLDRED "Error al leer pagina en el swap" ANSI_COLOR_RESET);

}

void reemplazar_pagina(t_pagina* pagina_recibida_swap) {

	t_algoritmo_reemplazo algoritmo_reemplazo = obtener_codigo_algoritmo(arch->algoritmo_reemplazo);

	/** Obtener tabla de paginas del PID **/
	t_list* paginas_PID = obtener_tabla_paginas_by_PID(pagina_recibida_swap->PID);

	switch(algoritmo_reemplazo) {

	case FIFO : {

		/** Saco primer pagina de la memoria **/
		TPagina* pagina_obtenida = obtener_pagina_a_reemplazar(paginas_PID);

		bool findPageToAbsent(void* parametro) {
			TPagina* entrada = (TPagina*) parametro;
			return entrada->pagina == pagina_obtenida->pagina;
		}

		TPagina* pagina_a_ausentar = list_find(paginas_PID, findPageToAbsent);
		pagina_a_ausentar->presente = 0;
		pagina_a_ausentar->tiempo_uso = 0;

		/** Todo Escribo pagina en swap (si esta modificada) **/

		/** Actualizo presencia de la pagina traida a memoria**/

		bool findPageToPresent(void* parametro) {
			TPagina* entrada = (TPagina*) parametro;
			return entrada->pagina == pagina_recibida_swap->nro_pagina;
		}

		TPagina* pagina_a_poner_presente = list_find(paginas_PID, findPageToPresent);
		pagina_a_poner_presente->presente = 1;
		pagina_a_poner_presente->tiempo_uso = get_actual_time_integer();


		break;
	}
	case LRU : {

		/** Saco ultima pagina que se uso **/
		TPagina* pagina_obtenida = obtener_pagina_a_reemplazar(paginas_PID);

		bool findPageToAbsent(void* parametro) {
			TPagina* entrada = (TPagina*) parametro;
			return entrada->pagina == pagina_obtenida->pagina;
		}

		TPagina* pagina_a_ausentar = list_find(paginas_PID, findPageToAbsent);
		pagina_a_ausentar->presente = 0;
		pagina_a_ausentar->tiempo_uso = 0;

		/** Todo Escribo pagina en swap (si esta modificada) **/


		/** Actualizo presencia de la pagina traida a memoria**/

		bool findPageToPresent(void* parametro) {
			TPagina* entrada = (TPagina*) parametro;
			return entrada->pagina == pagina_recibida_swap->nro_pagina;
		}

		TPagina* pagina_a_poner_presente = list_find(paginas_PID, findPageToPresent);
		pagina_a_poner_presente->presente = 1;
		pagina_a_poner_presente->tiempo_uso = get_actual_time_integer();

		break;
	}
	case CLOCK_MODIFICADO : {

			/*VER TODO*/
		break;
	}

	default: {break;}

	}
}

t_algoritmo_reemplazo obtener_codigo_algoritmo(char* algoritmo) {

	if(string_equals_ignore_case(algoritmo, "FIFO")) return FIFO;
	if(string_equals_ignore_case(algoritmo, "LRU")) return LRU;
	if(string_equals_ignore_case(algoritmo, "CLOCK_MODIFICADO")) return CLOCK_MODIFICADO;

	return INDEFINIDO;
}

t_list* obtener_tabla_paginas_by_PID(int32_t PID) {

	bool findByPID(void* parametro) {
		t_paginas_proceso* entrada = (t_paginas_proceso*) parametro;
		return entrada->PID == PID;
	}

	return list_find(tabla_Paginas, findByPID);

}

TPagina* obtener_pagina_a_reemplazar(t_list* paginas_del_proceso) {

	bool isPresent(void* parametro) {
		TPagina* entrada = (TPagina*) parametro;
		return entrada->presente;
	}

	t_list* paginas_en_MP = list_filter(paginas_del_proceso, isPresent);

	/** Comparator **/
	bool page_use_comparator(void* param1, void* param2) {
		TPagina* unaPag = (TPagina*) param1;
		TPagina* otraPag = (TPagina*) param2;
		return unaPag->tiempo_uso < otraPag->tiempo_uso;
	}

	list_sort(paginas_en_MP, page_use_comparator);

	TPagina* pagina_a_reemplazar = list_get(paginas_en_MP, 0);

	return pagina_a_reemplazar;

}
