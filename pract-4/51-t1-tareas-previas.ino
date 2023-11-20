/*
Estudio del chip RTC DS3232 y realizar un esbozo de las
funciones básicas que serían adecuadas o necesarias para
programar de una forma cómoda el chip DS3232, accediendo
a cualquiera de sus recursos.

Rescatar las funciones del i2c de la práctica 3 e integrarlas
en el nuevo proyecto para utilizarlas cuando sean necesarias.
También la ISR() de la gestión entrelazada del display
7-segmentos y teclado de las prácticas anteriores.
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

#define TOP // TODO

#define ESC_SCL 4	 // puerto de salida para escribir el valor en la línea SCL-out =>IO4 =>PG5
#define ESC_SDA 39 // puerto de salida para escribir el valor en la línea SDA-out =>IO39=>PG2
#define LEE_SCL 40 // puerto de entrada para leer el estado de la línea SCL       =>IO40=>PG1
#define LEE_SDA 41 // puerto de entrada para leer el estado de la línea SDA       =>IO41=>PG0

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

// Variables para el bus I2C
int address;
int data;

// Variables para el ISR
String buffer;
volatile int digit;
volatile char option;
volatile int frequency;
volatile float period;
volatile int ICR3_old;
volatile int ICR3_new;

// Variables para el loop
int pup;
int pdown;
int pcenter;
int pleft;
int pright;
int counter;
int increment;
long int time_old;
int transition_time;

// Función para mostrar menú principal
void menu()
{
	Serial.println(" -- MENU --");
	Serial.println("1. Turnomatic");
	Serial.println("2. Memoria");
}

// Función para mostrar el menú del turnomatic
void menuTurnomatic()
{
	Serial.println("");
	Serial.println(" -- Turnomatic --");
	Serial.println("1.- Modo normal de visualizacion (tres digitos): OFF-centenas-decenas-unidades");
	Serial.println("2.- Modo reducido-inferior de visualizacion (dos digitos): OFF-OFF-decenas-unidades");
	Serial.println("3.- Modo reducido-superior de visualizacion (dos digitos): decenas-unidades-OFF-OFF");
	Serial.println("4.- Modo frecuencimetro");
}

// Función para mostrar el menú de la memoria
void menuMemory()
{
	Serial.println("");
	Serial.println(" -- Memoria --");
	Serial.println("1. Guardar un dato (de 0 a 255) en cualquier direccion de memoria del dispositivo 24LC64");
	Serial.println("2. Leer una posicion (de 0 a 8191) del 24LC64");
	Serial.println("3. Inicializar un bloque de 256 bytes contiguos de la memoria 24LC64 a un valor");
	Serial.println("4. Mostrar el contenido de un bloque de 256 bytes contiguos del 24LC64, comenzando en una direccion especificada");
	Serial.println("5. Inicializar usando 'Page Write' un bloque de 256 bytes contiguos del 24LC64 a un valor");
	Serial.println("6. Mostrar el contenido de un bloque de 256 bytes del 24LC64 (usando Sequential Read), comenzando en una direccion especificada");
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

// Funcion para mostrar la frequency
void freq_logic()
{
	PORTA = 0x00;
	Serial.print(frequency);
	Serial.println(" Hz");
}

// Función para leer un número por teclado
int readSerial()
{
	int number = 0;
	while (true)
	{
		if (Serial.available() > 0)
		{
			char element = Serial.read();
			if (element == 13)
			{
				break;
			}
			else
			{
				if (element >= '0' && element <= '9')
				{
					number = number * 10 + (element - '0');
					Serial.print(element);
				}
			}
		}
	}
	Serial.println();
	return number;
}

// Función para opción 1 del menú
void option1()
{
	Serial.println();
	Serial.println("> Opcion 1");
	Serial.println();
	Serial.println("Introduzca direccion de memoria (0 - 8191): [Enter para continuar]");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else
	{
		Serial.println();
		Serial.println("Introduzca valor del dato (0 - 255): [Enter para continuar]");
		data = readSerial();
		if (data < 0 || data > 255)
		{
			Serial.println("Error: Valor del dato incorrecto");
		}
		else
		{
			i2c_wmemory(address, data);
			Serial.println();
			Serial.println("Dato guardado correctamente");
			option = 0;
			Serial.println();
			menu();
		}
	}
}

// Función para opción 2 del menú
void option2()
{
	Serial.println();
	Serial.println("> Opcion 2");
	Serial.println();
	Serial.println("Introduzca direccion de memoria (0 - 8191): [Enter para continuar]");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else
	{
		Serial.println();
		Serial.println(i2c_rmemory(address));
		option = 0;
		Serial.println();
		menu();
	}
}

// Función para opción 3 del menú
void option3()
{
	Serial.println();
	Serial.println("> Opcion 3");
	Serial.println();
	Serial.println("Introduzca direccion de memoria (0 - 8191): [Enter para continuar]");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else
	{
		Serial.println();
		Serial.println("Introduzca valor del dato (0 - 255): [Enter para continuar]");
		data = readSerial();
		if (data < 0 || data > 255)
		{
			Serial.println("Error: Valor del dato incorrecto");
		}
		else
		{
			Serial.println();
			Serial.println("Inicializando bloque...");
			unsigned long start_time = millis();
			for (int i = 0; i < 256; i++)
			{
				if (address + i > 8191)
				{
					break;
				}
				i2c_wmemory(address + i, data);
			}
			unsigned long end_time = millis();
			Serial.println();
			Serial.print("Bloque inicializado correctamente (");
			Serial.print(end_time - start_time);
			Serial.println(" ms)");
			option = 0;
			Serial.println();
			menu();
		}
	}
}

// Función para opción 4 del menú
void option4()
{
	Serial.println();
	Serial.println("> Opcion 4");
	Serial.println();
	Serial.println("Introduzca direccion de memoria (0 - 8191): [Enter para continuar]");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else
	{
		int dataTemp[256];
		Serial.println();
		unsigned long start_time = millis();
		for (int i = 0; i < 256; i++)
		{
			if (address + i > 8191)
			{
				break;
			}
			dataTemp[i] = i2c_rmemory(address + i);
		}
		unsigned long end_time = millis();
		for (int i = 0; i < 256; i++)
		{
			if (address + i > 8191)
			{
				break;
			}
			if (i % 16 == 0 && i != 0)
			{
				Serial.println();
			}
			Serial.print("0x");
			Serial.print(hexadecimal[dataTemp[i] / 16]); // Parte alta
			Serial.print(hexadecimal[dataTemp[i] % 16]); // Parte baja
			Serial.print(" ");
		}
		Serial.println();
		Serial.print("Tiempo de lectura: ");
		Serial.print(end_time - start_time);
		Serial.println(" ms");
		option = 0;
		Serial.println();
		menu();
	}
}

// Función para opción 5 del menú
void option5()
{
	Serial.println();
	Serial.println("> Opcion 5");
	Serial.println();
	Serial.println("Introduzca direccion de memoria multiplo de 32 (0 - 8191): [Enter para continuar]");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else if (address % 32 != 0)
	{
		Serial.println("Error: Direccion de memoria dada no es multiplo de 32");
	}
	else
	{
		Serial.println();
		Serial.println("Introduzca valor del dato (0 - 255): [Enter para continuar]");
		data = readSerial();
		if (data < 0 || data > 255)
		{
			Serial.println("Error: Valor del dato incorrecto");
		}
		else
		{
			Serial.println();
			Serial.println("Inicializando bloque con Page Write...");
			unsigned long start_time = millis();
			for (int i = 0; i < 8; i++)
			{
				if (address + (i * 32) > 8191)
				{
					break;
				}
				i2c_wpage(address + (i * 32), data);
			}
			unsigned long end_time = millis();
			Serial.println();
			Serial.print("Pagina inicializada correctamente (");
			Serial.print(end_time - start_time);
			Serial.println(" ms)");
			option = 0;
			Serial.println();
			menu();
		}
	}
}

// Función para opción 6 del menú
void option6()
{
	Serial.println();
	Serial.println("> Opcion 6");
	Serial.println();
	Serial.println("Introduzca direccion de memoria (0 - 8191): [Enter para continuar]");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else
	{
		int dataTemp[256];
		Serial.println();
		unsigned long start_time = millis();
		i2c_rpage(address, dataTemp);
		unsigned long end_time = millis();
		for (int i = 0; i < 256; i++)
		{
			if (address + i > 8191)
			{
				break;
			}
			if (i % 16 == 0 && i != 0)
			{
				Serial.println();
			}
			Serial.print("0x");
			Serial.print(hexadecimal[dataTemp[i] / 16]); // Parte alta
			Serial.print(hexadecimal[dataTemp[i] % 16]); // Parte baja
			Serial.print(" ");
		}
		Serial.println();
		Serial.print("Tiempo de lectura: ");
		Serial.print(end_time - start_time);
		Serial.println(" ms");
		option = 0;
		Serial.println();
		menu();
	}
}

// Función start para el bus I2C
void i2c_start()
{
	digitalWrite(ESC_SCL, LOW);	 // clock = 0
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, LOW);	 // SDA = 0

	// termina la función con SCL = 0 y SDA = 0
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
}

// Función stop para el bus I2C
void i2c_stop()
{
	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, LOW);	 // SDA = 0

	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	// termina la función con SCL = 0 y SDA = 0
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
}

// Función para escribir un 1 en el bus I2C (También se usará para enviar un 1 como ACK)
void i2c_w1()
{
	digitalWrite(ESC_SCL, LOW);	 // clock = 0
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, LOW);	 // clock = 0
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, LOW); // clock = 0
	digitalWrite(ESC_SDA, LOW); // SDA = 0

	// termina la función con SCL = 0 y SDA = 0
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
}

// Función para escribir un 0 en el bus I2C (También se usará para enviar un 0 como ACK)
void i2c_w0()
{
	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, LOW);	 // SDA = 0

	digitalWrite(ESC_SCL, LOW); // clock = 0
	digitalWrite(ESC_SDA, LOW); // clock = 0

	// termina la función con SCL = 0 y SDA = 0
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
}

// Función para leer un bit del bus I2C (También se usará para leer el ACK)
int i2c_rbit()
{
	digitalWrite(ESC_SCL, LOW);	 // clock = 0
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, HIGH);			// clock = 1
	int value = digitalRead(LEE_SDA); // leer el valor de SDA

	digitalWrite(ESC_SCL, LOW);	 // clock = 0
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	// termina la función con SCL = 0 y SDA = 0
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);

	return value;
}

// Función para escribir un byte en el bus I2C
void i2c_wbyte(byte data)
{
	for (int i = 0; i < 8; i++)
	{
		if ((data & 0x80) != 0)
		{
			i2c_w1();
		}
		else
		{
			i2c_w0();
		}
		data = data << 1;
	}
}

// Función para leer un byte del bus I2C
byte i2c_rbyte()
{
	byte ibyte = 0;
	for (int i = 0; i < 8; i++)
	{
		ibyte = (ibyte << 1) | (i2c_rbit() & 1);
	}
	return ibyte;
}

// Función para escribir un byte en una dirección de memoria del bus I2C
void i2c_wmemory(int memory_address, byte data)
{
	int up_memory_addr = memory_address / 256;	// Parte alta de la dirección de memoria
	int low_memory_addr = memory_address % 256; // Parte baja de la dirección de memoria

WRITE:
	i2c_start();
	i2c_wbyte(0xA0);
	if (i2c_rbit() != 0)
	{
		goto WRITE;
	}
	i2c_wbyte(up_memory_addr);
	if (i2c_rbit() != 0)
	{
		goto WRITE;
	}
	i2c_wbyte(low_memory_addr);
	if (i2c_rbit() != 0)
	{
		goto WRITE;
	}
	i2c_wbyte(data);
	if (i2c_rbit() != 0)
	{
		goto WRITE;
	}
	i2c_stop();
}

// Función para leer un byte de una dirección de memoria del bus I2C
int i2c_rmemory(int memory_address)
{
	int dataTemp = 0;
	int up_memory_addr = memory_address / 256;	// Parte alta de la dirección de memoria
	int low_memory_addr = memory_address % 256; // Parte baja de la dirección de memoria

READ:
	i2c_start();
	i2c_wbyte(0xA0);
	if (i2c_rbit() != 0)
	{
		goto READ;
	}
	i2c_wbyte(up_memory_addr);
	if (i2c_rbit() != 0)
	{
		goto READ;
	}
	i2c_wbyte(low_memory_addr);
	if (i2c_rbit() != 0)
	{
		goto READ;
	}
	i2c_start();
	i2c_wbyte(0xA1);
	if (i2c_rbit() != 0)
	{
		goto READ;
	}
	dataTemp = i2c_rbyte();
	i2c_w1(); // ACK - Enviamos un 1 para indicar que no queremos leer más datos
	i2c_stop();

	return dataTemp;
}

// Función para escribir una página
void i2c_wpage(int memory_address, byte data)
{
	int up_memory_addr = memory_address / 256;	// Parte alta de la dirección de memoria
	int low_memory_addr = memory_address % 256; // Parte baja de la dirección de memoria

WRITEPAGE:
	i2c_start();
	i2c_wbyte(0xA0);
	if (i2c_rbit() != 0)
	{
		goto WRITEPAGE;
	}
	i2c_wbyte(up_memory_addr);
	if (i2c_rbit() != 0)
	{
		goto WRITEPAGE;
	}
	i2c_wbyte(low_memory_addr);
	if (i2c_rbit() != 0)
	{
		goto WRITEPAGE;
	}
	for (int i = 0; i < 32; i++)
	{
		i2c_wbyte(data);
		if (i2c_rbit() != 0)
		{
			goto WRITEPAGE;
		}
	}
	i2c_stop();
}

// Función para leer una página
void i2c_rpage(int memory_address, int dataArray[256])
{
	int up_memory_addr = memory_address / 256;	// Parte alta de la dirección de memoria
	int low_memory_addr = memory_address % 256; // Parte baja de la dirección de memoria
READPAGE:
	i2c_start();
	i2c_wbyte(0xA0);
	if (i2c_rbit() != 0)
	{
		goto READPAGE;
	}
	i2c_wbyte(up_memory_addr);
	if (i2c_rbit() != 0)
	{
		goto READPAGE;
	}
	i2c_wbyte(low_memory_addr);
	if (i2c_rbit() != 0)
	{
		goto READPAGE;
	}
	i2c_start();
	i2c_wbyte(0xA1);
	if (i2c_rbit() != 0)
	{
		goto READPAGE;
	}
	for (int i = 0; i < 256; i++)
	{
		if (memory_address + i > 8191)
		{
			break;
		}
		dataArray[i] = i2c_rbyte();
		if (i != 255)
		{
			i2c_w0(); // ACK - Enviamos un 0 para indicar que queremos leer más datos
		}
	}
	i2c_w1(); // ACK - Enviamos un 1 para indicar que no queremos leer más datos
	i2c_stop();
}

// Función setup
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

	// Habilitacion de la interrupcion Timer1 en modo Fast PWM
	// Timer 1 que genera la interrupcion cada 0.5s (500ms) por desbordamiento
	// f = 16 MHz / (N * (1 + TOP))
	// T = 0.5s; f = 1/T = 2Hz; N = 1024; TOP = OCR1A = 31249 (0x7A11)
	pinMode(5, OUTPUT); // OC3A
	pinMode(2, OUTPUT); // OC3B
	pinMode(3, OUTPUT); // OC3C

	cli();												// Deshabilitamos las interrupciones
	TCCR3A = TCCR3B = TCCR3C = 0; // Deshabilitamos el temporizador
	TCNT3 = 0;										// Inicializamos el counter

	OCR3A = 0;
	OCR3B = 0;
	OCR3C = 0;

	TCCR3A = B01000000; // Modo CTC
	TCCR3B = B00001010; // Modo CTC, prescaler 8

	TIMSK3 = B00100010; // Habilitamos la interrupcion OCIE3A y la interrupcion ICIE3 con el bit 1
	sei();

	// Inicialización de los terminales de entrada
	pinMode(LEE_SDA, INPUT);
	pinMode(LEE_SCL, INPUT);
	// Inicialización de los terminales de salida
	pinMode(ESC_SDA, OUTPUT);
	pinMode(ESC_SCL, OUTPUT);
	// Para asegurarse de no intervenir (bloquear) el bus, poner SDA _out y SCL _out a "1"....
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);

	// Variables para el bus I2C
	address = 0;
	data = 0;

	// Variables para el ISR
	option = 0;
	digit = 0;
	buffer = "";

	// Variables para el loop
	counter = 0;
	increment = 1;
	time_old = millis();
	transition_time = 550;

	delay(150);
	menu();
}

// Función loop
void loop()
{
	if (Serial.available() > 0)
	{
		option = Serial.read();
	}

	switch (option)
	{
	case '1':
		option1();
		break;
	case '2':
		option2();
		break;
	case '3':
		option3();
		break;
	case '4':
		option4();
		break;
	case '5':
		option5();
		break;
	case '6':
		option6();
		break;
	}
}

// ISR para la interrupcion OCIE3A
ISR(TIMER3_CAPT_vect)
{
	ICR3_old = ICR3_new;
	ICR3_new = ICR3;

	period = ICR3_new - ICR3_old;

	if (period < 0)
	{
		period = TOP - ICR3_old + ICR3_new;
	}

	period *= 0.0000005;

	frequency = 1 / period;
}

// ISR para la interrupcion ICIE3
ISR(TIMER3_COMPA_vect)
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
			if ((frequency > 9999))
			{
				freq_logic();
			}
			else
			{
				PORTA = hex_value[frequency % 10];
			}
		}
		PORTL = B00001110; // Visualizacion de unidades
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
			if ((frequency > 9999))
			{
				freq_logic();
			}
			else
			{
				PORTA = hex_value[(frequency / 10) % 10];
			}
		}
		PORTL = B00001101; // Visualizacion de decenas
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
			if ((frequency > 9999))
			{
				freq_logic();
			}
			else
			{
				PORTA = hex_value[(frequency / 100) % 10];
			}
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
			PORTA = hex_value[(counter / 10) % 10];
		}
		else if (option == '4')
		{
			if ((frequency > 9999))
			{
				freq_logic();
			}
			else
			{
				PORTA = hex_value[(frequency / 1000) % 10];
			}
		}
		PORTL = B00000111; // Visualizacion de unidades de millar
		digit = 0;
		break;
	}
}
