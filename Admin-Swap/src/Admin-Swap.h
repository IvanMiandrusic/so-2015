/*Header File*/
#ifndef ADMIN_SWAP_H_
#define ADMIN_SWAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include "libsocket.h"
#include "Comunicacion.h"


typedef struct estructura_configuracion			//estructura que contiene los datos del archivo de configuracion
{
  int32_t puerto_escucha;
  char* nombre_swap;
  int32_t cantidad_paginas;
  int32_t tamanio_pagina;
  int32_t retardo;
  int32_t retardo_comp;
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

typedef struct pedido_memoria {
	int32_t pid;
	int32_t cantidad_paginas;
}t_pedido_memoria;

typedef struct estructura_metricas			//estructura que contiene los datos lecturas y escrituras
{
 int32_t PID;
 int32_t lecturas;
 int32_t escrituras;
}t_metrica;

void crear_estructura_config(char*);
void ifProcessDie();
void inicializoSemaforos();
void crearArchivoDeLog();
void creoEstructuraSwap();
int32_t compactar();
int32_t calcularEspacioLibre();
void graficoCompactar();
void leer_pagina(t_pagina*);
int32_t reservarEspacio(t_pedido_memoria* );
int32_t borrarEspacio(int32_t );
int32_t escribir_pagina(t_pagina* );
void limpiar_estructuras_swap();
void recibir_operaciones_memoria(sock_t*);
FILE* abrirArchivoConTPagina(t_pagina*);
void agregarMetrica(int32_t, int32_t );
#endif /* ADMIN_SWAP_H_ */
