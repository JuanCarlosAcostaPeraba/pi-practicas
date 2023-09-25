/*
Uso de las comunicaciones serie (TX0/RX0). Aprovechando el
codigo de las tareas anteriores, escribir un nuevo programa
consistente en presentar al usuario un menu en el "monitor serie"
del entorno de programacion Arduino (Herramientas->Monitor Serie)
con al menos 6 opciones con las que podamos seleccionar una de las
siguientes acciones (el caracter inicial a visualizar en los
digitos del display sera todos los segmentos encendidos):

1. Parpadeo de las unidades
2. Parpadeo de las decenas
3. Parpadeo de las centenas
4. Parpadeo de las unidades de millar
5. Parpadeo secuencial con todos los digitos (tarea1)
6. Seleccion del caracter hexadecimal (0-F) a visualizar en el display
*/

// Pulsadores
#define PRIGHT 30	 // PC[7] pulsador right
#define PLEFT 31	 // PC[6] pulsador left
#define PUP 32		 // PC[5] pulsador up
#define PDOWN 33	 // PC[4] pulsador down
#define PSELECT 34 // PC[3] pulsador select/enter
#define PSTART 37	 // PC[0] speaker

// Display 8 segmentos
#define D4 49 // Pin 49 - unidades
#define D3 48 // Pin 48 - decenas
#define D2 47 // Pin 47 - centenas
#define D1 46 // Pin 46 - unidades de millar

char option;
char back;
char caracter;

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

void setup()
{
	Serial.begin(9600); // Inicializamos el puerto serie

	// Puerto A salida
	DDRA = B11111111;	 // Configuramos el puerto A como salida (0xFF)
	PORTA = B11111111; // Inicializamos el puerto A a 1 (0xFF)

	// Puerto L teclado
	DDRL = B00001111;	 // Configuramos los pines 0, 1, 2 y 3 del puerto L como salida (0x0F)
	PORTL = B11111111; // Inicializamos el puerto L a 1 (0xFF)

	// Puerto C
	DDRC = B00000000;	 // Configuramos el pin 0 del puerto C como salida (0x00)
	PORTC = B11111111; // Inicializamos el puerto C a 1 (0cFF)

	menu();

	back = '7';
}

void loop()
{
	// Leer opcion
	if (Serial.available() > 0)
	{
		// Sonar zumbador
		tone(PSTART, 1000, 100);
		option = Serial.read();
		if (option != '6')
		{
			back = option;
		}
	}

	// Ejecutar opcion
	switch (option)
	{
	case '1': // Opcion 1: Parpadeo de las unidades
		digit_flashing(0);
		break;
	case '2': // Opcion 2: Parpadeo de las decenas
		digit_flashing(1);
		break;
	case '3': // Opcion 3: Parpadeo de las centenas
		digit_flashing(2);
		break;
	case '4': // Opcion 4: Parpadeo de las unidades de millar
		digit_flashing(3);
		break;
	case '5': // Opcion 5: Parpadeo secuencial con todos los digitos (tarea1)
		sequential_flashing();
		break;
	case '6': // Opcion 6: Seleccion del caracter hexadecimal (0-F) a visualizar en el display
		hexadecimal_selection();
		option = back; // Volver a la opcion anterior
		break;
	}
}

void menu()
{
	Serial.println("Seleccione una opcion:");
	Serial.println("1. Parpadeo de las unidades");
	Serial.println("2. Parpadeo de las decenas");
	Serial.println("3. Parpadeo de las centenas");
	Serial.println("4. Parpadeo de las unidades de millar");
	Serial.println("5. Parpadeo secuencial con todos los digitos");
	Serial.println("6. Seleccion del caracter hexadecimal (0-F) a visualizar en el display");
}

// Funcion parpadeo digito seleccionado
void digit_flashing(int digit)
{
	for (int i = 0; i < 2; i++)
	{
		digitalWrite(display_map[digit], LOW);
		delay(200);
		digitalWrite(display_map[digit], HIGH);
		delay(200);
	}
}

// Funcion "Parpadeo secuencial con todos los digitos" (tarea1)
void sequential_flashing()
{
	// Sonar zumbador
	tone(PSTART, 1000, 100);

	// Parpadeo
	for (int i = 0; i < 4; i++)
	{
		digit_flashing(i);
	}
}

// Función "Selección del carácter hexadecimal (0-F) a visualizar en el display"
void hexadecimal_selection()
{
	Serial.println("Selecciona un caracter hexadecimal (0-F):");
	while (Serial.available() == 0)
	{
	}
	caracter = Serial.read();
	if (caracter >= 48 && caracter <= 57) // Si es un numero (0-9)
	{
		PORTA = hex_value[caracter - 48]; //
	}
	else if (caracter >= 65 && caracter <= 70) // Si es una letra (A-F)
	{
		PORTA = hex_value[caracter - 55];
	}
	else if (caracter >= 97 && caracter <= 102) // Si es una letra (a-f)
	{
		PORTA = hex_value[caracter - 87];
	}
	else
	{
		Serial.println("Caracter no valido");
	}
}