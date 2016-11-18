# PPSDriverRdP

## [Laboratorio de Arquitectura de Computadoras - FCEFyN - UNC](http://computacion.efn.uncor.edu/lac)

### Practica Profesional Supervisada - PPS

  **Director de PPS:** Ing. Orlando Micolini
  
  **Integrantes:** Garcia Cannata Nicolas - Sosa Ludueña Gabriel
  
  **Tema:** Desarrollo de modulos sobre el kernel de Linux
  

<h1 align="center" >Nuevo Driver de RdP Generalizada</h1>
<p align="center">
 
<img src="https://github.com/gslAgile/PPSDriverRdP/blob/master/modulo_RdPG/intro2_RdPG.png" title="Intro RdP Generalizada">
<div align="center"></div>
 
</p>


### **Lo nuevo en el Driver RdP**
* En base a las transiciones sensibilizadas se debe generar un vetor de **transiciones sensibilizadas E**.
  - Esto implica disparar todas las transiociones **ti** de la RdP de manera implicita (sin modificar el marcado actual) para determinar
  cuales transiciones estan sensibilizadas creando el **vector E**.
* Como **B = H x Q**. Entonces se debe
  - Permitir el ingreso de un comando en los write del driver para cargar la **Matriz de brazos inhibidores B -> H**.
  - Se debe crear el **vector B de transiciones des-sensibilizadas** (por arcor inhibidor) a partir del **vector Q** cuyos elementos **qi**
  son **qi =cero(M(pi))** -> 0 si M(pi)>0 : 1 en el resto de los casos... logrando asi determinar cuales transiciones estan
  des-sensibilizadas y creando el **vector B**.
* Como **L = R x W**. Entonces se debe
  - Permitir el ingreso de un comando en los write del driver para caragar la **Matriz de brazos lectores L -> R**.
  - Se debe crear el **vector L de transiciones des-sensibilizadas** (por arcor lector) a partir del **vector W** cuyos elementos **wi**
  son **wi =uno(M(pi))** -> 1 si M(pi)>0 : 0 en el resto de los casos... logrando asi determinar cuales transiciones estan
  des-sensibilizadas y creando el **vector L**.
* Se permita el ingreso de comando en los write del driver para comando para cargar la **Matriz de brazos reset R**.

### **Lo nuevo en el programa de usuario de control del Driver RdP**
*
*
