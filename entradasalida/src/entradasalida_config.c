#include "entradasalida_config.h"

t_IO_config* crear_IO_config() {
    t_IO_config* config = malloc(sizeof(t_IO_config));
    if (config == NULL) {
        return NULL;
    }
    return config;
}

void destruir_IO_config(t_IO_config* config) {
    if (config) {
        free(config);
    }
}

bool cargar_IO_config(t_IO_config* config, t_config* cf) {
    if (!config || !cf) {
        return false;
    }

    config->TIPO_INTERFAZ = config_get_string_value(cf, "TIPO_INTERFAZ");

    if(_requiere_tiempo_trabajo(config)) {
        config->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(cf, "TIEMPO_UNIDAD_TRABAJO");
    }

    config->IP_KERNEL = config_get_string_value(cf, "IP_KERNEL");
    config->PUERTO_KERNEL = config_get_int_value(cf, "PUERTO_KERNEL");

    if(_requiere_datos_memoria(config)) {
        config->IP_MEMORIA = config_get_string_value(cf, "IP_MEMORIA");
        config->PUERTO_MEMORIA = config_get_int_value(cf, "PUERTO_MEMORIA");
    }

    if(_requiere_datos_fs(config)) {
        config->PATH_BASE_DIALFS = config_get_string_value(cf, "PATH_BASE_DIALFS");
        config->BLOCK_SIZE = config_get_int_value(cf, "BLOCK_SIZE");
        config->BLOCK_COUNT = config_get_int_value(cf, "BLOCK_COUNT");
        config->RETRASO_COMPACTACION = config_get_int_value(cf, "RETRASO_COMPACTACION");
    }

    return true;
}

bool tiene_configuracion_memoria(t_IO_config* config) {
    return (config->IP_MEMORIA != NULL && config->PUERTO_MEMORIA != 0);
}

bool tiene_configuracion_kernel(t_IO_config* config) {
    return (config->IP_KERNEL != NULL && config->PUERTO_KERNEL != 0);
}

char* obtener_tipo_interfaz(t_IO_config* config) {
    return config->TIPO_INTERFAZ;
}

uint32_t obtener_tiempo_unidad_trabajo(t_IO_config* config) {
    return config->TIEMPO_UNIDAD_TRABAJO;
}

char* obtener_ip_kernel(t_IO_config* config) {
    return config->IP_KERNEL;
}

uint16_t obtener_puerto_kernel(t_IO_config* config) {
    return config->PUERTO_KERNEL;
}

char* obtener_ip_memoria(t_IO_config* config) {
    return config->IP_MEMORIA;
}

uint16_t obtener_puerto_memoria(t_IO_config* config) {
    return config->PUERTO_MEMORIA;
}

char* obtener_path_base_dialfs(t_IO_config* config) {
    return config->PATH_BASE_DIALFS;
}

uint32_t obtener_block_size(t_IO_config* config) {
    return config->BLOCK_SIZE;
}

uint32_t obtener_block_count(t_IO_config* config) {
    return config->BLOCK_COUNT;
}

uint32_t obtener_retraso_compactacion(t_IO_config* config) {
    return config->RETRASO_COMPACTACION;
}

bool _requiere_tiempo_trabajo(t_IO_config* config) {
    return (strcmp(obtener_tipo_interfaz(config), "GENERICA") == 0) || (strcmp(obtener_tipo_interfaz(config), "DIALFS") == 0);
}

bool _requiere_datos_memoria(t_IO_config* config) {
    return (strcmp(obtener_tipo_interfaz(config), "GENERICA") != 0);
}

bool _requiere_datos_fs(t_IO_config* config) {
    return (strcmp(obtener_tipo_interfaz(config), "DIALFS") == 0);
}