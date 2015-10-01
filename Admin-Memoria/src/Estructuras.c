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
		nuevaEntrada->tiempo_referencia = 0;

		sem_wait(&sem_mutex_tlb);
		list_add(TLB_tabla, nuevaEntrada);
		sem_post(&sem_mutex_tlb);
	}
}

void TLB_flush() {

	void limpiar_entradas(void* parametro) {
		TLB* entrada = (TLB*) parametro;
		entrada->PID = 0;
		entrada->marco = 0;
		entrada->modificada = 0;
		entrada->pagina = 0;
		entrada->presente = 0;
		entrada->tiempo_referencia = 0;
	}

	sem_wait(&sem_mutex_tlb);
	list_iterate(TLB_tabla, limpiar_entradas);
	sem_post(&sem_mutex_tlb);
}

bool TLB_habilitada() {

	return string_equals_ignore_case(arch->tlb_habilitada, "SI");
}

t_resultado_busqueda TLB_buscar_pagina(int32_t cod_Operacion, t_pagina* pagina) {

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
		return buscar_pagina_tabla_paginas(cod_Operacion, pagina);
	}
	else{

		/** Hago el retardo que encontro la pagina **/
		sleep(arch->retardo);

		int32_t offset=entrada_pagina->marco*arch->tamanio_marco;
		if(cod_Operacion==LEER)	memcpy(pagina->contenido, mem_principal+offset, arch->tamanio_marco);
		if(cod_Operacion==ESCRIBIR){
			entrada_pagina->modificada=1;
			memcpy(mem_principal+offset,pagina->contenido, arch->tamanio_marco);
		}

		/** Se actualiza la tabla de paginas **/
		tabla_paginas_refresh(entrada_pagina);

		return FOUND;
	}

}

void TLB_refresh(int32_t PID, TPagina* pagina_a_actualizar) {

	/** Ordeno la TLB segun tiempo de ultima referencia **/
	TLB_sort();

	TLB* nueva_entrada = malloc(sizeof(TLB));
	nueva_entrada->PID = PID;
	nueva_entrada->marco = pagina_a_actualizar->marco;
	nueva_entrada->pagina = pagina_a_actualizar->pagina;
	nueva_entrada->modificada = pagina_a_actualizar->modificada;
	nueva_entrada->presente = pagina_a_actualizar->presente;
	nueva_entrada->tiempo_referencia = pagina_a_actualizar->tiempo_referencia;

	sem_wait(&sem_mutex_tlb);
	list_replace_and_destroy_element(TLB_tabla, 0, nueva_entrada, free);
	sem_post(&sem_mutex_tlb);

}

void TLB_sort() {

	/** Comparator **/
	bool page_use_comparator(void* param1, void* param2) {
		TLB* unaPag = (TLB*) param1;
		TLB* otraPag = (TLB*) param2;
		return unaPag->tiempo_referencia < otraPag->tiempo_referencia;
	}

	sem_wait(&sem_mutex_tlb);
	list_sort(TLB_tabla, page_use_comparator);
	sem_post(&sem_mutex_tlb);
}

void TLB_clean(int32_t PID) {

	void limpiar(void* parametro){
		TLB* entrada = (TLB*) parametro;
		if (entrada->PID==PID){
			entrada->PID=0;
			entrada->marco=0;
			entrada->modificada=0;
			entrada->pagina=0;
			entrada->presente=0;
			entrada->tiempo_referencia = 0;
		}
	}
	list_iterate(TLB_tabla, limpiar);
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

void tabla_paginas_refresh(TLB* entrada_tlb) {

	t_list* paginas_PID = obtener_tabla_paginas_by_PID(entrada_tlb->PID);

	bool find_by_ID(void* arg) {
		TPagina* pagina = (TPagina*) arg;
		return (pagina->pagina == entrada_tlb->pagina) && (pagina->marco == entrada_tlb->marco);
	}

	/** Actualizo la pagina en la tabla de paginas **/
	TPagina* pagina_a_modificar = list_find(paginas_PID, find_by_ID);
	pagina_a_modificar->modificada = entrada_tlb->modificada;

	/** Si es LRU, me interesa el tiempo de referencia **/
	if(string_equals_ignore_case("LRU", arch->algoritmo_reemplazo))
		pagina_a_modificar->tiempo_referencia = get_actual_time_integer();

}

int32_t tabla_paginas_clean(int32_t PID) {

	bool obtenerTabPagina(void* parametro){
		t_paginas_proceso* entrada = (t_paginas_proceso*) parametro;
		return entrada->PID==PID;
	}

	t_paginas_proceso* tablaPagina=list_find(tabla_Paginas, obtenerTabPagina);

	if(tablaPagina!=NULL){
		list_destroy_and_destroy_elements(tablaPagina->paginas, free);
		list_remove_and_destroy_by_condition(tabla_Paginas, obtenerTabPagina, free);
		return RESULTADO_OK	;
	}else return RESULTADO_ERROR;
}

void frames_create() {

	frames = malloc(sizeof(int32_t)*arch->cantidad_marcos);
	frames_init();
}

void frames_init() {

	int32_t i;
	for(i=0;i<arch->cantidad_marcos;i++) {
		frames[i] = 0;
	}
}

void frames_destroy() {

	free(frames);
}


void crear_tabla_pagina_PID(int32_t processID, int32_t cantidad_paginas) {


	t_paginas_proceso* nueva_entrada_proceso = malloc(sizeof(t_paginas_proceso));
	nueva_entrada_proceso->PID = processID;
	nueva_entrada_proceso->paginas = list_create();

	int i;
	for (i = 1; i <= cantidad_paginas; i++) {

		TPagina* nuevaEntrada = malloc(sizeof(TPagina));
		nuevaEntrada->marco = 0;
		nuevaEntrada->pagina = i;
		nuevaEntrada->modificada = 0;
		nuevaEntrada->presente = 0;
		nuevaEntrada->tiempo_referencia = 0;
		list_add(nueva_entrada_proceso->paginas, nuevaEntrada);
	}
	sem_wait(&sem_mutex_tabla_paginas);
	list_add(tabla_Paginas, nueva_entrada_proceso);
	sem_post(&sem_mutex_tabla_paginas);

	log_debug(loggerDebug,"Se reserva espacio para pid:%d paginas:%d", processID, cantidad_paginas);

}

t_resultado_busqueda buscar_pagina_tabla_paginas(int32_t codOperacion, t_pagina* pagina) {

	t_list* tabla_paginas_PID = obtener_tabla_paginas_by_PID(pagina->PID);

	bool obtenerMarco_Pagina(void* parametro){
		TPagina* entradaBuscada = (TPagina*) parametro;
		return entradaBuscada->pagina == pagina->nro_pagina && (entradaBuscada->presente == 1);
	}

	TPagina* entradaFound = list_find(tabla_paginas_PID, obtenerMarco_Pagina);

	if(entradaFound != NULL){

		/** Si es LRU me interesa saber en que instante se referencia la pag en MP **/
		if(string_equals_ignore_case("LRU", arch->algoritmo_reemplazo))
			entradaFound->tiempo_referencia = get_actual_time_integer();

		/** Hago el retardo si encuentra la pagina en la tabla de paginas **/
		sleep(arch->retardo);

		int32_t offset=(entradaFound->marco)*(arch->tamanio_marco);
		if(codOperacion==LEER) memcpy(pagina->contenido, mem_principal+offset, arch->tamanio_marco);
		if(codOperacion==ESCRIBIR){
			memcpy(mem_principal+offset, pagina->contenido, arch->tamanio_marco);
			entradaFound->modificada=1;
		}
		return FOUND;
	}
	else {
		log_debug(loggerDebug, "No se encontro en la tabla de paginas, se pide al swap");
		t_resultado_busqueda resultado = pedido_pagina_swap(pagina, LEER_PAGINA);

		if(resultado == FOUND) {
			log_debug(loggerDebug, "cargue pagina del swap");
			return buscar_pagina(codOperacion, pagina);
		}

		return NOT_FOUND;
	}

}

t_resultado_busqueda escribir_pagina_modificada_en_swap(int32_t PID, TPagina* pagina) {

	t_pagina* pagina_a_escribir = malloc(sizeof(t_pagina));
	pagina_a_escribir->PID = PID;
	pagina_a_escribir->nro_pagina = pagina->pagina;

	char* contenido_marco = obtener_contenido_marco(pagina);
	pagina_a_escribir->tamanio_contenido = strlen(contenido_marco);
	pagina_a_escribir->contenido = malloc(arch->tamanio_marco);
	memcpy(pagina_a_escribir, contenido_marco, pagina_a_escribir->tamanio_contenido);

	return pedido_pagina_swap(pagina_a_escribir, ESCRIBIR_PAGINA);
}

t_resultado_busqueda pedido_pagina_swap(t_pagina* pagina, int32_t operacion_swap) {

		int32_t enviado;
		int32_t recibido;
		int32_t tamanio;

		/** Envio al swap para pedir la pagina **/
		header_t* headerSwap;
		if(operacion_swap==LEER_PAGINA){
			headerSwap= _create_header(operacion_swap, 3 * sizeof(int32_t));
			tamanio=0;
		}else{
			headerSwap= _create_header(operacion_swap, 3 * sizeof(int32_t)+pagina->tamanio_contenido);
			tamanio=pagina->tamanio_contenido;
		}
		enviado = _send_header(socketSwap, headerSwap);
		if (enviado == ERROR_OPERATION) return SEARCH_ERROR;
		free(headerSwap);

		enviado = _send_bytes(socketSwap, &(pagina->PID), sizeof(int32_t));
		if (enviado == ERROR_OPERATION) return SEARCH_ERROR;

		enviado = _send_bytes(socketSwap, &(pagina->nro_pagina), sizeof(int32_t));
		if (enviado == ERROR_OPERATION) return SEARCH_ERROR;

		enviado = _send_bytes(socketSwap, &tamanio, sizeof(int32_t));
		if (enviado == ERROR_OPERATION) return SEARCH_ERROR;
		log_debug(loggerDebug, "se envia a leer del pid:%d, la pag:%d, tam:%d", pagina->PID, pagina->nro_pagina, tamanio);

		if(operacion_swap==ESCRIBIR_PAGINA){
			enviado = _send_bytes(socketSwap, pagina->contenido, pagina->tamanio_contenido);
			if (enviado == ERROR_OPERATION) return SEARCH_ERROR;
		}
		header_t* header_resultado_swap = _create_empty_header();
		recibido = _receive_header(socketSwap, header_resultado_swap);
		if(recibido == ERROR_OPERATION) return SEARCH_ERROR;

		t_pagina* pagina_Nueva = malloc(sizeof(t_pagina));
		pagina_Nueva->PID=pagina->PID;
		pagina_Nueva->nro_pagina=pagina->nro_pagina;

		if(operacion_swap==LEER_PAGINA) {

			if(get_operation_code(header_resultado_swap)==RESULTADO_OK) {

				recibido = _receive_bytes(socketSwap, &(pagina_Nueva->tamanio_contenido), sizeof(int32_t));
				if (enviado == ERROR_OPERATION) return SEARCH_ERROR;

				recibido = _receive_bytes(socketSwap, pagina_Nueva->contenido, pagina_Nueva->tamanio_contenido);
				if (enviado == ERROR_OPERATION) return SEARCH_ERROR;

				log_info(loggerInfo, ANSI_COLOR_BOLDGREEN "Exito al leer pagina en el swap" ANSI_COLOR_RESET);

				/** Se reemplaza la pagina en MP **/
				return asignar_pagina(pagina_Nueva);

			}

			else
				log_error(loggerError, ANSI_COLOR_BOLDRED "Error al leer pagina en el swap" ANSI_COLOR_RESET);

			return NOT_FOUND;
		}

		return FOUND;
}

t_resultado_busqueda asignar_pagina(t_pagina* pagina_recibida_swap) {

	int32_t marco_libre;
	log_debug(loggerDebug, "Debo buscar pagina a asignar");

	/** Obtener tabla de paginas del PID **/
	t_list* paginas_PID = obtener_tabla_paginas_by_PID(pagina_recibida_swap->PID);

	bool isPresent(void* parametro) {
		TPagina* entrada = (TPagina*) parametro;
		return entrada->presente;
	}

	int32_t presentes=list_count_satisfying(paginas_PID, isPresent);
	log_debug(loggerDebug, "Tengo con presencia:%d, en una lista con :%d paginas", presentes, list_size(paginas_PID));

	if(presentes < arch->maximo_marcos) {

		/** Obtengo frame libre para asignar pagina **/
		log_debug(loggerDebug, "Debo obtener frame libre");
		marco_libre = obtener_frame_libre();

		log_debug(loggerDebug, "Frame libre:%d", marco_libre);
		if(marco_libre==-1 && presentes > 0) marco_libre = reemplazar_pagina(pagina_recibida_swap->PID, paginas_PID);
	}
	else {
		marco_libre = reemplazar_pagina(pagina_recibida_swap->PID, paginas_PID);
	}

	/*
	 * Si por esas casualidades no me queda espacio ni tengo chance de reemplazar otra pagina del proceso
	 *  porque no tengo ninguna cargada en memoria, deberia finalizar el proceso con error
	 */

	if(marco_libre==-1 && presentes == 0)
		return finalizar_proceso_error(pagina_recibida_swap->PID);

	log_debug(loggerDebug, "Busco la pagina: %d", pagina_recibida_swap->nro_pagina);

	/** Actualizo presencia de la pagina traida a memoria**/
	bool findByID(void* parametro) {
		TPagina* entrada = (TPagina*) parametro;
		return entrada->pagina == pagina_recibida_swap->nro_pagina;
	}

	void mostrar(void* parametro) {
			TPagina* entrada = (TPagina*) parametro;
			log_debug(loggerDebug, "Pagina numero:%d, tiempo:%d", entrada->pagina, entrada->tiempo_referencia);
		}

	list_iterate(paginas_PID, mostrar);
	TPagina* pagina_a_poner_presente = list_find(paginas_PID, findByID);
	log_debug(loggerDebug, "Tengo pagina para cambiar:%d", pagina_a_poner_presente->pagina);
	pagina_a_poner_presente->presente = 1;
	log_debug(loggerDebug, "Tengo presente");
	pagina_a_poner_presente->marco = marco_libre;
	log_debug(loggerDebug, "Tengo marco");
	pagina_a_poner_presente->tiempo_referencia = get_actual_time_integer();
	log_debug(loggerDebug, "Tengo tiempo");
	pagina_a_poner_presente->modificada = 0;

	/** Actualizo contenido en la TLB **/
	TLB_refresh(pagina_recibida_swap->PID, pagina_a_poner_presente);

	return FOUND;
}

t_resultado_busqueda finalizar_proceso_error(int32_t PID) {

	int32_t recibido;

	header_t* headerSwap = _create_header(BORRAR_ESPACIO, 1 * sizeof(int32_t));
	int32_t enviado = _send_header(socketSwap, headerSwap);
	if (enviado == ERROR_OPERATION) return SEARCH_ERROR;

	free(headerSwap);

	enviado = _send_bytes(socketSwap, &(PID), sizeof(int32_t));
	if (enviado == ERROR_OPERATION)	return SEARCH_ERROR;

	log_debug(loggerDebug, "Envie al swap para finalizar el proceso:%d", PID);

	header_t* headerNuevo=_create_empty_header();
	recibido = _receive_header(socketSwap,headerNuevo);
	int32_t resultado_operacion=get_operation_code(headerNuevo);
	if (recibido == ERROR_OPERATION) return SEARCH_ERROR;

	log_debug(loggerDebug, "Recibo del swap la operacion: %d", resultado_operacion);

	/** Libero el espacio ocupado en memoria por el pid finalizado **/
	limpiar_Informacion_PID(PID);

	/** Todo de alguna forma avisarle a la CPU que finalize, aca no tengo forma de conocer el socket **/
	return FOUND;
}

int32_t obtener_frame_libre() {

	int32_t i;
	for(i=0;i<arch->cantidad_marcos;i++) {

		if(frames[i]==0) {
			frames[i] = 1;
			return i;
		}

	}
	return -1;
}

int32_t reemplazar_pagina(int32_t PID, t_list* paginas_PID) {

	int32_t marco_a_devolver;

	t_algoritmo_reemplazo algoritmo_reemplazo = obtener_codigo_algoritmo(arch->algoritmo_reemplazo);

	t_list* paginasConPresencia=obtengoPaginasConPresencia(paginas_PID);

	if((algoritmo_reemplazo == FIFO) || (algoritmo_reemplazo == LRU)) {

		/** Saco primer pagina de la memoria **/
		TPagina* pagina_obtenida = obtener_pagina_a_reemplazar(paginasConPresencia);

		bool findByID(void* parametro) {
			TPagina* entrada = (TPagina*) parametro;
			return entrada->pagina == pagina_obtenida->pagina;
		}

		TPagina* pagina_a_ausentar = list_find(paginas_PID, findByID);
		pagina_a_ausentar->presente = 0;
		marco_a_devolver = pagina_a_ausentar->marco;
		pagina_a_ausentar->marco = 0;
		pagina_a_ausentar->tiempo_referencia = 0;

		/** Puede que este en la TLB esa pagina que se ausente **/ //Todo
//		if(TLB_exist(pagina_a_ausentar))
//			TLB_remove(pagina_a_ausentar);

		/** Escribo pagina en swap (si esta modificada) **/
		if(pagina_a_ausentar->modificada == 1) {

			t_resultado_busqueda resultado = escribir_pagina_modificada_en_swap(PID, pagina_a_ausentar);

			/** Le saco el bit de modificada **/
			if(resultado == FOUND)
				pagina_a_ausentar->modificada = 0;
		}

		return marco_a_devolver;
	}
	else if(algoritmo_reemplazo == CLOCK_MODIFICADO) {

		int i=0;
		bool encontre_pagina_a_ausentar=false;
		TPagina* mejorPagina=list_get(paginasConPresencia,i);
		if(esClase0(mejorPagina)){
			encontre_pagina_a_ausentar=true;
		}
		while(encontre_pagina_a_ausentar==false){
			i++;
			TPagina* paginaObtenida=list_get(paginasConPresencia,i);
			if(esClase0(paginaObtenida)){
				mejorPagina=paginaObtenida;
				encontre_pagina_a_ausentar=true;
			}else if (esClase1(paginaObtenida)){
				if(esClase2(mejorPagina) || esClase3(mejorPagina) ) mejorPagina=paginaObtenida;
			}else if(esClase2(paginaObtenida)){
				if(esClase3(mejorPagina)) mejorPagina=paginaObtenida;
				paginaObtenida->bitUso=0;
				paginaObtenida->tiempo_referencia=get_actual_time_integer();
			}else if(esClase3(paginaObtenida)){
				paginaObtenida->bitUso=0;
				paginaObtenida->tiempo_referencia=get_actual_time_integer();
			}
				if (list_size(paginasConPresencia)==i) encontre_pagina_a_ausentar=true;
		}

		/** Escribo pagina en swap (si esta modificada) **/
		if(mejorPagina->modificada == 1) {

			t_pagina* pagina_a_escribir = malloc(sizeof(t_pagina));
			pagina_a_escribir->PID = PID;
			pagina_a_escribir->nro_pagina = mejorPagina->pagina;

			char* contenido_marco = obtener_contenido_marco(mejorPagina);
			pagina_a_escribir->tamanio_contenido = strlen(contenido_marco);
			pagina_a_escribir->contenido = malloc(arch->tamanio_marco);
			memcpy(pagina_a_escribir, contenido_marco, pagina_a_escribir->tamanio_contenido);

			t_resultado_busqueda resultado = pedido_pagina_swap(pagina_a_escribir, ESCRIBIR_PAGINA);

			/** Le saco el bit de modificada **/
			if(resultado == FOUND)
				mejorPagina->modificada = 0;
		}
		return mejorPagina->marco;
	}
	return ERROR_OPERATION;

}

bool esClase0(TPagina* pagina){
	return pagina->bitUso==0 && pagina->modificada==0;
}

bool esClase1(TPagina* pagina){
	return pagina->bitUso==0 && pagina->modificada==1;
}

bool esClase2(TPagina* pagina){
	return pagina->bitUso==1 && pagina->modificada==0;
}

bool esClase3(TPagina* pagina){
	return pagina->bitUso==1 && pagina->modificada==1;
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

	t_paginas_proceso* paginas_PID = list_find(tabla_Paginas, findByPID);

	return paginas_PID->paginas;

}

t_list* obtengoPaginasConPresencia(t_list* paginas_del_proceso){

	bool isPresent(void* parametro) {
			TPagina* entrada = (TPagina*) parametro;
			return entrada->presente;
		}

		t_list* paginas_en_MP = list_filter(paginas_del_proceso, isPresent);

		/** Comparator **/
		bool page_use_comparator(void* param1, void* param2) {
			TPagina* unaPag = (TPagina*) param1;
			TPagina* otraPag = (TPagina*) param2;
			return unaPag->tiempo_referencia < otraPag->tiempo_referencia;
		}

		list_sort(paginas_en_MP, page_use_comparator);
		return paginas_en_MP;

}

TPagina* obtener_pagina_a_reemplazar(t_list* paginas_en_MP) {

	TPagina* pagina_a_reemplazar = list_get(paginas_en_MP, 0);
	return pagina_a_reemplazar;

}

/** Obtiene el contenido de un marco de una pagina dada **/
char* obtener_contenido_marco(TPagina* pagina) {

	char* contenido_marco = malloc(arch->tamanio_marco);
	int32_t offset = pagina->marco*arch->tamanio_marco;
	memcpy(contenido_marco, mem_principal+offset, arch->tamanio_marco);
	return contenido_marco;

}
