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

#define TEMP A1 // Sensor de temperatura

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
volatile int temp_degree;
volatile int temp_selector;
volatile bool temperature_selector;
volatile char option;

int temperature;

String buffer;

int contador;
int increment;

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
	cli();																// Deshabilitamos las interrupciones
	EICRA |= (1 << ISC31) | (1 << ISC30); // INT3 activada por flanco de subida
	EIMSK |= (1 << INT3);									// Desenmascaramos la interrupcion INT3 para habilitar la interrupcion externa 3
	sei();																// Habilitamos las interrupciones

	digit = 0;
	temperature_selector = false;
	temp_selector = 0;

	temperature = analogRead(TEMP);
	temp_degree = ((temperature / 1024.0) * 5000) / 10;

	buffer = "";

	contador = 0;
	increment = 1;

	time_old = millis();
	transition_time = 250;

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
	buttons_increment();
}

ISR(INT3_vect)
{
	PORTL = DOFF;
	if (!temperature_selector)
	{
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
		calc_temp_selector();
	}
	else
	{
		temperature = analogRead(TEMP);
		temp_degree = ((temperature / 1024.0) * 5000) / 10;

		switch (digit)
		{
		case 0:
			PORTA = 0x63;
			PORTL = B00001110;
			digit++;
			break;
		case 1:
			PORTA = hex_value[int(temp_degree % 10)];
			PORTL = B00001101;
			digit++;
			break;
		case 2:
			PORTA = hex_value[int((temp_degree / 10) % 10)];
			PORTL = B00001011;
			digit = 0;
			break;
		}

		calc_temp_selector();
	}
}

// Funcion para calcular el selector de temperatura
void calc_temp_selector()
{
	temp_selector++;
	if (temp_selector == 500)
	{
		temp_selector = 0;
		temperature_selector = !temperature_selector;
		digit = 0;
	}
}

// Funcion que muestra el menu de opciones
void menu()
{
	Serial.println(" -- TURNOMATIC -- ");
	Serial.println("1.- Modo normal de visualizacion (tres digitos): OFF-centenas-decenas-unidades");
	Serial.println("2.- Modo reducido-inferior de visualizacion (dos digitos): OFF-OFF-decenas-unidades");
	Serial.println("3.- Modo reducido-superior de visualizacion (dos digitos): decenas-unidades-OFF-OFF");
	Serial.println("4.- Modo frecuencimetro");
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