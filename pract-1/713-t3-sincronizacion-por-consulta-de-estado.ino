/*
Realizar un contador electrónico de dos dígitos (decenas-unidades)
que se actualiza dependiendo del pulsador que se pulse:
- pulsador “pup” incrementa en una unidad
- pulsador “pdown” decrementa en una unidad
- pulsador"pcenter" inicializa a “0”
La detección de la pulsación se ha de hacer por consulta de estado.
Si estando el contador a 00 se pulsa el pulsador “pdown” el contador
ha de cambiar a 99. Si con este último valor se pulsa “pup”
el contador deberá regresar a 00. Cada vez que cambie el estado
del contador ha de oírse un pitido corto por el zumbador.

En este apartado se deberá afrontar la complejidad adicional
de mostrar varios dígitos simultáneamente en el visualizador.
Para ello, es necesario de un mecanismo de visualización “entrelazada”
en el que se va alternando la visualización de las unidades
y las decenas a una cierta frecuencia (100Hz, 10ms).
Por ejemplo: en el primer ciclo (10 ms), se visualizan las unidades
y se apagan las decenas; en el segundo ciclo, se apagan las unidades
y se encienden las decenas; en el tercer ciclo se visualizan las
unidades y se apagan las decenas y así, sucesivamente. Si el
entrelazado se hace lentamente, se observa un parpadeo en el visualizador.
Sin embargo, a partir de una cierta frecuencia se ve estático
porque el ojo humano no es capaz de apreciar la conmutación entre dígitos.
A continuación, se muestra la estructura general del programa
que le puede ayudar a realizar este apartado:

```
boolean estado = false;
Setup(): Inicialización
Loop(): Programa principal {
	Leer pulsadores y actualizar variables del contador
	if (estado) {
		Apagar decenas, encender y visualizar unidades
		Delay 10ms
	} else {
		Apagar unidades, encender y visualizar decenas
		Delay 10ms
	}
	estado = !estado;
}
```
*/

// Pulsadores
#define PRIGHT 30	 // PC[7] pulsador right
#define PDOWN 31	 // PC[6] pulsador down
#define PLEFT 32	 // PC[5] pulsador left
#define PSELECT 33 // PC[4] pulsador select/enter
#define PUP 34		 // PC[3] pulsador up
#define PSTART 37	 // PC[0] speaker

// Display 8 segmentos
#define D4 49 // Pin 49 - unidades
#define D3 48 // Pin 48 - decenas
#define D2 47 // Pin 47 - centenas
#define D1 46 // Pin 46 - unidades de millar

bool estado;
int unidades;
int decenas;

int pup;
int pdown;
int pcenter;

long int time_old;
int transition_time;

// Matriz display 8 segmentos
char display_map[4] = {D4, D3, D2, D1};

// Matriz valores hexadecimales
char hexadecimal[16] = {
		'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'A', 'B',
		'C', 'D', 'E', 'F'};

// Matriz valores hexadecimales en binario
char hex_value[16] = {
		0x3F, 0x06, 0x5B, 0x4F,
		0x66, 0x6D, 0x7D, 0x07,
		0x7F, 0x6F, 0x77, 0x7C,
		0x39, 0x5E, 0x79, 0x71};

// Matriz teclado
char teclado_map[][3] = {
		{'1', '2', '3'},
		{'4', '5', '6'},
		{'7', '8', '9'},
		{'*', '0', '#'}};

void setup()
{
	Serial.begin(9600); // Inicializamos el puerto serie

	// Puerto A salida
	DDRA = B11111111;	 // Configuramos el puerto A como salida (0xFF)
	PORTA = B11111111; // Inicializamos el puerto A a 1 (0xFF)

	// Puerto L teclado
	DDRL = B00001111;	 // Configuramos los pines 0, 1, 2 y 3 del puerto L como entrada, y el resto como salida (0x0F)
	PORTL = B11111111; // Inicializamos el puerto L a 1 (0xFF)

	// Puerto C
	DDRC = B00000000;	 // Configuramos el pin 0 del puerto C como entrada (0x00)
	PORTC = B11111111; // Inicializamos el puerto C a 1 (0cFF)

	estado = false;
	unidades = 9;
	decenas = 1;
	time_old = millis();
	transition_time = 250;
}

void loop()
{
	pup = digitalRead(PUP);
	pdown = digitalRead(PDOWN);
	pcenter = digitalRead(PSELECT);

	if (pup == 0)
	{
		if (millis() - time_old > transition_time)
		{
			unidades++;
			// logica para que no se pase de 99
			time_old = millis();
		}
	}
	else if (pdown == 0)
	{
		if (millis() - time_old > transition_time)
		{
			unidades--;
			// logica para que no se pase de 99
			time_old = millis();
		}
	}
	else if (pcenter == 0)
	{
		if (millis() - time_old > transition_time)
		{
			unidades = 0;
			decenas = 0;
			time_old = millis();
		}
	}

	// Encender display
	if (estado)
	{
		// Apagar decenas, encender y visualizar unidades
		digitalWrite(D3, HIGH);
		PORTA = hex_value[unidades];
		digitalWrite(D4, LOW);
	}
	else
	{
		// Apagar unidades, encender y visualizar decenas
		digitalWrite(D4, HIGH);
		PORTA = hex_value[decenas];
		digitalWrite(D3, LOW);
	}

	estado = !estado;
}