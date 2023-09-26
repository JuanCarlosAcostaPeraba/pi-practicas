/*
Implementar una aplicacion con una funcionalidad similar
a la de un Turnomatic (dispositivo para dar un turno en
una cola de clientes) en el que la visualizacion en
el display y la exploracion del teclado se haga de forma
entrelazada y sincronizada por una interrupcion cada
5 ms generada por el pin 18 (INT3).

La aplicacion se implementara de acuerdo a las siguientes especificaciones:

1. Contador de 3 digitos
2. Control basado en 4 pulsadores con las siguientes funcionalidades:
	a. PUP: Incrementa el contador
	b. PDOWN: Decrementa el contador
	c. PENTER: Puesta a cero (reset) del contador
	d. PLEFT: El contador incrementara o decrementara su cuenta de 1 en 1
	e. PRIGHT: El contador incrementara o decrementara su cuenta de 2 en 2.
	f. Señal acustica por el altavoz del sistema cada vez cambie
		el estado del contador para asi avisar a los clientes.
3. Inicializacion del contador a cualquier valor, entre 000 y 999 mediante el teclado de 4x3
	a. Para modificar la cuenta con el teclado bastara
		con teclear los numeros y terminar con la tecla
		“#” que hara la funcion de “enter o entrar”. Ejemplo; 235#
4. Mostrar en la pantalla del PC (o en el terminal virtual de Proteus) el siguiente menu de opciones para poder elegir una de las acciones mostradas:
	1.- Modo normal de visualizacion (tres digitos): OFF-centenas-decenas-unidades
	2.- Modo reducido-inferior de visualizacion (dos digitos): OFF-OFF-decenas-unidades
	3.- Modo reducido-superior de visualizacion (dos digitos): decenas-unidades-OFF-OFF
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

int contador;

volatile int estado;
volatile char option;

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

	estado = 0;
	contador = 0;

	menu();
}

void loop()
{
	while (Serial.available() == 0)
	{
	}
	option = Serial.read();

	pup = digitalRead(PUP);
	pdown = digitalRead(PDOWN);
	pcenter = digitalRead(PSELECT);
	pleft = digitalRead(PLEFT);
	pright = digitalRead(PRIGHT);

	buttons_increment();
}

ISR(INT3_vect)
{
	switch (option)
	{
	case '1':
		mode_1();
		break;
	case '2':
		mode_2();
		break;
	case '3':
		mode_3();
		break;
	default:
		break;
	}
}

// Funcion que muestra el menu de opciones
void menu()
{
	Serial.println(" -- TURNOMATIC -- ");
	Serial.println("1.- Modo normal de visualizacion (tres digitos): OFF-centenas-decenas-unidades");
	Serial.println("2.- Modo reducido-inferior de visualizacion (dos digitos): OFF-OFF-decenas-unidades");
	Serial.println("3.- Modo reducido-superior de visualizacion (dos digitos): decenas-unidades-OFF-OFF");
}

// Funcion que enciende tres digitos: OFF-centenas-decenas-unidades
void mode_1()
{
	// Encender display
	if (estado == 0)
	{
		// Apagar decenas y centetas, encender y visualizar unidades
		digitalWrite(D3, HIGH);								 // Apagar decenas
		digitalWrite(D2, HIGH);								 // Apagar centenas
		digitalWrite(D1, HIGH);								 // Apagar unidades de millar
		PORTA = hex_value[int(contador % 10)]; // Visualizar unidades
		digitalWrite(D4, LOW);								 // Encender unidades
		estado++;
	}
	else if (estado == 1)
	{
		// Apagar unidades y centenas, encender y visualizar decenas
		digitalWrite(D4, HIGH);												// Apagar unidades
		digitalWrite(D2, HIGH);												// Apagar centenas
		digitalWrite(D1, HIGH);												// Apagar unidades de millar
		PORTA = hex_value[int((contador / 10) % 10)]; // Visualizar decenas
		digitalWrite(D3, LOW);												// Encender decenas
		estado++;
	}
	else
	{
		// Apagar decenas y unidades, encender y visualizar centenas
		digitalWrite(D3, HIGH);												 // Apagar decenas
		digitalWrite(D4, HIGH);												 // Apagar unidades
		digitalWrite(D1, HIGH);												 // Apagar unidades de millar
		PORTA = hex_value[int((contador / 100) % 10)]; // Visualizar unidades
		digitalWrite(D2, LOW);												 // Encender unidades
		estado = 0;
	}
}

// Funcion que enciende dos digitos: OFF-OFF-decenas-unidades
void mode_2()
{
	// Encender display
	if (estado == 0)
	{
		// Apagar decenas, encender y visualizar unidades
		digitalWrite(D3, HIGH);								 // Apagar decenas
		digitalWrite(D2, HIGH);								 // Apagar centenas
		digitalWrite(D1, HIGH);								 // Apagar unidades de millar
		PORTA = hex_value[int(contador % 10)]; // Visualizar unidades
		digitalWrite(D4, LOW);								 // Encender unidades
		estado++;
	}
	else
	{
		// Apagar unidades, encender y visualizar decenas
		digitalWrite(D4, HIGH);												// Apagar unidades
		digitalWrite(D2, HIGH);												// Apagar centenas
		digitalWrite(D1, HIGH);												// Apagar unidades de millar
		PORTA = hex_value[int((contador / 10) % 10)]; // Visualizar decenas
		digitalWrite(D3, LOW);												// Encender decenas
		estado = 0;
	}
}

// Funcion que enciende dos digitos: decenas-unidades-OFF-OFF
void mode_3()
{
	// Encender display
	if (estado == 0)
	{
		// Apagar millares, encender y visualizar centenenas
		digitalWrite(D4, HIGH);								 // Apagar unidades
		digitalWrite(D3, HIGH);								 // Apagar decenas
		digitalWrite(D1, HIGH);								 // Apagar unidades de millar
		PORTA = hex_value[int(contador % 10)]; // Visualizar unidades
		digitalWrite(D2, LOW);								 // Encender centenenas
		estado++;
	}
	else
	{
		// Apagar centenenas, encender y visualizar millares
		digitalWrite(D4, HIGH);												// Apagar unidades
		digitalWrite(D3, HIGH);												// Apagar decenas
		digitalWrite(D2, HIGH);												// Apagar centenas
		PORTA = hex_value[int((contador / 10) % 10)]; // Visualizar decenas
		digitalWrite(D1, LOW);												// Encender unidades de millar
		estado = 0;
	}
}

// Funcion para incrementar el contador por botones
void buttons_increment()
{
	if (pup == 0)
	{
		if (millis() - time_old > transition_time)
		{
			contador++;
			tone(PSTART, 1000, 100);
			logic();
			time_old = millis();
		}
	}
	else if (pdown == 0)
	{
		if (millis() - time_old > transition_time)
		{
			contador--;
			tone(PSTART, 1000, 100);
			logic();
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
			contador += 2;
			tone(PSTART, 1000, 100);
			logic();
			time_old = millis();
		}
	}
	else if (pleft == 0)
	{
		if (millis() - time_old > transition_time)
		{
			contador -= 2;
			tone(PSTART, 1000, 100);
			logic();
			time_old = millis();
		}
	}
}

void logic()
{
	// logica para que no se pase de 999
	if (contador > 999)
	{
		contador = 0;
	}
	else if (contador < 0)
	{
		contador = 999;
	}
}