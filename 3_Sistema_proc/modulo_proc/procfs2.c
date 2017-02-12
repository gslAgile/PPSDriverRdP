/**
 * Procfs2.c - crear un "archivo" en /proc
 *
 */

#include <linux/module.h>	/* En concreto, un modulo */
#include <linux/kernel.h>	/* Trabajando en el nucleo */
#include <linux/proc_fs.h>	/* Necesario porque usamos proc fs */
#include <asm/uaccess.h>	/* Para copy_from_user */

#define PROCFS_MAX_SIZE		1024
#define PROCFS_NAME 		"buffer1k"

/* Licencia del módulo */
MODULE_LICENSE("GPL");

/*Autores*/
MODULE_AUTHOR("Ing. Micolini Orlando, Garcia Cannata , Sosa Ludueña");

/* Descripcion del modulo*/
MODULE_DESCRIPTION
("Escribe y lee caracteres desde el archivo de dispositivo asociado al presente modulo en /proc.");

/**
 * Esta estructura posee informacion sobre el archivo /proc
 *
 */
static struct proc_dir_entry *Our_Proc_File;

/**
 * El buffer usado para almacenar el caracter de este modulo
 *
 */
static char procfs_buffer[PROCFS_MAX_SIZE];

/**
 * El tamaño del buffer
 *
 */
static unsigned long procfs_buffer_size = 0;

/** 
 * Esta funcion es llamada cuando se lee el archivo /proc
 *
 */
int 
procfile_read(char *buffer,
	      char **buffer_location,
	      off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;
	
	printk(KERN_INFO "procfile_read (/proc/%s) llamado\n", PROCFS_NAME);
	
	if (offset > 0) {
		/* Hemos terminado de leer, devuelve un 0 */
		ret  = 0;
	} else {
		/* Llenar el buffer, devuelve tamaño del buffer */
		memcpy(buffer, procfs_buffer, procfs_buffer_size);
		ret = procfs_buffer_size;
	}

	return ret;
}

/**
 * Esta funcion se llama cuando el archivo /proc es escrito
 *
 */
int procfile_write(struct file *file, const char *buffer, unsigned long count,
		   void *data)
{
	/* Obtiene tamaño del buffer */
	procfs_buffer_size = count;
	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}
	
	/* Escribir datos en el buffer */
	if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}
	
	return procfs_buffer_size;
}

/**
 * Esta funcion es llamada cuando se carga el modulo
 *
 */
int init_module()
{
	/* Crear el archivo /proc */
	Our_Proc_File = create_proc_entry(PROCFS_NAME, 0644, NULL);
	
	if (Our_Proc_File == NULL) {
		remove_proc_entry(PROCFS_NAME, &proc_root);
		printk(KERN_ALERT "Error: No se pudo inicializar /proc/%s\n",
			PROCFS_NAME);
		return -ENOMEM;
	}

	Our_Proc_File->read_proc  = procfile_read;
	Our_Proc_File->write_proc = procfile_write;
	Our_Proc_File->owner 	  = THIS_MODULE;
	Our_Proc_File->mode 	  = S_IFREG | S_IRUGO;
	Our_Proc_File->uid 	  = 0;
	Our_Proc_File->gid 	  = 0;
	Our_Proc_File->size 	  = 37;

	printk(KERN_INFO "/proc/%s creado\n", PROCFS_NAME);	
	return 0;	/* Todo esta bien */
}

/**
 * Esta funcion es llamada cuando se descarga el modulo
 *
 */
void cleanup_module()
{
	remove_proc_entry(PROCFS_NAME, &proc_root);
	printk(KERN_INFO "/proc/%s eliminado\n", PROCFS_NAME);
}
