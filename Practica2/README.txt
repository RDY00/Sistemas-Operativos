INTEGRANTES DEL EQUIPO:
* Fernando Márquez Pérez
* Juan Antonio Jasso Oviedo

PREGUNTAS Y RESPUESTAS:

1. En términos de la arquitectura Intel x86 ¿Qué significa que un thread o proceso está en ejecución?

Significa que un procesador está ejecutando sus instrucciones en ese momento, es decir, que estas están pasando por el ciclo fetch-decode-execute.

2. La función switch_threads es la encargada de cambiar de ejecución un hilo por otro. En resumen, la función sustituye los valores de ciertos registros. El registro EIP (Instruction Pointer o Program Counter) guarda el valor de la siguiente instrucción a ejecutar. ¿Por qué la función switch_threads no sustituye dicho registro?

Porque cuando se llama a la función switch_threads ambos hilos la estan ejecutando (como se comenta en switch.S), por lo que el tope del stack de ambos es igual (ambos tienen la llamada a esta funcion) y la EIP no requiere cambiarse pues se está ejecutando la misma función en el nuevo hilo.

3. De las dos técnicas para implementar el calendarizador de prioridades: mantener la lista ordenada o buscar el máximo. ¿Cúal es más factible utilizar? y ¿Por qué?.

Son equivalentes. Con la lista ordenada podemos obtener al siguiente hilo en tiempo constante pero al insertar tenemos tiempo linear justo porque tenemos que mantener el orden, mientras que para buscar el máximo podemos insertar en tiempo constante, pues no importa el orden de la lista, pero en el peor caso debemos recorrerla toda para obtener el siguiente hilo. Basicamente las complejidades de insersión y borrado se invierten y, como ambas funciones se usan con la misma frecuencia, no importa cuál de los métodos usemos, el desempeño será escencialmente el mismo.
