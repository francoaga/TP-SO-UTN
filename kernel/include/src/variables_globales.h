#ifndef VARIABLES_GLOBALES_H
#define VARIABLES_GLOBALES_H

#include <stdio.h>
#include <stdlib.h> 
#include <commons/log.h>
#include "kernel_config.h"

#define SERVERNAME "KERNEL"
#define SERVER_CPU "SERVER CPU"
#define SERVER_MEMORIA "SERVER MEMORIA"
#define CLIENTE_ENTRADASALIDA "CLIENTEENTRADASALIDA"

// VARIABLES GLOBALES
extern t_log* logger_kernel;
extern t_log* logger_kernel_debug;

//CONFIG
extern t_config* config_kernel;
extern t_kernel_config* kernel_config;

//FD
extern int fd_kernel_memoria;
extern int fd_kernel_IO;
extern int fd_cpu_dispatch;

#endif