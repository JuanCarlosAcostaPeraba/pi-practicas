/*
Añadir a la aplicación del apartado anterior una nueva
opción al menú (opción 4) para seleccionar otra funcionalidad
para el sistema correspondiente a un frecuencímetro (instrumento que mide la
frecuencia de una señal) suponiendo que el Timer 3 está
programado según el apartado anterior. Esa programación
nos condiciona el rango de frecuencias a medir (frecuencia mínima y frecuencia
máxima). Las opciones del menú quedan ahora como:

4.- Modo frecuencímetro

Para ello, utilizaremos las capacidades que tiene el
Timer 3 para medir el tiempo entre eventos (por
ejemplo, medir el tiempo entre dos flancos de bajada
o de subida) haciendo uso de la sección de
captura (registro ICR3 y circuitería asociada). Con
ello ya se tendría el periodo de la señal cuya
inversa sería la frecuencia y que, luego, se mostraría
en el display de 7 segmentos.

La señal de la que queremos medir la frecuencia
se conecta al pin ICP3 (pata 9 del
microcontrolador, no disponible en los conectores del Arduino mega).
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

#define TOP 0x270F // 9999

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
volatile char option = 1;
String buffer;

int contador;
int increment;
int backup = -1;

volatile int frecuencia;
volatile int periodo;
int ICR3_old;
int ICR3_new;

int pup;
int pdown;
int pcenter;
int pleft;
int pright;

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

	// Habilitacion de la interrupcion INT3
	// Timer 3 en modo CTC (modo 4)
	// f = 16 MHz / (2 * N * (1 + TOP))
	// f = 10Hz; N = 8; TOP = 0x270F

	pinMode(5, OUTPUT); // OC3A
	pinMode(2, OUTPUT); // OC3B
	pinMode(3, OUTPUT); // OC3C

	cli();												// Deshabilitamos las interrupciones
	TCCR3A = TCCR3B = TCCR3C = 0; // Deshabilitamos el temporizador
	TCNT3 = 0;										// Inicializamos el contador

	OCR3A = TOP; // Establecemos el valor de comparacion
	OCR3B = 0;
	OCR3C = 0;

	TCCR3A = B01000000; // Modo CTC
	TCCR3B = B00001010; // Modo CTC, prescaler 8

	TIMSK3 = B00100010; // Habilitamos la interrupcion OCIE3A y la interrupcion ICIE3 con el bit 1
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
	frecuencia = 1 / periodo;
	if (Serial.available() > 0)
	{
		option = Serial.read();
	}

	pup = digitalRead(PUP);
	pdown = digitalRead(PDOWN);
	pcenter = digitalRead(PSELECT);
	pleft = digitalRead(PLEFT);
	pright = digitalRead(PRIGHT);

	read_buffer();
	buttons_increment();
}

ISR(TIMER3_CAPT_vect)
{
	ICR3_old = ICR3_new;
	ICR3_new = ICR3;

	periodo = ICR3_new - ICR3_old;

	if (periodo < 0)
	{
		periodo = TOP - ICR3_old + ICR3_new;
	}

	periodo *= 0.0000005;
}

ISR(TIMER3_COMPA_vect)
{
	PORTL = DOFF;
	switch (digit)
	{
	case 0:
		if (option == '1' || option == '2')
		{
			if (backup != -1)
			{
				contador = backup; // Recuperamos el valor del contador
			}
			PORTA = hex_value[contador % 10];
		}
		else if (option == '3')
		{
			PORTA = 0x00;
		}
		else if (option == '4')
		{
			frecuencimetro();
		}
		PORTL = B00001110; // Visualizacion de unidades
		keyboard(digit);
		digit++;
		break;
	case 1:
		if (option == '1' || option == '2')
		{
			if (backup != -1)
			{
				contador = backup; // Recuperamos el valor del contador
			}
			PORTA = hex_value[(contador / 10) % 10];
		}
		else if (option == '3')
		{
			PORTA = 0x00;
		}
		else if (option == '4')
		{
			frecuencimetro();
		}
		PORTL = B00001101; // Visualizacion de decenas
		keyboard(digit);
		digit++;
		break;
	case 2:
		if (option == '1')
		{
			if (backup != -1)
			{
				contador = backup; // Recuperamos el valor del contador
			}
			PORTA = hex_value[(contador / 100) % 10];
		}
		else if (option == '2')
		{
			PORTA = 0x00;
		}
		else if (option == '3')
		{
			if (backup != -1)
			{
				contador = backup; // Recuperamos el valor del contador
			}
			PORTA = hex_value[contador % 10];
		}
		else if (option == '4')
		{
			frecuencimetro();
		}
		PORTL = B00001011; // Visualizacion de centenas
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
			if (backup != -1)
			{
				contador = backup; // Recuperamos el valor del contador
			}
			PORTA = hex_value[(contador / 10) % 10];
		}
		else if (option == '4')
		{
			frecuencimetro();
		}
		PORTL = B00000111; // Visualizacion de unidades de millar
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
	Serial.println("4.- Modo frecuencímetro");
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

// Funcion para mostar el valor del fecuencimetro
void frecuencimetro()
{
	if (frecuencia > 9999)
	{
		Serial.print(frecuencia);
		Serial.println(" Hz - ");
	}
	else
	{
		backup = contador;
		contador = frecuencia;
	}
}
