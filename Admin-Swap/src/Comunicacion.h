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
#include "Admin-Swap.h"

/** Codigos de operacion en los envios hacia la memoria **/
#define CONTENIDO_PAGINA 1
#define RESULTADO_OK 2
#define RESULTADO_ERROR 3

/** Codigos de operacion en las recepciones desde la memoria **/
#define LEER_PAGINA 1
#define ESCRIBIR_PAGINA 2
#define RESERVAR_ESPACIO 3
#define BORRAR_ESPACIO 4

#endif /* COMUNICACION_H_ */
