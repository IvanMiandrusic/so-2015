/*
 * libsocket.h
 *
 * Author: fcatinello
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#define LISTEN_CONNECTIONS_SIZE 20
#define SOCKET_CLOSED -2
#define ERROR_OPERATION -1
#define SUCCESS_OPERATION 0

typedef struct sock
{
	int32_t fd;
	struct sockaddr_in* sockaddr;

} sock_t;

typedef struct header_operation
{
	int32_t cod_op;
	int32_t size_message;

}header_t;

/* FUNCIONES PRIVADAS */

/**
 * Crea un Socket
 */
sock_t* _create_socket();

/*
 * Crea una estructura socket a partir del descriptor de fichero
 */
sock_t* _create_socket_from_fd(int32_t);

/**
 * Prepara la conexion para trabajar con el socket.
 * Si es localhost usar NULL en la IP
 */
void _prepare_conexion(sock_t*, char*, uint32_t);

/**
 * Limpia el puerto para reutilizarlo (para los casos en que se corre
 * varias veces un bind sobre el mismo puerto)
 */
int32_t _refresh_port(sock_t*);

/**
 * Bindea el puerto al Socket
 */
int32_t _bind_port(sock_t*);

/**
 * Envia un mensaje a traves del socket
 * Devuelve la cantidad de bytes que se enviaron realmente, o -1 en caso de error
 */
int32_t _send(sock_t*, void*, uint32_t);

/**
 * Lee un mensaje y lo almacena en buff
 * Devuelve la cantidad leia de bytes, 0 en caso de que la conexion este cerrada, o -1 en caso de error
 */
int32_t _receive(sock_t*, void*, uint32_t);

/*
 * Procedemos a cerrar el socket con la opcion SHUT_RDWR (No more receptions or transmissions)
 */
void _close_socket(sock_t* );

/**
 * Liberamos la memoria ocupada por el struct
 */
void _free_socket(sock_t*);

/*
 * Obtiene el tamaño de una cabecera de mensaje
 */
int32_t _get_header_size();

/*
 * Crea una cabecera vacia
 */
header_t* _create_empty_header();

/*
 * Crea una cabecera a partir de datos (codigo operacion y tamaño mensaje)
 */
header_t* _create_header(int32_t, int32_t);

/**
 * Trata de enviar el mensaje completo, aunque este sea muy grande.
 * Deja en len la cantidad de bytes NO enviados
 * Devuelve 0 en caso de exito, o -1 si falla
 */
int32_t _send_bytes(sock_t*, void*, int32_t);


/**
 * Trata de recibir el mensaje completo, aunque este se envie en varias rafagas.
 * Deja en len la cantidad de bytes NO recibidos
 * Devuelve 0 en caso de exito; o -1 si falla
 */
int32_t _receive_bytes(sock_t*, void*, int32_t);

/*
 * Realizo el envio de una cabecera
 * @PARAMS: El socket a enviar, la cabecera
 */
int32_t _send_header(sock_t*, header_t*);

/*
 * Recibo datos de una cabecera
 * @PARAMS: El socket de donde recibe, y el header donde alocar los datos
 */
int32_t _receive_header(sock_t*, header_t*);


/*** FUNCIONES PUBLICAS ***/

/**
 * Crea un socket para recibir mensajes.
 * Recibe el puerto de la maquina remota que se va a conectar.
 * NO REALIZA EL LISTEN
 * @RETURNS: El struct socket.
 */
sock_t* create_server_socket(uint32_t);

/**
 * Crea un socket para enviar mensajes.
 * Recibe el puerto de la maquina remota que esta escuchando.
 * NO REALIZA EL CONNECT.
 *
 * @RETURNS: El struct socket.
 */
sock_t* create_client_socket(char*, uint32_t);

/**
 * Conecta con otra PC
 *
 * @RETURNS: -1 en caso de error
 * NOTA: Recordar que es una funcion bloqueante
 */
int32_t connect_to_server(sock_t*);

/**
 * Establece el socket para escuchar
 *
 *	NOTA: Esta funcion es NO bloqueante
 * @RETURNS: -1 en caso de error
 */
int32_t listen_connections(sock_t*);

/**
 * Acepta una conexion entrante
 *
 * @RETURNS: el nuevo FD; o NULL en caso de error
 * @NOTE: Recordar que es una funcion bloqueante
 */
sock_t* accept_connection(sock_t*);

/*
 * Realiza el envio de una cabecera y posterior el mensaje correspondiente,
 * La funcion recibe el socket destino,el codigo de operacion y el mensaje (informacion) que se desea enviar
 */
int32_t send_msg(sock_t*, int32_t, char*, int32_t);

/*
 * Realiza el envio de una estructura compactada en memoria
 * @NOTE: debe tener el attribute_packed
 */
int32_t send_struct(sock_t*, void*, int32_t);

/*
 * Realiza la recepcion de una cabecera y luego el correspondiente mensaje,
 * La funcion recibe el socket emisor del mensaje, y el buffer donde se colocara el mensaje recibido
 */
int32_t receive_msg(sock_t*, char**);

/*
 * Realiza la recepcion de una estructra compactada en memoria
 * @NOTE: debe tener el attribute_packed
 */
int32_t receive_struct(sock_t*, void*, int32_t);

/*
 * Cierra y libera el espacio ocupado por el socket en memoria
 */
void clean_socket(sock_t*);

/**
 * Crea un servidor para manejar pool de conexiones.
 * Recibe el socket servidor que actua de multiplexor
 * y la funcion que procesa el pedido ---> process_request(sock_t*, header_t*);
 * @NOTE: REALIZA EL SELECT
 */
 void connection_pool_server_listener(sock_t* srv_sock, void(*process_request)(sock_t*, header_t*));

/*
 * Obtiene el codigo de operacion a partir de un header
 */
 int32_t get_operation_code(header_t*);

 /*
  * Setea el codigo de operacion en un header
  */
void set_operation_code(header_t*, int32_t);

/*
 * Obtiene el tamaño de un mensaje a partir de un header
 */
 int32_t get_message_size(header_t*);
 
 /*
  * Setea el tamaño de un mensaje en un header
  */
 void set_message_size(header_t*, int32_t);


#endif /* SOCKETS_H_ */
