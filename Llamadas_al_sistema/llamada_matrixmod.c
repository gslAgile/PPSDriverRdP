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
void mostrar_I(char *cadena);
void mostrar_MI(char *cadena);
void mostrar_MA(char *cadena);
void mostrar_MN(char *cadena);
void disparar_trasnsicion(char * cadena);


int main()
{
	char cadena[256];
	int n, opcion;

    while ( opcion != 6 )
    {
        printf( "\n   1. Ver matriz de incidencia de la RdP.");
        printf( "\n   2. Ver marcado inicial de la RdP.");
        printf( "\n   3. Ver marcado actual de la RdP.");
        printf( "\n   4. Ver marcado nuevo de la RdP.");
        printf( "\n   5. Disparar transicion sobre RdP.");
        printf( "\n   6. Salir." );
        printf( "\n\n   Introduzca opcion (1-6): ");

        scanf( "%d", &opcion );

        /* Inicio del anidamiento */

        switch ( opcion )
        {
            case 1: printf( "\n  Matriz de incidencia de la RdP:\n " );
                    mostrar_I(cadena);
                    break;

            case 2: printf( "\n  Marcado inicial de la RdP:\n" );
                    mostrar_MI(cadena);
                    break;

            case 3: printf( "\n  Marcado actual de la RdP:\n " );
                    mostrar_MA(cadena);
                    break;

            case 4: printf( "\n  Marcado nuevo de la RdP:\n " );
                    mostrar_MN(cadena);
                    break;

            case 5: printf( "\n  Introduzca comando de disparo de transicion: ");
            		scanf( "%s", cadena);
                    disparar_trasnsicion(cadena);
         }

         /* Fin del anidamiento */

    }

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


void mostrar_I(char *cadena)
{
	// Write sobre driver
	driver_write("mostrar I\n");

	// Read fichero
	driver_read(cadena);

	printf("\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
}


void mostrar_MI(char *cadena)
{
	// Write sobre driver
	driver_write("mostrar MI\n");

	// Read fichero
	driver_read(cadena);

	printf("\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
}


void mostrar_MA(char *cadena)
{
	// Write sobre driver
	driver_write("mostrar MA\n");

	// Read fichero
	driver_read(cadena);

	printf("\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
}


void mostrar_MN(char *cadena)
{
	// Write sobre driver
	driver_write("mostrar MN\n");

	// Read fichero
	driver_read(cadena);

	printf("\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
}


void disparar_trasnsicion(char *cadena)
{
	//
	printf("Cadena ingresada: %s\n\n", cadena);
	memset(cadena, '\0', 256);// se limpia cadena
}