#include "cpu_dispatch.h"

// case de ejecucion de instrucciones:
void ejecutar_instruccion(t_instruction *instruccion, t_cpu_registers *cpu_registers)
{
    log_info(logger_cpu, "EJECUTANDO INSTRUCCION");

    switch (instruccion->name)
    {
    case SET:
    {
        char *reg = (char *)list_get(instruccion->params, 0);
        int value = atoi((char *)list_get(instruccion->params, 1));
        _establecer_registro(cpu_registers, reg, value);
        log_info(logger_cpu, "PID: <%d> - Ejecutando: <SET> - <%s %d>\n", pcb_execute->pid, reg, value);
        break;
    }
    case MOV_IN:
    {
        char *reg_datos = (char *)list_get(instruccion->params, 0);
        char *reg_direccion = (char *)list_get(instruccion->params, 1);

        uint32_t direccion_logica = _obtener_valor_registro(cpu_registers, reg_direccion);

        uint32_t valor_memoria = 0;
        exec_mov_in(direccion_logica, reg_datos, &valor_memoria);
        _establecer_registro(cpu_registers, reg_datos, valor_memoria);
        log_info(logger_cpu, "PID: <%d> - Ejecutando: <MOV_IN> - <%s %s>\n", pcb_execute->pid, reg_datos, reg_direccion);
        break;
    }
    case MOV_OUT:
    {
        char *reg_direccion = (char *)list_get(instruccion->params, 0);
        char *reg_datos = (char *)list_get(instruccion->params, 1);

        uint32_t direccion_logica = _obtener_valor_registro(cpu_registers, reg_direccion);
        uint32_t valor_datos = _obtener_valor_registro(cpu_registers, reg_datos);

        exec_mov_out(direccion_logica, &valor_datos, obtener_tamano_registro(reg_datos));

        log_info(logger_cpu, "PID: <%d> - Ejecutando: <MOV_OUT> - <%s %s>\n", pcb_execute->pid, reg_direccion, reg_datos);
        break;
    }
    case SUM:
    {
        char *reg_dest = (char *)list_get(instruccion->params, 0);
        char *reg_src = (char *)list_get(instruccion->params, 1);
        uint8_t valor_src = _obtener_valor_registro(cpu_registers, reg_src);
        uint8_t valor_operacion = _obtener_valor_registro(cpu_registers, reg_dest) + valor_src;
        _establecer_registro(cpu_registers, reg_dest, valor_operacion);
        log_info(logger_cpu, "PID: <%d> - Ejecutando: <SUM> - <%s %s>\n", pcb_execute->pid, reg_dest, reg_src);
        break;
    }
    case SUB:
    {
        char *reg_dest = (char *)list_get(instruccion->params, 0);
        char *reg_src = (char *)list_get(instruccion->params, 1);
        uint32_t valor_src = _obtener_valor_registro(cpu_registers, reg_src);
        uint32_t valor_operacion = _obtener_valor_registro(cpu_registers, reg_dest) - valor_src;
        _establecer_registro(cpu_registers, reg_dest, valor_operacion);
        log_info(logger_cpu, "PID: <%d> - Ejecutando: <SUB> - <%s %s>\n", pcb_execute->pid, reg_dest, reg_src);
        break;
    }
    case JNZ:
    {
        char *reg = (char *)list_get(instruccion->params, 0);
        int instruction_index = atoi((char *)list_get(instruccion->params, 1));
        uint32_t valor_reg = _obtener_valor_registro(cpu_registers, reg);
        uint32_t valor_cero = 0;
        if (valor_reg != valor_cero)
        {
            cpu_registers->pc = instruction_index;
            log_info(logger_cpu, "El registro %s era igual a cero, por lo tanto se salto a %d (JNZ)\n", reg, instruction_index);
            cambio_pc = true;
        }
        log_info(logger_cpu, "PID: <%d> - Ejecutando: <JNZ> - <%s %d>\n", pcb_execute->pid, reg, instruction_index);
        break;
    }
    case RESIZE:
    {
        int nuevo_tamano = atoi((char *)list_get(instruccion->params, 0));

        // Función para solicitar a la memoria el ajuste de tamaño
        if (!ajustar_tamano_proceso(pcb_execute->pid, nuevo_tamano))
        {
            informar_kernel_error("Out of Memory");
            out_of_memory = true;
        }
        else
        {
            log_info(logger_cpu, "PID: <%d> - Ejecutando: <RESIZE> - <%d>\n", pcb_execute->pid, nuevo_tamano);
        }
        break;
    }
    case COPY_STRING:
    {
        int tamano = atoi((char *)list_get(instruccion->params, 0));

        uint32_t direccion_origen = cpu_registers->si;
        uint32_t direccion_destino = cpu_registers->di;

        copiar_cadena(direccion_origen, direccion_destino, tamano);

        log_info(logger_cpu, "PID: <%d> - Ejecutando: <COPY_STRING> - <%d>\n", pcb_execute->pid, tamano);
        break;
    }
    case IO_GEN_SLEEP:
    {
        char *interface = (char *)list_get(instruccion->params, 0);
        int units = atoi((char *)list_get(instruccion->params, 1));

        log_info(logger_cpu, "PID: <%d> - Ejecutando: <IO_GEN_SLEEP> - <%s %d>\n", pcb_execute->pid, interface, units);
        solicitar_IO(instruccion);
        solicitud_io = true;
        break;
    }
    case WAIT:
    {
        // variable con nombre de recurso
        char *nombre_recurso = (char *)list_get(instruccion->params, 0);
        // logica de WAIT de un recurso:
        // mandar mensaje a kernel para que haga cositas con el recurso
        handle_wait_or_signal(pcb_execute, nombre_recurso, WAIT);
        log_info(logger_cpu, "PID: <%d> - Ejecutando: <WAIT> - <%s>\n", pcb_execute->pid, nombre_recurso);
        // activo flag:
        solicitud_recurso = true;
        break;
    }
    case SIGNAL:
    {
        // variable con nombre de recurso
        char *nombre_recurso = (char *)list_get(instruccion->params, 0);
        // logica de SIGNAL de un recurso:
        // mandar mensaje a kernel para que haga cositas con el recurso
        handle_wait_or_signal(pcb_execute, nombre_recurso, SIGNAL);
        log_info(logger_cpu, "PID: <%d> - Ejecutando: <SIGNAL> - <%s>\n", pcb_execute->pid, nombre_recurso);
        // activo flag:
        solicitud_recurso = true;
        break;
    }
    case IO_STDIN_READ:
    {
        char *interface = (char *)list_get(instruccion->params, 0);
        char *reg_direccion = (char *)list_get(instruccion->params, 1);
        char *reg_tamano = (char *)list_get(instruccion->params, 2);

        log_info(logger_cpu, "PID: <%d> - Ejecutando: <IO_STDIN_READ> - <%s %s %s>\n", pcb_execute->pid, interface, reg_direccion, reg_tamano);
        solicitar_IO(instruccion);
        solicitud_io = true;
        break;
    }
    case IO_STDOUT_WRITE:
    {
        char *interface = (char *)list_get(instruccion->params, 0);
        int units = atoi((char *)list_get(instruccion->params, 1));

        log_info(logger_cpu, "PID: <%d> - Ejecutando: <IO_STDOUT_WRITE> - <%s %d>\n", pcb_execute->pid, interface, units);
        solicitar_IO(instruccion);
        solicitud_io = true;
        break;
    }
    case IO_FS_CREATE:
    {
        char *interface = (char *)list_get(instruccion->params, 0);
        char *nombre_archivo = (char *)list_get(instruccion->params, 1);

        log_info(logger_cpu, "PID: <%d> - Ejecutando: <IO_FS_CREATE> - <%s %s>\n", pcb_execute->pid, interface, nombre_archivo);
        solicitar_IO(instruccion);
        solicitud_io = true;
        break;
    }
    case IO_FS_DELETE:
    {
        char *interface = (char *)list_get(instruccion->params, 0);
        char *nombre_archivo = (char *)list_get(instruccion->params, 1);

        log_info(logger_cpu, "PID: <%d> - Ejecutando: <IO_FS_DELETE> - <%s %s>\n", pcb_execute->pid, interface, nombre_archivo);
        solicitar_IO(instruccion);
        solicitud_io = true;
        break;
    }
    case IO_FS_TRUNCATE:
    {
        char *interface = (char *)list_get(instruccion->params, 0);
        char *nombre_archivo = (char *)list_get(instruccion->params, 1);
        char *reg_tamano = (char *)list_get(instruccion->params, 2);

        log_info(logger_cpu, "PID: <%d> - Ejecutando: <IO_FS_TRUNCATE> - <%s %s %s>\n", pcb_execute->pid, interface, nombre_archivo, reg_tamano);
        solicitar_IO(instruccion);
        solicitud_io = true;
        break;
    }
    case IO_FS_WRITE:
    {
        char *interface = (char *)list_get(instruccion->params, 0);
        char *nombre_archivo = (char *)list_get(instruccion->params, 1);
        char *reg_direccion = (char *)list_get(instruccion->params, 2);
        char *reg_tamano = (char *)list_get(instruccion->params, 3);
        char *reg_puntero = (char *)list_get(instruccion->params, 4);

        log_info(logger_cpu, "PID: <%d> - Ejecutando: <IO_FS_WRITE> - <%s %s %s %s %s>\n", pcb_execute->pid, interface, nombre_archivo, reg_direccion, reg_tamano, reg_puntero);
        solicitar_IO(instruccion);
        solicitud_io = true;

        break;
    }
    case IO_FS_READ:
    {

        char *interface = (char *)list_get(instruccion->params, 0);
        char *nombre_archivo = (char *)list_get(instruccion->params, 1);
        char *reg_direccion = (char *)list_get(instruccion->params, 2);
        char *reg_tamano = (char *)list_get(instruccion->params, 3);
        char *reg_puntero = (char *)list_get(instruccion->params, 4);

        log_info(logger_cpu, "PID: <%d> - Ejecutando: <IO_FS_READ> - <%s %s %s %s %s>\n", pcb_execute->pid, interface, nombre_archivo, reg_direccion, reg_tamano, reg_puntero);
        solicitar_IO(instruccion);
        solicitud_io = true;

        break;
    }
    case EXIT:
    {
        log_info(logger_cpu, "PID: <%d> - Ejecutando: <EXIT>\n", pcb_execute->pid);
        enviar_pcb_finalizado();
        llego_a_exit = true;
        break;
    }
    default:
    {
        printf("Instrucción no reconocida\n");
        break;
    }
    }
}

// FUNCIONES AUXILIARES DE EJECUCION DE INSTRUCCIONES:

// #############################################################################################################
//  TODO: REVISAR MACHEO DE INT, debuggear.
// #############################################################################################################
//  Función para obtener el puntero a un registro basado en su nombre:
uint32_t _obtener_registro(t_cpu_registers *registros, const char *nombre)
{

    // pc:
    if (strcmp(nombre, "PC") == 0)
    {
        return (uint32_t)registros->pc;
    }
    if (strcmp(nombre, "AX") == 0)
    {
        // log_info(logger_cpu, "AX = %u", (uint32_t)registros->ax);
        return (uint32_t)registros->ax;
        // uint32_t valor = (uint32_t)registros->ax;
        // return  valor;
    }
    if (strcmp(nombre, "BX") == 0)
    {
        // log_info(logger_cpu, "BX = %u", (uint32_t)registros->bx);
        return (uint32_t)registros->bx;
        //     uint32_t valor = (uint32_t)registros->bx;
        //     return  valor;
    }
    if (strcmp(nombre, "CX") == 0)
    {
        log_info(logger_cpu, "CX = %u", (uint32_t)registros->cx);
        return (uint32_t)registros->cx;
    }
    if (strcmp(nombre, "DX") == 0)
    {
        log_info(logger_cpu, "DX = %u", (uint32_t)registros->dx);
        return (uint32_t)registros->dx;
    }
    if (strcmp(nombre, "EAX") == 0)
    {
        log_info(logger_cpu, "EAX = %u", registros->eax);
        return registros->eax;
    }
    if (strcmp(nombre, "EBX") == 0)
    {
        log_info(logger_cpu, "EBX = %u", registros->ebx);
        return registros->ebx;
    }
    if (strcmp(nombre, "ECX") == 0)
    {
        log_info(logger_cpu, "ECX = %u", registros->ecx);
        return registros->ecx;
    }
    if (strcmp(nombre, "EDX") == 0)
    {
        log_info(logger_cpu, "EDX = %u", registros->edx);
        return registros->edx;
    }
    if (strcmp(nombre, "SI") == 0)
    {
        log_info(logger_cpu, "SI = %u", registros->si);
        return registros->si;
    }
    if (strcmp(nombre, "DI") == 0)
    {
        log_info(logger_cpu, "DI = %u", registros->di);
        return registros->di;
    }
    return EXIT_FAILURE;
}

void _establecer_registro(t_cpu_registers *registros, char *nombre, uint32_t valor)
{
    if (strcmp(nombre, "PC") == 0)
    {
        registros->pc = valor;
        cambio_pc = true;
    }
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
    uint32_t reg = _obtener_registro(registros, nombre);
    return reg ? reg : 0;
}

void informar_kernel_error(const char *mensaje)
{
    // TODO: Implementa la lógica para informar al Kernel sobre un error.
    log_info(logger_cpu, "Expulsion del PCB de pid <%d> debido a: %s\n", pcb_execute->pid, mensaje);
    send_pcb_kernel(MSG_CPU_OUT_OF_MEMORY);
}

void solicitar_instruccion(uint32_t pid, uint32_t pc)
{
    log_info(logger_cpu, "PID: <%d> - FETCH - Program Counter: <%d>", pid, pc);
    // Pido la siguiente instruccion a memoria
    send_get_next_instruction_memoria(pid, pc);
}

void recibir_pcb()
{
    sem_wait(&SEM_SOCKET_KERNEL_DISPATCH);

    // Recibo el pcb que manda kernel para ejecutar sus instrucciones
    pcb_execute = recv_pcb_kernel();

    // Cargo el contexto de ejecucion del pcb en los registros de la cpu
    cargar_contexto_ejecucion(pcb_execute);

    // Pido a memoria que me mande las instrucciones del proceso
    solicitar_instruccion(pcb_execute->pid, pcb_execute->program_counter);
}

void manejar_ciclo_de_instruccion()
{
    // FETCH: Recibo la instruccion que manda memoria
    t_instruction *instruccion = recv_instruction_memoria();

    log_info(logger_cpu, "Instrucción recibida de memoria");
    log_info(logger_cpu, "antes: AX: %u", cpu_registers->ax);
    log_info(logger_cpu, "antes: BX: %u", cpu_registers->bx);
    log_info(logger_cpu, "antes: CX: %u", cpu_registers->cx);
    log_info(logger_cpu, "antes: DX: %u", cpu_registers->dx);
    log_info(logger_cpu, "antes: EAX: %u", cpu_registers->eax);
    log_info(logger_cpu, "antes: EBX: %u", cpu_registers->ebx);
    log_info(logger_cpu, "antes: ECX: %u", cpu_registers->ecx);
    log_info(logger_cpu, "antes: EDX: %u", cpu_registers->edx);
    log_info(logger_cpu, "antes: SI: %u", cpu_registers->si);
    log_info(logger_cpu, "antes: DI: %u", cpu_registers->di);

    // EXECUTE: Ejecuto la instruccion recibida
    ejecutar_instruccion(instruccion, cpu_registers);
    // imprimir todos los cpu_registers:
    log_info(logger_cpu, "despues: AX: %u", cpu_registers->ax);
    log_info(logger_cpu, "despues: BX: %u", cpu_registers->bx);
    log_info(logger_cpu, "despues: CX: %u", cpu_registers->cx);
    log_info(logger_cpu, "despues: DX: %u", cpu_registers->dx);
    log_info(logger_cpu, "despues: EAX: %u", cpu_registers->eax);
    log_info(logger_cpu, "despues: EBX: %u", cpu_registers->ebx);
    log_info(logger_cpu, "despues: ECX: %u", cpu_registers->ecx);
    log_info(logger_cpu, "despues: EDX: %u", cpu_registers->edx);
    log_info(logger_cpu, "despues: SI: %u", cpu_registers->si);
    log_info(logger_cpu, "despues: DI: %u", cpu_registers->di);

    eliminar_instruccion(instruccion);

    // se debe hacer un "return;", si el proceso llego a exit
    if (llego_a_exit)
    {
        llego_a_exit = false; // seteamos en false para el siguiente pcb
        log_info(logger_cpu, "El PCB de pid <%d> se retira del ciclo de instruccion por una instruccion EXIT", pcb_execute->pid);
        return;
    }
    // se debe hacer un "return;", si el proceso maneja un recurso:
    if (solicitud_recurso)
    {
        solicitud_recurso = false;
        // log: se envio el proceso a kernel para el manejo de recursos
        log_info(logger_cpu, "El PCB de pid <%d> se envio al KERNEL para hacer un WAIT/SIGNAL un recurso (puede no volver) ", pcb_execute->pid);
        return;
    }

    // se debe hacer un "return;", si el proceso solicito una io ó hay un out of memory
    if (solicitud_io || out_of_memory)
    {
        solicitud_io = out_of_memory = false;
        return;
    }

    aumentar_program_counter();

    log_info(logger_cpu, "El PCB de pid <%d> tiene el pc en <%d>", pcb_execute->pid, pcb_execute->program_counter);

    // INTERRUPT: verificar y manejar interrupciones después de ejecutar la instrucción
    if (manejar_interrupcion())
        return;

    solicitar_instruccion(pcb_execute->pid, pcb_execute->program_counter);
}

bool manejar_interrupcion()
{
    pthread_mutex_lock(&MUTEX_INTERRUPT);
    if (interrupcion_pendiente)
    {
        log_info(logger_cpu, "Interrupción de %s recibida, devolviendo PCB al Kernel", get_string_from_interruption(tipo_de_interrupcion));
        cargar_contexto_ejecucion_a_pcb(pcb_execute);
        log_info(logger_cpu, "PCB enviado al Kernel");
        send_pcb_kernel_interruption(tipo_de_interrupcion); // aca esta la logica de cual mensaje enviar al kernel segun cual sea el tipo de interrupccion
        sem_post(&SEM_INTERRUPT);
        interrupcion_pendiente = false;
        pthread_mutex_unlock(&MUTEX_INTERRUPT);
        sem_post(&SEM_SOCKET_KERNEL_DISPATCH);

        return true;
    }
    else
    {
        pthread_mutex_unlock(&MUTEX_INTERRUPT);
    }
    return false;
}

void solicitar_IO(t_instruction *instruccion)
{
    aumentar_program_counter();
    cargar_contexto_ejecucion_a_pcb(pcb_execute);

    switch (obtener_nombre_instruccion(instruccion))
    {
    case IO_GEN_SLEEP:
        send_solicitud_io_generica_kernel(pcb_execute, instruccion);
        break;
    case IO_STDIN_READ:
        send_solicitud_io_stdin_kernel(pcb_execute, instruccion);
        break;
    case IO_STDOUT_WRITE:
        send_solicitud_io_stdout_kernel(pcb_execute, instruccion);
        break;
    case IO_FS_CREATE:
        send_solicitud_io_fs_create(pcb_execute, instruccion);
        break;
    case IO_FS_DELETE:
        send_solicitud_io_fs_delete(pcb_execute, instruccion);
        break;
    case IO_FS_TRUNCATE:
        send_solicitud_io_fs_truncate(pcb_execute, instruccion);
        break;
    case IO_FS_WRITE:
        send_solicitud_io_fs_write(pcb_execute, instruccion);
        break;
    case IO_FS_READ:
        send_solicitud_io_fs_read(pcb_execute, instruccion);
        break;
    default:

        break;
    }
    log_info(logger_cpu, "El PCB de pid <%d> se envio al KERNEL para solicitar una IO", pcb_execute->pid);
    sem_post(&SEM_SOCKET_KERNEL_DISPATCH);
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

void enviar_pcb_finalizado()
{
    cargar_contexto_ejecucion_a_pcb(pcb_execute);
    log_info(logger_cpu, "Se envia un PCB al Kernel por EXIT con PC= %d", pcb_execute->program_counter);
    send_pcb_kernel(MSG_PCB_KERNEL_EXIT);
    sem_post(&SEM_SOCKET_KERNEL_DISPATCH);

    // Controlo si llego una interrupcion
    if (interrupcion_pendiente)
    {
        pthread_mutex_lock(&MUTEX_INTERRUPT);
        interrupcion_pendiente = false; // la desestimo
        pthread_mutex_unlock(&MUTEX_INTERRUPT);
    }
}

void aumentar_program_counter()
{
    // si se toca el pc, ya sea por SET o por JNZ...
    if (cambio_pc)
    {
        cambio_pc = false;
        pcb_execute->program_counter = cpu_registers->pc;
        return;
    }
    // actualizar PC:
    cpu_registers->pc++;
    pcb_execute->program_counter = cpu_registers->pc;
}

void handle_wait_or_signal(t_PCB *pcb, char *resource_name, t_name_instruction tipo_de_interrupcion)
{
    t_msg_header msg_header;
    if (tipo_de_interrupcion == WAIT)
    {
        msg_header = MSG_CPU_KERNEL_WAIT;
    }
    else if (tipo_de_interrupcion == SIGNAL)
    {
        msg_header = MSG_CPU_KERNEL_SIGNAL;
    }
    else
    {
        log_error(logger_cpu, "Tipo de interrupcion invalido para recurso.");
        return;
    }

    remove_newline(resource_name);

    // actualizar PC:
    aumentar_program_counter();
    cargar_contexto_ejecucion_a_pcb(pcb);

    t_manejo_recurso *t_manejo_recurso = manejo_recurso_create(pcb, resource_name);
    // Crear un paquete con el PCB y el nombre del recurso (t_manjejo_recurso) y enviarlo al Kernel:

    // TODO: MODULARIZAR ESTA PARTE
    t_package *package = package_create(msg_header, get_manejo_recurso_size(t_manejo_recurso));
    serialize_manejo_recurso(package->buffer, t_manejo_recurso);

    // Enviar el paquete al Kernel:
    package_send(package, fd_kernel_dispatch);
    package_destroy(package);
    sem_post(&SEM_SOCKET_KERNEL_DISPATCH);

    manejo_recurso_destroy(t_manejo_recurso);
}
