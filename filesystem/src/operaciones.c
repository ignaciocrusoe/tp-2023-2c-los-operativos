
#include "operaciones.h"
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

extern uint32_t cant_bloques_total;
extern uint32_t cant_bloques_swap;
extern uint32_t tam_bloque;
extern uint32_t retardo_acceso_bloque;
extern uint32_t retardo_acceso_fat;

extern t_list* archivos_abiertos;

extern t_fat* fat;

int32_t abrir_archivo(char* path_fcb, char* nombre)
{
	char* ruta = malloc(strlen(path_fcb) + 1 + strlen(nombre) + 4 + 1);
	strcpy(ruta, path_fcb);
	uint32_t tam_archivo;
	strcat(ruta, "/");
    strcat(ruta, nombre);
    strcat(ruta, ".fcb");
	int archivo_fcb = open(ruta, O_RDONLY);
	if(archivo_fcb == -1)
	{
		return -1;
	}
	//El archivo no va estar realmente abierto (o podríamos dejarlo abierto...).
    close(archivo_fcb);
    t_fcb* fcb = leer_fcb(path_fcb, nombre);
	list_add(archivos_abiertos, fcb);
	tam_archivo = fcb->tam_archivo;
	printf("aa_tam_archivo = %i\n", tam_archivo);
    return tam_archivo;
}

uint32_t crear_archivo(char* path_fcb, char* nombre)
{
    t_fcb* fcb = crear_fcb(nombre);
	char* ruta = malloc(strlen(path_fcb) + 1 + strlen(nombre) + 4 + 1);
	strcpy(ruta, path_fcb);
	uint32_t tam_archivo;
	strcat(ruta, "/");
    strcat(ruta, nombre);
    strcat(ruta, ".fcb");
	printf("PATH: %s", ruta);
	int archivo_fcb = open(ruta, O_CREAT | O_RDWR, 0664);
	t_config* config_fcb = iniciar_config(ruta);
	if(archivo_fcb == -1)
	{
		return -1;
	}
	list_add(archivos_abiertos, fcb);
	fcb->config = config_fcb;
	config_set_value(config_fcb, "NOMBRE_ARCHIVO", nombre);
	config_set_value(config_fcb, "TAMANIO_ARCHIVO", "0");
	config_set_value(config_fcb, "BLOQUE_INICIAL", int_to_string(UINT32_MAX));
	config_save(config_fcb);
	//El archivo no va estar realmente abierto (o podríamos dejarlo abierto...).
    close(archivo_fcb);
    return 1;
}

int32_t agrandar_archivo(t_fcb* archivo, uint32_t size)
{
	uint32_t por_asignar = size - archivo->tam_archivo;
	uint32_t tam_teorico = ceil(archivo->tam_archivo / tam_bloque) * tam_bloque;
	uint32_t bytes_libres = tam_teorico - archivo->tam_archivo;
	
	if(archivo->bloque_inicial == UINT32_MAX)
	{
		archivo->bloque_inicial = obtener_bloque_libre();
		fat->memory_map[archivo->bloque_inicial] = UINT32_MAX;
		uint32_t asignado = MIN(tam_bloque, por_asignar);
		archivo->tam_archivo += asignado;
		printf("pr_asignar = %i\n", por_asignar);
		por_asignar -= asignado;
		printf("Asigné el orimer bloque y me falta %i\n", por_asignar);
		config_set_value(archivo->config, "BLOQUE_INICIAL", int_to_string(archivo->bloque_inicial));
		config_save(archivo->config);

		if(por_asignar == 0)
		{
			return;
		}
		
	}
	if(bytes_libres > 0)
	{
		uint32_t asignado = MIN(bytes_libres, por_asignar);
		bytes_libres -= asignado;
		por_asignar -= asignado;
		archivo->tam_archivo += asignado;
	}
	while(por_asignar > 0)
	{
		uint32_t nuevo_bloque = obtener_bloque_libre();
		fat->memory_map[ultimo_bloque(archivo->bloque_inicial)] = nuevo_bloque;
		fat->memory_map[nuevo_bloque] = UINT32_MAX;

		uint32_t asignado = MIN(tam_bloque, por_asignar);
		archivo->tam_archivo += asignado;
		
		bytes_libres = tam_bloque - asignado;
		por_asignar -= asignado;
	}
	{
		return;
	}
}


uint32_t ultimo_bloque(uint32_t puntero)
{
	uint32_t puntero_sig = puntero;
	while(puntero_sig != UINT32_MAX)
	{
		puntero_sig = fat->memory_map[puntero];
		if(puntero_sig != UINT32_MAX)
		{
			puntero = puntero_sig;
		}
	}
	return puntero;
}

uint32_t penultimo_bloque(uint32_t puntero)
{
	uint32_t puntero_sig = puntero;
	uint32_t puntero_penult = puntero;
	while(puntero_sig != UINT32_MAX)
	{
		puntero_sig = fat->memory_map[puntero];
		if(puntero_sig != UINT32_MAX)
		{
			puntero = puntero_sig;
			puntero_penult = puntero;
		}
	}
	return puntero_penult;
}

void achicar_archivo(t_fcb* archivo, uint32_t size)
{
	uint32_t por_quitar = archivo->tam_archivo - size;
	uint32_t tam_teorico = ceil(archivo->tam_archivo / tam_bloque) * tam_bloque;
	uint32_t bytes_libres = tam_teorico - archivo->tam_archivo;

	if(por_quitar< bytes_libres)
	{
		archivo->tam_archivo -= por_quitar;
		return;
	}
	else
	{
		archivo->tam_archivo -= bytes_libres;
		por_quitar -= bytes_libres;

		while(por_quitar > 0)
		{
			uint32_t ult_bloque = ultimo_bloque(archivo->bloque_inicial);
			uint32_t penult_bloque = penultimo_bloque(archivo->bloque_inicial);
			fat->memory_map[ult_bloque] = 0;
			fat->memory_map[penult_bloque] = UINT32_MAX;
			archivo->tam_archivo -= MIN(tam_bloque, por_quitar);
			por_quitar -= MIN(tam_bloque, por_quitar);;
		}
	}
}

int32_t truncar_archivo(char* nombre, uint32_t size)
{
	printf("truncar_archivo\n");
	t_fcb* archivo = buscar_archivo(nombre, archivos_abiertos);
	printf("Encontré al archivo\n");

	if(size > archivo->tam_archivo) //Se amplía
	{
		agrandar_archivo(archivo, size);
		mem_hexdump(fat->memory_map, (cant_bloques_total - cant_bloques_swap) * sizeof(uint32_t));
		config_set_value(archivo->config, "TAMANIO_ARCHIVO", int_to_string(archivo->tam_archivo));
		config_save(archivo->config);
		printf("tam_archivo = %s\n", int_to_string(archivo->tam_archivo));
		fsync(fat->file_descriptor);
	}
	else if (size < archivo->tam_archivo) // Valido que no sea igual porque en ese caso no se hace nada
	{
		achicar_archivo(archivo, size);
		mem_hexdump(fat->memory_map, (cant_bloques_total - cant_bloques_swap) * sizeof(uint32_t));
		config_set_value(archivo->config, "TAMANIO_ARCHIVO", int_to_string(archivo->tam_archivo));
		config_save(archivo->config);
	}
	
archivo->tam_archivo = size;

}

