#include "main.h"

t_registros* registros;

sem_t mutex_flag_interrupciones;

int flag_interrupciones;
int execute;
int flag_contexto_enviado;
t_motivo_desalojo motivo_desalojo;

uint32_t valor_de_registro(char* registro)
{
    uint32_t valor = 0;
     if(!strcmp(registro, "AX"))
    {
        valor = registros->AX;
    }
    else if(!strcmp(registro, "BX"))
    {
        valor = registros->BX;
    }
    else if(!strcmp(registro, "CX"))
    {
        valor = registros->CX;
    }
    else if(!strcmp(registro, "DX"))
    {
        valor = registros->DX;
    }

    return valor;
}

void sumar_a_registro(char* registro, uint32_t numero)
{
    if(!strcmp(registro, "AX"))
    {
        registros->AX += numero;
    }
    else if(!strcmp(registro, "BX"))
    {
        registros->BX += numero;
    }
    else if(!strcmp(registro, "CX"))
    {
        registros->CX += numero;
    }
    else if(!strcmp(registro, "DX"))
    {
        registros->DX += numero;
    }
}

void escribir_registro(char* registro, uint32_t numero)
{
    if(!strcmp(registro, "AX"))
    {
        registros->AX = numero;
    }
    else if(!strcmp(registro, "BX"))
    {
        registros->BX = numero;
    }
    else if(!strcmp(registro, "CX"))
    {
        registros->CX = numero;
    }
    else if(!strcmp(registro, "DX"))
    {
        registros->DX = numero;
    }
}

void restar_a_registro(char* registro, uint32_t numero)
{
    if(!strcmp(registro, "AX"))
    {
        registros->AX -= numero;
    }
    else if(!strcmp(registro, "BX"))
    {
        registros->BX -= numero;
    }
    else if(!strcmp(registro, "CX"))
    {
        registros->CX -= numero;
    }
    else if(!strcmp(registro, "DX"))
    {
        registros->DX -= numero;
    }
}

void sub(char* destino, char* origen){
    if(!strcmp(origen, "AX"))
    {
        restar_a_registro(destino, registros->AX);
    }
    else if(!strcmp(origen, "BX"))
    {
        restar_a_registro(destino, registros->BX);
    }
    else if(!strcmp(origen, "CX"))
    {
        restar_a_registro(destino, registros->CX);
    }
    else if(!strcmp(origen, "DX"))
    {
        restar_a_registro(destino, registros->DX);
    }
}

void sum(char* destino, char* origen){
    if(!strcmp(origen, "AX"))
    {
        sumar_a_registro(destino, registros->AX);
    }
    else if(!strcmp(origen, "BX"))
    {
        sumar_a_registro(destino, registros->BX);
    }
    else if(!strcmp(origen, "CX"))
    {
        sumar_a_registro(destino, registros->CX);
    }
    else if(!strcmp(origen, "DX"))
    {
        sumar_a_registro(destino, registros->DX);
    }
}

uint8_t atender_page_fault(uint32_t pid, uint32_t pagina, uint32_t frame, int socket_kernel_dispatch)
{
    if(frame == -1)
    {
        execute = 0;
        registros->PC--;
        enviar_contexto_de_ejecucion(registros, socket_kernel_dispatch);
        flag_contexto_enviado = 1;
        enviar_motivo_desalojo(socket_kernel_dispatch, PAGE_FAULT);
        send(socket_kernel_dispatch, &pagina, sizeof(uint32_t), NULL);
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]){
    t_log* logger = iniciar_logger("log_cpu.log","CPU");
    t_config* config = iniciar_config("./cfg/cpu.config");
    char* puerto_escucha_dispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    char* puerto_escucha_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
    char* ip_memoria = config_get_string_value(config,"IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
    int tam_pagina;

    printf("PUERTO_ESCUCHA=%s\n",puerto_escucha_dispatch);  
    int conexion_memoria = crear_conexion(logger, ip_memoria, puerto_memoria);
    
    int socket_servidor_dispatch = iniciar_servidor(logger,puerto_escucha_dispatch);
    int socket_servidor_interrupt = iniciar_servidor(logger,puerto_escucha_interrupt);
    int socket_kernel_dispatch = esperar_cliente(logger, socket_servidor_dispatch);
    if(socket_kernel_dispatch){
        log_info(logger,"Se conectó kernel al puerto dispatch");
    }
    int socket_kernel_interrupt = esperar_cliente(logger, socket_servidor_interrupt);
    if(socket_kernel_interrupt){
        log_info(logger,"Se conectó kernel al puerto interrupt");
    }
    /*
    if(strcmp(recibir_mensaje(conexion_memoria), "LISTO_PARA_RECIBIR_PEDIDOS"))
    {
        log_error(logger, "No se pudo establecer la conexión con Memoria");
    }
    */
   
 

    
    uint32_t pid = 0;
    uint32_t puntero, size;
    registros = malloc(sizeof(t_registros));
    flag_interrupciones = 0;
    sem_init(&mutex_flag_interrupciones, 0, 1);

    t_args_hilo args_conexion_kernel;
    args_conexion_kernel.socket_dispatch = socket_kernel_dispatch;
    args_conexion_kernel.socket_interrupt = socket_kernel_interrupt; 
    pthread_t hilo_interrupt_handler;
    pthread_create(&hilo_interrupt_handler, NULL, &recibir_interrupciones, (void*)&args_conexion_kernel);
    pthread_detach(&hilo_interrupt_handler);

                     t_pcb* pcb_prueba = crear_pcb(1,"");
                     void* serializado = serializar_contexto(pcb_prueba->contexto);
        pcb_prueba->contexto = deserializar_contexto(serializado);
        log_info(logger, "AX:%i - BX:%i - CX:%i - DX:%i - PC:%i", pcb_prueba->contexto->AX, pcb_prueba->contexto->BX, pcb_prueba->contexto->CX, pcb_prueba->contexto->DX, pcb_prueba->contexto->PC);
        
/*
          enviar_operacion(conexion_memoria, PEDIDO_SIZE_PAGINA);
    recv(conexion_memoria, &tam_pagina, sizeof(int), MSG_WAITALL);
    */
   /*HABRÍA QUE HACER UN HANDSHAKE*/
   tam_pagina = 16;


    while(1)
    {
        execute = 1;
        flag_contexto_enviado = 0;

        if(recv(socket_kernel_dispatch, &pid, sizeof(uint32_t), MSG_WAITALL) <= 0)
        {
            liberar_conexion(socket_kernel_dispatch);
            return;
        }
        log_info(logger, "recibí pid %i", pid);

        registros = recibir_contexto_de_ejecucion(socket_kernel_dispatch);
        
        //recv(socket_kernel_dispatch, &(registros->AX), sizeof(uint32_t), MSG_WAITALL);
        //recv(socket_kernel_dispatch, &(registros->BX), sizeof(uint32_t), MSG_WAITALL);
        //recv(socket_kernel_dispatch, &(registros->CX), sizeof(uint32_t), MSG_WAITALL);
        //recv(socket_kernel_dispatch, &(registros->DX), sizeof(uint32_t), MSG_WAITALL);
        //recv(socket_kernel_dispatch, &(registros->PC), sizeof(uint32_t), MSG_WAITALL);
        
        //t_pcb* pcb_prueba = crear_pcb(1,"");
        //pcb_prueba->contexto = registros;
        //log_info(logger, "AX:%i - BX:%i - CX:%i - DX:%i - PC:%i", pcb_prueba->contexto->AX, pcb_prueba->contexto->BX, pcb_prueba->contexto->CX, pcb_prueba->contexto->DX, pcb_prueba->contexto->PC);
        //
        /*CICLO DE INSTRUCCIÓN*/
        while(execute)
        {
            /*FETCH*/
            op_code codigo = FETCH_INSTRUCCION;
            //printf("pid= %i - ip= %i\n", pid, registros->PC);
            log_info(logger, "PID: %i - FETCH - Program Counter: %i", pid, registros->PC);
            send(conexion_memoria, &codigo, sizeof(op_code), 0);
            send(conexion_memoria, &pid, sizeof(uint32_t), 0);
            send(conexion_memoria, &(registros->PC), sizeof(uint32_t), 0) ;  //SERIALIZAR
            //log_info(logger, "Envié el pedido");     
            char* instruccion = recibir_mensaje(conexion_memoria);
            //log_info(logger, "%s", instruccion);
            

            /*DECODE*/
            char* parametros[4];
            {
                char* token;
                int i = 0;
                token = strtok(instruccion, " ");
                while(token != NULL && i < 4)
                {
                    parametros[i] = strdup(token);
                    token = strtok(NULL, " ");
                    i++;
                }
            }
            //log_info(logger, "%s", parametros[0]);
            //log_info(logger, "%s", parametros[1]);
            //log_info(logger, "%s", parametros[2]);
            //log_info(logger, "%s %s %s", parametros[0], parametros[1], parametros[2]);

            registros->PC++;
            /*EXECUTE*/
            
            /*Uso ifs anidados en lugar de pasar a enum y usar switch porque, independientemente de que
            usemos un enum, vamos a tener que hacer strcmp en ifs anidados.*/
            if(!strcmp(parametros[0], "SET"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s %s", pid, parametros[0], parametros[1], parametros[2]);
                if(!strcmp(parametros[1], "AX"))
                {
                    registros->AX = atoi(parametros[2]);
                }
                else if(!strcmp(parametros[1], "BX"))
                {
                    registros->BX = atoi(parametros[2]);
                }
                else if(!strcmp(parametros[1], "CX"))
                {
                    registros->CX = atoi(parametros[2]);
                }
                else if(!strcmp(parametros[1], "DX"))
                {
                    registros->DX = atoi(parametros[2]);
                }
                //log_info(logger, "AX:%i - BX:%i - CX:%i - DX:%i - PC:%i", pcb_prueba->contexto->AX, pcb_prueba->contexto->BX, pcb_prueba->contexto->CX, pcb_prueba->contexto->DX, pcb_prueba->contexto->PC);
            }
            else if(!strcmp(parametros[0], "SUM"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s %s", pid, parametros[0], parametros[1], parametros[2]);
                sum(parametros[1], parametros[2]);
            }
            else if(!strcmp(parametros[0], "SUB"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s %s", pid, parametros[0], parametros[1], parametros[2]);
                sub(parametros[1], parametros[2]);
            }
            else if(!strcmp(parametros[0], "JNZ"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s %s", pid, parametros[0], parametros[1], parametros[2]);
                uint32_t valor = valor_de_registro(parametros[1]);

                if(valor != 0)
                {
                    registros->PC = atoi(parametros[2]);
                }
            }
            else if(!strcmp(parametros[0], "SLEEP"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s", pid, parametros[0], parametros[1]);
                //sleep(atoi(parametros[1]) / 1000);
                execute = 0;
                uint32_t sleep_time = atoi(parametros[1]);
                enviar_contexto_de_ejecucion(registros, socket_kernel_dispatch);
                flag_contexto_enviado = 1;
                enviar_motivo_desalojo(socket_kernel_dispatch, SLEEP);
                send(socket_kernel_dispatch, &sleep_time, sizeof(uint32_t), NULL);
            }
            else if(!strcmp(parametros[0], "WAIT"))
            {
            log_info(logger, "PID: %i - Ejecutando: %s - %s", pid, parametros[0], parametros[1]);
            execute = 0;
            enviar_contexto_de_ejecucion(registros, socket_kernel_dispatch);
            flag_contexto_enviado = 1;
            enviar_motivo_desalojo(socket_kernel_dispatch, WAIT);
            enviar_mensaje(parametros[1],socket_kernel_dispatch);
            }
            else if(!strcmp(parametros[0], "SIGNAL"))
            {
            log_info(logger, "PID: %i - Ejecutando: %s - %s", pid, parametros[0], parametros[1]);
            execute = 0;
            enviar_contexto_de_ejecucion(registros, socket_kernel_dispatch);
            flag_contexto_enviado = 1;
            enviar_motivo_desalojo(socket_kernel_dispatch, SIGNAL);
            enviar_mensaje(parametros[1],socket_kernel_dispatch);
            }
            else if(!strcmp(parametros[0], "MOV_IN"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s %s", pid, parametros[0], parametros[1], parametros[2]);
                t_direccion_fisica* direccion = traducir_direccion(parametros[2], tam_pagina, conexion_memoria, pid);

                if(!atender_page_fault(pid, floor(atof(parametros[2])/tam_pagina), direccion->frame, socket_kernel_dispatch))
                {
                    enviar_operacion(conexion_memoria, PEDIDO_LECTURA);
                    enviar_direccion(conexion_memoria, direccion);
                    uint32_t a_escribir;
                    recv(conexion_memoria, &a_escribir, sizeof(uint32_t), NULL);
                    printf("Recibí %i de memoria\n", a_escribir);
                    escribir_registro(parametros[1], a_escribir);
                    log_info(logger, "PID: %i - Acción: LECTURA - Dirección Física: %i:%i - Valor: %i", pid, direccion->frame, direccion->offset, a_escribir);
                }
            }
            else if(!strcmp(parametros[0], "MOV_OUT"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s %s", pid, parametros[0], parametros[1], parametros[2]);
                t_direccion_fisica* direccion = traducir_direccion(parametros[1], tam_pagina, conexion_memoria, pid);
                
                if(!atender_page_fault(pid, floor(atof(parametros[1])/tam_pagina), direccion->frame, socket_kernel_dispatch))
                {
                    enviar_operacion(conexion_memoria, PEDIDO_ESCRITURA);
                    uint32_t a_enviar = valor_de_registro(parametros[2]);
                    enviar_direccion(conexion_memoria, direccion);
                    send(conexion_memoria, &a_enviar, sizeof(uint32_t), NULL);
                    log_info(logger, "PID: %i - Acción: ESCRIBIR - Dirección Física: %i:%i - Valor: %i", pid, direccion->frame, direccion->offset, a_enviar);
                }
            }
            else if(!strcmp(parametros[0], "F_OPEN"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s %s", pid, parametros[0], parametros[1], parametros[2]);
                execute = 0;
                enviar_contexto_de_ejecucion(registros, socket_kernel_dispatch);
                flag_contexto_enviado = 1;
                enviar_motivo_desalojo(socket_kernel_dispatch, F_OPEN);
                enviar_mensaje(parametros[1],socket_kernel_dispatch);
                enviar_mensaje(parametros[2],socket_kernel_dispatch);
            }
            else if(!strcmp(parametros[0], "F_CLOSE"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s", pid, parametros[0], parametros[1]);
                execute = 0;
                log_info(logger, "AX:%i - BX:%i - CX:%i - DX:%i - PC:%i", registros->AX, registros->BX, registros->CX, registros->DX, registros->PC);
                enviar_contexto_de_ejecucion(registros, socket_kernel_dispatch);
                flag_contexto_enviado = 1;
                enviar_motivo_desalojo(socket_kernel_dispatch, F_CLOSE);
                enviar_mensaje(parametros[1],socket_kernel_dispatch);
            
            }
            else if(!strcmp(parametros[0], "F_SEEK"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s %s", pid, parametros[0], parametros[1], parametros[2]);
                execute = 0;
                puntero = atoi(parametros[2]);
                enviar_contexto_de_ejecucion(registros, socket_kernel_dispatch);
                flag_contexto_enviado = 1;
                enviar_motivo_desalojo(socket_kernel_dispatch, F_SEEK);
                enviar_mensaje(parametros[1],socket_kernel_dispatch);
                send(socket_kernel_dispatch, &puntero, sizeof(uint32_t), NULL);
            
            }
            else if(!strcmp(parametros[0], "F_READ"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s %s", pid, parametros[0], parametros[1], parametros[2]);
                execute = 0;
                t_direccion_fisica* direccion_fisica = traducir_direccion(parametros[2], tam_pagina, conexion_memoria, pid); //revisar
                
                if(!atender_page_fault(pid, floor(atof(parametros[2])/tam_pagina), direccion_fisica->frame, socket_kernel_dispatch))
                {
                    enviar_contexto_de_ejecucion(registros, socket_kernel_dispatch);
                    enviar_motivo_desalojo(socket_kernel_dispatch, F_READ);
                    enviar_mensaje(parametros[1],socket_kernel_dispatch);
                    enviar_direccion(socket_kernel_dispatch, direccion_fisica);
                }
            }
            else if(!strcmp(parametros[0], "F_WRITE"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s %s", pid, parametros[0], parametros[1], parametros[2]);
                execute = 0;
                t_direccion_fisica* direccion_fisica = traducir_direccion(parametros[2], tam_pagina, conexion_memoria, pid);
                
                if(!atender_page_fault(pid, floor(atof(parametros[2])/tam_pagina), direccion_fisica->frame, socket_kernel_dispatch))
                {
                    enviar_contexto_de_ejecucion(registros, socket_kernel_dispatch);
                    flag_contexto_enviado = 1;
                    enviar_motivo_desalojo(socket_kernel_dispatch, F_WRITE);
                    enviar_mensaje(parametros[1],socket_kernel_dispatch);
                    printf("Direccion = %i:%i\n", direccion_fisica->frame, direccion_fisica->offset);
                    enviar_direccion(socket_kernel_dispatch, direccion_fisica);
                }
            }
            else if(!strcmp(parametros[0], "F_TRUNCATE"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s - %s %s", pid, parametros[0], parametros[1], parametros[2]);
                execute = 0;
                size = atoi(parametros[2]);
                enviar_contexto_de_ejecucion(registros, socket_kernel_dispatch);
                flag_contexto_enviado = 1;
                enviar_motivo_desalojo(socket_kernel_dispatch, F_TRUNCATE);
                enviar_mensaje(parametros[1],socket_kernel_dispatch);
                send(socket_kernel_dispatch, &size, sizeof(uint32_t), NULL);
            }
            else if(!strcmp(parametros[0], "EXIT"))
            {
                log_info(logger, "PID: %i - Ejecutando: %s", pid, parametros[0]);
                execute = 0;
                enviar_contexto_de_ejecucion(registros, socket_kernel_dispatch);
                flag_contexto_enviado = 1;
                enviar_motivo_desalojo(socket_kernel_dispatch, SUCCESS);
            }
            atender_interrupciones(socket_kernel_dispatch);
        }
    }
}