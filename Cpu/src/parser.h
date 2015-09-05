/*
 * parser.h
 *
 *  Created on: 31/8/2015
 *      Author: utnso
 */

#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

#include "Operaciones.h"

typedef  struct {
	int pagina;
	int tamanio_texto;
	char* texto;
}structEscribir;

#define INICIAR 1
#define FINALIZAR 2
#define ENTRADASALIDA 3
#define LEER 4
#define ESCRIBIR 5
#define ERROR 6

int32_t analizar_operacion_asociada(char* );
int esEspacio(char );
int esPuntoyComa(char );
int buscarPrimerParametro(char*);
char* buscarSegundoParametro(char*);
structEscribir buscarAmbosParametros(char*);
t_respuesta* analizadorLinea(int32_t,PCB*,char* const );



#endif /* SRC_PARSER_H_ */
