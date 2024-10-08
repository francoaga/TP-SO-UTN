#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int iniciar_servidor(t_log *logger, const char *name, char *ip, char *puerto);
int esperar_cliente(t_log *logger, const char *name, int socket_servidor);
int crear_conexion(t_log *logger, const char *server_name, char *ip, char *puerto);
int recibir_operacion(int socket_cliente);
void liberar_conexion(int socket_cliente);
void recibir_mensaje(t_log *, int);
void *recibir_buffer(int *, int);

#endif