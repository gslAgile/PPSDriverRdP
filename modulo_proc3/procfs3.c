/* 
 *  procfs3.c -  crear un "archivo" en /proc, utiliza la forma file_operation 
 *  		 para gestionar el archivo.
 */
 
#include <linux/kernel.h>	/* Para trabajar en el kernel */
#include <linux/module.h>	/* Especificamente, un modulo */
#include <linux/proc_fs.h>	/* Necesario porque usamos proc fs */
#include <asm/uaccess.h>	/* para copy_from/to_user */

/* Licencia del módulo */
MODULE_LICENSE("GPL");

/*Autores*/
MODULE_AUTHOR("Ing. Micolini Orlando, Garcia Cannata , Sosa Ludueña");

/* Descripcion del modulo*/
MODULE_DESCRIPTION
("Escribe y lee caracteres desde el archivo de dispositivo asociado al presente modulo en /proc con una fops.");

#define PROC_ENTRY_FILENAME 	"buffer2k"
#define PROCFS_MAX_SIZE 	2048

/**
 * Buffer de (2k) para este modulo
 *
 */
static char procfs_buffer[PROCFS_MAX_SIZE];

/**
 * El tamaño de datos mantenido por el buffer
 *
 */
static unsigned long procfs_buffer_size = 0;

/**
 * La estructura que mantiene la informacion del archivo en /proc
 *
 */
static struct proc_dir_entry *Our_Proc_File;

/**
 * Esta funcion se llama cuando el archivo / proc se lee 
 *
 */
static ssize_t procfs_read(struct file *filp,	/* ver include/linux/fs.h   */
			     char *buffer,	/* buffer para llenar con los datos */
			     size_t length,	/* longitud del buffer     */
			     loff_t * offset)
{
	static int finished = 0;

	/* 
	 * Volvemos 0 para indicar el final del archivo, que no
	 * tenemos más información. De lo contrario, los procesos
	 * continuarán leyendo en un bucle sin fin. 
	 */
	if ( finished ) {
		printk(KERN_INFO "procfs_read: END\n");
		finished = 0;
		return 0;
	}
	
	finished = 1;
		
	/* 
	 * Utilizamos put_to_user para copiar la cadena del segmento
	 * de memoria del kernel para el segmento de memoria del
	 * proceso que nos ha llamado. get_from_user, por cierto, se
 	 * utiliza para la operacion inversa.
	 */
	if ( copy_to_user(buffer, procfs_buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}

	printk(KERN_INFO "procfs_read: read %lu bytes\n", procfs_buffer_size);

	return procfs_buffer_size;	/* Return el numero de bytes leidos "read" */
}

/*
 * Esta función es llamada cuando en el archivo de /proc se está escribiendo
 */
static ssize_t
procfs_write(struct file *file, const char *buffer, size_t len, loff_t * off)
{
	if ( len > PROCFS_MAX_SIZE )	{
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}
	else	{
		procfs_buffer_size = len;
	}
	
	if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}

	printk(KERN_INFO "procfs_write: write %lu bytes\n", procfs_buffer_size);
	
	return procfs_buffer_size;
}

/* 
  * Esta función decide si permite una operación (retorna cero) o no
  * lo permite (no devuelve cero, lo que indica por qué no está
  * permitido).
  *
  * La operación puede ser uno de los siguientes valores:
  * 0 - Ejecuta (ejecuta el "archivo" - sin sentido en nuestro caso)
  * 2 - escritura (entrada al módulo del kernel)
  * 4 - Lee (salida desde el módulo del núcleo)
  *
  * Esta es la verdadera función que comprueba los permisos de
  * archivo. Los permisos retornados por ls -l son sólo referencias,
  * y se pueden anular aquí.
 */

static int module_permission(struct inode *inode, int op, struct nameidata *foo)
{
	/* 
	 * Permitimos que todo el mundo pueda leer desde nuestro módulo , pero 
	 * sólo el root (uid 0) puede escribir en él
	 */
	if (op == 4 || (op == 2 && current->euid == 0))
		return 0;

	/* 
	 * Si se trata de cualquier otra cosa, el acceso se niega 
	 */
	return -EACCES;
}

/* 
  * El archivo se abre - que realmente no se preocupan por eso, pero sí  
  * significa que tenemos que incrementar la cuenta de referencia del
  * módulo. 
 */
int procfs_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

/* 
 * El archivo se cierra - de nuevo, importante sólo por 
 * la cuenta de referencia.
 */
int procfs_close(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;		/* exitoso */
}

static struct file_operations File_Ops_4_Our_Proc_File = {
	.read 	 = procfs_read,
	.write 	 = procfs_write,
	.open 	 = procfs_open,
	.release = procfs_close,
};

/* 
 * Operaciones de inodos para nuestro fichero proc. Lo necesitamos
 * por lo que tendremos un lugar para especificar la estructura de las
 * operaciones de archivo que queremos usar, y la función que utiliza
 * permisos. También es posible especificar las funciones que habrán de
 * exigirse para cualquier otra cosa que podría hacerse en un inodo (a
 * pesar de que no se moleste, sólo hay que poner NULL). 
 */

static struct inode_operations Inode_Ops_4_Our_Proc_File = {
	.permission = module_permission,	/* comprobar permisos */
};

/* 
 * Inicializacion y limpieza del modulo
 */
int init_module()
{
	/* creacion del archivo en /proc */
	Our_Proc_File = create_proc_entry(PROC_ENTRY_FILENAME, 0644, NULL);
	
	/* comprobar si tuvo exito la creacion del archivo en /proc */
	if (Our_Proc_File == NULL){
		printk(KERN_ALERT "Error: No se pudo inicializar /proc/%s\n",
		       PROC_ENTRY_FILENAME);
		return -ENOMEM;
	}
	
	Our_Proc_File->owner = THIS_MODULE;
	Our_Proc_File->proc_iops = &Inode_Ops_4_Our_Proc_File;
	Our_Proc_File->proc_fops = &File_Ops_4_Our_Proc_File;
	Our_Proc_File->mode = S_IFREG | S_IRUGO | S_IWUSR;
	Our_Proc_File->uid = 0;
	Our_Proc_File->gid = 0;
	Our_Proc_File->size = 80;

	printk(KERN_INFO "/proc/%s creado\n", PROC_ENTRY_FILENAME);

	return 0;	/* exito */
}

void cleanup_module()
{
	remove_proc_entry(PROC_ENTRY_FILENAME, &proc_root);
	printk(KERN_INFO "/proc/%s removido\n", PROC_ENTRY_FILENAME);
}
