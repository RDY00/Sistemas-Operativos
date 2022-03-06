INTEGRANTES DEL EQUIPO:
* Fernando Márquez Pérez
* Juan Antonio Jasso Oviedo

PREGUNTAS Y RESPUESTAS:

1. En términos de la arquitectura Intel x86 ¿Qué significa que un thread o proceso está en ejecución?

Significa, que el CPU le está dando recursos para que se pueda ejecutar, memoria, registros, etc. Esto se puede interpretar como que el proceso "tiene control del Procesador".

2. La función switch_threads es la encargada de cambiar de ejecución un hilo por otro. En resumen, la función sustituye los valores de ciertos registros. El registro EIP (Instruction Pointer o Program Counter) guarda el valor de la siguiente instrucción a ejecutar. ¿Por qué la función switch_threads no sustituye dicho registro?

Porque si la sustituyera, al regresar de la interrupt, no sabría el contexto del stack de ese nuevo hilo, sea NEXT. Es decir, con mantener el EIP y cambiar el ESP podemos conservar un contexto para que, cuando CUR(current) sea interrumpido NEXT tenga un contexto al cual acceder y el ESP apunte al stack de CUR.

3. De las dos técnicas para implementar el calendarizador de prioridades: mantener la lista ordenada o buscar el máximo. ¿Cúal es más factible utilizar? y ¿Por qué?.

Son equivalentes. Con la lista ordenada podemos obtener al siguiente hilo en tiempo constante pero al insertar tenemos tiempo linear justo porque tenemos que mantener el orden, mientras que para buscar el máximo podemos insertar en tiempo constante, pues no importa el orden de la lista, pero en el peor caso debemos recorrerla toda para obtener el siguiente hilo. Basicamente las complejidades de insersión y borrado se invierten y, como ambas funciones se usan con la misma frecuencia, no importa cuál de los métodos usemos, el desempeño será escencialmente el mismo.
