#include "cpu_dispatch.h"

// case de ejecucion de instrucciones:
void ejecutar_instruccion(t_instruction *instruccion, t_cpu_registers *cpu_registers)
{
    log_info(logger_cpu, "EJECUTANDO INSTRUCCION");
    log_info(logger_cpu, "INSTRUCCION => %d", instruccion->name);

    switch (instruccion->name)
    {
    case SET:
    {
        char *reg = (char *)list_get(instruccion->params, 0);
        int value = atoi((char *)list_get(instruccion->params, 1));
        _establecer_registro(cpu_registers, reg, value);
        log_info(logger_cpu, "SET %s %d\n", reg, value);
        break;
    }
    case MOV_IN:
    {
        char *reg_datos = (char *)list_get(instruccion->params, 0);
        char *reg_direccion = (char *)list_get(instruccion->params, 1);

        uint32_t direccion_logica = _obtener_valor_registro(cpu_registers, reg_direccion);
        uint32_t valor_memoria = 0;
        valor_memoria = leer_memoria(direccion_logica, &valor_memoria);
        _establecer_registro(cpu_registers, reg_datos, valor_memoria);

        log_info(logger_cpu, "MOV_IN %s %s\n", reg_datos, reg_direccion);
        break;
    }
    case MOV_OUT:
    {
        char *reg_direccion = (char *)list_get(instruccion->params, 0);
        char *reg_datos = (char *)list_get(instruccion->params, 1);

        uint32_t direccion_logica = _obtener_valor_registro(cpu_registers, reg_direccion);
        uint32_t valor_datos = _obtener_valor_registro(cpu_registers, reg_datos);
        escribir_memoria(direccion_logica, valor_datos);

        log_info(logger_cpu, "MOV_OUT %s %s\n", reg_direccion, reg_datos);
        break;
    }
    case SUM:
    {
        char *reg_dest = (char *)list_get(instruccion->params, 0);
        char *reg_src = (char *)list_get(instruccion->params, 1);
        uint8_t valor_src = _obtener_valor_registro(cpu_registers, reg_src);
        uint8_t valor_operacion = _obtener_valor_registro(cpu_registers, reg_dest) + valor_src;
        _establecer_registro(cpu_registers, reg_dest, valor_operacion);
        log_info(logger_cpu, "SUM %s %s\n", reg_dest, reg_src);
        break;
    }
    case SUB:
    {
        char *reg_dest = (char *)list_get(instruccion->params, 0);
        char *reg_src = (char *)list_get(instruccion->params, 1);
        uint32_t valor_src = _obtener_valor_registro(cpu_registers, reg_src);
        uint32_t valor_operacion = _obtener_valor_registro(cpu_registers, reg_dest) - valor_src;
        _establecer_registro(cpu_registers, reg_dest, valor_operacion);
        log_info(logger_cpu, "SUB %s %s\n", reg_dest, reg_src);
        break;
    }
    case JNZ:
    {
        char *reg = (char *)list_get(instruccion->params, 0);
        int instruction_index = atoi((char *)list_get(instruccion->params, 1));
        uint32_t valor_reg = _obtener_valor_registro(cpu_registers, reg);

        if (valor_reg != 0)
        {
            cpu_registers->pc = instruction_index;
            log_trace(logger_cpu, "el registro %s era igual a cero, por lo tanto se salto a %d (JNZ)\n", reg, instruction_index);
        }
        log_info(logger_cpu, "JNZ %s %d\n", reg, instruction_index);

        // le restamos uno al PC ya que al finalizar Execute se le va a sumar 1 al PC: entonces esto se cancela.
        cpu_registers->pc--;
        break;
    }
    case RESIZE:
    {
        int nuevo_tamano = atoi((char *)list_get(instruccion->params, 0));

        // Función para solicitar a la memoria el ajuste de tamaño
        if (!ajustar_tamano_proceso(cpu_registers, nuevo_tamano))
        {
            // TODO: Si la memoria devuelve Out of Memory, devolver el contexto al Kernel
            informar_kernel_error("Out of Memory");
        }
        else
        {
            log_info(logger_cpu, "RESIZE %d\n", nuevo_tamano);
        }
        break;
    }
    case COPY_STRING:
    {
        int tamano = atoi((char *)list_get(instruccion->params, 0));

        uint32_t direccion_origen = cpu_registers->si;
        uint32_t direccion_destino = cpu_registers->di;

        copiar_cadena(direccion_origen, direccion_destino, tamano);

        log_info(logger_cpu, "COPY_STRING %d\n", tamano);
        break;
    }
    case IO_GEN_SLEEP:
    {
        char *interface = (char *)list_get(instruccion->params, 0);
        int units = atoi((char *)list_get(instruccion->params, 1));

        // Enviar el PCB al kernel con el tipo de interfaz
        solicitar_IO(instruccion);
        log_info(logger_cpu, "IO_GEN_SLEEP %s %d\n", interface, units);
        break;
    }
    default:
        printf("Instrucción no reconocida\n");
        break;
    }
}

// FUNCIONES AUXILIARES DE EJECUCION DE INSTRUCCIONES:

// #############################################################################################################
//  TODO: REVISAR MACHEO DE INT, debuggear.
// #############################################################################################################
//  Función para obtener el puntero a un registro basado en su nombre:
uint32_t *_obtener_registro(t_cpu_registers *registros, const char *nombre)
{
    if (strcmp(nombre, "AX") == 0)
        return (uint32_t *)&registros->ax;
    if (strcmp(nombre, "BX") == 0)
        return (uint32_t *)&registros->bx;
    if (strcmp(nombre, "CX") == 0)
        return (uint32_t *)&registros->cx;
    if (strcmp(nombre, "DX") == 0)
        return (uint32_t *)&registros->dx;
    if (strcmp(nombre, "EAX") == 0)
        return &registros->eax;
    if (strcmp(nombre, "EBX") == 0)
        return &registros->ebx;
    if (strcmp(nombre, "ECX") == 0)
        return &registros->ecx;
    if (strcmp(nombre, "EDX") == 0)
        return &registros->edx;
    if (strcmp(nombre, "SI") == 0)
        return &registros->si;
    if (strcmp(nombre, "DI") == 0)
        return &registros->di;
    return NULL;
}

// Función para establecer el valor de un registro
// void _establecer_registro(t_cpu_registers *registros, const char *nombre, uint32_t valor) {
//     uint32_t *reg = _obtener_registro(registros, nombre);
//     if (reg) *reg = valor;
// }
void _establecer_registro(t_cpu_registers *registros, char *nombre, uint32_t valor)
{
    if (strcmp(nombre, "AX") == 0)
        registros->ax = valor;
    if (strcmp(nombre, "BX") == 0)
        registros->bx = valor;
    if (strcmp(nombre, "CX") == 0)
        registros->cx = valor;
    if (strcmp(nombre, "DX") == 0)
        registros->dx = valor;
    if (strcmp(nombre, "EAX") == 0)
        registros->eax = valor;
    if (strcmp(nombre, "EBX") == 0)
        registros->ebx = valor;
    if (strcmp(nombre, "ECX") == 0)
        registros->ecx = valor;
    if (strcmp(nombre, "EDX") == 0)
        registros->edx = valor;
    if (strcmp(nombre, "SI") == 0)
        registros->si = valor;
    if (strcmp(nombre, "DI") == 0)
        registros->di = valor;
}

// Función para obtener el valor de un registro
uint32_t _obtener_valor_registro(t_cpu_registers *registros, char *nombre)
{
    remove_newline(nombre);
    uint32_t *reg = _obtener_registro(registros, nombre);
    return reg ? *reg : 0;
}

void remove_newline(char *str)
{
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
    {
        str[len - 1] = '\0';
    }
}

// #############################################################################################################
// Es importante tener en cuenta las siguientes aclaraciones:
// Una dirección lógica se traduce a una dirección física, pero al copiar un string/registro a memoria,
// podría estar presente en más de una página (ver sección de MMU).
// #############################################################################################################
uint32_t leer_memoria(uint32_t direccion_logica, uint32_t *memory_value)
{
    // TODO: se relaciona con la MMU, implementar cuando la MMU esté desarrollada.

    t_datos_dir_logica *dir_logica = crear_dir_logica(direccion_logica);
    uint32_t frame;

    // TLB
    if (obtener_marco(pcb_execute->pid, dir_logica->num_pagina, &frame))
    {
        // TLB HIT

        // consultar a la memoria para obtener el frame correspondiente
        //(pid, dir_logica, frame, memory_value)

        free(dir_logica);

        return 1;
    }
    return 0;
}

void escribir_memoria(uint32_t direccion_logica, uint32_t valor_datos)
{
    // TODO: se relaciona con la MMU, implementar cuando la MMU esté desarrollada.
}

bool ajustar_tamano_proceso(t_cpu_registers *cpu_registers, int nuevo_tamano)
{
    // TODO: Implementa la lógica para solicitar el ajuste de tamaño a la memoria
    // Devuelve true si el ajuste es exitoso, false si hay un error (por ejemplo, Out of Memory)
    // Ejemplo simplificado
    // if (memoria_tiene_espacio(nuevo_tamano)) {
    //     // Ajustar el tamaño en la memoria
    //     return true;
    // } else {
    //     return false;
    // }
    return true;
}

void informar_kernel_error(const char *mensaje)
{
    // TODO: Implementa la lógica para informar al Kernel sobre un error.
    printf("Kernel error: %s\n", mensaje);
}

void copiar_cadena(uint32_t origen, uint32_t destino, int tamano)
{
    // Implementa la lógica para copiar una cadena de bytes de origen a destino
}

void cargar_contexto_ejecucion(t_PCB *pcb)
{
    t_cpu_registers *contexto = get_cpu_registers(pcb);

    // Cargo el contexto de ejecucion del pcb en la CPU
    cpu_registers->pc = contexto->pc;
    cpu_registers->ax = contexto->ax;
    cpu_registers->bx = contexto->bx;
    cpu_registers->cx = contexto->cx;
    cpu_registers->dx = contexto->dx;
    cpu_registers->eax = contexto->eax;
    cpu_registers->ebx = contexto->ebx;
    cpu_registers->ecx = contexto->ecx;
    cpu_registers->edx = contexto->edx;
    cpu_registers->si = contexto->si;
    cpu_registers->di = contexto->di;
}

void solicitar_instruccion(uint32_t pid, uint32_t pc)
{
    // Pido la siguiente instruccion a memoria
    send_get_next_instruction(pid, pc);
}

void recibir_pcb()
{
    // Recibo el pcb que manda kernel para ejecutar sus instrucciones
    pcb_execute = recv_pcb_cpu();

    // Cargo el contexto de ejecucion del pcb en los registros de la cpu
    cargar_contexto_ejecucion(pcb_execute);

    // Pido a memoria que me mande las instrucciones del proceso
    solicitar_instruccion(pcb_execute->pid, pcb_execute->program_counter);
}

void manejar_ciclo_de_instruccion()
{
    // FETCH: Recibo la instruccion que manda memoria
    t_instruction *instruccion = recv_instruction();

    log_info(logger_cpu, "Instrucción recibida de memoria");
    log_info(logger_cpu, "antes: AX: %d", cpu_registers->ax);
    log_info(logger_cpu, "antes: BX: %d", cpu_registers->bx);
    log_info(logger_cpu, "antes: CX: %d", cpu_registers->cx);

    // EXECUTE: Ejecuto la instruccion recibida
    ejecutar_instruccion(instruccion, cpu_registers);
    // imprimir todos los cpu_registers:
    log_info(logger_cpu, "despues: AX: %d", cpu_registers->ax);
    log_info(logger_cpu, "despues: BX: %d", cpu_registers->bx);
    log_info(logger_cpu, "despues: CX: %d", cpu_registers->cx);

    eliminar_instruccion(instruccion);

    // INTERRUPT: verificar y manejar interrupciones después de ejecutar la instrucción
    if (manejar_interrupcion())
        return;

    // actualizar PC:
    cpu_registers->pc++;
    pcb_execute->program_counter = cpu_registers->pc;
    // TODO: Se debe actualizar el PC antes de pedir la siguiente instruccion a memoria
    solicitar_instruccion(pcb_execute->pid, pcb_execute->program_counter);
}

bool manejar_interrupcion()
{
    if (interrupcion_pendiente)
    {
        log_info(logger_cpu, "Interrupción recibida, devolviendo PCB al Kernel");
        // TODO: se debe cargar el nuevo contexto de ejecucion asociado al PCB antes
        //  de enviar de nuevo al kernel
        cargar_contexto_ejecucion_a_pcb(pcb_execute);
        send_pcb_kernel_interruption(tipo_de_interrupcion); // aca esta la logica de cual mensaje enviar al kernel segun cual sea el tipo de interrupccion
        sem_post(&SEM_INTERRUPT);
        interrupcion_pendiente = false;
        log_info(logger_cpu, "PCB enviado al Kernel");
        return true;
    }

    return false;
}

void solicitar_IO(t_instruction *instruccion)
{
    // t_interface interface = create_interface(pcb_execute, instruccion);
    send_interface_kernel(/*interface*/);
}

// cargar contexto de ejecucion del cpu a los registros del pcb
void cargar_contexto_ejecucion_a_pcb(t_PCB *pcb)
{
    t_cpu_registers *contexto = get_cpu_registers(pcb);

    // Cargo el contexto de ejecucion de la CPU en el pcb
    contexto->pc = cpu_registers->pc;
    contexto->ax = cpu_registers->ax;
    contexto->bx = cpu_registers->bx;
    contexto->cx = cpu_registers->cx;
    contexto->dx = cpu_registers->dx;
    contexto->eax = cpu_registers->eax;
    contexto->ebx = cpu_registers->ebx;
    contexto->ecx = cpu_registers->ecx;
    contexto->edx = cpu_registers->edx;
    contexto->si = cpu_registers->si;
    contexto->di = cpu_registers->di;
}

void _data_read(uint32_t pid, t_datos_dir_logica *dir_logica, uint32_t frame, uint32_t *memory_value)
{
    send_msg_cpu_memoria_data_read(pid, dir_logica->num_pagina, frame, dir_logica->desplazamiento_pagina, fd_memoria);

    t_package *paquete = package_create(NULL_HEADER, sizeof(uint32_t));
    package_recv(paquete, fd_memoria);

    if (paquete->msg_header != MSG_MEMORIA_CPU_DATA_READ)
    {
        log_debug(logger_cpu, "Se espera recibir mensaje desde memoria de data read");
        exit(EXIT_FAILURE);
    }

    recv_msg_memoria_cpu_data(paquete->buffer, memory_value);
    package_destroy(paquete);
}

int read_from_memory(t_PCB *pcb, uint32_t pid, uint32_t logical_address, uint32_t *memory_value)
{
    uint32_t frame;
    t_datos_dir_logica *dir_logica = crear_dir_logica(logical_address);

    if (obtener_marco(pid, dir_logica->num_pagina, &frame))
    {
        // TLB HIT
        _data_read(pid, dir_logica, frame, memory_value);

        free(dir_logica);
        // Retorno que no fue page fault

        return 1;
    }

    // TLB MISS
    send_msg_cpu_memoria_page(pid, dir_logica->num_pagina, fd_memoria);

    t_package *paquete = package_create(NULL_HEADER, sizeof(uint32_t));
    package_recv(paquete, fd_memoria);

    if (paquete->msg_header == MSG_MEMORIA_CPU_FRAME)
    {

        recv_msg_memoria_cpu_frame(paquete->buffer, &frame);
        reemplazar(pid, dir_logica->num_pagina, frame); // Reemplazamos en la TLB
        _data_read(pid, dir_logica, frame, memory_value);
        package_destroy(paquete);

        free(dir_logica);
        return 1;
    }
    else
    {
        log_debug(logger_cpu, "Se espera recibir mensaje desde memoria de page");
        exit(EXIT_FAILURE);
    }
}

void _write_data(uint32_t pid, t_datos_dir_logica *dir_logica, uint32_t frame, uint32_t write_value)
{
    send_msg_cpu_memoria_data_write(pid, dir_logica->num_pagina, frame, dir_logica->desplazamiento_pagina, write_value, fd_memoria);
}

int write_into_memory(t_PCB *pcb, uint32_t pid, uint32_t logical_address, uint32_t cpu_register)
{
    uint32_t frame;
    t_datos_dir_logica *dir_logica = crear_dir_logica(logical_address);

    if (obtener_marco(pid, dir_logica->num_pagina, &frame))
    {
        // TLB HIT
        _write_data(pid, dir_logica, frame, cpu_register);

        free(dir_logica);
        // Retorno que no fue page fault
        return 1;
    }

    // TLB MISS
    send_msg_cpu_memoria_page(pid, dir_logica->num_pagina, fd_memoria);

    t_package *paquete = package_create(NULL_HEADER,sizeof(uint32_t));
    package_recv(paquete, fd_memoria);

    if (paquete->msg_header == MSG_MEMORIA_CPU_FRAME)
    {
        recv_msg_memoria_cpu_frame(paquete->buffer, &frame);
        reemplazar(pid, dir_logica->num_pagina, frame); // Reemplazamos en la TLB
        _write_data(pid, dir_logica, frame, cpu_register);
        package_destroy(paquete);

        free(dir_logica);
        return 1;
    }

    else
    {
        log_debug(logger_cpu, "Se espera recibir mensaje desde memoria de page");
        exit(EXIT_FAILURE);
    }
}