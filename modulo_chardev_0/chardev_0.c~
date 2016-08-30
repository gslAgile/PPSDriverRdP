/*
* chardev_0.c: Crea un solo un modulo de dispositivo de lectura/escritura de caracteres que 
* que puede ser escrito y leido desde su archivo de dispositivo en el directorio /dev
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h> /* para put_user */

/* Licencia del módulo */
MODULE_LICENSE("GPL");

/*Autores*/
MODULE_AUTHOR("Ing. Micolini Orlando, Garcia Cannata, Sosa Ludueña");

/* Descripcion del modulo*/
MODULE_DESCRIPTION
("Escribe y lee caracteres desde el archivo de dispositivo asociado al presente modulo en /dev.");

/*
* Prototipos − Esto normalmente deberia ir en un archivo ".h"
*/
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
#define SUCCESS 0 // exitoso
#define DEVICE_NAME "chardev_0" /* Dev nombre tal como aparece en /proc/devices */
#define BUF_LEN 256 /* Longitud máxima del mensaje desde el dispositivo */

/*
* Las variables globales se declaran como estática, por lo que son globales 
* dentro del archivo.
*/

static int Major;	/* Numero Major asignado para nuestro device driver */
static int Device_Open = 0; /* es un device open? */
			    /* Uso para prevenir multiples accesos en el dispositivo */

/* El mensaje que el dispositivo dará cuando se consulte
 * el archivo de dispositivo asociado al modulo */
static char msg[BUF_LEN]; 

/* Puntero que se asignara a la direccion del mensaje*/
static char *msg_Ptr;

// Interfaz de operaciones utilizadas en modulo
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

/*
* Esta funcion es llamada cuando el modulo es cargado.
*/
int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) {
		printk(KERN_ALERT "El registro del dispositivo de caracteres ha fallado con %d\n", Major);
		return Major;
	}
	printk(KERN_INFO "Me asignaron numero mayor %d.Para hablar con\n", Major);
	printk(KERN_INFO "el dispositivo, debe crearse un archivo en /dev con\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Para asociar el archivo con el modulo\n");
	printk(KERN_INFO "de dispositivos de caracteres.\n");
	printk(KERN_INFO "Pruebe varios números menores. Trate con cat y echo para\n");
	printk(KERN_INFO "el archivo de dispositivo.\n");
	printk(KERN_INFO "Borre el archivo de dispositivo y el modulo cuando termine.\n");

	return SUCCESS;
}

/*
* Esta funcion es llamada cuando el modulo es descargado.
*/
void cleanup_module(void)
{
	/*
	* Sacar registro del device.
	*/
	unregister_chrdev(Major, DEVICE_NAME);
	printk(KERN_INFO "El modulo de dispositivo de caracter se \n");
	printk(KERN_INFO "ha quitado exitosamente del kernel.\n");
}

/*
* Funciones relacionadas a la interfaz de operaciones
*/

/*
* Llamada cuando  un proceso intentó abrir el archivo de dispositivo, como
* "cat /dev/mycharfile".
*/
static int device_open(struct inode *inode, struct file *file)
{
	if (Device_Open)
		return -EBUSY;

	Device_Open++;

	msg_Ptr = msg;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

/*
* Se llama cuando un proceso cierra el archivo de dispositivo.
*/
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;
	/* No estamos listos para nuestro siguiente llamada*/
	/*
	* Decrementar el contador de uso, o de lo contrario una vez que ha
	* abierto el archivo, usted nunca se deshacera del módulo.
	*/
	module_put(THIS_MODULE);
	return SUCCESS;
}

/*
* Se llama cuando un proceso, que ya abrió el archivo dev, intenta 
* leer de él.
*/
static ssize_t device_read(struct file *filp, /* ver include/linux/fs.h*/
				char *buffer, /* buffer para rellenar con los datos */
				size_t length, /* longitud(tamaño) del buffer*/
				loff_t * offset)
{
	/*
	* Numero de bytes actualmente escritos en el buffer.
	*/
	int bytes_read = 0;

	/*
	* Si estamos en el final del mensaje, devuelve 0 significa el final del archivo.
	*/
	if (*msg_Ptr == 0)
		return 0;

	/*
	* Actualmente agrega(put) un dato en el buffer.
	*/
	while (length && *msg_Ptr) {
		/*
		* El buffer esta en el segmento de datos del usuario,
		* no en el del kernel (núcleo) por lo que la asignacion "*"
		* no funcionará. Tenemos que usar put_user que copia los datos
		* en el segmento de datos del núcleo al segmento de datos del usuario.
		*/
		put_user(*(msg_Ptr++), buffer++);
	
		length--;
		bytes_read++;
	}
	/*
	* La mayoría de las funciones de lectura (read) devuelven el número
	* de bytes puesto en el buffer.
	*/
	return bytes_read;
}

/*
* Se llama cuando un proceso se escribe en el archivo: echo "hi" > /dev/hello.
*/
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	int i;

	// contador de entradas a funcion de escritura
	static int counter = 1;
	
	// Limpiamos buffer global de modulo (msg[])
	memset(msg, '\0',  BUF_LEN);
	
	// guardamos cada caracter escrito en buff( en modo usuario) hacia msg[](en modo kernel)
	for (i = 0; i < len && i < BUF_LEN; i++)
                get_user(msg[i], buff + i);
	
	/* Agregamos el fin de cadena '\0' */
	msg[len] = '\0';
	
	// Asignamos puntero al espacio de memoria del mensaje msg[0]
	msg_Ptr = msg;
	
	return i;
}
