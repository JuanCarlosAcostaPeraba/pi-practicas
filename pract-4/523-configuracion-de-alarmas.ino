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

#define TOP 31249 // 0x7A11

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

// Matriz con los meses
String months[] = {
		"JAN", "FEB", "MAR", "APR",
		"MAY", "JUN", "JUL", "AUG",
		"SEP", "OCT", "NOV", "DEC"};

// Variables para el bus I2C
int address;
int data;
boolean alarm1;
boolean alarm2;
int secondsAlarm;

// Variables para el ISR
String buffer;
volatile int digit;
volatile char option;

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
	Serial.println(" -- Menu de configuracion --");
	Serial.println();
	Serial.println("1.- Ajustar hora");
	Serial.println("2.- Ajustar fecha");
	Serial.println();
	Serial.println("3.- Ajustar Alarma 1");
	Serial.println("4.- Alarma 1 ON");
	Serial.println("5.- Alarma 1 OFF");
	Serial.println();
	Serial.println("6.- Ajustar Alarma 2");
	Serial.println("7.- Alarma 2 ON");
	Serial.println("8.- Alarma 2 OFF");
	Serial.println();
	Serial.println("9.- Apagar sonido Alarma 1 y 2");
	Serial.println();
	Serial.println("Introduzca una opcion: [Enter para continuar]");
}

/* -- ISR -- */
// Funcion para incrementar el counter por botones
void buttons_increment()
{
	if (pup == 0)
	{
		if (millis() - time_old > transition_time)
		{
			// logic(false);
			tone(PSTART, 1000, 100);
			time_old = millis();
		}
	}
	else if (pdown == 0)
	{
		if (millis() - time_old > transition_time)
		{
			// logic(true);
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
	if (buffer.length() == 2 && buffer.charAt(0) == '#' && buffer.charAt(0) == '*')
	{
		menu();
		buffer = "";
	}
	else if (buffer.length() == 2 && buffer.charAt(0) == '*' && buffer.charAt(0) == '#')
	{
		// TODO - Función para dejar de configurar el reloj
		buffer = "";
	}
}

/* -- LCD -- */
// Función para establecer cursor en la pantalla LCD
void setCursor(int row, byte col)
{
	if (row == 1)
	{
		Serial3.write(0xFE);
		Serial3.write(0x80 + col);
	}
	else if (row == 2)
	{
		Serial3.write(0xFE);
		Serial3.write(0xC0 + col);
	}
	else if (row == 3)
	{
		Serial3.write(0xFE);
		Serial3.write(0x94 + col);
	}
	else if (row == 4)
	{
		Serial3.write(0xFE);
		Serial3.write(0xD4 + col);
	}
}

// Función para escribir la fecha en la pantalla LCD
void writeDate()
{
	// info
	delay(100);
	setCursor(3, 13);
	Serial3.write("DDMMMYY");
	// day
	delay(100);
	setCursor(4, 13);
	int day = i2c_rrtc(4);
	if (day < 10)
	{
		Serial3.write("0");
		setCursor(4, 14);
		Serial3.print(day, HEX);
	}
	else
	{
		Serial3.print(day, HEX);
	}
	// month
	delay(100);
	setCursor(4, 15);
	int month = i2c_rrtc(5) - 1;
	String monthHex = String(month, HEX);
	Serial3.print(months[monthHex.toInt()]);
	// year
	delay(100);
	setCursor(4, 18);
	int year = i2c_rrtc(6);
	if (year < 10)
	{
		Serial3.write("0");
		setCursor(4, 19);
		Serial3.print(year, HEX);
	}
	else
	{
		Serial3.print(year, HEX);
	}
}

// Función para escribir la hora en la pantalla LCD
void writeTime()
{
	// hours
	setCursor(1, 5);
	int hours = i2c_rrtc(2);
	if (hours < 10)
	{
		Serial3.write("0");
		setCursor(1, 6);
		Serial3.print(hours, HEX);
	}
	else
	{
		Serial3.print(hours, HEX);
	}
	// separator
	setCursor(1, 7);
	Serial3.write(":");
	// minutes
	setCursor(1, 8);
	int minutes = i2c_rrtc(1);
	if (minutes < 10)
	{
		Serial3.write("0");
		setCursor(1, 9);
		Serial3.print(minutes, HEX);
	}
	else
	{
		Serial3.print(minutes, HEX);
	}
	// separator
	setCursor(1, 10);
	Serial3.write(":");
	// seconds
	setCursor(1, 11);
	int seconds = i2c_rrtc(0);
	if (seconds < 10)
	{
		Serial3.write("0");
		setCursor(1, 12);
		Serial3.print(seconds, HEX);
	}
	else
	{
		Serial3.print(seconds, HEX);
	}
}

// Función para escribir la temperatura en la pantalla LCD
void writeTemperature(int tempperature)
{
	i2c_wrtc(0x11, tempperature);
	setCursor(2, 14);
	Serial3.write("T=");
	int temp = i2c_rrtc(0x11);
	setCursor(2, 16);
	if (temp >= 0)
	{
		Serial3.write("+");
	}
	else
	{
		Serial3.write("-");
	}
	setCursor(2, 17);
	Serial3.print(temp);
	setCursor(2, 19);
	Serial3.write("C");
}

// Función para escribir la alarma 1 en la pantalla LCD
void writeAlarm1()
{
	setCursor(2, 0);
	Serial3.write("ALARM");
	// hours
	setCursor(3, 0);
	int hours = i2c_rrtc(0x09);
	if (hours < 10)
	{
		Serial3.write("0");
		setCursor(3, 1);
		Serial3.print(hours, HEX);
	}
	else
	{
		Serial3.print(hours, HEX);
	}
	// separator
	setCursor(3, 2);
	Serial3.write(":");
	// minutes
	setCursor(3, 3);
	int minutes = i2c_rrtc(0x08);
	if (minutes < 10)
	{
		Serial3.write("0");
		setCursor(3, 4);
		Serial3.print(minutes, HEX);
	}
	else
	{
		Serial3.print(minutes, HEX);
	}
	// seconds
	secondsAlarm = i2c_rrtc(0x07);
	secondsAlarm = secondsAlarm & 0x7F;
	secondsAlarm = ((secondsAlarm / 10) * 16) + (secondsAlarm % 10); // BCD -> decimal
}

// Función para escribir la alarma 2 en la pantalla LCD
void writeAlarm2()
{
	// hours
	setCursor(4, 0);
	int hours = i2c_rrtc(0x0B);
	if (hours < 10)
	{
		Serial3.write("0");
		setCursor(4, 1);
		Serial3.print(hours, HEX);
	}
	else
	{
		Serial3.print(hours, HEX);
	}
	// separator
	setCursor(4, 2);
	Serial3.write(":");
	// minutes
	setCursor(4, 3);
	int minutes = i2c_rrtc(0x0A);
	if (minutes < 10)
	{
		Serial3.write("0");
		setCursor(4, 4);
		Serial3.print(minutes, HEX);
	}
	else
	{
		Serial3.print(minutes, HEX);
	}
}

// Función para comprobar si las alarmas están activadas y sonarlas
void checkAlarms()
{
	if (alarm1)
	{
		// TODO
	}
	else if (alarm2)
	{
		// TODO
	}
}

void melody(boolean type)
{
	if (type) // Alarma 1
	{
		for (int i = 0; i < 10; i++)
		{
			tone(PSTART, 1000, 30);
			delay(20);
		}
	}
	else // Alarma 2
	{
		for (int i = 0; i < 10; i++)
		{
			if (i % 2 == 0)
			{
				tone(PSTART, 500, 30);
			}
			else
			{
				tone(PSTART, 50, 10);
			}
			delay(20);
		}
	}
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

/* -- I2C -- */
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

// Función para escribir la hora del RTC
void i2c_wrtc(int memory_address, byte data)
{
WRITERTC:
	i2c_start();
	i2c_wbyte(0xD0); // 0xD0 = 11010000
	if (i2c_rbit() != 0)
	{
		goto WRITERTC;
	}
	i2c_wbyte(memory_address);
	if (i2c_rbit() != 0)
	{
		goto WRITERTC;
	}
	i2c_wbyte(data);
	if (i2c_rbit() != 0)
	{
		goto WRITERTC;
	}
	i2c_stop();
}

// Función para leet la hora del RTC
byte i2c_rrtc(int memory_address)
{
READRTC:
	i2c_start();
	i2c_wbyte(0xD0); // 0xD0 = 11010000
	if (i2c_rbit() != 0)
	{
		goto READRTC;
	}
	i2c_wbyte(memory_address);
	if (i2c_rbit() != 0)
	{
		goto READRTC;
	}
	i2c_start();
	i2c_wbyte(0xD1); // 0xD1 = 11010001
	if (i2c_rbit() != 0)
	{
		goto READRTC;
	}
	byte dataTemp = i2c_rbyte();
	i2c_w1(); // ACK - Enviamos un 1 para indicar que no queremos leer más datos
	i2c_stop();
	return dataTemp;
}

/* -- MAIN -- */
// Función setup
void setup()
{
	Serial.begin(9600);	 // Inicializamos el puerto serie
	Serial3.begin(9600); // Inicializamos el puerto serie 3 (LCD)

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
	// T = 0.5s; f = 1/T = 2Hz; N = 256; TOP = OCR1A = 31249 => 16MHz / (256 * (1 + 31249)) = 2Hz
	pinMode(5, OUTPUT); // OC3A
	pinMode(2, OUTPUT); // OC3B
	pinMode(3, OUTPUT); // OC3C

	cli();												// Deshabilitamos las interrupciones
	TCCR1A = TCCR1B = TCCR1C = 0; // Deshabilitamos el temporizador
	TCNT1 = 0;										// Inicializamos el counter

	OCR1A = TOP; // Establecemos el valor de comparacion
	OCR1B = 0;
	OCR1C = 0;

	TCCR1A = B10000011; // Modo Fast PWM con TOP en OCR1A
	TCCR1B = B00011101; // Modo Fast PWM con TOP en OCR1A y preescalador de 1024

	TIMSK1 = B00000001; // Habilitamos la interrupcion TOIE1 con el bit 1 para habilitar por overflow
	sei();

	// Inicialización de los terminales de entrada
	pinMode(LEE_SDA, INPUT);
	pinMode(LEE_SCL, INPUT);
	// Inicialización de los terminales de salida
	pinMode(ESC_SDA, OUTPUT);
	pinMode(ESC_SCL, OUTPUT);
	// Para asegurarse de no intervenir (bloquear) el bus, poner SDA _out y SCL _out a "1"
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);

	// Habilitar interrupciones de la alarma
	i2c_wrtc(0x0E, 0x1F); // Habilitar interrupciones de las alarmas

	i2c_wrtc(0x0A, 0x80); // Alarma 1
	i2c_wrtc(0x0D, 0x80); // Alarma 2

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

	dealy(500);
	Serial.println("Pulse '*#' para configurar el reloj");
}

// Función loop
void loop()
{
	Serial3.write(0xFE);
	Serial3.write(0x0C);
	delay(100);

	read_buffer();
}

// ISR para la interrupcion ICIE3
ISR(TIMER1_OVF_vect)
{
	switch (digit)
	{
	case 0:
		keyboard(digit);
		digit++;
		break;
	case 1:
		keyboard(digit);
		digit++;
		break;
	case 2:
		keyboard(digit);
		digit = 0;
		break;
	}
}
