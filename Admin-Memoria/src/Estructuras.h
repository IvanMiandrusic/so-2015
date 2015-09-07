/*
 * Estructuras.h
 *
 *  Created on: 05/09/2015
 *      Author: federico
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "Comunicacion.h"

typedef struct estructura_TLB			//estructura que contiene los datos de la tlb
{
  int32_t marco;
  int32_t modificada; 		//1 si, 0 no
  int32_t presente;			//Presente en MP (1 si, 0 no)
  int32_t pagina;
  int32_t PID;
}TLB;

typedef struct estructura_tabla_paginas			//estructura que contiene los datos de la tabla de paginas
{
  int32_t marco;
  int32_t pagina;
  int32_t modificada; 		//1 si, 0 no
  int32_t presente;			//Presente en MP (1 si, 0 no)
}TPagina;

//Lista doblemente enlazada de procesos
typedef struct estructura_proceso_paginas {
	int32_t PID;
	t_list* paginas;
}t_paginas_proceso;

typedef enum resultado_operacion_busqueda_pagina {
	FOUND=1,
	NOT_FOUND=0
}t_resultado_busqueda;

/** Declaracion de variables globales **/
extern t_list* TLB_tabla;
extern char* mem_principal;
extern t_list* tabla_Paginas;

/** TLB functions **/
void TLB_crear();
void TLB_init();
void TLB_flush();
bool TLB_habilitada();
t_resultado_busqueda TLB_buscar_pagina(t_pagina*, char**); //Recibe la direccion del char* donde se guardara el contenido de esa pagina
t_resultado_busqueda pedidoPagina_Swap(t_pagina* , char** );

/** Funciones tabla_paginas y MP **/
void MP_crear();
void crear_tabla_pagina_PID(int32_t , int32_t);
t_resultado_busqueda buscar_pagina_tabla_paginas(t_pagina*, char**);


#endif /* ESTRUCTURAS_H_ */
