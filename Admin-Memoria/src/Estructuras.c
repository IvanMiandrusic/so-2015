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

	sem_wait(&sem_mutex_tlb);
	TLB* entrada_pagina = list_find(TLB_tabla, find_by_PID_page);
	sem_post(&sem_mutex_tlb);

	if(entrada_pagina == NULL) {
		//Todo ir a buscar a tabla de paginas

	}

	else{
		//Todo Traer el contenido de MP
	}

	return NOT_FOUND; //depende si encuentra

}

t_resultado_busqueda buscar_pagina_tabla_paginas(int32_t PID, t_pagina* pagina, char** contenido) {

	//Todo buscar pagina en la tabla de paginas del PID
	return NOT_FOUND; //depende si encuentra
}

t_resultado_busqueda obtener_pagina_MP(t_pagina* pagina, char** contenido) {

	//Todo traer contenido de una pagina de la memoria principal
	return NOT_FOUND; //depende si encuentra

}
