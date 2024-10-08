#include "entradasalida_server.h"


//=========================================================
// FUNCIONES PARA INICIALIZAR EL CLIENTE DE ENTRADA/SALIDA
//=========================================================

void inicializar_sockets() {
    // Inicializar socket de conexión con Kernel
    if (tiene_configuracion_kernel(entradasalida_config)) {
        kernel_port = string_itoa(obtener_puerto_kernel(entradasalida_config));
        fd_kernel = crear_conexion(logger_entradasalida, SERVER_KERNEL, obtener_ip_kernel(entradasalida_config), kernel_port);
        if (fd_kernel == -1) {
            log_error(logger_entradasalida, "Error al conectar con el Kernel. ABORTANDO");
            exit(EXIT_FAILURE);
        }
        
        enviar_io_interface_kernel();
        
    } else {
        log_error(logger_entradasalida, "No se encontró configuración para el Kernel. ABORTANDO");
        exit(EXIT_FAILURE);
    }

    // Consultar el tipo de interfaz
    char* tipo_interfaz = obtener_tipo_interfaz(entradasalida_config);

    // Inicializar socket de conexión con Memoria si no es una interfaz genérica
    if (strcmp(tipo_interfaz, "GENERICA") != 0) {
        if (tiene_configuracion_memoria(entradasalida_config)) {
            memoria_port = string_itoa(obtener_puerto_memoria(entradasalida_config));
            fd_memoria = crear_conexion(logger_entradasalida, SERVER_MEMORIA, obtener_ip_memoria(entradasalida_config), memoria_port);
            if (fd_memoria == -1) {
                log_error(logger_entradasalida, "Error al conectar con la Memoria. ABORTANDO");
                exit(EXIT_FAILURE);
            }
            enviar_io_interface_memoria();
        } else {
            log_error(logger_entradasalida, "No se encontró configuración para la Memoria. ABORTANDO");
            exit(EXIT_FAILURE);
        }
    }
}

//==========================================================
// FUNCIONES PARA CREAR HILOS DE INTERFAZ DE ENTRADA/SALIDA
//==========================================================

void crear_hilos_conexiones() 
{
    pthread_t hilo_kernel;

    // Hilo para manejar mensajes del Kernel
    if (pthread_create(&hilo_kernel, NULL, (void *)atender_solicitudes_io_kernel, NULL) != 0)
    {
        log_error(logger_entradasalida, "Error al crear el hilo para atender al KERNEL. ABORTANDO");
        exit(EXIT_FAILURE);
    }

    pthread_join(hilo_kernel, NULL);
}

//====================================================
// FUNCIONES PARA ATENDER SOLICITUDES DE ENTRADA/SALIDA
//====================================================

void atender_solicitudes_io_kernel() {
    bool esperar = true;
    while (esperar) {
        int cod_operacion = recibir_operacion(fd_kernel);
        switch (cod_operacion) {
            case MSG_KERNEL_IO_GENERICA:
                atender_solicitud_generica(fd_kernel);
                break;
            case MSG_KERNEL_IO_STDIN:
                atender_solicitud_stdin(fd_kernel);
                break;
            case MSG_KERNEL_IO_STDOUT:
                atender_solicitud_stdout(fd_kernel);
                break;
            case MSG_KERNEL_IO_DIALFS:
                atender_solicitud_dialfs(fd_kernel);
                break;
            case -1:
                log_error(logger_entradasalida, "ERROR: Ha surgido un problema inesperado, se desconectó el Kernel.");
                esperar = false;
                break;
            default:
                log_warning(logger_entradasalida, "WARNING: El módulo de entrada/salida ha recibido una solicitud con una operación desconocida");
                break;
        }
    }
}


//====================================================
// FUNCIONES PARA CIERRE DE CLIENTE DE ENTRADA/SALIDA
//====================================================

void cerrar_cliente() {
    _cerrar_puertos();
    _cerrar_conexiones();
    log_info(logger_entradasalida, "Cliente de entrada/salida cerrado correctamente.");
}

void _cerrar_puertos() {
    // Liberar los puertos utilizados por el cliente de entrada/salida
    free(kernel_port);
    free(memoria_port);
    // Liberar otros puertos si es necesario
}

void _cerrar_conexiones() {
    // Liberar las conexiones utilizadas por el cliente de entrada/salida
    liberar_conexion(fd_kernel);
    liberar_conexion(fd_memoria);
    // Liberar otras conexiones si es necesario
}


void enviar_io_interface_kernel()
{
    send_IO_interface_kernel();
}

void enviar_io_interface_memoria()
{
    send_IO_interface_memoria();
}
