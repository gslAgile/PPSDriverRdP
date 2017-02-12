#include <linux/module.h> /* Requerido por todos los módulos */
#include <linux/kernel.h> /* Definición de KERN_INFO */
#include <linux/init.h> 	/* Ofrece las macros de inicio y fin */
//#include <linux/kernel_levels.h>

MODULE_LICENSE("GPL"); /* Licencia del módulo */

int init_module(void) /* Función que se invoca cuando se carga el módulo en el kernel */
{
printk(KERN_ALERT "Modulo LIN cargado. Hola kernel.\n");
/* Devolver 0 para indicar una carga correcta del módulo */
return 0;
}

void cleanup_module(void) /* Función que se invoca cuando se descarga el módulo del kernel */
{
	printk(KERN_ALERT "Modulo LIN descargado. Adios kernel.\n");
}

/* Declaración de funciones init y cleanup
init_module(modulo_lin_init);
cleanup_module(modulo_lin_clean);//no funciona con las declaraciones init y cleanup*/
