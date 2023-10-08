# include "pcb.h"

uint32_t contador_pid;

t_list* parsear_instrucciones(char* str)
{
    if(str == NULL)
    {
       return NULL;
    }
    t_list* instrucciones = list_create();
    char* str_cpy = strdup(str);
    char* token; strtok(str_cpy, "\n");
    while(token != NULL)
    {
        if (strlen(token) > 0)
        {
            char* token_cpy = strdup(token);
            token_cpy[strlen(token_cpy)] = '\0';
            list_add(instrucciones, token_cpy);
        }
        token = strtok(NULL, "\n");
    }
    return instrucciones;
}

t_pcb* crear_pcb(uint32_t prioridad)
{
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->prioridad = prioridad;
    pcb->estado = NEW;
    pcb->pid = contador_pid;
    contador_pid++;
    //pcb->contexto
    pcb->tabla_de_archivos_abiertos = list_create();
    return pcb;
}

void liberar_pcb(t_pcb* pcb)
{
    list_destroy_and_destroy_elements(pcb->tabla_de_archivos_abiertos, free);
    free(pcb->contexto);
    free(pcb);
}