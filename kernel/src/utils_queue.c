#include "utils_queue.h"

void agregar_de_execute_a_ready(t_PCB* pcb)
{

    sem_wait(&SEM_PLANIFICACION_READY_INICIADA);

    if(strcmp(obtener_algoritmo_planificacion(kernel_config), "VRR") == 0) {
        agregar_a_cola_ready_VRR(pcb);
    } else {
        agregar_a_cola_ready(pcb);
    }

    sem_post(&SEM_PLANIFICACION_READY_INICIADA);

}

void agregar_a_cola_ready_VRR(t_PCB* pcb){
    if (obtener_quantum(kernel_config) > pcb->quantum) {
        log_warning(logger_kernel, "El tiempo del cronometro es: <%d>", cronometro_obtener_tiempo());
        log_warning(logger_kernel, " PID: <%d> a la COLA AUXILIAR DE READY!!!. QUANTUM: %d ",pcb->pid, pcb->quantum);
            agregar_a_cola_aux_ready(pcb);
    } else {
        log_warning(logger_kernel, "El tiempo del cronometro es: <%d>", cronometro_obtener_tiempo());
        log_warning(logger_kernel, " PID: <%d> a la COLA READY NORMAL!!!. QUANTUM: %d ",pcb->pid ,pcb->quantum);
            agregar_a_cola_ready(pcb);
    }
}

void agregar_a_cola_ready(t_PCB* pcb) 
{
    pthread_mutex_lock(&MUTEX_READY);
        pcb->state = READY;
        //queue_push(COLA_READY, pcb);
        list_add(COLA_READY, pcb);
    pthread_mutex_unlock(&MUTEX_READY);

    // LOG obligatorio:
    // Ingreso a Ready: "Cola Ready / Ready Prioridad: [<LISTA DE PIDS>]"
    mostrar_elementos_de_cola(COLA_READY, "Ready");
    log_warning(logger_kernel, "###################### grado de multiprogramacion: %d#####################", grado_multiprogramacion);

    sem_post(&SEM_READY);
}

void agregar_a_cola_new(t_PCB* pcb)
{
    pthread_mutex_lock(&MUTEX_NEW);
    //queue_push(COLA_NEW, pcb);
    list_add(COLA_NEW, pcb);
    pthread_mutex_unlock(&MUTEX_NEW);
}

t_PCB* siguiente_pcb_cola_new()
{
    pthread_mutex_lock(&MUTEX_NEW);
    //t_PCB* pcb = queue_pop(COLA_NEW);
    t_PCB* pcb = list_remove(COLA_NEW, 0);
    pthread_mutex_unlock(&MUTEX_NEW);

    return pcb;
}

t_PCB *get_next_pcb_ready_to_exec()
{
    sem_wait(&SEM_CPU);
    t_PCB *pcb_a_tomar;
 
    //log_info(logger_kernel, "Se va a tomar el siguiente PCB de la cola de READY");
    
    // do{
    // sem_wait(&SEM_READY); // Espera a que haya un PCB en la cola de READ
    // pthread_mutex_lock(&MUTEX_READY);
    //     pcb_a_tomar = queue_pop(COLA_READY);
    // pthread_mutex_unlock(&MUTEX_READY);
    // } while(pcb_a_tomar->state == FINISHED);

    sem_wait(&SEM_READY); 
    pthread_mutex_lock(&MUTEX_READY);
    pcb_a_tomar = list_remove(COLA_READY, 0);    
    pthread_mutex_unlock(&MUTEX_READY);

    return pcb_a_tomar;
}

t_PCB *get_next_pcb_aux_ready_to_exec(){
    sem_wait(&SEM_CPU);
    sem_wait(&SEM_AUX_READY); // Espera a que haya un PCB en la cola de READY
    t_PCB *pcb_a_tomar;

    // do{
    // pthread_mutex_lock(&MUTEX_AUX_READY);
    //     pcb_a_tomar = queue_pop(COLA_AUX_READY);
    // pthread_mutex_unlock(&MUTEX_AUX_READY);
    // } while(pcb_a_tomar->state == FINISHED);

    pthread_mutex_lock(&MUTEX_AUX_READY);
    pcb_a_tomar = list_remove(COLA_AUX_READY, 0);
    pthread_mutex_unlock(&MUTEX_AUX_READY);
     
    return pcb_a_tomar;
}

void agregar_a_cola_aux_ready(t_PCB* pcb) 
{
    pthread_mutex_lock(&MUTEX_AUX_READY);
    pcb->state = READY;
    //queue_push(COLA_AUX_READY, pcb);
    list_add(COLA_AUX_READY, pcb);
    pthread_mutex_unlock(&MUTEX_AUX_READY);

    // LOG obligatorio:
    // Ingreso a Ready: "Cola Ready / Ready Prioridad: [<LISTA DE PIDS>]"
    mostrar_elementos_de_cola(COLA_AUX_READY, "Aux Ready");

    sem_post(&SEM_AUX_READY);
}

void agregar_de_new_a_ready(t_PCB* pcb)
{
    // semaforo:
    agregar_a_cola_ready(pcb);
}

void agregar_de_blocked_a_ready(t_PCB* pcb)
{
    sem_wait(&SEM_PLANIFICACION_READY_INICIADA);
    char* algoritmo_plani = obtener_algoritmo_planificacion(kernel_config);

    if(strcmp(algoritmo_plani, "FIFO") == 0){
        agregar_a_cola_ready(pcb);
    }
    else if(strcmp(algoritmo_plani, "RR") == 0){
        agregar_a_cola_ready(pcb);
    }
    else if(strcmp(algoritmo_plani, "VRR") == 0){
        // TODO: testear
        agregar_a_cola_ready_VRR(pcb);
    }
    sem_post(&SEM_PLANIFICACION_READY_INICIADA);
}

void agregar_a_cola_exit(t_PCB* pcb)
{
    pthread_mutex_lock(&MUTEX_EXIT);
    pcb->state = FINISHED;
    //queue_push(COLA_EXIT, pcb);
    list_add(COLA_EXIT, pcb);
    //log_debug(logger_kernel, "Se agregó el proceso %d a la cola EXIT", pcb->pid);
    pthread_mutex_unlock(&MUTEX_EXIT);

    sem_post(&SEM_EXIT);
}

t_PCB *get_next_pcb_exit()
{
    // sem_wait(&SEM_EXIT); // ESTO se hace a parte en end_process() dentro de largo_plazo.c
    t_PCB *pcb_a_tomar;

    //log_info(logger_kernel, "Se va a tomar el siguiente PCB de la cola de EXIT");

    pthread_mutex_lock(&MUTEX_EXIT);
        //pcb_a_tomar = queue_pop(COLA_EXIT);
        pcb_a_tomar = list_remove(COLA_EXIT, 0);
    pthread_mutex_unlock(&MUTEX_EXIT);

    return pcb_a_tomar;
}

// FUNCIONES AUXILIARES: TODO: llevar a modulo correspondiente
void mostrar_elementos_de_cola(t_list *COLA, char *nombre_cola) 
{

    t_list *lista_pids = listar_pids_de_queue(COLA);
    char *lista_pids_string = lista_a_string(lista_pids);
    log_info(logger_kernel, "Cola %s / %s Prioridad: %s",nombre_cola, nombre_cola, lista_pids_string);
    listar_pids_de_queue(COLA); // en realidad lo estamos sacando del diccionario pero a fines practicos es lo mismo
}

t_list* listar_pids_de_queue(t_list *queue) {
    t_list* pid_list = list_create();
    
    // if (queue == NULL || queue_is_empty(queue)) {
    //     return pid_list;
    // }

    if (queue == NULL || list_is_empty(queue)) {
        return pid_list;
    }

    // int size = queue_size(queue);
    int size = list_size(queue);
    for (int i = 0; i < size; i++) {
        //t_PCB *pcb = (t_PCB *) list_get(queue->elements, i);
        t_PCB *pcb = (t_PCB *) list_get(queue, i);
        uint32_t* pid = malloc(sizeof(uint32_t));
        *pid = pcb->pid;
        list_add(pid_list, pid);
    }

    return pid_list;
}

void execute_to_null() {
    pthread_mutex_lock(&MUTEX_EXECUTE);
    EXECUTE = NULL;
    pthread_mutex_unlock(&MUTEX_EXECUTE);
}
