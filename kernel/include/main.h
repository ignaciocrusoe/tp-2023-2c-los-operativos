#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <sockets/client_utils.c>
#include <readline/readline.h>
#include <readline/history.h>
#include <pcb/pcb.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <log/log_utils.h>
#include <pcb/pcb.h>
#include <pthread.h>
//#include "./../src/planificador_largo_plazo.c"

typedef struct
{
    int socket_dispatch;
    int socket_interrupt;
    int socket_memoria;
    uint32_t quantum;
}t_args_hilo;

#endif 