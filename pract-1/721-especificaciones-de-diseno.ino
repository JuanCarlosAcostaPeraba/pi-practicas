/*
Implementar una aplicación con una funcionalidad similar
a la de un Turnomatic (dispositivo para dar un turno en
una cola de clientes) en el que la visualización en
el display y la exploración del teclado se haga de forma
entrelazada y sincronizada por una interrupción cada
5 ms generada por el pin 18 (INT3).

La aplicación se implementará de acuerdo a las siguientes especificaciones:

1. Contador de 3 dígitos
2. Control basado en 4 pulsadores con las siguientes funcionalidades:
	a. PUP: Incrementa el contador
	b. PDOWN: Decrementa el contador
	c. PENTER: Puesta a cero (reset) del contador
	d. PLEFT: El contador incrementará o decrementará su cuenta de 1 en 1
	e. PRIGHT: El contador incrementará o decrementará su cuenta de 2 en 2.
	f. Señal acústica por el altavoz del sistema cada vez cambie
		el estado del contador para así avisar a los clientes.
3. Inicialización del contador a cualquier valor, entre 000 y 999 mediante el teclado de 4x3
	a. Para modificar la cuenta con el teclado bastará
		con teclear los números y terminar con la tecla
		“#” que hará la función de “enter o entrar”. Ejemplo; 235#
4. Mostrar en la pantalla del PC (o en el terminal virtual de Proteus) el siguiente menú de opciones para poder elegir una de las acciones mostradas:
	1.- Modo normal de visualización (tres dígitos): OFF-centenas-decenas-unidades
	2.- Modo reducido-inferior de visualización (dos dígitos): OFF-OFF-decenas-unidades
	3.- Modo reducido-superior de visualización (dos dígitos): decenas-unidades-OFF-OFF
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

// Matriz display 8 segmentos
char display_map[4] = {D4, D3, D2, D1};

// Matriz valores hexadecimales
char hexadecimal[16] = {
		'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'A', 'B',
		'C', 'D', 'E', 'F'};

// Matriz valores hexadecimales en binario
char hex_value[16] = {
		0x3F, 0x06, 0x5B, 0x4F,
		0x66, 0x6D, 0x7D, 0x07,
		0x7F, 0x6F, 0x77, 0x7C,
		0x39, 0x5E, 0x79, 0x71};

// Matriz teclado
char teclado_map[][3] = {
		{'1', '2', '3'},
		{'4', '5', '6'},
		{'7', '8', '9'},
		{'*', '0', '#'}};

volatile int estado;
int contador;

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

	// Habilitación de la interrupción INT3
	cli();																// Deshabilitamos las interrupciones
	EICRA |= (1 << ISC31) | (1 << ISC30); // INT3 activada por flanco de subida
	EIMSK |= (1 << INT3);									// Desenmascaramos la interrupción INT3 para habilitar la interrupción externa 3
	sei();																// Habilitamos las interrupciones

	estado = 0;
	contador = 0;

	menu();
}

void loop() {}

ISR(INT3_vect)
{
	// Encender display
	if (estado == 0)
	{
		// Apagar decenas y centetas, encender y visualizar unidades
		digitalWrite(D3, HIGH);						// Apagar decenas
		digitalWrite(D2, HIGH);						// Apagar centenas
		PORTA = hex_value[contador % 10]; // Visualizar unidades
		digitalWrite(D4, LOW);						// Encender unidades
		estado++;
	}
	else if (estado == 1)
	{
		// Apagar unidades y centenas, encender y visualizar decenas
		digitalWrite(D4, HIGH);						 // Apagar unidades
		digitalWrite(D2, HIGH);						 // Apagar centenas
		PORTA = hex_value[contador % 100]; // Visualizar decenas
		digitalWrite(D3, LOW);						 // Encender decenas
		estado++;
	}
	else
	{
		// Apagar decenas y unidades, encender y visualizar centenas
		digitalWrite(D3, HIGH);							// Apagar decenas
		digitalWrite(D4, HIGH);							// Apagar unidades
		PORTA = hex_value[contador % 1000]; // Visualizar unidades
		digitalWrite(D2, LOW);							// Encender unidades
		estado = 0;
	}
}

void menu()
{
	Serial.println(" -- TURNOMATIC -- ");
	Serial.println("1.- Modo normal de visualización (tres dígitos): OFF-centenas-decenas-unidades");
	Serial.println("2.- Modo reducido-inferior de visualización (dos dígitos): OFF-OFF-decenas-unidades");
	Serial.println("3.- Modo reducido-superior de visualización (dos dígitos): decenas-unidades-OFF-OFF");
}