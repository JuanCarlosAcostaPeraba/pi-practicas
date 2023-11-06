/*
En primer lugar, tenemos que aprender a utilizar el bus I2C.
Para facilitar la realización de la práctica, seguidamente
se van a proporcionar una serie de sugerencias desde el punto
de vista del implementador. Tienen como finalidad darles un
punto de partida y unas indicaciones que pretenden facilitar la
consecución de los objetivos. Evidentemente otros enfoques de
resolución son posibles ya que lo que se presenta aquí es demasiado
simplista (por un enfoque inicial docente, antes que de eficacia), y que para
algunos incluso podrían ser más directos o sencillos. Siéntanse
libres de elegir el procedimiento de implementación que le resulte
de mayor comodidad o efectividad (siempre que cumplan con los objetivos
de la práctica y que funcionen correctamente).

Seguidamente establecemos una lista de las operaciones más
elementales que tiene que realizar el Maestro, y por lo tanto que
tenemos que implementar en nuestro software de control.

En realidad vemos que en este modo simplificado, solo necesitamos
cinco de estas operaciones/funciones elementales, con las que
podremos implementar todas las demás (podrían ser cuatro funciones si E-BIT-0 y
E-BIT-1 se funden en 1 a la que se le pasa el valor 0/1, depende del estilo que se prefiera).

Un ejemplo muy básico (docente, i.e. fácilmente optimizables) de
secuencia de señales y acciones que se pueden usar (sabiendo que son
lentas) para generar se han resumido en las siguientes cinco tablas y se
explican detalladamente en la sesión de presentación inicial de la práctica:

Los valores leídos, que han sido resaltados en amarillo,
(en START, R-BIT y R-ACK) serán gestionados adecuadamente por
el software del usuario; por ejemplo, cuando se va a acceder al bus,
en el caso de START, si no se lee SDA=1 y SCL=1 hay que permanecer
esperando a que el bus se libere (aunque en esta práctica en el simulador
no debe suceder, salvo error en las conexiones o que se añada otro maestro).

Utilizando lo anterior como guía o ejemplo de test, podremos
hacer una secuencia que implemente las distintas operaciones necesarias,
que serán deducidas de la información proporcionada por el fabricante.
Por ejemplo, la secuencia para realizar un “BYTE WRITE” sería como se
especifica en el siguiente cuadro:

START
Enviar los 7 bits de dirección del dispositivo (1010 000)
R/W = 0 (W)
Leer ack del dispositivo; si ok, sigo
Poner la dirección del byte HIGHT = xxx0 0000 (en este caso 0x00)
Leer ack del dispositivo; si ok, sigo
Poner la dirección del byte LOW = 0000 0000 (en este caso 0x00)
Leer ack del dispositivo; si ok, sigo
Enviar el dato a escribir, por ejemplo 0101 0101 (0x55)
Leer ack del dispositivo; si ok, sigo
STOP

Consultar las correspondientes gráficas presentes en la documentación
para implementar la secuencia similar a la anterior para las
otras operaciones que se necesita utilizar. Cuando logramos que esto funcione,
ya podemos desarrollar las capas de más alto nivel que nos proporcione un manejo
secillo y eficiente de acceso a los dispositivos I2C.
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

// Función para mostrar el menú
void menu()
{
	Serial.println("\t--\tMENU\t--");
	Serial.println("1. Guardar un dato (de 0 a 255) en cualquier dirección de memoria del dispositivo 24LC64. Tanto el dato como la dirección se han de solicitar al usuario");
	Serial.println("2. Leer una posición (de 0 a 8191) del 24LC64");
	Serial.println("3. Inicializar un bloque de 256 bytes contiguos de la memoria 24LC64 a un valor");
	Serial.println("4. Mostrar el contenido de un bloque de 256 bytes contiguos del 24LC64, comenzando en una dirección especificada");
	Serial.println("5. Inicializar usando 'Page Write' un bloque de 256 bytes contiguos del 24LC64 a un valor");
	Serial.println("Mostrar el contenido de un bloque de 256 bytes del 24LC64 (usando Sequential Read), comenzando en una dirección especificada");
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
}

void loop()
{
	cli(); // Deshabilitamos las interrupciones
START:
	i2c_start();				 // START
	i2c_wbyte(0xA0);		 // Enviar los 7 bits de dirección del dispositivo (1010 000) + R/W = 0 (W)
	if (i2c_rbit() != 0) // Leer ack del dispositivo; si ok, sigo
	{
		goto START;
	}
	i2c_wbyte(0x00);		 // Poner la dirección del byte HIGHT = xxx0 0000 (en este caso 0x00)
	if (i2c_rbit() != 0) // Leer ack del dispositivo; si ok, sigo
	{
		goto START;
	}
	i2c_wbyte(0x00);		 // Poner la dirección del byte LOW = 0000 0000 (en este caso 0x00)
	if (i2c_rbit() != 0) // Leer ack del dispositivo; si ok, sigo
	{
		goto START;
	}
	i2c_wbyte(0x55);		 // Enviar el dato a escribir, por ejemplo 0101 0101 (0x55)
	if (i2c_rbit() != 0) // Leer ack del dispositivo; si ok, sigo
	{
		goto START;
	}
	i2c_stop(); // STOP
	sei();			// Habilitamos las interrupciones
}
