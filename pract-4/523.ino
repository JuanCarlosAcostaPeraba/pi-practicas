/* Práctica Lab4: Reloj despertador
 *
 * Descripción: Proyecto base con esquema, definiciones y programa demo
 * Programa demo: uso básico pantalla LCD
 *
 * Fichero: 	23-24_plab4_reloj_base.pdsprj
 * Creado: 		14 noviembre 2023
 * Autor:			--------
 */

// Declaración de variables
#define LEE_SCL 40 // puerto de entrada para leer el estado de la línea SCL
#define LEE_SDA 41 // puerto de entrada para leer el estado de la línea SDA
#define ESC_SCL 4	 // puerto de salida para escribir el valor de la línea SCL-out
#define ESC_SDA 39 // puerto de salida para escribir el valor de la línea SDA-out

#define PRIGHT 30	 // pulsador right
#define PDOWN 31	 // "" down
#define PLEFT 32	 // "" left
#define PENTER 33	 // "" center
#define PUP 34		 // "" up
#define SPEAKER 37 // speaker

#define D4 0xFE		// 1111 1110 unidades
#define D3 0xFD		// 1111 1101 decenas
#define D2 0xFB		// 1111 1011 centenas
#define D1 0xF7		// 1111 0111 millares
#define DOFF 0xFF // 1111 1111 apagado: todos los cátados comunes a "1"
#define DON 0xF0	// 1111 0000   todos los cátados comunes a "0"

// Definición de las teclas del nuevo teclado
char teclado_map[][3] = {
		{'1', '2', '3'},
		{'4', '5', '6'},
		{'7', '8', '9'},
		{'*', '0', '#'}};

int tabla_7seg[] = {
		0X3F, 0x06, 0x5B, 0x4F,
		0x66, 0x6D, 0x7D, 0x07,
		0x7F, 0x6F, 0x77, 0x7C,
		0x39, 0x5E, 0x79, 0x71};

// Declaración de variables i2c
char hexadecimal[16] = {
		'0', '1', '2', '3', // 0, 1, 2, 3
		'4', '5', '6', '7', // 4, 5, 6, 7
		'8', '9', 'A', 'B', // 8, 9, A, B
		'C', 'D', 'E', 'F'	// C, D, E, F
};

// Switch de la interrupción
volatile int digit;
// Lectura del teclado
volatile int row;
byte pinesFilas[] = {42, 43, 44, 45};
byte pinesColumnas[] = {47, 48, 49};
// Variables de onda
int opcion = 0;
int address;
int inicial;
int final;
int matriz[256];

// Variables LCD y RTC DS3232
volatile int flag_alarma = 0;

// Setup
void setup()
{
	// put your setup code here, to run once:
	// habilitar canal TX0/RX0, canal de comunicaciones serie con el virtual terminal.
	Serial.begin(9600);

	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH);
	pinMode(LEE_SDA, INPUT);
	pinMode(LEE_SCL, INPUT);
	pinMode(ESC_SDA, OUTPUT);
	pinMode(ESC_SCL, OUTPUT);

	// PORTA: Segmentos a-f
	DDRA = 0xFF;	// PORTA de salida
	PORTA = 0xFF; // activamos segmentos a-g

	// PORTL[7:4]: filas del teclado
	DDRL = 0x0F;	// input;
	PORTL = 0xFF; // pull-up activos, cátodos/columnas teclado desactivadas

	// PORTC: Pulsadores y altavoz
	DDRC = 0x01;	// PC7:1 input: PC0: output-speaker
	PORTC = 0xFE; // pull-up activos menos el speaker que es de salida

	// Prueba del la pantalla LCD
	// habilitar canal TX3/RX3, canal de comunicaciones serie con la pantalla LCD (MILFORD 4x20 BKP)
	Serial3.begin(9600); // canal 3, 9600 baudios, 8 bits, no parity, 1 stop bit

	Serial3.write(0xFE);
	Serial3.write(0x01); // Clear Screen
	delay(100);

	pinMode(5, OUTPUT);
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(21, INPUT);

	cli(); // Deshabilitamos las interrupciones
	// Timer 1: Modo 15 (Fast PWM, TOP = OCR1A), N=1024
	TCCR1A, TCCR1B, TCCR1C = 0;
	TCNT1 = 0;

	OCR1A = 7812;
	OCR1B, OCR1C = 0;

	TCCR1A = B00000011;
	TCCR1B = B00011101;

	TIMSK1 = B00000001;
	TIFR1 = B00000010;

	// Timer 3: Modo 4 (CTC, TOP = OCR3A), N= 64
	TCCR3A = TCCR3B = TCCR3C = 0;
	TCNT3 = 0;
	OCR3A = 1249;
	OCR3B = 0;
	OCR3C = 0;

	TCCR3A = B01010100;
	TCCR3B = B00001011;

	TIMSK3 = B00000010;
	TIFR3 = B00000001;

	EICRA |= B00000010;
	EIMSK |= (1 << INT0);
	sei();

	// Alarmas
	writeDirByte(B10000000, 0x0A);
	writeDirByte(B10000000, 0x0D);

	Serial.println("Seleccione por el teclado matricial:");
	Serial.println("- #* Modo visualizacion");
	Serial.println("- *# Modo configuracion");
}

void loop()
{
	// MILFORD LCD on/off
	Serial3.write(0xFE);
	Serial3.write(0x08); // Display off
	delay(500);
	Serial3.write(0xFE);
	Serial3.write(0x0C); // Display on
	delay(500);
}
