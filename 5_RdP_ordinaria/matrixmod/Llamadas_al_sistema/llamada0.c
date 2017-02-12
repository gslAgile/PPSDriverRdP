#include <string.h>
#include <fcntl.h> /* modos de apertura funciones ioctl*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/* Variables globales*/
int fd; // file descriptor para escribir/leer el driver

/* Funciones */
void driver_write(char cadena[256]);
void driver_read(char *cadena);


int main()
{
	char cadena[256];
	
	// Write sobre driver
	driver_write("mostrar I\n");

	// Read fichero
	driver_read(cadena);

	printf("Cadena leida:\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
		
	driver_write("mostrar MA\n");

	// Read fichero
	driver_read(cadena);

	printf("Cadena leida: %s\n", cadena);

	return 0;
}


void driver_write(char cadena[256])
{
	/* Abrimos modulo matrixmod*/
	fd = open("/proc/matrixmod", O_RDWR); // abrimos file descriptor en modo escritura/lectura

	if(fd<0 ) /* Verificacion de creacion*/
	{
		//error
		perror("Error al abrir file descriptor.");
		exit(1);
	}
	
	// Write fichero
	if (write(fd, cadena, strlen(cadena)) != strlen(cadena))
	{
		printf("Error de escritura sobre driver\n");
		// Close de files descriptors
		close(fd);
		exit(1);
	}

	//printf("Escritura exitosa sobre driver\n");

	// Close de files descriptors
	close(fd);
}


void driver_read(char *cadena)
{
	/* Abrimos modulo matrixmod*/
	fd = open("/proc/matrixmod", O_RDWR); // abrimos file descriptor en modo escritura/lectura

	if(fd<0 ) /* Verificacion de creacion*/
	{
		//error
		perror("Error al abrir file descriptor.");
		exit(1);
	}

	// Read fichero
	if(read(fd, cadena, 256) < 1)
	{
		printf("Error de lectura sobre driver\n");
		// Close de files descriptors
		close(fd);
		exit(1);
	}

	//printf("Lectura exitosa sobre driver\n");

	// Close de files descriptors
	close(fd);
}