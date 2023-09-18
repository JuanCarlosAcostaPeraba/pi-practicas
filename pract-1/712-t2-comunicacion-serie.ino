/*
Uso de las comunicaciones serie (TX0/RX0). Aprovechando el
código de las tareas anteriores, escribir un nuevo programa
consistente en presentar al usuario un menú en el "monitor serie"
del entorno de programación Arduino (Herramientas->Monitor Serie)
con al menos 6 opciones con las que podamos seleccionar una de las
siguientes acciones (el carácter inicial a visualizar en los
dígitos del display será todos los segmentos encendidos):

1. Parpadeo de las unidades
2. Parpadeo de las decenas
3. Parpadeo de las centenas
4. Parpadeo de las unidades de millar
5. Parpadeo secuencial con todos los dígitos (tarea1)
6. Selección del carácter hexadecimal (0-F) a visualizar en el display
*/

// Pulsadores
#define PRIGHT 30 // PC[7] pulsador right
#define PLEFT 31 // PC[6] pulsador down
#define PUP 32 // PC[5] pulsador left
#define PDOWN 33 // PC[4] pulsador enter
#define PSELECT 34 // PC[3] pulsador up
#define PSTART 37 // PC[0] speaker

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
	'C', 'D', 'E', 'F'
};

// Matriz valores hexadecimales en binario
char hex_value[16] = {
	0x3F, 0x06, 0x5B, 0x4F,
	0x66, 0x6D, 0x7D, 0x07,
	0x7F, 0x6F, 0x77, 0x7C,
	0x39, 0x5E, 0x79, 0x71
};

// Matriz teclado
char teclado_map[][3] = {
	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{'*', '0', '#'}
};

// Bool print menu
bool print_menu = true;

void setup() {
	Serial.begin(9600); // Inicializamos el puerto serie

	// Puerto A salida
	DDRA = 0xFF; // Configuramos el puerto A como salida (B11111111)
	PORTA = 0xFF; // Inicializamos el puerto A a 1 (B11111111)

	// Puerto L teclado
	DDRL = 0x0F; // Configuramos los pines 0, 1, 2 y 3 del puerto L como salida (B00001111)
	PORTL = 0xFF; // Inicializamos el puerto L a 1 (B11111111)

	// Puerto C
	DDRC = B00000001; // Configuramos el pin 0 del puerto C como salida (0x01)
	PORTC = B11111110; // Inicializamos el puerto C a 1 (0cFE)
}

void loop() {
	// Imprimir menú
	menu(print_menu);

	// Leer opción
	int option = Serial.read();

	// Ejecutar opción
	switch (option) {
		case '1': // Opción 1: Parpadeo de las unidades
			Serial.println("1. Parpadeo de las unidades");
			Serial.println("Presione 7 para salir");
			while (true) {
				digit_flashing(0);
				if (Serial.read() == 7) { // presionar 7 para salir
					break;
				}
			}
			print_menu = true;
			break;
		case '2': // Opción 2: Parpadeo de las decenas
			Serial.println("2. Parpadeo de las decenas");
			Serial.println("Presione 7 para salir");
			while (true) {
				digit_flashing(1);
				if (Serial.read() == 7) { // presionar 7 para salir
					break;
				}
			}
			print_menu = true;
			break;
		case '3': // Opción 3: Parpadeo de las centenas
			Serial.println("3. Parpadeo de las centenas");
			Serial.println("Presione 7 para salir");
			while (true) {
				digit_flashing(2);
				if (Serial.read() == 7) { // presionar 7 para salir
					break;
				}
			}
			print_menu = true;
			break;
		case '4': // Opción 4: Parpadeo de las unidades de millar
			Serial.println("4. Parpadeo de las unidades de millar");
			Serial.println("Presione 7 para salir");
			while (true) {
				digit_flashing(3);
				if (Serial.read() == 7) { // presionar 7 para salir
					break;
				}
			}
			print_menu = true;
			break;
		case '5': // Opción 5: Parpadeo secuencial con todos los dígitos (tarea1)
			Serial.println("5. Parpadeo secuencial con todos los dígitos");
			Serial.println("Presione 7 para salir");
			while (true) {
				sequential_flashing();
				if (Serial.read() == 7) { // presionar 7 para salir
					break;
				}
			}
			print_menu = true;
			break;
		case '6': // Opción 6: Selección del carácter hexadecimal (0-F) a visualizar en el display
			Serial.println("6. Selección del carácter hexadecimal (0-F) a visualizar en el display");
			Serial.println("Presione 7 para salir");
			select_hexadecimal();
			while (true) {
				if (Serial.read() == 7) { // presionar 7 para salir
					break;
				}
			}
			print_menu = true;
			break;
		default:
			Serial.println("Opción no válida");
			print_menu = true;
			break;
	}
	PORTA = 0xFF; // Resetar el puerto A a 1 (B11111111)
	for (int i = 0; i < 4; i++) { // Resetar el display apagando todos los dígitos
		digitalWrite(display_map[i], HIGH);
	}
}

void menu() {
	Serial.println("Seleccione una opción:");
	Serial.println("1. Parpadeo de las unidades");
	Serial.println("2. Parpadeo de las decenas");
	Serial.println("3. Parpadeo de las centenas");
	Serial.println("4. Parpadeo de las unidades de millar");
	Serial.println("5. Parpadeo secuencial con todos los dígitos");
	Serial.println("6. Selección del carácter hexadecimal (0-F) a visualizar en el display");
	print_menu = false;
}

// Función parpadeo dígito seleccionado
void digit_flashing(int digit) {
	for (int i = 0; i < 2; i++) {
		digitalWrite(display_map[digit], LOW);
		delay(500);
		digitalWrite(display_map[digit], HIGH);
		delay(500);
	}
}

// Función "Parpadeo secuencial con todos los dígitos" (tarea1)
void sequential_flashing() {
	for (int i = 0; i < 4; i++) {
		digit_flashing(i);
	}
}

// Función "Selección del carácter hexadecimal (0-F) a visualizar en el display"
void select_hexadecimal() {
	Serial.println("Seleccione un carácter hexadecimal entre los siguientes:");
	for (int i = 0; i < 16; i++) {
		Serial.print(hexadecimal[i] + " ");
	}
	Serial.println();
	int option = Serial.read();
	for (int i = 0; i < 16; i++) { // Recorremos la matriz hexadecimal
		if (option == hexadecimal[i]) { // Si la opción es igual a un elemento de la matriz
			PORTA = hex_value[i]; // Mostramos el valor del índice en el puerto A
			break;
		}
	}

	// Mostrar en el display
	Serial.println("Seleccione un dígito del display:");
	Serial.println("1. Unidades");
	Serial.println("2. Decenas");
	Serial.println("3. Centenas");
	Serial.println("4. Unidades de millar");
	int digit = Serial.read();
	switch (digit) {
		case '1':
			digitalWrite(display_map[0], LOW);
			break;
		case '2':
			digitalWrite(display_map[1], LOW);
			break;
		case '3':
			digitalWrite(display_map[2], LOW);
			break;
		case '4':
			digitalWrite(display_map[3], LOW);
			break;
		default:
			Serial.println("Opción no válida");
			break;
	}
}