#include "entradasalida_server.h"

void inicializar_sockets() {
    // Inicializar socket de conexión con Kernel
    kernel_port = string_itoa(obtener_puerto_kernel(entradasalida_config));
    fd_kernel = crear_conexion(logger_entradasalida, SERVER_KERNEL, obtener_ip_kernel(entradasalida_config), kernel_port);
    if (fd_kernel == -1) {
        log_error(logger_entradasalida, "Error al conectar con el Kernel. ABORTANDO");
        exit(EXIT_FAILURE);
    }

    // Consultar el tipo de interfaz
    char* tipo_interfaz = obtener_tipo_interfaz(entradasalida_config);

    // Inicializar socket de conexión con Memoria si no es una interfaz genérica
    if (strcmp(tipo_interfaz, "GENERICA") != 0) {
        memoria_port = string_itoa(obtener_puerto_memoria(entradasalida_config));
        fd_memoria = crear_conexion(logger_entradasalida, SERVER_MEMORIA, obtener_ip_memoria(entradasalida_config), memoria_port);
        if (fd_memoria == -1) {
            log_error(logger_entradasalida, "Error al conectar con la Memoria. ABORTANDO");
            exit(EXIT_FAILURE);
        }
    }
}

void crear_hilo_interfaz_generica() {
    pthread_t hilo_generica;
    if (pthread_create(&hilo_generica, NULL, (void *)atender_solicitudes_generica, NULL) != 0) {
        log_error(logger_entradasalida, "Error al crear el hilo para atender la interfaz genérica. ABORTANDO");
        exit(EXIT_FAILURE);
    }
    pthread_detach(hilo_generica);
}

void crear_hilo_interfaz_stdin() {
    pthread_t hilo_stdin;
    if (pthread_create(&hilo_stdin, NULL, (void *)atender_solicitudes_stdin, NULL) != 0) {
        log_error(logger_entradasalida, "Error al crear el hilo para atender la interfaz STDIN. ABORTANDO");
        exit(EXIT_FAILURE);
    }
    pthread_detach(hilo_stdin);
}

void crear_hilo_interfaz_stdout() {
    pthread_t hilo_stdout;
    if (pthread_create(&hilo_stdout, NULL, (void *)atender_solicitudes_stdout, NULL) != 0) {
        log_error(logger_entradasalida, "Error al crear el hilo para atender la interfaz STDOUT. ABORTANDO");
        exit(EXIT_FAILURE);
    }
    pthread_detach(hilo_stdout);
}

void crear_hilo_interfaz_dialfs() {
    pthread_t hilo_dialfs;
    if (pthread_create(&hilo_dialfs, NULL, (void *)atender_solicitudes_dialfs, NULL) != 0) {
        log_error(logger_entradasalida, "Error al crear el hilo para atender la interfaz DialFS. ABORTANDO");
        exit(EXIT_FAILURE);
    }
    pthread_detach(hilo_dialfs);
}