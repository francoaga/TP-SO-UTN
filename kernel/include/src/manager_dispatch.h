#ifndef MANAGER_DISPATCH_H_
#define MANAGER_DISPATCH_H_

#include <pthread.h>
#include <commons/log.h>
#include <commons/string.h>
#include "variables_globales.h"
#include "protocolo_kernel.h"
#include "utils/estructuras.h"
#include "utils/solicitudes_io.h"
#include "kernel_config.h"
#include "datos_hilos.h"
#include "utils_queue.h"
#include "table_pcb.h"
#include "recursos.h"
#include "cronometro.h"
#include "manager_ios.h"

void procesar_ios_genericas();

// procesa un pcb para finalizarlo
// pre: motivo y pcb
// post: finaliza el pcb
void procesar_pcb_exit(t_motivo_exit motivo);

void cancelar_hilo_quantum(uint32_t);
void procesar_interrupcion_quantum();
void cancelar_quantum_si_corresponde(t_PCB *pcb_exit);

// Función para manejar la operación WAIT
void handle_wait_request();

// Función para manejar la operación SIGNAL
void handle_signal_request();

// logica que activa semaforos de retorno de pcb en VRR
// pre: pcb con el pid del proceso que termino
// post: activa el semaforo de retorno de pcb y agraga a la cola de retorno al pcb
void logica_pcb_retorno_vrr(t_PCB *pcb);

void procesar_ios_stdin();

void procesar_ios_stdout();

void procesar_ios_dialfs();

#endif //MANAGER_DISPATCH_