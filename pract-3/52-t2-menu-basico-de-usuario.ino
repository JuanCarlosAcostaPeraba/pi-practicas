/*
Una vez implementadas las funciones básicas de bus I2C,
procederemos en esta tarea a la implementación de un menú,
con 6 opciones básicas, que permita al usuario la verificación y el uso de la memoria.

Se puede seguir un procedimiento de menos a más como, por ejemplo:

1. Comprobar el funcionamiento escribiendo un valor a una posición fija y leyéndolo después.
	a. En el Proteus podemos ver lo que contiene la memoria 24LC64, pulsando "Pause" y
	activándolo en el menú "Debug" si es necesario (primera vez o fase inical).
	b. Además, en Proteus se puede usar el Osciloscopio o el "I2C Debugger", muy útiles al
	principio o cuando surge algún problema. Se explicará su uso aparte, a interesados.
2. Comprobar la memoria completa escribiendo una secuencia de bytes y luego leer los bytes escritos
(o también verlos en el Debbuger).

Utilizando la experiencia anterior, procederemos a la implementación de la aplicación de usuario
consistente en un menú, a mostrar en el “Terminal Virtual” de Proteus, y que permitirá el uso de la
memoria a través de sus diferentes opciones.

1. Guardar un dato (de 0 a 255) en cualquier dirección de memoria del dispositivo
24LC64. Tanto el dato como la dirección se han de solicitar al usuario. (pidiéndolos
por teclado/pantalla)
2. Leer una posición (de 0 a 8191) del 24LC64 (pidiendo la posición por pantalla)
3. Inicializar un bloque de 256 bytes contiguos de la memoria 24LC64 a un valor (la
dirección del primer elemento y el valor a escribir se solicitan por pantalla; medir el
tiempo el tiempo empleado en acceso a la memoria)
4. Mostrar el contenido de un bloque de 256 bytes contiguos del 24LC64,
comenzando en una dirección especificada (en una matriz de 8 0 16 columnas, en
hexadecimal; medir el tiempo que consume el acceso a la memoria)
5. Inicializar usando "Page Write" un bloque de 256 bytes contiguos del 24LC64 a un
valor (la dirección del primer elemento y el valor a escribir se solicitan por pantalla;
medir el tiempo que consume el acceso a la memoria)
6. Mostrar el contenido de un bloque de 256 bytes del 24LC64 (usando Sequential
Read), comenzando en una dirección especificada (en una matriz de 8 o 16
columnas, en hexadecimal; medir el tiempo que consume el acceso a la memoria)

Cuando se ejecute el programa, se mostrará el menú anterior y permanecerá a la espera de que se le
solicite una de las opciones. Cuando se haya completado la toma de datos y/o visualización de resultados,
se volverá al principio, es decir, mostrar el menú y permanecer a la espera. En la lectura de valores, se
comprobará siempre la validez de los datos proporcionados por el usuario, y se implementará el
tratamiento de errores necesario.

Para lo anterior resultará útil crear procedimientos que sirvan de vínculo para programar más
cómodamente. Por ejemplo, crear, al menos, un procedimiento que podríamos denominar
“Escribe-en-Mem-I2C(dir,valor)” para escribir un “valor” en una determinada dirección “dir”;
y otro procedimiento para “Lee-de-Mem-I2C(dir)”, para leer una posición “dir” de la memoria.
Para implementar procedimientos que transfieran bloques mayores, pero recuérdese que el
máximo número de bytes que podemos transferir en una sola operación de escritura
(PAGE WRITE) es de 32 bytes (en otros chips puede ser diferente).

Ejemplo de listado del contenido de 128 bytes de memoria en 8 columnas (Se pide mostrar 256 bytes,
duplicando el número de filas o el de columnas):
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
char data;
char address;

// Función para mostrar el menú
void menu()
{
	Serial.println("\t--\tMENU\t--");
	Serial.println("1. Guardar un dato (de 0 a 255) en cualquier direccion de memoria del dispositivo 24LC64");
	Serial.println("2. Leer una posicion (de 0 a 8191) del 24LC64");
	Serial.println("3. Inicializar un bloque de 256 bytes contiguos de la memoria 24LC64 a un valor");
	Serial.println("4. Mostrar el contenido de un bloque de 256 bytes contiguos del 24LC64, comenzando en una dirección especificada");
	Serial.println("5. Inicializar usando 'Page Write' un bloque de 256 bytes contiguos del 24LC64 a un valor");
	Serial.println("6. Mostrar el contenido de un bloque de 256 bytes del 24LC64 (usando Sequential Read), comenzando en una dirección especificada");
}

option1()
{
	Serial.println("\tOpcion 1:");
ADDR:
	Serial.println("Introduce la direccion de memoria (de 0 a 8191):");
	while (Serial.available() == 0)
	{
	}
	if (Serial.available() > 0)
	{
		address = Serial.read();
	}
	if (address < 0 || address > 8191)
	{
		Serial.println("Error: La direccion de memoria debe estar entre 0 y 8191");
		goto ADDR;
	}
	// pasar direccion de memoria de decimal a hexadecimal
	char hex_address[4];
	int i = 0;
	while (address > 0)
	{
		hex_address[i] = hexadecimal[address % 16];
		address = address / 16;
		i++;
	}
	Serial.print("Direccion de memoria: 0x");
	for (int j = 3; j >= 0; j--)
	{
		Serial.print(hex_address[j]);
	}
	Serial.println();
DATA:
	Serial.println("Introduce el dato (de 0 a 255):");
	while (Serial.available() == 0)
	{
	}
	if (Serial.available() > 0)
	{
		data = Serial.read();
	}
	if (data < 0 || data > 255)
	{
		Serial.println("Error: El dato debe estar entre 0 y 255");
		goto DATA;
	}
	cli(); // Deshabilitamos las interrupciones
START1:
	i2c_start();				 // Iniciamos el bus I2C
	i2c_wbyte(0xA0);		 // Escribimos la dirección del dispositivo (0xA0)
	if (i2c_rbit() != 0) // Leer ack del dispositivo; si ok, sigo
	{
		goto START1;
	}
	i2c_wbyte(address); // Escribimos la dirección de memoria
	Serial.println("Dato guardado correctamente");
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
void i2c_wbyte(byte dato)
{
	for (int i = 0; i < 8; i++)
	{
		if ((dato & 0x80) != 0)
		{
			i2c_w1();
		}
		else
		{
			i2c_w0();
		}
		dato = dato << 1;
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

void setup()
{
	Serial.begin(9600); // Inicializamos el puerto serie

	// Inicialización de los terminales de entrada
	pinMode(LEE_SDA, INPUT);
	pinMode(LEE_SCL, INPUT);
	// Inicialización de los terminales de salida
	pinMode(ESC_SDA, OUTPUT);
	pinMode(ESC_SCL, OUTPUT);
	// Para asegurarse de no intervenir (bloquear) el bus, poner SDA _out y SCL _out a "1"....
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);

	// Puerto A salida
	DDRA = B11111111;	 // Configuramos el puerto A como salida (0xFF)
	PORTA = B11111111; // Inicializamos el puerto A a 1 (0xFF)

	// Puerto L teclado
	DDRL = B00001111;	 // Configuramos los pines 0, 1, 2 y 3 del puerto L como entrada, y el resto como salida (0x0F)
	PORTL = B11111111; // Inicializamos el puerto L a 1 (0xFF)

	// Puerto C
	DDRC = B00000000;	 // Configuramos el pin 0 del puerto C como entrada (0x00)
	PORTC = B11111111; // Inicializamos el puerto C a 1 (0cFF)

	option = 0;
	data = 0;
	address = 0;
}

void loop()
{
	menu();
	while (Serial.available() == 0)
	{
	}

	if (Serial.available() > 0)
	{
		option = Serial.read();
	}

	switch (option)
	{
	case '1':
		Serial.println("\tOpcion 1:");
		option1();
		break;
	}
}
