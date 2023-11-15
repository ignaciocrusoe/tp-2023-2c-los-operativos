#include "fcb.h"

t_fcb* crear_fcb()
{
    t_fcb* fcb = malloc(sizeof(t_fcb));
    return fcb;
}

void liberar_fcb(t_fcb* fcb)
{
    if(fcb->nombre != NULL)
    {
        free(fcb->nombre);
    }
    free(fcb);
}

t_fcb* leer_fcb(char* path_fcb, char* nombre)
{
    t_fcb* fcb = crear_fcb();
    char* ruta = malloc(strlen(path_fcb) + 1 + strlen(nombre) + 4 + 1);
	strcpy(ruta, path_fcb);
	uint32_t tam_archivo;
	strcat(ruta, "/");
    strcat(ruta, nombre);
    strcat(ruta, ".fcb");
    t_config* archivo_fcb = iniciar_config(ruta);
    fcb->nombre = config_get_string_value(archivo_fcb, "NOMBRE_ARCHIVO");
    fcb->bloque_inicial = config_get_int_value(archivo_fcb, "BLOQUE_INICIAL");
    fcb->tam_archivo = config_get_int_value(archivo_fcb, "TAMANIO_ARCHIVO");
    return fcb;
}