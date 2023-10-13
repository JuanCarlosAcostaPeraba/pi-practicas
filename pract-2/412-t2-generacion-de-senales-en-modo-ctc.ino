/*
Generar tres señales desfasadas 45º eléctricos
con el timer 3 utilizando las salidas OC3A (pin 5),
OC3B (pin 2) y OC3C (pin 3) de periodo T = 20 ms.
Hacer uso del registro ICR3 para definir el
TOP y escoger N=8 en el divisor de frecuencia del
reloj (prescaler). Comprobar que las señales
generadas por el timer son las correctas haciendo
uso del osciloscopio del simulador Proteus.
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

	// Modo CTC
	// Formula: f = (fclk / (2 * N * (1 + TOP)))
	// f = 1/T = 50Hz; T = 20ms; fclk = 16MHz; N = 8; TOP = ??

	pinMode(5, OUTPUT); // OC3A
	pinMode(2, OUTPUT); // OC3B
	pinMode(3, OUTPUT); // OC3C

	TCCR3A = TCCR3B = TCCR3C = 0; // Desactivamos todas las salidas del timer 3

	TCNT3 = 0; // Inicializamos el contador del timer 3 a 0

	OCR3A = 0x0001; // Registro de comparación A del timer 3
	OCR3B = 0x1387; // Registro de comparación B del timer 3
	OCR3C = 0x09C3; // Registro de comparación C del timer 3

	ICR3 = 0x4E1F; // Registro de comparación del timer 3

	TCCR3A = B01010100; // Modo CTC, toggle OC3A y OC3C en comparación
	TCCR3B = B00011010; // Prescaler 8
}

void loop()
{
}
