/*
El objetivo de esta tarea es que la visualización
de los dígitos del contador de la tarea anterior,
la realice una subrutina de servicio de interrupción,
de forma sistemática, cada 10 ms. Para ello,
generaremos una interrupción de tipo externa
cada 10 ms (frecuencia 100 Hz). Utilice un generador
de señal y, ajústelo adecuadamente, para obtener
la señal especificada y utilícela para generar una
interrupción externa de nivel 2, pin 19 de la tarjeta Arduino.

Veamos cuál sería la estructura general de un programa
para habilitar y tratar estas interrupciones adecuadamente.
Para más información consultar la documentación de las clases de teoría.

```
// Declaración de variables volátiles
// Las variables que se modifiquen en ISR() y se utilicen
// fuera de ISR() han de declararse “volatile”
// ejemplo:
volatile boolean estado;

// función de setup()
void setup(){
	// Habilitación de la interrupción INT2, flanco de subida (rising) cli();
	EICRA |= (1<<ISC21) | (1<<ISC20);
	EIMSK |= (1<<INT2);
	sei();
}

ISR (INT2_vect) {
	/*
	Rutina de servicio para la visualización entrelazada en el display de 4 dígitos de 7-segmentos y barrido del teclado (si es utilizado).
	Dependiendo de la frecuencia con la que se interrumpa (cada 10 ms, en nuestro caso) se ejecuta o se entra en esta función.
	En cada entrada en esta función se visualiza un dígito (y se explora la columna correspondiente del teclado si queremos integrarlo en la aplicación).
	Ejemplo:
	1a interrupción --> visualiza unidades y explora 1a columna teclado
	2a interrupción --> visualiza decenas y explora 2a columna teclado 3a interrupción --> visualiza centenas y explora 3 columna teclado 4a interrupción --> visualiza unidades de millar
	5a interrupción --> visualiza unidades y explora 1a columna teclado
	...
}
void loop() {
	...
}
```

Para implementar la visualización entrelazada se procederá
de forma que, la primera vez que se interrumpa, la ISR()
de la interrupción INT2 apagará todos los dígitos y activará
el dígito de las unidades (solo se visualizaría las unidades
del contador); en la segunda interrupción, la ISR() apagará
todos los dígitos y activará el dígito de las decenas (solo
se visualizará las decenas del contador) y así, sucesivamente,
de forma alternada cada 10 ms entre unidades, decenas, centenas
y unidades de millar, dependiendo de cuantos dígitos queramos
visualizar. Observe que, si baja la frecuencia del generador
de señal el display parpadeará y podrá observar el funcionamiento
de la visualización entrelazada.
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

volatile bool estado;
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
	unidades = 0;
	decenas = 0;
	time_old = millis();
	transition_time = 250;

	// Habilitación de la interrupción INT2
	cli();
	EICRA |= (1 << ISC21) | (1 << ISC20);
	EIMSK |= (1 << INT2);
	sei();
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
			logic_99();
			time_old = millis();
		}
	}
	else if (pdown == 0)
	{
		if (millis() - time_old > transition_time)
		{
			unidades--;
			logic_00();
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

	state();
}

ISR(INT2_vect)
{
	state();
}

void state()
{
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

void logic_99()
{
	// logica para que no se pase de 99
	if (unidades > 9)
	{
		unidades = 0;
		decenas++;
	}
	if (decenas > 9)
	{
		decenas = 0;
	}
}

void logic_00()
{
	// logica para que no se pase de 00
	if (unidades < 0)
	{
		unidades = 9;
		decenas--;
	}
	if (decenas < 0)
	{
		decenas = 9;
	}
}