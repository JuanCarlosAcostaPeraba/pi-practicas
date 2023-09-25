/*
Implementar una aplicación con una funcionalidad similar
a la de un Turnomatic (dispositivo para dar un turno en
una cola de clientes) en el que la visualización en
el display y la exploración del teclado se haga de forma
entrelazada y sincronizada por una interrupción cada
5 ms generada por el pin 18 (INT3).

La aplicación se implementará de acuerdo a las siguientes especificaciones:

1. Contador de 3 dígitos
2. Control basado en 4 pulsadores con las siguientes funcionalidades:
	a. PUP: Incrementa el contador
	b. PDOWN: Decrementa el contador
	c. PENTER: Puesta a cero (reset) del contador
	d. PLEFT: El contador incrementará o decrementará su cuenta de 1 en 1
	e. PRIGHT: El contador incrementará o decrementará su cuenta de 2 en 2.
	f. Señal acústica por el altavoz del sistema cada vez cambie
		el estado del contador para así avisar a los clientes.
3. Inicialización del contador a cualquier valor, entre 000 y 999 mediante el teclado de 4x3
	a. Para modificar la cuenta con el teclado bastará
		con teclear los números y terminar con la tecla
		“#” que hará la función de “enter o entrar”. Ejemplo; 235#
4. Mostrar en la pantalla del PC (o en el terminal virtual de Proteus) el siguiente menú de opciones para poder elegir una de las acciones mostradas:
	1.- Modo normal de visualización (tres dígitos): OFF-centenas-decenas-unidades
	2.- Modo reducido-inferior de visualización (dos dígitos): OFF-OFF-decenas-unidades
	3.- Modo reducido-superior de visualización (dos dígitos): decenas-unidades-OFF-OFF
*/
