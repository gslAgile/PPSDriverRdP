#include <linux/module.h> 	/* Requerido por todos los módulos */
#include <linux/kernel.h> 	/* Definición de KERN_INFO */
#include <linux/init.h> 	/* Ofrece las macros de inicio y fin */

/* Licencia del módulo */
MODULE_LICENSE("GPL");

/*Autores*/
MODULE_AUTHOR("Ing. Micolini Orlando, Garcia Cannata , Sosa Ludueña");

/* Descripcion del modulo*/
MODULE_DESCRIPTION("Se escribe 'Hola mundo' sobre el archivo de mensajes del kernel");

/* Funciones*/
int modulo_lin_init(void) /* Función que se invoca cuando se carga el módulo en el kernel */
{
    	printk(KERN_INFO "Modulo LIN cargado. Hola kernel.\n");
    	/* Devolver 0 para indicar una carga correcta del módulo */
    	return 0;
}

void modulo_lin_clean(void) /* Función que se invoca cuando se descarga el módulo del kernel */
{
	printk(KERN_INFO "Modulo LIN descargado. Adios kernel.\n");
}

/* Declaración de funciones init y cleanup */
module_init(modulo_lin_init);
module_exit(modulo_lin_clean);


