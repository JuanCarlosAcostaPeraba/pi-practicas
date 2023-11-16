/*
Como mejoras a la práctica básica se propone el uso de otro dispositivo I2C,
como puede ser un Reloj de Tiempo Real (RTC) que nos permite disponer de la
fecha y hora en un sistema microcomputador como el nuestro, entre otras funciones.

Para la implementación de esta tarea, se añadirán dos nuevas opciones al
menú básico: opción 7 y opción 8.

Opción 7 (0.5 puntos): Mostrar en pantalla la fecha y hora, tomándola del RTC;
en el Anexo se muestra resumen de los registros del RTC, pero lo mejor es
consultar la documentación del fabricante del DS3232 en el Moodle (Hay dos versiones).

Opción 8 (0.5 puntos) : Mostrar en Pantalla la Temperatura (la pueden cambiar
en propiedades) leyendo del termómetro interno del RTC.

La información del RTC DS3232 (su "Slave Address" es "1101 000"), también la
tienen accesible a través de Proteus, abriendo propiedades y pulsando el botón "Data".
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

char option;
int address;
int data;

// Función para mostrar el menú
void menu()
{
	Serial.println("\t--\tMENU\t--");
	Serial.println("1. Guardar un dato (de 0 a 255) en cualquier direccion de memoria del dispositivo 24LC64");
	Serial.println("2. Leer una posicion (de 0 a 8191) del 24LC64");
	Serial.println("3. Inicializar un bloque de 256 bytes contiguos de la memoria 24LC64 a un valor");
	Serial.println("4. Mostrar el contenido de un bloque de 256 bytes contiguos del 24LC64, comenzando en una dirección especificada");
	Serial.println("5. Inicializar usando 'Page Write' un bloque de 256 bytes contiguos del 24LC64 a un valor");
	Serial.println("6. Mostrar el contenido de un bloque de 256 bytes del 24LC64 (usando Sequential Read), comenzando en una direccion especificada");
	Serial.println("7. Mostrar en pantalla la fecha y hora, tomándola del RTC");
	Serial.println("8. Mostrar en Pantalla la Temperatura leyendo del termómetro interno del RTC");
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
	Serial.println("Opcion 1");
	Serial.println("Introduzca direccion de memoria (0 - 8191):");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else
	{
		Serial.println("Introduzca valor del dato (0 - 255):");
		data = readSerial();
		if (data < 0 || data > 255)
		{
			Serial.println("Error: Valor del dato incorrecto");
		}
		else
		{
			i2c_wmemory(address, data);
			Serial.println("Dato guardado correctamente");
			option = 0;
			menu();
		}
	}
}

// Función para opción 2 del menú
void option2()
{
	Serial.println("Opcion 2");
	Serial.println("Introduzca direccion de memoria (0 - 8191):");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else
	{
		Serial.println(i2c_rmemory(address));
		option = 0;
		menu();
	}
}

// Función para opción 3 del menú
void option3()
{
	Serial.println("Opcion 3");
	Serial.println("Introduzca direccion de memoria (0 - 8191):");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else
	{
		Serial.println("Introduzca valor del dato (0 - 255):");
		data = readSerial();
		if (data < 0 || data > 255)
		{
			Serial.println("Error: Valor del dato incorrecto");
		}
		else
		{
			int start_time = millis();
			for (int i = 0; i < 256; i++)
			{
				i2c_wmemory(address + i, data);
			}
			int end_time = millis();
			Serial.print("Bloque inicializado correctamente (");
			Serial.print(end_time - start_time);
			Serial.println(" ms)");
			option = 0;
			menu();
		}
	}
}

// Función para opción 4 del menú
void option4()
{
	Serial.println("Opcion 4");
	Serial.println("Introduzca direccion de memoria (0 - 8191):");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else
	{
		int start_time = millis();
		for (int i = 0; i < 256; i++)
		{
			Serial.print("0x");
			Serial.print(hexadecimal[i2c_rmemory(address + i) / 16]); // Parte alta
			Serial.print(hexadecimal[i2c_rmemory(address + i) % 16]); // Parte baja
			Serial.print(" ");
			if (i % 16 == 0)
			{
				Serial.println();
			}
		}
		int end_time = millis();
		Serial.print("Tiempo de lectura: ");
		Serial.print(end_time - start_time);
		Serial.println(" ms");
		option = 0;
		menu();
	}
}

// Función para opción 5 del menú
void option5()
{
	Serial.println("Opcion 5");
	Serial.println("Introduzca direccion de memoria (0 - 8191):");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else
	{
		Serial.println("Introduzca valor del dato (0 - 255):");
		data = readSerial();
		if (data < 0 || data > 255)
		{
			Serial.println("Error: Valor del dato incorrecto");
		}
		else
		{
			int start_time = millis();
			for (int i = 0; i < 8; i++)
			{
				i2c_wpage(address + (i * 32), data);
			}
			int end_time = millis();
			Serial.print("Pagina inicializada correctamente (");
			Serial.print(end_time - start_time);
			Serial.println(" ms)");
			option = 0;
			menu();
		}
	}
}

// Función para opción 6 del menú
void option6()
{
	Serial.println("Opcion 6");
	Serial.println("Introduzca direccion de memoria (0 - 8191):");
	address = readSerial();
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: Direccion de memoria incorrecta");
	}
	else
	{
		int start_time = millis();
		for (int i = 0; i < 8; i++)
		{
			i2c_rpage(address + (i * 32));
		}
		int end_time = millis();
		Serial.print("Tiempo de lectura: ");
		Serial.print(end_time - start_time);
		Serial.println(" ms");
		option = 0;
		menu();
	}
}

// Función para opción 7 del menú
void option7()
{
	Serial.println("Opcion 7");
}

// Función para opción 8 del menú
void option8()
{
	Serial.println("Opcion 8");
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
	cli();
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
	sei();
}

// Función para leer un byte de una dirección de memoria del bus I2C
int i2c_rmemory(int memory_address)
{
	int dataTemp = 0;
	int up_memory_addr = memory_address / 256;	// Parte alta de la dirección de memoria
	int low_memory_addr = memory_address % 256; // Parte baja de la dirección de memoria
	cli();
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
	sei();
	return dataTemp;
}

// Función para escribir una página
void i2c_wpage(int memory_address, byte data)
{
	int up_memory_addr = memory_address / 256;	// Parte alta de la dirección de memoria
	int low_memory_addr = memory_address % 256; // Parte baja de la dirección de memoria
	cli();
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
	sei();
}

// Función para leer una página
void i2c_rpage(int memory_address)
{
	int dataTemp = 0;
	int up_memory_addr = memory_address / 256;	// Parte alta de la dirección de memoria
	int low_memory_addr = memory_address % 256; // Parte baja de la dirección de memoria
	cli();
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
	for (int i = 0; i < 32; i++)
	{
		dataTemp = i2c_rbyte();
		Serial.print("0x");
		Serial.print(hexadecimal[dataTemp / 16]); // Parte alta
		Serial.print(hexadecimal[dataTemp % 16]); // Parte baja
		Serial.print(" ");
		if (i % 16 == 0)
		{
			Serial.println();
		}
		if (i != 31)
		{
			i2c_w0(); // ACK - Enviamos un 0 para indicar que queremos leer más datos
		}
	}
	i2c_w1(); // ACK - Enviamos un 1 para indicar que no queremos leer más datos
	i2c_stop();
	sei();
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

	// Inicialización de los terminales de entrada
	pinMode(LEE_SDA, INPUT);
	pinMode(LEE_SCL, INPUT);
	// Inicialización de los terminales de salida
	pinMode(ESC_SDA, OUTPUT);
	pinMode(ESC_SCL, OUTPUT);
	// Para asegurarse de no intervenir (bloquear) el bus, poner SDA _out y SCL _out a "1"....
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);

	option = 0;
	address = 0;
	data = 0;

	menu();
}

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
	case '7':
		option7();
		break;
	case '8':
		option8();
		break;
	}
}
