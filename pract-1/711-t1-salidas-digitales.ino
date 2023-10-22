/*
Hacer un programa para que haga parpadear (encender y apagar)
todos los segmentos del digito de las unidades (D4) durante
2 segundos, aproximadamente. Luego hacer que el parpadeo
pase de forma secuencial al resto de los digitos:

Unidades-D4 (parpadeo 2 seg) -> decenas-D3 (parpadeo 2 seg) ->
centenas-D2 (parpadeo 2 seg) -> millares-D1 (parpadeo 2 seg) ->
unidades-D4 (parpadeo 2 seg) y asi, sucesivamente.

Al cambiar de digito, generar una señal acustica para avisar al usuario.
Tiene completa libertad para elegir la duracion y la frequency
del pitido (o señal acustica) que se oira por el zumbador/altavoz.
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

// Matriz display 8 segmentos
char display_map[] = {D4, D3, D2, D1};

// Matriz teclado
char keyboard_map[][3] = {
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
}

void loop()
{
	Serial.println("Hola mundo!");

	// Imprime matriz de teclado
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			Serial.print(keyboard_map[i][j]);
		}
	}

	for (int i = 0; i < 4; i++)
	{
		// Sonar zumbador
		tone(PSTART, 1000, 100);

		// Parpadeo
		for (int j = 0; j < 2; j++)
		{
			digitalWrite(display_map[i], LOW);
			delay(500);
			digitalWrite(display_map[i], HIGH);
			delay(500);
		}
	}
}
