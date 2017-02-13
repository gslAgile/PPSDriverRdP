// Funciones de libreria
void generar_transiciones(struct matriz *T, struct matriz *MI);

// Implementacion de funciones
/*
* Descripcion de funcion: 
* Parametros:
* @param **x: Doble puntero de matriz de incidencia.
*/
void generar_transiciones(struct matriz *T, struct matriz *MI)
{
	int i,j,k;
	
	for(i=0; i<MI->columnas; i++)
	{
		T[i].filas=1;
		T[i].columnas=MI->columnas;
		
		T[i].matriz= crear_matriz(&T[i]);
		
		for(j=0; j<T[i].filas; j++)
		{
			for(k=0; k<T[i].columnas; k++)
			{
				if(i==k)
					T[i].matriz[j][k]=1;
				
				else
					T[i].matriz[j][k]=0;
			}
		}
	}
}

