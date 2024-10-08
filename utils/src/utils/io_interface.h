#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <pthread.h>


// Definición del enum tipo_interfaz_t
typedef enum {
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS
} tipo_interfaz_t;

// Definición de la estructura para gestionar interfaces de E/S
typedef struct {
    char* nombre_interfaz;
    tipo_interfaz_t tipo_interfaz;
} t_IO_interface;

// Definición de la estructura para representar la conexión de una interfaz de E/S
typedef struct {
    char* nombre_interfaz;
    tipo_interfaz_t tipo_interfaz;
    int file_descriptor;
    t_list* cola_procesos_bloqueados;
    sem_t sem_cola_bloqueados;
    pthread_mutex_t mutex_cola_bloqueados;
} t_IO_connection;


//===============================================
// FUNCIONES DE TIPO_INTERFAZ_T
//===============================================

// Convierte un string en mayúsculas al valor correspondiente del enum tipo_interfaz_t.
// Pre: El parámetro str debe ser un string en mayúsculas que representa un tipo de interfaz válido.
// Post: La función retorna el valor correspondiente del enum tipo_interfaz_t.
//       Si el string no coincide con ningún tipo de interfaz, se retorna -1.
tipo_interfaz_t string_to_tipo_interfaz(char*);

// Convierte un valor de tipo_interfaz_t a su representación en cadena de caracteres.
// Pre: El parámetro tipo debe ser un valor válido de tipo_interfaz_t.
// Post: Retorna una cadena de caracteres que representa el tipo de interfaz.
//       Si el valor de tipo no corresponde a ningún caso conocido, retorna "UNKNOWN".
char* tipo_interfaz_to_string(tipo_interfaz_t);

//===============================================
// FUNCIONES DE T_IO_INTERFACE
//===============================================

// Función para crear una interfaz de E/S
// Pre: El puntero nombre debe apuntar a una cadena válida y no debe ser NULL.
//      El tipo debe ser un valor válido de tipo_interfaz_t.
// Post: Se crea una nueva interfaz de E/S y se retorna un puntero a la misma.
//       Si hay un error en la asignación de memoria, se retorna NULL.
t_IO_interface* crear_IO_interface(const char* , tipo_interfaz_t);

// Función para liberar la memoria de una interfaz de E/S
// Pre: El puntero debe apuntar a una estructura de interfaz de E/S válida y no debe ser NULL.
// Post: La memoria asociada a la interfaz de E/S es liberada.
void liberar_IO_interface(t_IO_interface*);

// Función para obtener el nombre de una interfaz de E/S
// Pre: El puntero debe apuntar a una estructura de interfaz de E/S válida y no debe ser NULL.
// Post: Se retorna el nombre de la interfaz de E/S.
char* obtener_nombre_IO_interface(t_IO_interface*);

// Función para obtener el tipo de una interfaz de E/S
// Pre: El puntero debe apuntar a una estructura de interfaz de E/S válida y no debe ser NULL.
// Post: Se retorna el tipo de la interfaz de E/S.
tipo_interfaz_t obtener_tipo_IO_interface(t_IO_interface*);

// Obtiene el tamaño de una estructura t_IO_interface.
// Pre: El puntero a t_IO_interface debe ser válido y no NULL.
// Post: Retorna el tamaño de la estructura t_IO_interface en bytes.
uint32_t obtener_size_IO_interface(t_IO_interface*);

//===============================================
// FUNCIONES DE T_IO_CONNECTION
//===============================================

// Función para crear una conexión de interfaz de E/S
// Pre: El puntero nombre debe apuntar a una cadena válida y no debe ser NULL.
//      El tipo debe ser un valor válido de tipo_interfaz_t.
//      El file_descriptor debe ser un descriptor de archivo válido.
// Post: Se crea una nueva conexión de interfaz de E/S y se retorna un puntero a la misma.
//       Si hay un error en la asignación de memoria, se retorna NULL.
t_IO_connection* crear_IO_connection(const char* nombre, tipo_interfaz_t tipo, int fd);

// Función para liberar la memoria de una conexión de interfaz de E/S
// Pre: El puntero conexión debe apuntar a una estructura de conexión de interfaz de E/S válida y no debe ser NULL.
// Post: La memoria asociada a la conexión de E/S es liberada.
void liberar_IO_connection(t_IO_connection* conexion);

// Función para obtener el nombre de una conexión de E/S
// Pre: El puntero conexión debe apuntar a una estructura de conexión de E/S válida y no debe ser NULL.
// Post: Se retorna el nombre de la conexión de E/S.
char* obtener_nombre_conexion(t_IO_connection* conexion);

// Función para obtener el tipo de una conexión de E/S
// Pre: El puntero conexión debe apuntar a una estructura de conexión de E/S válida y no debe ser NULL.
// Post: Se retorna el tipo de la conexión de E/S.
tipo_interfaz_t obtener_tipo_conexion(t_IO_connection* conexion);

// Función para obtener el descriptor de archivo de una conexión de E/S
// Pre: El puntero conexión debe apuntar a una estructura de conexión de E/S válida y no debe ser NULL.
// Post: Se retorna el descriptor de archivo de la conexión de E/S.
int obtener_file_descriptor(t_IO_connection* conexion);

// Función para obtener la cola de procesos bloqueados de una conexión de E/S
// Pre: El puntero conexión debe apuntar a una estructura de conexión de E/S válida y no debe ser NULL.
// Post: Se retorna el puntero a la cola de procesos bloqueados de la conexión de E/S.
t_list* obtener_cola_procesos_bloqueados(t_IO_connection* conexion);

// Obtiene el semáforo de la cola de procesos bloqueados de una conexión.
// Pre: El puntero a t_IO_connection debe ser válido y no NULL.
// Post: Retorna un puntero al semáforo de la cola de procesos bloqueados, o NULL si la conexión es NULL.
sem_t* obtener_semaforo_cola_bloqueados(t_IO_connection* conexion);

// Obtiene el mutex de la cola de procesos bloqueados de una conexión.
// Pre: El puntero a t_IO_connection debe ser válido y no NULL.
// Post: Retorna un puntero al mutex de la cola de procesos bloqueados, o NULL si la conexión es NULL.
pthread_mutex_t* obtener_mutex_cola_bloqueados(t_IO_connection* conexion);

// Obtiene un proceso bloqueado de la cola de procesos bloqueados de una conexión.
// Pre: El puntero a t_IO_connection debe ser válido y no NULL.
// Post: Retorna un puntero al proceso bloqueado extraído de la cola, o NULL si la cola está vacía o la conexión es NULL.
void* obtener_proceso_bloqueado(t_IO_connection* conexion);

// Agrega un proceso bloqueado a la cola de procesos bloqueados de una conexión.
// Pre: El puntero a t_IO_connection y el puntero al proceso deben ser válidos y no NULL.
// Post: Retorna true si el proceso fue agregado exitosamente, false en caso contrario.
bool agregar_proceso_bloqueado(t_IO_connection* conexion, void* proceso);

// Verifica si una conexión tiene procesos bloqueados en su cola.
// Pre: El puntero a t_IO_connection debe ser válido y no NULL.
// Post: Retorna true si la cola de procesos bloqueados está vacía, false en caso contrario.
bool tiene_procesos_bloqueados(t_IO_connection* cliente_io);

#endif // IO_INTERFACE_H
