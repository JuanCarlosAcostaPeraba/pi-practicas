/*
Realizar las modificaciones que estime oportunas
para implementar un nuevo modo de funcionamiento
del sistema correspondiente a un cronómetro (de milisegundos).
Para cambiar de modo de funcionamiento, se añadirá la opción
4 al proyecto de partida que se visualiza en el terminal.

El modo cronómetro usará el display de 7 segmento de 4 dígitos
para visualizar los milisegundos del cronómetro, que van desde
0000 hasta 9999 ms, y los siguientes pulsadores:

- Pulsador "pup": Pone en marcha el cronómetro
- Pulsador "pcenter": Para el cronómetro y lo pone a cero
- Pulsador "pdown": Detiene el cronómetro. Si luego se pulsa
"pup", el cronómetro sigue contando
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

volatile int crono;
volatile bool crono_state;

long int time_old;
int transition_time;

// Funcion que muestra el menu de opciones
void menu()
{
	Serial.println(" -- TURNOMATIC -- ");
	Serial.println("1.- Modo normal de visualizacion (tres digitos): OFF-centenas-decenas-unidades");
	Serial.println("2.- Modo reducido-inferior de visualizacion (dos digitos): OFF-OFF-decenas-unidades");
	Serial.println("3.- Modo reducido-superior de visualizacion (dos digitos): decenas-unidades-OFF-OFF");
	Serial.println("4.- Modo cronometro");
}

// Funcion para incrementar el counter por botones
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

// Funcion para que el counter cambie
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

// Funcion para controlar el cronometro
void crono_buttons()
{
	if (pup == 0)
	{
		Serial.print("pup");
		if (millis() - time_old > transition_time)
		{
			crono_state = true;
			tone(PSTART, 1000, 100);
			time_old = millis();
		}
	}
	else if (pdown == 0)
	{
		Serial.print("pdown");
		if (millis() - time_old > transition_time)
		{
			crono_state = false;
			crono = 0;
			tone(PSTART, 1000, 100);
			time_old = millis();
		}
	}
	else if (pcenter == 0)
	{
		Serial.print("pcenter");
		if (millis() - time_old > transition_time)
		{
			crono_state = false;
			tone(PSTART, 1000, 100);
			time_old = millis();
		}
	}
}

// Funcions para controlar la logica del cronometro
void crono_logic()
{
	if (crono > 9999)
	{
		crono = 0;
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

	// Habilitacion de la interrupcion INT3
	cli();																// Deshabilitamos las interrupciones
	EICRA |= (1 << ISC31) | (1 << ISC30); // INT3 activada por flanco de subida
	EIMSK |= (1 << INT3);									// Desenmascaramos la interrupcion INT3 para habilitar la interrupcion externa 3
	sei();																// Habilitamos las interrupciones

	digit = 0;
	buffer = "";

	counter = 0;
	increment = 1;

	crono = 0;
	crono_state = true;

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

	read_buffer();
	if (option == '4')
	{
		crono_buttons();
	}
	else
	{
		buttons_increment();
	}
}

ISR(INT3_vect)
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
			if (crono_state)
			{
				crono += 5;
			}
			crono_logic();
			PORTA = hex_value[crono % 10];
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
			if (crono_state)
			{
				crono += 5;
			}
			crono_logic();
			PORTA = hex_value[(crono / 10) % 10];
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
			if (crono_state)
			{
				crono += 5;
			}
			crono_logic();
			PORTA = hex_value[(crono / 100) % 10];
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
			if (crono_state)
			{
				crono += 5;
			}
			crono_logic();
			PORTA = hex_value[crono / 1000];
		}
		PORTL = B00000111;
		digit = 0;
		break;
	}
}
