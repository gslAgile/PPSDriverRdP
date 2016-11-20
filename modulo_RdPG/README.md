# PPSDriverRdP

## [Laboratorio de Arquitectura de Computadoras - FCEFyN - UNC](http://computacion.efn.uncor.edu/lac)

### Practica Profesional Supervisada - PPS

  **Director de PPS:** Ing. Orlando Micolini
  
  **Integrantes:** Garcia Cannata Nicolas - Sosa Ludueña Gabriel
  
  **Tema:** Desarrollo de modulos sobre el kernel de Linux
  
  **Referencia:** [enlace externo](http://sedici.unlp.edu.ar/bitstream/handle/10915/56579/Documento_completo.pdf-PDFA.pdf?sequence=1)
  

<h1 align="center" >Diseño de nuevo Driver para RdP Generalizada</h1>
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
* Como A = marca(Re x Mj). Entonces se debe
  - Permitir el ingreso de un comando en los write del driver para cargar la **Matriz de brazos reset R -> Re**.
  - Se debe crear el **vector A de transiciones des-sensibilizadas** (por arcor reset) a partir de las marcas **Mj**.
  - Cada valor de A va ser 
     - 0 si (Re x Mj)>0 (hay brazo reset, se resetea valor de la marca)
     - es => 1 si (Re x Mj) = 0 (no hay brazo reset, se mantiene valor de la marca) 
     - Logrando asi determinar cuales transiciones estan des-sensibilizadas y creando el **vector A**.
* Se debera crear el **vector de sensibilizado extendido Ex** a partir de **Ex = E & B & L**
* Entonces para disparar la **RdPG** se va utilizar todos los vectores anteriores con la nueva ecuacion de estado
  - **Mj+1 = Mj + I x ((d & Ex)#A)**
  
### **Lo nuevo en el programa de usuario de control del Driver RdP**
* Adaptar el programa para disparar con la nueva funcion de disparo asociada a la RdPG.
* Permitir en el menu de opciones
   - La visualizacion de todas las matrices de incidencia (arcos inhibidor, lector, reset).
   - La visualizacion de los vectores de sensibilizado y des-sensibilizado (E, B, L, etc) como tambien el vector de sensibilizado extendido.
