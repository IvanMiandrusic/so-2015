/*
 * Comunicacion.h
 *
 *  Created on: 30/08/2015
 *      Author: federico
 */

#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct pedido_cpu {
	int32_t pid;
	int32_t cantidad_paginas;
}t_pedido_cpu;

/** Codigos de operacion en los envios hacia el swap **/
#define LEER_PAGINA 1
#define ESCRIBIR_PAGINA 2
#define RESERVAR_ESPACIO 3
#define BORRAR_ESPACIO 4
#define RECONEXION 5

/** Codigos de operacion en las recepciones desde el swap **/
#define CONTENIDO_PAGINA 1
#define RESULTADO_OK 2
#define RESULTADO_ERROR 3

/** Codigos de operacion en los envios hacia el cpu **/
#define OK 1
#define ERROR 2
#define CONTENIDO_PAGINA_LEIDA 3

/** Codigos de operacion en las recepciones desde el cpu **/
#define INICIAR 1
#define LEER 2
#define ESCRIBIR 3
#define FINALIZAR 4

typedef struct pagina {
	int32_t nro_pagina;
	int32_t PID;
	int32_t tamanio_contenido;
	char* contenido;
}t_pagina;

int32_t obtener_tamanio_pagina(t_pagina*);
char* serializar_pedido(t_pagina*);
t_pagina* deserializar_pedido(char* );
char* serializarTexto (char* );


#endif /* COMUNICACION_H_ */
