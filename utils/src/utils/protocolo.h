#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

// Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

// Libraries
#include <inttypes.h>

// Commons
#include <sys/socket.h>
#include <commons/collections/list.h>

#include "estructuras.h"
#include "buffer.h"
#include "instruccion.h"
#include "next_instruction.h"

/*
 *  Messages: enum & structs
 */

// Message headers
typedef enum
{

    // KERNEL <-> CPU
    MSG_KERNEL_CPU_DISPATCH,
    MSG_CPU_DISPATCH_KERNEL,
    MSG_IO_KERNEL,
    MSG_KERNEL_IO,
    // KERNEL <-> MEMORIA
    MSG_KERNEL_MEMORIA,
    MSG_MEMORIA_KERNEL,
    MSG_KERNEL_CREATE_PROCESS,
    // KERNEL <-> IO
    MSG_IO_GEN_SLEEP,
    //CPU <-> MEMORIA
    MSG_MEMORIA_CPU,
    MSG_CPU_MEMORIA,
    MSG_INSTRUCTION_MEMORIA,
    MSG_NEXT_INSTRUCTION_CPU,
    //IO <-> MEMORIA
    MSG_IO_MEMORIA,
    EXAMPLE,

    //Empty package
    NULL_HEADER
} t_msg_header;

// Message package
typedef struct
{
    t_msg_header msg_header; // Message header
    t_buffer *buffer;        // Buffer
} t_package;

typedef struct
{
    char *cadena;
    uint8_t entero;
} t_message_example;

/*
 *  package functions
 */

// Package
t_package *package_create(t_msg_header msg_header , u_int32_t buffer_size);
void package_destroy(t_package *package);
int package_send(t_package *package, int fd);
int package_recv(t_package *package, int fd);
t_buffer* get_buffer(t_package*);
t_msg_header get_message_header(t_package*);


// serialize
void *serializar_paquete(t_package *paquete, int bytes);
void example_serialize_msg(t_buffer *buffer, t_message_example *msg);
void serialize_pcb(t_buffer *buffer, t_PCB *pcb);
void serialize_cpu_registers(t_buffer *buffer, t_cpu_registers *cpu_registers);

// deserialize
void example_deserialize_msg(t_buffer *buffer, t_message_example *msg);

t_PCB* deserialize_pcb(t_buffer *buffer);
void deserialize_cpu_registers(t_buffer *buffer, t_cpu_registers *cpu_registers);

void serialize_nuevo_proceso(t_buffer *buffer, t_new_process *nuevo_proceso);
t_new_process* deserialize_nuevo_proceso(t_buffer *buffer);
//send


/*********** Serialize and Deserialize 't_instruction' ***********/
// Serializa una instrucción en el buffer.
// pre: el buffer y la instrucción deben ser válidos y no NULL.
// post: la instrucción es serializada en el buffer.
void serialize_instruction(t_buffer*, t_instruction*);

// Deserializa una instrucción a partir de un buffer.
// Pre: El buffer debe ser válido y no NULL, debe contener datos.
// Post: Retorna un puntero a una estructura t_instruction con el nombre y los parámetros deserializados. 
t_instruction* deserialize_instruction(t_buffer*);

/*********** Serialize and Deserialize 't_next_instruction' ***********/
// Serializa una estructura t_next_instruction en un buffer.
// Pre: Los punteros a t_buffer y t_next_instruction deben ser válidos y no NULL.
// Post: Los datos de la estructura t_next_instruction se añade al buffer.
void serialize_next_instruction(t_buffer*, t_next_instruction*);

// Deserializa una estructura t_next_instruction a partir de un buffer.
// Pre: El puntero a t_buffer debe ser válido y no NULL.
// Post: Retorna un puntero a una estructura t_next_instruction creada a partir de los datos del buffer.
//       Si ocurre un error en la deserialización, retorna NULL.
t_next_instruction* deserialize_next_instruction(t_buffer*);

#endif