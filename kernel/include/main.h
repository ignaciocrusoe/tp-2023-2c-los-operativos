#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include "./../../shared/src/client_utils.c"
#include "./../../shared/src/log_utils.c"
#include <readline/readline.h>
#include <readline/history.h>
#include "./../../shared/src/pcb.c"
#include <semaphore.h>
#include <commons/collections/queue.h>

t_log* logger;

sem_t grado_de_multiprogramacion;
sem_t mutex_cola_new;
sem_t mutex_cola_ready;

//t_queue* cola_new;
//t_queue* cola_ready;

#endif 