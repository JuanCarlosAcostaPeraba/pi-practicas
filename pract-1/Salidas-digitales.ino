// Pulsadores
#define PRIGHT 30 // PC[7] pulsador right
#define PLEFT 31 // PC[6] pulsador down
#define PUP 32 // PC[5] pulsador left
#define PDOWN 33 // PC[4] pulsador enter
#define PSELECT 34 // PC[3] pulsador up
#define PSTART 37 // PC[0] speaker

// Teclado
#define D4 49 // Pin 49 - unidades
#define D3 48 // Pin 48 - decenas
#define D2 47 // Pin 47 - centenas
#define D1 46 // Pin 46 - unidades de millar

// Matriz teclado
char teclado_map[][3] = {
	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{'*', '0', '#'}
};

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
	Serial.println("Hola mundo!");

	// Imprime matriz de teclado
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			Serial.print(teclado_map[i][j]);
		}
	}

	// Parpadear unidades (2s)
	for (int = 0; i < 2; i++) {
		digitalWrite(D4, LOW);
		dealy(500)
		digitalWrite(D4, HIGH);
		dealy(500)
	}
	// Parpadear decenas (2s)
	for (int = 0; i < 2; i++) {
		digitalWrite(D3, LOW);
		dealy(500)
		digitalWrite(D3, HIGH);
		dealy(500)
	}
	// Parpadear centenas (2s)
	for (int = 0; i < 2; i++) {
		digitalWrite(D2, LOW);
		dealy(500)
		digitalWrite(D2, HIGH);
		dealy(500)
	}
	// Parpadear decenas (2s)
	for (int = 0; i < 2; i++) {
		digitalWrite(D1, LOW);
		dealy(500)
		digitalWrite(D1, HIGH);
		dealy(500)
	}
}