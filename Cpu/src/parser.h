/*
 * parser.h
 *
 *  Created on: 31/8/2015
 *      Author: utnso
 */

#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

typedef  struct {
	int pagina;
	int tamanio_texto;
	char* texto;
}structEscribir;

typedef enum comandos {
	INICIAR=1,
	FINALIZAR=2,
	ENTRADASALIDA=3,
	LEER=4,
	ESCRIBIR=5,
	ERROR=6
}t_command;

int32_t analizar_operacion_asociada(char* );
int esEspacio(char );
int esPuntoyComa(char );
int buscarPrimerParametro(char*);
char* buscarSegundoParametro(char*);
structEscribir buscarAmbosParametros(char*);
void mAnsisOp_iniciar(int );
void mAnsisOp_leer(int );
void mAnsisOp_escribir(int , char* );
void mAnsisOp_IO(int );
void mAnsisOp_finalizar();
void analizadorLinea(char* const );



#endif /* SRC_PARSER_H_ */
