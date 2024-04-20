#include "procesar_solicitudes.h"

void solicitudes_cpu() {
    bool esperar = true;

    while(esperar) {
        int cod_operacion = recibir_operacion(fd_cpu);

        switch(cod_operacion){
            case MENSAJE:
                // Se procesa el request
                recibir_mensaje(logger_memoria, fd_cpu);
            break;
            //TODO:
            /*
                Agregar operaciones a las que dara servicio el modulo
            */ 
            case ERROR:
                log_error(logger_memoria, "ERROR: Ha surgido un problema inesperado, se desconecto el modulo de memoria.");
                esperar = false; //Cortamos la espera de solicitudes
            break;
            default:
                log_warning(logger_memoria, "WARNING: El modulo de memoria ha recibido una solicitud con una operacion desconocida");
            break;
        }
    }
}

void solicitudes_kernel() {
    bool esperar = true;

    while(esperar) {
        int cod_operacion = recibir_operacion(fd_kernel);

        switch(cod_operacion){
            case MENSAJE:
                // Se procesa el request
                recibir_mensaje(logger_memoria, fd_kernel);
            break;
            //TODO:
            /*
                Agregar operaciones a las que dara servicio el modulo
            */ 
            case ERROR:
                log_error(logger_memoria, "ERROR: Ha surgido un problema inesperado, se desconecto el modulo de memoria.");
                esperar = false; //Cortamos la espera de solicitudes
            break;
            default:
                log_warning(logger_memoria, "WARNING: El modulo de memoria ha recibido una solicitud con una operacion desconocida");
            break;
        }
    }
}

void solicitudes_entradasalida() {
    bool esperar = true;

    while(esperar) {
        int cod_operacion = recibir_operacion(fd_entradasalida);

        switch(cod_operacion){
            case MENSAJE:
                // Se procesa el request
                recibir_mensaje(logger_memoria, fd_entradasalida);
            break;
            //TODO:
            /*
                Agregar operaciones a las que dara servicio el modulo
            */ 
            case ERROR:
                log_error(logger_memoria, "ERROR: Ha surgido un problema inesperado, se desconecto el modulo de memoria.");
                esperar = false; //Cortamos la espera de solicitudes
            break;
            default:
                log_warning(logger_memoria, "WARNING: El modulo de memoria ha recibido una solicitud con una operacion desconocida");
            break;
        }
    }
}