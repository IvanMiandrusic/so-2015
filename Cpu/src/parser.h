
#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_


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
int esComilla(char );
int32_t buscarPrimerParametro(char*);
char* buscarSegundoParametro(char*);
structEscribir buscarAmbosParametros(char*);



#endif /* SRC_PARSER_H_ */
