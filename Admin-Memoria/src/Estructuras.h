
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
  int32_t tiempo_referencia;   	//Para saber cual fue la primera en ingresar
}TLB;

typedef struct estructura_tabla_paginas			//estructura que contiene los datos de la tabla de paginas
{
  int32_t marco;
  int32_t pagina;
  int32_t modificada; 		//1 si, 0 no
  int32_t presente;			//Presente en MP (1 si, 0 no)
  int32_t tiempo_referencia;   	//Para saber cual fue la primera en ingresar
  int32_t bitUso;
  int32_t puntero;
}TPagina;

typedef struct metricas {
	int32_t PID;
	int32_t page_fault;
	int32_t accesos_memoria;
}t_metricas;

//Lista doblemente enlazada de procesos
typedef struct estructura_proceso_paginas {
	int32_t PID;
	t_list* paginas;
}t_paginas_proceso;

typedef enum resultado_operacion_busqueda_pagina {
	FOUND=1,
	NOT_FOUND=0,
	SEARCH_ERROR=-1
}t_resultado_busqueda;

typedef enum algoritmo_reemplazo {
	FIFO=1,
	LRU=2,
	CLOCK_MODIFICADO=3,
	INDEFINIDO=4
}t_algoritmo_reemplazo;

typedef enum operaciones_metricas {
	ACCESO = 1,
	PF = 2
}t_operacion_metrica;

/** Declaracion de variables globales **/
extern t_list* TLB_tabla;
extern char* mem_principal;
extern t_list* tabla_Paginas;
extern int32_t TLB_accesos;
extern int32_t TLB_hit;

/** Metrics functions **/
void metricas_create();
void metricas_destroy();

/** TLB functions **/
void TLB_create();
void TLB_destroy();
void TLB_init();
void TLB_flush();
bool TLB_habilitada();
t_resultado_busqueda TLB_buscar_pagina(int32_t, t_pagina*);
void TLB_refresh(int32_t, TPagina*);
void TLB_sort();
void TLB_clean_by_PID(int32_t);
void TLB_clean_by_page(int32_t, TPagina*);
bool TLB_exist(TPagina*);

/** Funciones tabla_paginas y MP **/
void MP_create();
void MP_destroy();
void tabla_paginas_create();
void tabla_paginas_destroy();
void tabla_paginas_refresh(TLB*);
int32_t tabla_paginas_clean(int32_t);
void frames_create();
void frames_init();
void frames_destroy();
void crear_tabla_pagina_PID(int32_t , int32_t);
t_resultado_busqueda buscar_pagina_tabla_paginas(int32_t, t_pagina*);
int32_t reemplazar_pagina(int32_t, t_list*);
int32_t obtener_frame_libre();
t_resultado_busqueda asignar_pagina(t_pagina*);

bool esClase0(int ,int);
bool esClase1(int ,int);
bool esClase2(int ,int);
bool esClase3(int ,int);
t_list* obtengoPaginasConPresencia(t_list* );

/** Utils **/
t_algoritmo_reemplazo obtener_codigo_algoritmo(char*);
t_resultado_busqueda escribir_pagina_modificada_en_swap(int32_t, TPagina*);
t_resultado_busqueda pedido_pagina_swap(t_pagina*, int32_t);
t_list* obtener_tabla_paginas_by_PID(int32_t );
TPagina* obtener_pagina_a_reemplazar(t_list* );
char* obtener_contenido_marco(TPagina*);
void sumar_metrica(t_operacion_metrica, int32_t);
t_metricas* obtener_metrica_PID(int32_t);
void mostrarEstadoActualEstructuras(int32_t, TPagina*);

#endif /* ESTRUCTURAS_H_ */
