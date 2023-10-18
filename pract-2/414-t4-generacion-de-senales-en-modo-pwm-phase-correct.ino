/*
Generar una señal PWM Phase Correct por la salida
OC3A (pin 5) cuyo ciclo de trabajo (voltaje medio
de la señal) sea controlado por un potenciómetro
conectado a la entrada analógica A0 con el
que se aplicará un voltaje entre 0 y 5 voltios a
la entrada analógica. La lectura de este canal
analógico permitirá luego controlar el ciclo de
trabajo de la señal PWM entre el 0% (voltaje medio
mínimo =0v) y el 100% (voltaje medio máximo = 5v).
El periodo de la señal PWM Phase Correct será de
T = 5 ms, N=8 (prescaler) y el TOP será definido
por el registro ICR3.

Conecte la señal generada (OC3A) a un motor de
corriente continua (componente MOTOR-DC)
motor para comprobar que el control de potencia se
realiza correctamente (el motor gira a mayor o
menor velocidad según el potenciómetro). Comprobar que
la señal generada por el timer es
correcta haciendo uso del osciloscopio del simulador
Proteus. En el esquema del proyecto base
puede apreciar el uso de un driver (etapa de potencia)
para gobernar al motor ya que la salida OC3A
del microcontrolador es muy débil para alimentar directamente al motor.

Las propiedades del motor ya están ajustadas a los valores que figuran en la siguiente captura:
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

#define POTE A0

#define TOP 0x1388 // 5000

unsigned int conv = 0;

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

	// Modo PWM Phase Correct
	// Formula: f = (fclk / (2 * N * TOP))
	// f = 1/T = 200; T = 5ms; fclk = 16MHz; N = 8; TOP = ??

	pinMode(5, OUTPUT); // OC3A
	pinMode(2, OUTPUT); // OC3B
	pinMode(3, OUTPUT); // OC3C

	TCCR3A = TCCR3B = TCCR3C = 0; // Desactivamos todas las salidas del timer 3

	TCNT3 = 0; // Inicializamos el contador del timer 3 a 0

	OCR3A = 0x0FA0;
	OCR3B = 0x0000;
	OCR3C = 0x0000;

	ICR3 = TOP; // TOP

	TCCR3A = B11000010; // Modo Fast PWM
	TCCR3B = B00010010; // Prescaler 8
}

void loop()
{
	OCR3A = map(analogRead(POTE), 0, 1023, 0, TOP);
}
