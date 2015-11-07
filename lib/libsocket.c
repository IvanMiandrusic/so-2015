/*
 * libsocket.c
 *
 *  Author: fcatinello
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libsocket.h"

/**** FUNCIONES PRIVADAS ****/

sock_t* _create_socket() {
	sock_t* sock = (sock_t*) malloc(sizeof(sock_t));
	sock->sockaddr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr));

	sock->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock->fd == ERROR_OPERATION) {
		printf("Error en la creacion del socket, volviendo a intentar..");
		sock->fd = socket(AF_INET, SOCK_STREAM, 0);
	}

	return sock;
}

sock_t* _create_socket_from_fd(int32_t fd) {

	sock_t* sock = (sock_t*) malloc(sizeof(sock_t));
	sock->fd = fd;
	sock->sockaddr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr));
	return sock;
}

void _prepare_conexion(sock_t* socket, char* ip, uint32_t puerto) {
	// Seteamos la IP, si es NULL es localhost
	if (ip == NULL)
		socket->sockaddr->sin_addr.s_addr = htonl(INADDR_ANY); // Conexion Local
	else
		socket->sockaddr->sin_addr.s_addr = inet_addr(ip);	// Conexion Remota

	socket->sockaddr->sin_family = AF_INET;
	socket->sockaddr->sin_port = htons(puerto);
	memset(&(socket->sockaddr->sin_zero), '\0', 8);
}

int32_t _refresh_port(sock_t* socket) {
	int32_t yes = 1;
	return setsockopt(socket->fd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int32_t));
}

int32_t _bind_port(sock_t* socket) {
	_refresh_port(socket);
	return bind(socket->fd, (struct sockaddr *) socket->sockaddr,
			sizeof(struct sockaddr));
}

int32_t _send(sock_t* socket, void* buffer, uint32_t len) {
	return send(socket->fd, buffer, len, 0);
}

int32_t _receive(sock_t* socket, void* buffer, uint32_t len) {
	return recv(socket->fd, buffer, len, 0);
}

void _close_socket(sock_t* socket) {
	shutdown(socket->fd, 2);
}

void _free_socket(sock_t* socket) {
	free(socket->sockaddr);
	free(socket);
}

int32_t _get_header_size() {
	return sizeof(header_t);
}

header_t* _create_empty_header() {
	return (header_t*) malloc(_get_header_size());
}

header_t* _create_header(int32_t cod_op, int32_t msg_size) {

	header_t* header = _create_empty_header();
	header->cod_op = cod_op;
	header->size_message = msg_size;
	return header;

}

int32_t _send_bytes(sock_t* socket, void* buffer, int32_t len) //me aseguro que se envie toda la informacion
{
	int32_t total = 0;
	int32_t bytesLeft = len;
	int32_t n;
	while (total < len) {
		n = _send(socket, buffer + total, bytesLeft);
		if (n == -1)
			break;
		total += n;
		bytesLeft -= n;
	}
	len = total;
	return n == ERROR_OPERATION ? ERROR_OPERATION : SUCCESS_OPERATION;
}

int32_t _receive_bytes(sock_t* sock, void* bufferSalida, int32_t lenBuffer) {
	int32_t n = 0;
	int32_t bytesLeft = lenBuffer;
	int32_t recibido = 0;

	while (recibido < lenBuffer) {
		n = _receive(sock, bufferSalida + recibido, lenBuffer - recibido);
		if (n == -1 || n == 0)
			break;
		recibido += n;
		bytesLeft -= n;
	}
	lenBuffer = recibido;

	if(n == 0) return SOCKET_CLOSED;

	if(n == -1) return ERROR_OPERATION;

	return SUCCESS_OPERATION;
}


int32_t _send_header(sock_t* socket, header_t* header) {
	int32_t n = _send_bytes(socket, &(header->cod_op), sizeof(int32_t));
	if (n == ERROR_OPERATION)
		return ERROR_OPERATION;

	n = _send_bytes(socket, &(header->size_message), sizeof(int32_t));
	return (n == ERROR_OPERATION) ? ERROR_OPERATION : SUCCESS_OPERATION;
}

int32_t _receive_header(sock_t* socket, header_t* header) {

	int32_t n = _receive_bytes(socket, &(header->cod_op), sizeof(int32_t));
	if (n == ERROR_OPERATION || n == SOCKET_CLOSED)
		return n;

	n = _receive_bytes(socket, &(header->size_message), sizeof(int32_t));
	return n;

}

/**** FUNCIONES PUBLICAS ****/

/* Funcion para crear un socket servidor */

sock_t* create_server_socket(uint32_t puertoListen) {

	sock_t* sock = _create_socket();

	_prepare_conexion(sock, NULL, puertoListen);

	if (_bind_port(sock) == ERROR_OPERATION)
		return NULL;

	return sock;
}

/* Funcion para crear un socket cliente */

sock_t* create_client_socket(char* ipServidor, uint32_t puertoServidor) {

	sock_t* clientSocket = _create_socket();

	_prepare_conexion(clientSocket, ipServidor, puertoServidor);

	return clientSocket;

}

/* Conecta un cliente al socket servidor */

int32_t connect_to_server(sock_t* socket) {
	return connect(socket->fd, (struct sockaddr *) socket->sockaddr,
			sizeof(struct sockaddr));
}

/* El socket servidor escucha conexiones de clientes entrantes */

int32_t listen_connections(sock_t* socket) {
	return listen(socket->fd, LISTEN_CONNECTIONS_SIZE);
}

/* El socket servidor acepta conexion entrante */

sock_t* accept_connection(sock_t* socket) {
	sock_t* sock_nuevo = _create_socket();

	uint32_t i = sizeof(struct sockaddr_in);

	sock_nuevo->fd = accept(socket->fd,
			(struct sockaddr *) sock_nuevo->sockaddr, &i);

	return sock_nuevo->fd == ERROR_OPERATION ? NULL : sock_nuevo;
}

/* Envio un mensaje serializado con cabecera */

int32_t send_msg(sock_t* sock, int32_t cod_op, char* msg, int32_t msg_size) {

	header_t* header = _create_header(cod_op, msg_size);

	int32_t n = _send_header(sock, header);

	if(n == ERROR_OPERATION) return ERROR_OPERATION;

	if(header->size_message != 0)
		n = _send_bytes(sock, msg, header->size_message);

	return n;
}

/* Envio una estructura de tipo attribute_packed */

int32_t send_struct(sock_t* sock, void* pack, int32_t len_pack) {

	int32_t n = _send_bytes(sock, pack, len_pack);

	return n;
}

/* Recibo un mensaje serializado con cabecera */

int32_t receive_msg(sock_t* sock, char** output) {

	header_t* header = _create_empty_header();

	int32_t n = _receive_header(sock, header);

	if (n != SUCCESS_OPERATION)
		return n;

	if(header->size_message != 0) {

		*output = malloc(header->size_message);

		n = _receive_bytes(sock, *output, header->size_message);

	}

	return n;
}

/* Recibo una estructura de tipo attribute_packed */

int32_t receive_struct(sock_t* sock, void* pack, int32_t len_pack) {

	int32_t n = _receive_bytes(sock, pack, len_pack);

	return n;
}

/* Cierro el socket y libero el espacio consumido por su estructura */

void clean_socket(sock_t* sock) {
	_close_socket(sock);
	_free_socket(sock);
}

/* Funcion para crear un servidor que maneje un pool de conexiones */

void connection_pool_server_listener(sock_t* srv_sock, void(*process_request)(sock_t*, header_t*)) {

	fd_set readset, tempset;
	int32_t maxfd, current_fd, result;
	sock_t* client_sock;

	int32_t optval = 1;
	ioctl(srv_sock->fd, FIONBIO, &optval);

	FD_ZERO(&readset);
	FD_SET(srv_sock->fd, &readset);
	maxfd = srv_sock->fd;

	do {

		memcpy(&tempset, &readset, sizeof(tempset));

		result = select(maxfd + 1, &tempset, NULL, NULL, NULL);

		if (result == 0)
			printf("select() timed out!\n");

		else if (result < 0 && errno != EINTR)
			printf("Error in select(): %s\n", strerror(errno));

		else if (result > 0) {

			if (FD_ISSET(srv_sock->fd, &tempset)) {     //New connection request

				client_sock = accept_connection(srv_sock);

				if (client_sock == NULL)
					printf("Error in accept(): %s\n", strerror(errno));

				else {
					FD_SET(client_sock->fd, &readset);
					maxfd = (maxfd < client_sock->fd) ? client_sock->fd : maxfd;
				}

				FD_CLR(srv_sock->fd, &tempset);
			}

			for (current_fd = 0; current_fd < maxfd + 1; current_fd++) {

				if (FD_ISSET(current_fd, &tempset)) {     //New data to be read

					header_t* header = _create_empty_header();

					sock_t* sock = _create_socket_from_fd(current_fd);

					int32_t result = _receive_header(sock, header);
					//Need to be implemented in context
					if (result == SUCCESS_OPERATION) {
						process_request(sock, header);
					}

					else if (result == SOCKET_CLOSED) {
						close(current_fd);
						FD_CLR(current_fd, &readset);
					}

					else
						printf("Error in recv(): %s\n", strerror(errno));

					_free_socket(sock);
				}      // end if (FD_ISSET(current_fd, &tempset))
			}      // end for (current_fd=0;...)
		}      // end else if (result > 0)
	} while (1);

}

int32_t get_operation_code(header_t* header) {

	return header->cod_op;
}

int32_t get_message_size(header_t* header) {

	return header->size_message;
}
