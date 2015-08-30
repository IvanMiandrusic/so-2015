/*
 * Admin-Memoria.h
 *
 *  Created on: 27/8/2015
 *      Author: utnso
 */
/*Header File*/

#ifndef ADMIN_MEMORIA_H_
#define ADMIN_MEMORIA_H_

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
  int32_t valida;			//1 si, 0 no
}TLB;

typedef struct estructura_tabla_paginas			//estructura que contiene los datos de la tabla de paginas
{
  int32_t marco;
  int32_t pagina;
}TPagina;


ProcesoMemoria* crear_estructura_config(char*);
void ifProcessDie();
void inicializoSemaforos();
void crearArchivoDeLog();
void creoEstructurasDeManejo();
void llenoTLB();
void llenoTPag();
void ifSigurs1();
void ifSigurs2();


#endif /* ADMIN_MEMORIA_H_ */
