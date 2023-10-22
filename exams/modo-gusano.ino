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

Modo gusano: Cuando se entra en opción 4, la variable “sense”
(modificada por los pulsadores “pright2” y “pleft2”) indicará si el
desplazamiento del “segmento encendido” se hace hacia la derecha o hacia
la izquierda. Cada pulsación generará una interrupción que
será atendida por la correspondiente ISR() que actualizará la
variable “sense” de acuerdo a:

Si se pulsa “pright2” entonces sense = 0 movimiento a la derecha
Si se pulsa “pleft2” entonces sense = 1 movimiento a la izquierda

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
	// Si (opción == 4)à funcionamiento en modo gusano. Realiza un bucle completo hacia la derecha o hacia la izquierda dependiendo de la variable sense. El “sense” en el que se va encendiendo el segmento activo se cambia con los pulsadores “pright2” y “pleft2”. Cuando termina un bucle completo del worm vuelve a loop() donde comprobará si ha llegado una nueva opción del menú que implique un cambio de modo.
}

Especificaciones para programar el Timer 3:
* Modo PWM, Phase correct, TOP = ICR3 , T = 4 ms (period señal PWM)
* frequency de la interrupción: Usar la interrupción por
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

#define PRIGHT2 21 // INT0 (pin 21) - pulsador right2 sentido derecha
#define PLEFT2 20	 // INT1 (pin 20) - pulsador left2 sentido izquierda

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

// Array valores hexadecimales en binario con punto
int hex_value_point[16] = {
		0xBF, 0x86, 0xDB, 0xCF, // 0., 1., 2., 3.
		0xE6, 0xED, 0xFD, 0x87, // 4., 5., 6., 7.
		0xFF, 0xEF, 0xF7, 0xFC, // 8., 9., A., B.
		0xB9, 0xDE, 0xF9, 0xF1	// C., D., E., F.
};

// Matriz teclado
char keyboard_map[][3] = {
		{'1', '2', '3'},
		{'4', '5', '6'},
		{'7', '8', '9'},
		{'*', '0', '#'}};

volatile int digit;
volatile char option;
String buffer;

int counter;
int increment;

int pup;
int pdown;
int pcenter;
int pleft;
int pright;

int pright2;
int pleft2;

volatile int sense; // 0 derecha, 1 izquierda
volatile int worm_state;

char worm_states[12] = {
		0x1, 0x1, 0x1, 0x1,
		0x2, 0x4, 0x8, 0x8,
		0x8, 0x8, 0x10, 0x20};

long int time_old;
int transition_time;

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
			counter = 0;
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
		counter++;
	}
	else if (increment == 1 && pdown)
	{
		counter--;
	}
	if (increment == 1)
	{
		if (counter > 999)
		{
			counter = 0;
		}
		else if (counter < 0)
		{
			counter = 999;
		}
	}

	if (increment == 2 && !pdown)
	{
		if (counter == 998)
		{
			counter = 0;
		}
		else if (counter == 999)
		{
			counter = 1;
		}
		else
		{
			counter += 2;
		}
	}
	else if (increment == 2 && pdown)
	{
		if (counter == 1)
		{
			counter = 999;
		}
		else if (counter == 0)
		{
			counter = 998;
		}
		else
		{
			counter -= 2;
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
		buffer += keyboard_map[0][column];
		break;
	case 11:
		buffer += keyboard_map[1][column];
		break;
	case 13:
		buffer += keyboard_map[2][column];
		break;
	case 14:
		buffer += keyboard_map[3][column];
		break;
	}
}

// Funcion para leer el buffer introducido por el teclado
void read_buffer()
{
	if (buffer.length() == 4 && buffer.charAt(3) == '#')
	{
		counter = (buffer.substring(0, 4)).toInt();
		buffer = "";
		tone(PSTART, 1000, 100);
	}
	else if (buffer.length() == 3 && buffer.charAt(2) == '#')
	{
		counter = (buffer.substring(0, 3)).toInt();
		buffer = "";
		tone(PSTART, 1000, 100);
	}
	else if (buffer.length() == 2 && buffer.charAt(1) == '#')
	{
		counter = (buffer.substring(0, 2)).toInt();
		buffer = "";
		tone(PSTART, 1000, 100);
	}
	else if ((buffer.length() == 1 && buffer.charAt(0) == '#') || (buffer.length() > 4))
	{
		buffer = "";
	}
}

// Funcion para el modo gusano
void worm()
{
	if (sense == 0)
	{
		switch (worm_state)
		{
		case 0:
			PORTA = worm_states[worm_state];
			PORTL = B00000111;
			break;
		case 1:
			PORTA = worm_states[worm_state];
			PORTL = B00001011;
			break;
		case 2:
			PORTA = worm_states[worm_state];
			PORTL = B00001101;
			break;
		case 3:
			PORTA = worm_states[worm_state];
			PORTL = B00001110;
			break;
		case 4:
			PORTA = worm_states[worm_state];
			PORTL = B00001110;
			break;
		case 5:
			PORTA = worm_states[worm_state];
			PORTL = B00001110;
			break;
		case 6:
			PORTA = worm_states[worm_state];
			PORTL = B00000111;
			break;
		case 7:
			PORTA = worm_states[worm_state];
			PORTL = B00001101;
			break;
		case 8:
			PORTA = worm_states[worm_state];
			PORTL = B00001011;
			break;
		case 9:
			PORTA = worm_states[worm_state];
			PORTL = B00000111;
			break;
		case 10:
			PORTA = worm_states[worm_state];
			PORTL = B00000111;
			break;
		case 11:
			PORTA = worm_states[worm_state];
			PORTL = B00000111;
			break;
		}
		worm_state++;
		if (worm_state == 12)
		{
			worm_state = 0;
		}
	}
	else if (sense == 1)
	{
		switch (worm_state)
		{
		case 0:
			PORTA = worm_states[worm_state];
			PORTL = B00000111;
			break;
		case 1:
			PORTA = worm_states[worm_state];
			PORTL = B00001011;
			break;
		case 2:
			PORTA = worm_states[worm_state];
			PORTL = B00001101;
			break;
		case 3:
			PORTA = worm_states[worm_state];
			PORTL = B00001110;
			break;
		case 4:
			PORTA = worm_states[worm_state];
			PORTL = B00001110;
			break;
		case 5:
			PORTA = worm_states[worm_state];
			PORTL = B00001110;
			break;
		case 6:
			PORTA = worm_states[worm_state];
			PORTL = B00000111;
			break;
		case 7:
			PORTA = worm_states[worm_state];
			PORTL = B00001101;
			break;
		case 8:
			PORTA = worm_states[worm_state];
			PORTL = B00001011;
			break;
		case 9:
			PORTA = worm_states[worm_state];
			PORTL = B00000111;
			break;
		case 10:
			PORTA = worm_states[worm_state];
			PORTL = B00000111;
			break;
		case 11:
			PORTA = worm_states[worm_state];
			PORTL = B00000111;
			break;
		}
		worm_state--;
		if (worm_state == -1)
		{
			worm_state = 11;
		}
	}
}

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
	// Habilitacion de la interrupcion externa INT1 (pin 20, pleft2, falling)
	cli();
	pinMode(PRIGHT2, INPUT);
	pinMode(PLEFT2, INPUT);
	EICRA = B00001010;
	EIMSK = B00000011;

	// Modo PWM Phase Correct
	// f = 16 MHz / (N * 2 * TOP)
	// f = 250 Hz; N = 64; TOP = 500
	// Habilitacion de la interrupcion por overflow del Timer 3
	pinMode(5, OUTPUT); // OC3A
	pinMode(2, OUTPUT); // OC3B
	pinMode(3, OUTPUT); // OC3C

	TCCR3A = TCCR3B = TCCR3C = 0; // resetear registros
	TCNT3 = 0;										// Inicializamos el contador a 0

	OCR3A = 0;
	OCR3B = 200;
	OCR3C = 0;

	ICR3 = TOP;

	TCCR3A = B00100010;
	TCCR3B = B00010011;

	TIMSK3 = B00000101;
	sei();

	digit = 0;
	buffer = "";

	counter = 0;
	increment = 1;

	sense = 0;
	worm_state = 0;

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

ISR(INT0_vect) // Pulsador pright2 (pin 21) - sentido derecha
{
	if (millis() - time_old > transition_time)
	{
		sense = 0;
		Serial.println("pright2");
		Serial.println(sense);
	}
}

ISR(INT1_vect) // Pulsador pleft2 (pin 20) - sentido izquierda
{
	if (millis() - time_old > transition_time)
	{
		sense = 1;
		Serial.println("pleft2");
		Serial.println(sense);
	}
}

ISR(TIMER3_OVF_vect)
{
	PORTL = DOFF;
	switch (digit)
	{
	case 0:
		if (option == '1' || option == '2')
		{
			PORTA = hex_value[counter % 10];
		}
		else if (option == '3')
		{
			PORTA = 0x00;
		}
		else if (option == '4')
		{
			worm();
			PORTA = 0x00;
		}
		PORTL = B00001110;
		keyboard(digit);
		digit++;
		break;
	case 1:
		if (option == '1' || option == '2')
		{
			PORTA = hex_value[(counter / 10) % 10];
		}
		else if (option == '3')
		{
			PORTA = 0x00;
		}
		else if (option == '4')
		{
			worm();
			PORTA = 0x00;
		}
		PORTL = B00001101;
		keyboard(digit);
		digit++;
		break;
	case 2:
		if (option == '1')
		{
			PORTA = hex_value[(counter / 100) % 10];
		}
		else if (option == '2')
		{
			PORTA = 0x00;
		}
		else if (option == '3')
		{
			PORTA = hex_value[counter % 10];
		}
		else if (option == '4')
		{
			worm();
			PORTA = 0x00;
		}
		PORTL = B00001011;
		keyboard(digit);
		digit++;
		break;
	case 3:
		if (option == '1' || option == '2')
		{
			PORTA = 0x00;
		}
		else if (option == '3')
		{
			PORTA = hex_value[(counter / 10) % 10];
		}
		else if (option == '4')
		{
			worm();
			PORTA = 0x00;
		}
		PORTL = B00000111;
		digit = 0;
		break;
	}
}
