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
int32_t TLB_accesos;
int32_t TLB_hit;
t_list* metricas;

void* thread_hit(void* arg){

	int32_t tasa_acierto;

	while(true){

		sleep(60);

		if(TLB_accesos == 0){
			tasa_acierto = 0;
		}else{
			tasa_acierto = (TLB_hit*100)/TLB_accesos;
		}
		log_info(loggerInfo,ANSI_COLOR_BOLDYELLOW "La tasa de aciertos de la TLB es:%d%% (%d/%d)" ANSI_COLOR_RESET, tasa_acierto, TLB_hit, TLB_accesos);

		tasa_acierto = 0;
	}
	return NULL;
}

/** FUNCIONES DE METRICAS **/

void metricas_create() {

	metricas = list_create();
}

void metricas_destroy() {

	list_destroy_and_destroy_elements(metricas, free);
}

/** FUNCIONES DE LA TLB **/
void TLB_create() {

	if (TLB_habilitada()) {
		TLB_tabla = list_create();
		TLB_init();
	}
	/** Creo hilo de tasa de aciertos TLB **/
	if(TLB_habilitada()) {
		pthread_t TLB_aciertos;
		int32_t resultado_acierto = pthread_create(&TLB_aciertos, NULL, thread_hit, NULL);
		if (resultado_acierto != 0) {
			log_error(loggerError, ANSI_COLOR_RED "Error al crear el hilo de aciertos de TLB "ANSI_COLOR_RESET);
			abort();
		}
	}
}

void TLB_destroy() {

	if (TLB_habilitada()) {
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

	/** Inicializo contadores globales **/
	TLB_accesos = 0;
	TLB_hit = 0;
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
		/** Sumo un acceso a la TLB **/
		TLB_accesos++;

		sem_wait(&sem_mutex_tlb);
		entrada_pagina = list_find(TLB_tabla, find_by_PID_page);
		sem_post(&sem_mutex_tlb);
	}

	if(entrada_pagina == NULL) {
		if(TLB_habilitada()) log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "TLB Miss" ANSI_COLOR_RESET);
		log_debug(loggerDebug, "No se encontro la pagina en la TLB, se busca en la tabla de paginas");
		return buscar_pagina_tabla_paginas(cod_Operacion, pagina);
	}
	else{
		/** Hago el retardo que encontro la pagina **/
		sleep(arch->retardo);
		log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "TLB HIT! Pagina:%d->Marco:%d" ANSI_COLOR_RESET, pagina->nro_pagina, entrada_pagina->marco);
		/** Hubo un TLB_HIT, se encontro la pagina **/
		TLB_hit++;
		sumar_metrica(ACCESO, pagina->PID);

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
	log_debug(loggerDebug, "Cargo en TLB pid: %d, pagina:%d, presencia:%d", PID, pagina_a_actualizar->pagina, pagina_a_actualizar->presente);
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

void TLB_clean_by_PID(int32_t PID) {

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

bool TLB_exist(TPagina* pagina) {

	bool page_exist(void* parametro) {
		TLB* entrada = (TLB*) parametro;
		return entrada->pagina == pagina->pagina;
	}

	return list_any_satisfy(TLB_tabla, page_exist);
}

void TLB_clean_by_page(TPagina* pagina) {

	void limpiar_entrada(void* parametro){
		TLB* entrada = (TLB*) parametro;
		if (entrada->pagina == pagina->pagina){
			entrada->PID=0;
			entrada->marco=0;
			entrada->modificada=0;
			entrada->pagina=0;
			entrada->presente=0;
			entrada->tiempo_referencia = 0;
		}
	}
	list_iterate(TLB_tabla, limpiar_entrada);
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
	pagina_a_modificar->bitUso=1;

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

	/** Creo un elemento de metricas de ese mProc **/
	t_metricas* metrica = malloc(sizeof(t_metricas));
	metrica->PID = processID;
	metrica->accesos_memoria = 0;
	metrica->page_fault = 0;
	list_add(metricas, metrica);

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
		nuevaEntrada->tiempo_referencia = 0;
		nuevaEntrada->bitUso=0;
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

		log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "Acceso a Memoria Pid: %d - Pagina:%d->Marco:%d" ANSI_COLOR_RESET,pagina->PID, pagina->nro_pagina, entradaFound->marco);

		/** Sumo acceso a ese PID **/
		sumar_metrica(ACCESO, pagina->PID);

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
		log_debug(loggerDebug, "No se encontro en la tabla de paginas, se pide al swap, pagina id:%d nro:%d", pagina->PID, pagina->nro_pagina);
		t_resultado_busqueda resultado = pedido_pagina_swap(pagina, LEER_PAGINA);

		/** Ocurre un PF en la memoria **/
		sumar_metrica(PF, pagina->PID);

		if(resultado == FOUND) {
			return TLB_buscar_pagina(codOperacion, pagina);
		}

		return NOT_FOUND;
	}

}

t_resultado_busqueda escribir_pagina_modificada_en_swap(int32_t PID, TPagina* pagina) {
	log_debug(loggerDebug, "Debo reemplzar la pagina del pid:%d", PID);
	t_pagina* pagina_a_escribir = malloc(sizeof(t_pagina));
	pagina_a_escribir->PID = PID;
	pagina_a_escribir->nro_pagina = pagina->pagina;

	char* contenido_marco = obtener_contenido_marco(pagina);
	pagina_a_escribir->tamanio_contenido = string_length(contenido_marco);
	pagina_a_escribir->contenido = malloc(arch->tamanio_marco);
	memcpy(pagina_a_escribir->contenido, contenido_marco, pagina_a_escribir->tamanio_contenido);

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
			log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "Ocurrio un FALLO DE PAGINA" ANSI_COLOR_RESET);
			log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "Se envia al SWAP a leer del mProc:%d, la pag:%d, tam:%d" ANSI_COLOR_RESET, pagina->PID, pagina->nro_pagina, tamanio);
		}else{
			headerSwap= _create_header(operacion_swap, 3 * sizeof(int32_t) + pagina->tamanio_contenido);
			tamanio=pagina->tamanio_contenido;
			pagina->contenido[tamanio]='\0';
			log_info(loggerInfo, "Se envia al Swap a escribir del pid:%d, la pag:%d, tam:%d, contenido:%s", pagina->PID, pagina->nro_pagina, tamanio, pagina->contenido);
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

		if(operacion_swap==ESCRIBIR_PAGINA){
			enviado = _send_bytes(socketSwap, pagina->contenido, pagina->tamanio_contenido);
			if (enviado == ERROR_OPERATION) return SEARCH_ERROR;
		}
		header_t* header_resultado_swap = _create_empty_header();
		recibido = _receive_header(socketSwap, header_resultado_swap);
		if(recibido == ERROR_OPERATION) return SEARCH_ERROR;



		if(operacion_swap==LEER_PAGINA) {

			if(get_operation_code(header_resultado_swap)==RESULTADO_OK) {

				t_pagina* pagina_Nueva = malloc(sizeof(t_pagina));
				pagina_Nueva->PID=pagina->PID;
				pagina_Nueva->nro_pagina=pagina->nro_pagina;
				recibido = _receive_bytes(socketSwap, &(pagina_Nueva->tamanio_contenido), sizeof(int32_t));
				if (enviado == ERROR_OPERATION) return SEARCH_ERROR;
				if(pagina_Nueva->tamanio_contenido>0){
					pagina_Nueva->contenido = malloc(1 + pagina_Nueva->tamanio_contenido);
					recibido = _receive_bytes(socketSwap, pagina_Nueva->contenido, pagina_Nueva->tamanio_contenido);
					if (enviado == ERROR_OPERATION) return SEARCH_ERROR;
					pagina_Nueva->contenido[pagina_Nueva->tamanio_contenido]='\0';
				}else{
					pagina_Nueva->contenido=NULL;
				}

				log_info(loggerInfo, ANSI_COLOR_BOLDCYAN "Exito al leer pagina en el swap: tamanio:%d, contenido:%s" ANSI_COLOR_RESET, pagina_Nueva->tamanio_contenido, pagina_Nueva->contenido);

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
	log_debug(loggerDebug, "Debo buscar pagina a asignar del pid: %d", pagina_recibida_swap->PID);

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

	if(marco_libre==-1 && presentes == 0)
		return NOT_FOUND;

	log_debug(loggerDebug, "Busco la pagina: %d", pagina_recibida_swap->nro_pagina);

	/** Actualizo presencia de la pagina traida a memoria**/
	bool findByID(void* parametro) {
		TPagina* entrada = (TPagina*) parametro;
		return entrada->pagina == pagina_recibida_swap->nro_pagina;
	}

	TPagina* pagina_a_poner_presente = list_find(paginas_PID, findByID);
	log_debug(loggerDebug, "Tengo pagina para cambiar:%d", pagina_a_poner_presente->pagina);
	log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "Resultado del algoritmo %s, se sustituye el marco %d"ANSI_COLOR_RESET, arch->algoritmo_reemplazo, marco_libre);
	pagina_a_poner_presente->presente = 1;
	pagina_a_poner_presente->marco = marco_libre;
	pagina_a_poner_presente->tiempo_referencia = get_actual_time_integer();
	pagina_a_poner_presente->modificada = 0;

	int32_t offset=marco_libre * arch->tamanio_marco;
	if(pagina_recibida_swap->contenido==NULL){
		log_debug(loggerDebug, "Tengo contenido nulo");
		char* texto=string_repeat('\0', arch->tamanio_marco);
		memcpy(mem_principal+offset,texto,arch->tamanio_marco);
	}else{
		log_debug(loggerDebug, "Tengo contenido no nulo:%s", pagina_recibida_swap->contenido);
		int32_t diferencia= arch->tamanio_marco-pagina_recibida_swap->tamanio_contenido;
		log_debug(loggerDebug, "La diferencia sería:%d (%d - %d", diferencia, arch->tamanio_marco,pagina_recibida_swap->tamanio_contenido );
		char* texto=string_repeat('\0', diferencia);
		string_append(&(pagina_recibida_swap->contenido), texto);
		log_debug(loggerDebug,"Resultado del append:%s", pagina_recibida_swap->contenido);
		memcpy(mem_principal+offset,pagina_recibida_swap->contenido,arch->tamanio_marco);
	}

	/** Actualizo contenido en la TLB **/
	if(TLB_habilitada()) TLB_refresh(pagina_recibida_swap->PID, pagina_a_poner_presente);

	/** Muestro como quedaron las estructuras luego del reemplazo **/
	mostrarEstadoActualEstructuras(pagina_recibida_swap->PID, pagina_a_poner_presente);

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
		log_debug(loggerDebug, "Algoritmo LRU");

		/** Saco primer pagina de la memoria **/
		TPagina* pagina_obtenida = obtener_pagina_a_reemplazar(paginasConPresencia);

		bool findByID(void* parametro) {
			TPagina* entrada = (TPagina*) parametro;
			return entrada->pagina == pagina_obtenida->pagina;
		}

		TPagina* pagina_a_ausentar = list_find(paginas_PID, findByID);
		pagina_a_ausentar->presente = 0;
		marco_a_devolver = pagina_a_ausentar->marco;
		pagina_a_ausentar->tiempo_referencia = 0;

		/** Puede que este en la TLB esa pagina que se ausente **/
		if(TLB_exist(pagina_a_ausentar))
			TLB_clean_by_page(pagina_a_ausentar);
		log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "Se reemplaza por algoritmo %s, deja de estar en memoria la pagina %d del mProc %d" ANSI_COLOR_RESET, arch->algoritmo_reemplazo, pagina_a_ausentar->pagina, PID);

		/** Escribo pagina en swap (si esta modificada) **/
		if(pagina_a_ausentar->modificada == 1) {

			t_resultado_busqueda resultado = escribir_pagina_modificada_en_swap(PID, pagina_a_ausentar);

			/** Le saco el bit de modificada **/
			if(resultado == FOUND)
				pagina_a_ausentar->modificada = 0;
		}
		pagina_a_ausentar->marco = 0;

		return marco_a_devolver;
	}
	else if(algoritmo_reemplazo == CLOCK_MODIFICADO) {
		log_debug(loggerDebug, "Algoritmo CLOCK-M");
		log_debug(loggerDebug, "Cantidad de paginas con presencia:%d", list_size(paginasConPresencia));

		int i=0;
		bool encontre_pagina_a_ausentar=false;
		TPagina* paginaObtenida=list_get(paginasConPresencia,i);
		int indiceMejorPagina=i;

		while(encontre_pagina_a_ausentar==false){
			if(esClase0(i,paginasConPresencia)){
				indiceMejorPagina=i;
				encontre_pagina_a_ausentar=true;
			}else if (esClase1(i,paginasConPresencia)){
				if(esClase2(indiceMejorPagina,paginasConPresencia) || esClase3(indiceMejorPagina,paginasConPresencia)) indiceMejorPagina=i;
			}else if(esClase2(i,paginasConPresencia)){
				if(esClase3(indiceMejorPagina,paginasConPresencia)) indiceMejorPagina=i;
				paginaObtenida->bitUso=0;
			}else if(esClase3(i,paginasConPresencia)){
				paginaObtenida->bitUso=0;
			}
			i++;
			if (list_size(paginasConPresencia)==(i)){
				encontre_pagina_a_ausentar=true;
			}else{
				paginaObtenida=list_get(paginasConPresencia,i);
			}
		}

		TPagina* mejorPagina=list_get(paginasConPresencia,indiceMejorPagina);
		printf("la mejor pagina es: %d",mejorPagina->pagina );
		int asd;
		for (asd=0;asd<list_size(paginasConPresencia);asd++){
			TPagina* pagina=list_get(paginasConPresencia, asd);
			printf(ANSI_COLOR_BOLDBLUE"\nNro: %d, marco:%d, tiempo:%d, bitUso:%d, modificada:%d \n"ANSI_COLOR_RESET, pagina->pagina, pagina->marco, pagina->tiempo_referencia,pagina->bitUso,pagina->modificada);
		}

		log_info(loggerInfo, "Por algoritmo %s, deja de estar en memoria la pagina %d", arch->algoritmo_reemplazo, mejorPagina->pagina);

		/** Escribo pagina en swap (si esta modificada) **/
		if(mejorPagina->modificada == 1) {

			t_pagina* pagina_a_escribir = malloc(sizeof(t_pagina));
			pagina_a_escribir->PID = PID;
			pagina_a_escribir->nro_pagina = mejorPagina->pagina;

			char* contenido_marco = obtener_contenido_marco(mejorPagina);
			pagina_a_escribir->tamanio_contenido = string_length(contenido_marco);
			pagina_a_escribir->contenido = malloc(arch->tamanio_marco);
			memcpy(pagina_a_escribir->contenido, contenido_marco, pagina_a_escribir->tamanio_contenido);

			log_debug(loggerDebug, "Debo reemplzar la pagina del pid:%d", PID);
			t_resultado_busqueda resultado = pedido_pagina_swap(pagina_a_escribir, ESCRIBIR_PAGINA);

			/** Le saco el bit de modificada **/
			if(resultado == FOUND)
				mejorPagina->modificada = 0;
		}
		mejorPagina->presente=0;
		return mejorPagina->marco;
	}
	return ERROR_OPERATION;

}

bool esClase0(int indice,t_list* paginasConPresencia){
	TPagina* pagina=list_get(paginasConPresencia,indice);
	return pagina->bitUso==0 && pagina->modificada==0;
}

bool esClase1(int indice,t_list* paginasConPresencia){
	TPagina* pagina=list_get(paginasConPresencia,indice);
	return pagina->bitUso==0 && pagina->modificada==1;
}

bool esClase2(int indice,t_list* paginasConPresencia){
	TPagina* pagina=list_get(paginasConPresencia,indice);
	return pagina->bitUso==1 && pagina->modificada==0;
}

bool esClase3(int indice,t_list* paginasConPresencia){
	TPagina* pagina=list_get(paginasConPresencia,indice);
	return pagina->bitUso==1 && pagina->modificada==1;
}

t_algoritmo_reemplazo obtener_codigo_algoritmo(char* algoritmo) {

	if(string_equals_ignore_case(algoritmo, "FIFO")) return FIFO;
	if(string_equals_ignore_case(algoritmo, "LRU")) return LRU;
	if(string_equals_ignore_case(algoritmo, "CLOCK-M")) return CLOCK_MODIFICADO;

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

void sumar_metrica(t_operacion_metrica cod_op, int32_t PID) {

	bool find_metrica_by_PID(void* arg) {
		t_metricas* metrica = (t_metricas*) arg;
		return metrica->PID == PID;
	}

	t_metricas* metrica_pid = list_find(metricas, find_metrica_by_PID);

	if(cod_op == ACCESO) metrica_pid->accesos_memoria++;
	if(cod_op == PF) metrica_pid->page_fault++;

}

t_metricas* obtener_metrica_PID(int32_t PID) {

	bool find_metrica_by_PID(void* arg) {
		t_metricas* metrica = (t_metricas*) arg;
		return metrica->PID == PID;
	}

	return (t_metricas*) list_remove_by_condition(metricas, find_metrica_by_PID);
}

void mostrarEstadoActualEstructuras(int32_t PID, TPagina* pagina) {

	log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "Estado actual de estructuras en memoria" ANSI_COLOR_RESET);
	log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "Tabla de Paginas del mProc %d" ANSI_COLOR_RESET,PID);

	void mostrarEntradaTablaPaginas(TPagina* entrada) {

			if(entrada->pagina == pagina->pagina) {
				log_info(loggerInfo, ANSI_COLOR_BOLDGREEN "Pagina %d - Marco %d - Uso %d - Modificada %d - Presente %d - Tiempo Ultima Referencia %d" ANSI_COLOR_RESET,
						entrada->pagina,
						entrada->marco,
						entrada->bitUso,
						entrada->modificada,
						entrada->presente,
						entrada->tiempo_referencia);
			}
			else {
				log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "Pagina %d - Marco %d - Uso %d - Modificada %d - Presente %d - Tiempo Ultima Referencia %d" ANSI_COLOR_RESET,
						entrada->pagina,
						entrada->marco,
						entrada->bitUso,
						entrada->modificada,
						entrada->presente,
						entrada->tiempo_referencia);
			}
		}
	t_list* tabla_paginas_PID = obtener_tabla_paginas_by_PID(PID);
	list_iterate(tabla_paginas_PID, mostrarEntradaTablaPaginas);

	log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "TLB" ANSI_COLOR_RESET);

	void mostrarEntradaTLB(void* arg) {

				TLB* entrada = (TLB*) arg;
				if(entrada->pagina == pagina->pagina) {
					log_info(loggerInfo, ANSI_COLOR_BOLDGREEN "mProc %d - Pagina %d - Marco %d - Modificada %d - Presente %d - Tiempo Ultima Referencia %d" ANSI_COLOR_RESET,
							PID,
							entrada->pagina,
							entrada->marco,
							entrada->modificada,
							entrada->presente,
							entrada->tiempo_referencia);
				}
				else {
					log_info(loggerInfo, ANSI_COLOR_BOLDYELLOW "mProc %d - Pagina %d - Marco %d - Modificada %d - Presente %d - Tiempo Ultima Referencia %d" ANSI_COLOR_RESET,
							PID,
							entrada->pagina,
							entrada->marco,
							entrada->modificada,
							entrada->presente,
							entrada->tiempo_referencia);
				}
	}

	if(TLB_habilitada())list_iterate(TLB_tabla, mostrarEntradaTLB);
}
