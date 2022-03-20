INTEGRANTES DEL EQUIPO:
* Fernando Márquez Pérez
* Juan Antonio Jasso Oviedo

PREGUNTAS Y RESPUESTAS:

1. ¿Por qué no utilizamos para el valor load_avg simplemente el número de
threads de la ready list?.

Porque no tendríamos una medida granular y a cada segundo de la verdadera carga del sistema. Esto porque requerimos que de cada proceso su "tiempo en ejecución" no valga lo mismo. Es decir hacemos uso del concepto visto en clase "no todo valor tiene el mismo peso, este va envejeciendo". Por eso si lo usamos únicamente con el largo o tamaño de la lista, es posible que pasemos por alto o que no se vuelvan tan dinámicas las prioridades como quisieramos ya que al obtener un promedio sobre la ejecución de cada proceso podemos manejar el "envejecimiento" de los valores de recent_cpu de cada proceso.


2. ¿Por qué razón al utilizar el módulo de punto flotante en modo kernel vuelve
 más lento al kernel?.

Primeramente porque el kernel maneja el cambio de entero a punto flotante cuando un proceso en modo usuario usa el módulo punto flotante. Esto mediante un "trap".
Y si el kernel es quien usa el módulo punto flotante, algunos no pueden
hacer "trap" a sí mismos. Además, hacer la transición(en modo kernel) requiere manualmente guardar y restaurar los registros de punto flotante, lo cual lleva tiempo y es ineficiente cuando hay muchos más procesos en el sistema.
