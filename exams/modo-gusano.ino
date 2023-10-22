/*
Partiendo del proyecto base implementar una nueva funcionalidad (modo gusano)
consistente en desplazar la visualización de un segmento según un bucle formado
por los 4 dígitos del display de 7-segmentos.

Para entrar en este modo de visualización se utilizará una nueva opción del menú
(opción 4) que aparecerá en el “virtual terminal”.

Para implementar esta nueva
funcionalidad será necesario añadir dos nuevos pulsadores, “pright2” y “pleft2”,
que utilizan las interrupciones
como método de sincronización. Las
interrupciones quedarán configuradas como:

* Pulsador “pright2” al pin 21 (interrupción externa INT0)
* Pulsador “pleft2” al pin 20 (interrupción externa INT1)
* La interrupción ISR(INT3_vect) será sustituidapor otra generada
por el Timer 3 cuando se produzca overflow: ISR(TIMER3_OVF_vect).

Modo Turnomatic: En las opciones 1, 2 y 3, el sistema trabaja en modo
Turnomatic según las especificaciones del proyecto base de partida.

Modo gusano: Cuando se entra en opción 4, la variable “sentido”
(modificada por los pulsadores “pright2” y “pleft2”) indicará si el
desplazamiento del “segmento encendido” se hace hacia la derecha o hacia
la izquierda. Cada pulsación generará una interrupción que
será atendida por la correspondiente ISR() que actualizará la
variable “sentido” de acuerdo a:

Si se pulsa “pright2” entonces sentido = 0 movimiento a la derecha
Si se pulsa “pleft2” entonces sentido = 1 movimiento a la izquierda

La estructuración del programa se puede hacer en base a
las siguientes funciones básicas:

// Inicialización de variables globales
void Setup()
{
	// inicialización de puertos
	//inicializar interrupciones externas: INT0 (pin21, pright2, falling), INT1 (pin 20, pleft2, falling), TIMER3_OVF (interrupción cada 4 ms, 250 Hz).
	// Definir pines 20 y 21 de entrada y activar resistencias de pull-up
}

ISR(TIMER3_OVF_vect)
{
	// gestión del barrido entrelazado del display de 7-segmentos. Se activa cada 4 ms.
}

ISR(¿?)
{
	// Interrupción INT0, pin 21, pulsador “pright2”. Gestión del pulsador “pright2” para el modo gusano
}

ISR(¿?)
{
	// Interrupción INT1, pin 20, pulsador “pleft2”. Gestión del pulsador “pleft2” para el modo gusano
}

void loop() {
	// Si (opción == 1 || opción == 2 || opción == 3) funcionamiento modo Turnomatic
	// Si (opción == 4)à funcionamiento en modo gusano. Realiza un bucle completo hacia la derecha o hacia la izquierda dependiendo de la variable sentido. El “sentido” en el que se va encendiendo el segmento activo se cambia con los pulsadores “pright2” y “pleft2”. Cuando termina un bucle completo del gusano vuelve a loop() donde comprobará si ha llegado una nueva opción del menú que implique un cambio de modo.
}

Especificaciones para programar el Timer 3:
* Modo PWM, Phase correct, TOP = ICR3 , T = 4 ms (periodo señal PWM)
* Frecuencia de la interrupción: Usar la interrupción por
overflow (TIMER3_OVF_vect), que se producirá cada 4 ms (cuando
el contador del timer llega al BOTTOM).
* TOP (ICR3): Calcular el valor del TOP suponiendo que el preescaler se programa a N=64
* Generar una señal PWM por el canal OC3B con las características que se muestran a continuación:
T = 4 ms
Interrupción 1.6 ms
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

// Array display 8 segmentos
char display_map[4] = {D4, D3, D2, D1};

// Teclado
#define ROW0 42 // PL[7] fila 0
#define ROW1 43 // PL[6] fila 1
#define ROW2 44 // PL[5] fila 2
#define ROW3 45 // PL[4] fila 3

#define DOFF B00001111;
#define DON B00000000;

#define PRIGHT2 21 // INT0
#define PLEFT2 20	 // INT1

#define TOP 0x01F4 // 500

// Array valores hexadecimales
char hexadecimal[16] = {
		'0', '1', '2', '3', // 0, 1, 2, 3
		'4', '5', '6', '7', // 4, 5, 6, 7
		'8', '9', 'A', 'B', // 8, 9, A, B
		'C', 'D', 'E', 'F'	// C, D, E, F
};

// Array valores hexadecimales en binario
char hex_value[16] = {
		0x3F, 0x06, 0x5B, 0x4F, // 0, 1, 2, 3
		0x66, 0x6D, 0x7D, 0x07, // 4, 5, 6, 7
		0x7F, 0x6F, 0x77, 0x7C, // 8, 9, A, B
		0x39, 0x5E, 0x79, 0x71	// C, D, E, F
};

// Matriz teclado
char teclado_map[][3] = {
		{'1', '2', '3'},
		{'4', '5', '6'},
		{'7', '8', '9'},
		{'*', '0', '#'}};

volatile int digit;
volatile char option;
String buffer;

int contador;
int increment;

int pup;
int pdown;
int pcenter;
int pleft;
int pright;

int pright2;
int pleft2;

volatile int sentido; // 0 derecha, 1 izquierda

char gusano_states[12] = {
		0x1, 0x1, 0x1, 0x1,
		0x2, 0x4, 0x8, 0x8,
		0x8, 0x8, 0x10, 0x20};

long int time_old;
int transition_time;

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

	// Habilitacion de la interrupcion externa INT0 (pin 21, pright2, falling)
	cli();
	pinMode(PRIGHT2, INPUT);
	EICRA = B00000010; // Configuramos la interrupcion externa INT0 para que se produzca en flanco de bajada (0x02)
	EIMSK = B00000001; // Habilitamos la interrupcion externa INT0 (0x01)
	sei();

	// Habilitacion de la interrupcion externa INT1 (pin 20, pleft2, falling)
	cli();
	pinMode(PLEFT2, INPUT);
	EICRA = B00001000; // Configuramos la interrupcion externa INT1 para que se produzca en flanco de bajada (0x08)
	EIMSK = B00000010; // Habilitamos la interrupcion externa INT1 (0x02)
	sei();

	// Modo PWM Phase Correct
	// f = 16 MHz / (N * 2 * TOP)
	// f = 250 Hz; N = 64; TOP = 500
	// Habilitacion de la interrupcion por overflow del Timer 3
	cli(); // Deshabilitamos las interrupciones

	pinMode(5, OUTPUT); // OC3A
	pinMode(2, OUTPUT); // OC3B
	pinMode(3, OUTPUT); // OC3C

	TCCR3A = TCCRB = TCCR3C = 0; // resetear registros
	TCNT3 = 0;									 // Inicializamos el contador a 0

	OCR3A = 0;
	OCR3B = 0x0001;
	OCR3C = 0;

	ICR3 = TOP;

	TCCR3A = B00100010;
	TCCR3B = B00010011;

	TIMSK3 = B00000001;
	sei();

	digit = 0;
	buffer = "";

	contador = 0;
	increment = 1;

	time_old = millis();
	transition_time = 550;

	menu();
}

void loop()
{
	if (Serial.available() > 0)
	{
		option = Serial.read();
	}

	pup = digitalRead(PUP);
	pdown = digitalRead(PDOWN);
	pcenter = digitalRead(PSELECT);
	pleft = digitalRead(PLEFT);
	pright = digitalRead(PRIGHT);

	pright2 = digitalRead(PRIGHT2);
	pleft2 = digitalRead(PLEFT2);

	read_buffer();
	buttons_increment();
}

// TODO ISR(INT0_vect)

// TODO ISR(INT1_vect)

ISR(TIMER3_OVF_vect)
{
	PORTL = DOFF;
	switch (digit)
	{
	case 0:
		if (option == '1' || option == '2')
		{
			PORTA = hex_value[contador % 10];
		}
		else if (option == '3')
		{
			PORTA = 0x00;
		}
		else if (option == '4')
		{
			gusano();
		}
		PORTL = B00001110;
		keyboard(digit);
		digit++;
		break;
	case 1:
		if (option == '1' || option == '2')
		{
			PORTA = hex_value[(contador / 10) % 10];
		}
		else if (option == '3')
		{
			PORTA = 0x00;
		}
		PORTL = B00001101;
		keyboard(digit);
		digit++;
		break;
	case 2:
		if (option == '1')
		{
			PORTA = hex_value[(contador / 100) % 10];
		}
		else if (option == '2')
		{
			PORTA = 0x00;
		}
		else if (option == '3')
		{
			PORTA = hex_value[contador % 10];
		}
		PORTL = B00001011;
		keyboard(digit);
		digit++;
		break;
	case 3:
		if (option == '3')
		{
			PORTA = hex_value[(contador / 10) % 10];
		}
		else
		{
			PORTA = 0x00;
		}
		PORTL = B00000111;
		digit = 0;
		break;
	}
}

// Funcion que muestra el menu de opciones
void menu()
{
	Serial.println(" -- TURNOMATIC -- ");
	Serial.println("1.- Modo normal de visualizacion (tres digitos): OFF-centenas-decenas-unidades");
	Serial.println("2.- Modo reducido-inferior de visualizacion (dos digitos): OFF-OFF-decenas-unidades");
	Serial.println("3.- Modo reducido-superior de visualizacion (dos digitos): decenas-unidades-OFF-OFF");
	Serial.println("4.- Modo gusano");
}

// Funcion para incrementar el contador por botones
void buttons_increment()
{
	if (pup == 0)
	{
		if (millis() - time_old > transition_time)
		{
			logic(false);
			tone(PSTART, 1000, 100);
			time_old = millis();
		}
	}
	else if (pdown == 0)
	{
		if (millis() - time_old > transition_time)
		{
			logic(true);
			tone(PSTART, 1000, 100);
			time_old = millis();
		}
	}
	else if (pcenter == 0)
	{
		if (millis() - time_old > transition_time)
		{
			contador = 0;
			tone(PSTART, 1000, 100);
			time_old = millis();
		}
	}
	else if (pright == 0)
	{
		if (millis() - time_old > transition_time)
		{
			increment = 2;
			time_old = millis();
		}
	}
	else if (pleft == 0)
	{
		if (millis() - time_old > transition_time)
		{
			increment = 1;
			time_old = millis();
		}
	}
}

// Funcion para que el contador cambie
void logic(bool pdown)
{
	if (increment == 1 && !pdown)
	{
		contador++;
	}
	else if (increment == 1 && pdown)
	{
		contador--;
	}
	if (increment == 1)
	{
		if (contador > 999)
		{
			contador = 0;
		}
		else if (contador < 0)
		{
			contador = 999;
		}
	}

	if (increment == 2 && !pdown)
	{
		if (contador == 998)
		{
			contador = 0;
		}
		else if (contador == 999)
		{
			contador = 1;
		}
		else
		{
			contador += 2;
		}
	}
	else if (increment == 2 && pdown)
	{
		if (contador == 1)
		{
			contador = 999;
		}
		else if (contador == 0)
		{
			contador = 998;
		}
		else
		{
			contador -= 2;
		}
	}
}

// Funcion para leer el teclado
void keyboard(int column)
{
	int val = PINL >> 4;
	if (val == 15)
	{
		return;
	}
	while (PINL >> 4 != 15)
	{
	}
	switch (val)
	{
	case 7:
		buffer += teclado_map[0][column];
		break;
	case 11:
		buffer += teclado_map[1][column];
		break;
	case 13:
		buffer += teclado_map[2][column];
		break;
	case 14:
		buffer += teclado_map[3][column];
		break;
	}
}

// Funcion para leer el buffer introducido por el teclado
void read_buffer()
{
	if (buffer.length() == 4 && buffer.charAt(3) == '#')
	{
		contador = (buffer.substring(0, 4)).toInt();
		buffer = "";
		tone(PSTART, 1000, 100);
	}
	else if (buffer.length() == 3 && buffer.charAt(2) == '#')
	{
		contador = (buffer.substring(0, 3)).toInt();
		buffer = "";
		tone(PSTART, 1000, 100);
	}
	else if (buffer.length() == 2 && buffer.charAt(1) == '#')
	{
		contador = (buffer.substring(0, 2)).toInt();
		buffer = "";
		tone(PSTART, 1000, 100);
	}
	else if ((buffer.length() == 1 && buffer.charAt(0) == '#') || (buffer.length() > 4))
	{
		buffer = "";
	}
}