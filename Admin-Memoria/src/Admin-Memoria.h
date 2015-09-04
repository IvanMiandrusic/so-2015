/*
 * Admin-Memoria.h
 *
 *  Created on: 27/8/2015
 *      Author: utnso
 */
/*Header File*/

#ifndef ADMIN_MEMORIA_H_
#define ADMIN_MEMORIA_H_

#include "Comunicacion.h"

typedef struct estructura_configuracion			//estructura que contiene los datos del archivo de configuracion
{
  int32_t puerto_escucha;
  char* ip_swap;
  int32_t puerto_swap;
  int32_t maximo_marcos;
  int32_t cantidad_marcos;
  int32_t tamanio_marco;
  int32_t entradas_tlb;
  char* tlb_habilitada;
  int32_t retardo;
}ProcesoMemoria;

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

/** Funciones de configuracion inicial **/
ProcesoMemoria* crear_estructura_config(char*);
void inicializoSemaforos();
void crearArchivoDeLog();
void creoEstructurasDeManejo();
void crear_tabla_pagina_PID(int32_t , int32_t);
void dump();
/** Funciones de señales **/
void ifProcessDie();
void ifSigurs1();
void ifSigurs2();

/** Funciones de operaciones con CPU **/
void procesar_pedido(sock_t* socketCpu, header_t* header);
void iniciar_proceso(sock_t*, t_pedido_cpu*);
char* buscar_pagina(t_pagina*);
int32_t borrarEspacio(int32_t);

/** TLB functions **/
void TLB_init();
void TLB_flush();
bool TLB_habilitada();
t_resultado_busqueda TLB_buscar_pagina(t_pagina*, char**); //Recibe la direccion del char* donde se guardara el contenido de esa pagina


/** Funciones tabla_paginas y MP **/
t_resultado_busqueda buscar_pagina_tabla_paginas(int32_t, t_pagina*, char**);
t_resultado_busqueda obtener_pagina_MP(t_pagina*, char**);


#endif /* ADMIN_MEMORIA_H_ */
