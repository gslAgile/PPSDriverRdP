/*
* chardev_2.c: Crea un solo un dispositivo de lectura/escritura de caracteres que 
* 
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h> 	/* Ofrece las macros de inicio y fin */
#include <asm/uaccess.h> /* para put_user */
#include <linux/ioctl.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

/* Licencia del módulo */
MODULE_LICENSE("GPL");

/*Autores*/
MODULE_AUTHOR("Ing. Micolini Orlando, Garcia Cannata , Sosa Ludueña");

/* Descripcion del modulo*/
MODULE_DESCRIPTION(
"Escribe y lee caracteres desde el archivo de dispositivo asociado al presente modulo en el /proc.");

/*
* Prototipos − this would normally go in a .h file.
*/
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
#define SUCCESS 0 // exitoso
#define DEVICE_NAME "chardev" /* Dev nombre tal como aparece en /proc/devices */
#define BUF_LEN 80 /* Longitud máxima del mensaje desde el dispositivo */

/*
* Las variables globales se declaran como estática, por lo que son globales dentro del archivo.
*/

static int Major;	/* Numero Major asignado para nuestro device driver */
static int Device_Open = 0; /* es un device open? */
				/* Uso para prevenir multiples accesos en el dispositivo */
static char msg[BUF_LEN]; /* El mensaje que el dispositivo dará cuando se consulte */
static char *msg_Ptr;

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
	if(proc_create(DEVICE_NAME, 0666, NULL, &fops) == NULL)
	   	return -ENOMEM;

	printk(KERN_INFO "Me asignaron numero mayor %d.Para hablar con\n", Major);
	printk(KERN_INFO "el dispositivo, debe crearse un archivo dev con\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Para asociar el archivo con el modulo de dispositivos de caracteres.\n");
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
	* Descargar el device.
	*/
	//int ret = unregister_chrdev(Major, DEVICE_NAME);
/*	if (unregister_chrdev(Major, DEVICE_NAME) < 0)
		printk(KERN_ALERT "Error en unregister_chrdev: %d\n", ret);*/
	unregister_chrdev(Major, DEVICE_NAME);
	printk(KERN_INFO "El modulo de dispositivo de caracter se \n");
	printk(KERN_INFO "ha quitado exitosamente del kernel.\n");
}

/*
* Metodos
*/

/*
* Llamada cuando  un proceso intentó abrir el archivo de dispositivo, como
* "cat /dev/mycharfile".
*/
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;
	
	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "Dije %d veces Hola Mundo!!!\n", counter++);
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
	* Decrementar el contador de uso, o de lo contrario una vez que ha abierto el archivo,
	* usted nunca se deshacera del módulo.
	*/
	module_put(THIS_MODULE);
	return 0;
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
		* El buffer esta en el segmento de datos del usuario, no en el del kernel (núcleo) 
		* por lo que la asignacion "*" no funcionará. Tenemos que usar put_user que copia
        	* los datos en el segmento de datos del núcleo al segmento de datos del usuario.
		*/
		put_user(*(msg_Ptr++), buffer++);
	
		length--;
		bytes_read++;
	}
	/*
	* La mayoría de las funciones de lectura (read) devuelven el número de bytes puesto en el buffer.
	*/
	return bytes_read;
}

/*
* Se llama cuando un proceso se escribe en el archivo: echo "hi" > /dev/hello.
*/
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "Perdon, pero esta operacion no es soportada.\n");
		return -EINVAL;
}
