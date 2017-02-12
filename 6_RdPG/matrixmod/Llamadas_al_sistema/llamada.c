#include <string.h>
#include <fcntl.h> /* modos de apertura funciones ioctl*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


int main()
{
	/* Abrimos modulo matrixmod*/
	char cadena[256];
	int fd1 = open("/proc/matrixmod", O_WRONLY); // file descriptor de escritura
	int fd2 = open("/proc/matrixmod", O_RDONLY); // file descriptor de lectura

	if(fd1<0 || fd2<0)
	{
		//error
		perror("Error al abrir file descriptor.");
		exit(1);
	}

	printf("Creacion exitosa\n");
	memset(cadena, '\0', 256);// se limpia cadena
	strcpy(cadena,"mostrar I\n");
	
	// Write fichero
	write(fd1, cadena, strlen(cadena));
	close(fd1);
	fd1 = open("/proc/matrixmod", O_WRONLY); // file descriptor de escritura
	
	memset(cadena, '\0', 256);// se limpia cadena

	// Read fichero
	read(fd2, cadena, 256);
	close(fd2);
	fd2 = open("/proc/matrixmod", O_RDONLY); // file descriptor de lectura

	printf("Cadena leida:\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
	strcpy(cadena,"mostrar MA\n");
	
	// Write fichero
	write(fd1, cadena, strlen(cadena));
	memset(cadena, '\0', 256);// se limpia cadena

	// Read fichero
	read(fd2, cadena, 256);

	printf("Cadena leida: %s\n", cadena);

	// Close de files descriptors
	close(fd1);
	close(fd2);

	return 0;


}
