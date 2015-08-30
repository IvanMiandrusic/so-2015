/*
 * Admin-Swap.h
 *
 *  Created on: 27/8/2015
 *      Author: utnso
 */
/*Header File*/
#ifndef ADMIN_SWAP_H_
#define ADMIN_SWAP_H_

typedef struct estructura_configuracion			//estructura que contiene los datos del archivo de configuracion
{
  int32_t puerto_escucha;
  char* nombre_swap;
  int32_t cantidad_paginas;
  int32_t tamanio_pagina;
  int32_t retardo;
}ProcesoSwap;

typedef struct estructura_nodo_libre			//estructura que contiene los datos de un hueco
{
 int32_t comienzo;
 int32_t paginas;
}NodoLibre;

typedef struct estructura_nodo_ocupado			//estructura que contiene los datos de un espacio ocupado
{
  int32_t PID;
  int32_t comienzo;
  int32_t paginas;
}NodoOcupado;


ProcesoSwap* crear_estructura_config(char*);
void ifProcessDie();
void inicializoSemaforos();
void crearArchivoDeLog();
void creoEstructuraSwap();
void compactar();




#endif /* ADMIN_SWAP_H_ */
