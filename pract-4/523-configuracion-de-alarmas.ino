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

volatile int tec_config = 0;
volatile int tec_vis = 0;
char option;
volatile int opcion_hora = 0;
volatile int opcion_fecha = 0;
volatile int valor;
volatile int configuracion = 0;

// Switch de la interrupción
volatile int digit;
// Lectura del teclado
volatile int row;
byte pinesFilas[] = {42, 43, 44, 45};
byte pinesColumnas[] = {47, 48, 49};
// Variables de onda

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

	// Asignación de valor a variables

	// Prueba del la pantalla LCD
	// habilitar canal TX3/RX3, canal de comunicaciones serie con la pantalla LCD (MILFORD 4x20 BKP)
	Serial3.begin(9600); // canal 3, 9600 baudios,
											 //  8 bits, no parity, 1 stop bit

	Serial3.write(0xFE);
	Serial3.write(0x01); // Clear Screen
	delay(100);

	pinMode(5, OUTPUT);
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(21, INPUT);

	// Timer 1: Modo 15 (Fast PWM, TOP = OCR1A), N=1024
	cli(); // Deshabilitamos las interrupciones
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1C = 0;
	TCNT1 = 0;

	OCR1A = 7812;
	OCR1B = 0;
	OCR1C = 0;

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

	Serial.println("#* Entrar modo visualizacion");
	Serial.println("*# Salir modo configuracion");
}

void loop()
{
	opciones();
	sonido_alarma();
}

void sonido_alarma()
{
	if (flag_alarma == 1)
	{
		for (int i = 0; i < 100; i++)
		{
			tone(37, 200, 100);
			delay(50);
		}
		flag_alarma = 0;
	}
}

// Parte práctica 4
byte rtcHigh(byte nbyte)
{
	byte a = (nbyte >> 4);
	return a;
}

byte rtcLow(byte nbyte)
{
	byte b = (nbyte << 4);
	b = (b >> 4);
	return b;
}

byte bcdByte(int newInt)
{
	byte tmp = newInt % 10;
	tmp |= (int(newInt / 10) % 10) << 4;
	return tmp;
}

void menu1()
{
	Serial3.write(0xFE);
	Serial3.write(0xC0); // posicionarse en linea 2
	Serial3.write("Alarm");
	delay(10); // linea 2

	Serial3.write(0xFE);
	Serial3.write(0xCE); // posicionarse en linea 2
	Serial3.write("T=");
	delay(10); // linea 2

	Serial3.write(0xFE);
	Serial3.write(0xD3); // posicionarse en linea 2
	Serial3.write("C");
	delay(10); // linea 2

	Serial3.write(0xFE);
	Serial3.write(0x4D); // posicionarse en linea 2
	Serial3.write("T=");
	delay(10); // linea 2

	Serial3.write(0xFE);
	Serial3.write(0xA1); // posicionarse en linea 3
	Serial3.write("DDMMYY");
	delay(10); // linea 3
}

void menu()
{
	Serial.println("");
	Serial.println("*** Menu de configuracion ***");
	Serial.println("1.- Ajustar hora");
	Serial.println("2.-Ajustar fecha");
	Serial.println("");
	Serial.println("3.-Configurar alarma 1");
	Serial.println("4.-Alarma 1 ON");
	Serial.println("5.-Alarma  1 OFF");
	Serial.println("");
	Serial.println("6.-Configurar alarma 2");
	Serial.println("7.-Alarma 2 ON");
	Serial.println("8.-Alarma 2 OFF");
	Serial.println("");
	Serial.println("9.-Apagar sonido de alarmas 1 y 2");
	Serial.println("Entrar opcion: ");
}

void menu_hora()
{
	Serial.println("");
	Serial.println("1.- Ajustar segundos");
	Serial.println("2.-Ajustar minutos");
	Serial.println("3.- Ajustar horas");
	Serial.println("4.-Salir al menu de opciones");
}

void menu_fecha()
{
	Serial.println("");
	Serial.println("1.- Ajustar dia");
	Serial.println("2.-Ajustar mes ");
	Serial.println("3.- Ajustar ano");
	Serial.println("4.-Salir al menu de opciones");
}

void menu_alarma()
{
	Serial.println("");
	Serial.println("1.-Ajustar minutos");
	Serial.println("2.- Ajustar horas");
	Serial.println("3.-Salir al menu de opciones");
}

// Parte de la práctica 1 y 2

// Actualización del contador en función delas teclas pulsadas
void modificador(char key)
{
	switch (key)
	{
	case '#':
		if (tec_config == 1)
		{
			Serial.println("Entrando modo configuracion");
			Serial.println("-----------------------------------------");
			menu();
			tec_config = 0;
			configuracion = 1;
		}
		else
		{
			tec_vis = 1;
		}
		break;
	case '*':
		if (tec_vis == 1)
		{
			Serial.println("Saliendo modo configuracion");
			tec_vis = 0;
			configuracion = 0;
		}
		else
		{
			tec_config = 1;
		}
		break;
	default:
		tec_config = 0;
		tec_vis = 0;
		break;
	}
}

// Seleccionar opcion
void opciones()
{
	if (configuracion == 1)
	{
		if (Serial.available() > 0)
		{
			opcion = Serial.parseInt();
		}
		switch (opcion)
		{
		case 1:
			menu_hora();
			while (Serial.available() == 0)
			{
			}
			opcion_hora = Serial.parseInt();
			switch (opcion_hora)
			{

			case 1:
				Serial.println("Introduce unos segundos validos(0-60):");
				while (Serial.available() == 0)
				{
				}
				valor = Serial.parseInt();
				Serial.println(valor);
				if (valor < 60 && valor >= 0)
				{
					writeDirByte(bcdByte(valor), 0x00);
				}
				else
				{
					Serial.println("Minutos no validos");
				}
				break;

			case 2:
				Serial.println("Introduce unos minutos validos(0-60):");
				while (Serial.available() == 0)
				{
				}
				valor = Serial.parseInt();
				Serial.println(valor);
				if (valor < 60 && valor >= 0)
				{
					writeDirByte(bcdByte(valor), 0x01);
				}
				else
				{
					Serial.println("Minutos no validos");
				}
				break;

			case 3:
				Serial.println("Introduce una hora valida(0-24):");
				while (Serial.available() == 0)
				{
				}
				valor = Serial.parseInt();
				Serial.println(valor);
				if (valor < 24 && valor >= 0)
				{
					writeDirByte(bcdByte(valor), 0x02);
				}
				else
				{
					Serial.println("Hora no valida");
				}
				break;
			case 4:
				Serial.println("Salir del ajuste");
				Serial.println("---------------------------------");
				menu();
				opcion = 0;
				break;
			}
			break;

		case 2:
			menu_fecha();
			while (Serial.available() == 0)
			{
			}
			opcion_fecha = Serial.parseInt();
			switch (opcion_fecha)
			{
			case 1:
				Serial.println("Introduce un dia valido(0-31):");
				while (Serial.available() == 0)
				{
				}
				valor = Serial.parseInt();
				Serial.println(valor);
				if (valor < 32 && valor >= 0)
				{
					writeDirByte(bcdByte(valor), 0x04);
				}
				else
				{
					Serial.println("Dia no valido");
				}
				break;
			case 2:
				Serial.println("Introduce un valor de mes valido:");
				while (Serial.available() == 0)
				{
				}
				valor = Serial.parseInt();
				Serial.println(valor);
				if (valor <= 12 && valor >= 0)
				{
					writeDirByte(bcdByte(valor), 0x05);
				}
				else
				{
					Serial.println("Mes no valido");
				}
				break;
			case 3:
				Serial.println("Introduce un valor de ano valido:");
				while (Serial.available() == 0)
				{
				}
				valor = Serial.parseInt();
				Serial.println(valor);
				if (valor < 10000 && valor >= 0)
				{
					writeDirByte(bcdByte(valor), 0x06);
				}
				else
				{
					Serial.println("Ano no valido");
				}
				break;
			case 4:
				Serial.println("Salir del ajuste");
				Serial.println("---------------------------------");
				menu();
				opcion = 0;
				break;
			}
			break;

		case 3:
			Serial.print("");
			Serial.print("Configuracion de alarma 1");
			menu_alarma();
			while (Serial.available() == 0)
			{
			}
			opcion_hora = Serial.parseInt();
			switch (opcion_hora)
			{
			case 1:
				Serial.println("Introduce unos minutos validos(0-60):");
				while (Serial.available() == 0)
				{
				}
				valor = Serial.parseInt();
				Serial.println(valor);
				if (valor < 60 && valor >= 0)
				{
					writeDirByte(bcdByte(valor), 0x08);
				}
				else
				{
					Serial.println("Minutos no validos");
				}
				break;

			case 2:
				Serial.println("Introduce una hora valida(0-24):");
				while (Serial.available() == 0)
				{
				}
				valor = Serial.parseInt();
				Serial.println(valor);
				if (valor < 24 && valor >= 0)
				{
					writeDirByte(bcdByte(valor), 0x09);
				}
				else
				{
					Serial.println("Hora no valida");
				}
				break;
			case 3:
				Serial.println("Salir del ajuste");
				Serial.println("---------------------------------");
				menu();
				opcion = 0;
				break;
			}
			break;

		case 4:
			writeDirByte(((B00000001) | readDir(0x0E)), 0x0E);
			menu();
			opcion = 0;
			break;

		case 5:
			Serial.println("Alarma 1 OFF");
			writeDirByte(((B11111110)&readDir(0x0E)), 0x0E);
			menu();
			opcion = 0;
			break;

		case 6:
			Serial.print("");
			Serial.print("Configuracion de alarma 2");
			menu_alarma();
			while (Serial.available() == 0)
			{
			}
			opcion_hora = Serial.parseInt();
			switch (opcion_hora)
			{
			case 1:
				Serial.println("Introduce unos minutos validos(0-60):");
				while (Serial.available() == 0)
				{
				}
				valor = Serial.parseInt();
				Serial.println(valor);
				if (valor < 60 && valor >= 0)
				{
					writeDirByte(bcdByte(valor), 0x0B);
				}
				else
				{
					Serial.println("Minutos no validos");
				}
				break;

			case 2:
				Serial.println("Introduce una hora valida(0-24):");
				while (Serial.available() == 0)
				{
				}
				valor = Serial.parseInt();
				Serial.println(valor);
				if (valor < 24 && valor >= 0)
				{
					writeDirByte(bcdByte(valor), 0x0C);
				}
				else
				{
					Serial.println("Hora no valida");
				}
				break;
			case 3:
				Serial.println("Salir del ajuste");
				Serial.println("---------------------------------");
				menu();
				opcion = 0;
				break;
			}
			break;

		case 7:
			Serial.println("Alarma 2 ON");
			writeDirByte(((B00000010) | readDir(0x0E)), 0x0E);
			menu();
			opcion = 0;
			break;

		case 8:
			Serial.println("Alarma 2 OFF");
			writeDirByte(((B11111101)&readDir(0x0E)), 0x0E);
			menu();
			opcion = 0;
			break;
		}
	}
}

// Función de lectura del teclado
void teclado(int digit)
{
	if (digitalRead(42) == LOW)
	{
		while (digitalRead(42) == LOW)
		{
			delay(50);
		}
		row = 0;
		modificador(teclado_map[row][digit]);
	}
	if (digitalRead(43) == LOW)
	{
		while (digitalRead(43) == LOW)
		{
			delay(50);
		}
		row = 1;
		modificador(teclado_map[row][digit]);
	}
	if (digitalRead(44) == LOW)
	{
		while (digitalRead(44) == LOW)
		{
			delay(50);
		}
		row = 2;
		modificador(teclado_map[row][digit]);
	}
	if (digitalRead(45) == LOW)
	{
		while (digitalRead(45) == LOW)
		{
			delay(50);
		}
		row = 3;
		modificador(teclado_map[row][digit]);
	}
}

// Parte de la práctica 3 i2c
// FUNCIONES OPTIMIZADAS
// Función start
void i2c_start()
{
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH); // Aquí habría que mirar que ambas estén a 1
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, LOW); // Flanco de bajada =>Esto causa el start
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW); // Se pone para asegurarse de deja CLK a 0
}
// Función stop
void i2c_stop()
{
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH); // Flanco de subida =>Esto causa el stop
}
// Función que envía 1 bit
void i2c_Ebit1()
{
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SCL, LOW); // Se baja la señal de reloj para cerrar el ciclo
}
// Función que envía un 0
void i2c_Ebit0()
{
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SCL, LOW); // Se baja la señal de reloj para cerrar el ciclo
}

// Función que lee un bit
int i2c_Rbit()
{
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);
	int val = digitalRead(LEE_SDA); // Aqui se produce la lectura y se guarda en val
	digitalWrite(ESC_SCL, LOW);			// Se baja la señal de reloj para cerrar el ciclo
	return val;											// Aqui se devuelve val
}

// FUNCIONES CON PROPOSITO DE AYUDA EN NUESTRAS OPCIONES
//  Función que escribe un byte en el bus
void writeByte(int dato)
{
	for (int i = 0; i < 8; i++)
	{
		if ((dato & 0x80) != 0)
		{ // Determinamos si es un 1 o un 0
			i2c_Ebit1();
		}
		else
		{
			i2c_Ebit0();
		}
		dato = dato << 1;
	}
}

// Función que lee un byte del bus
byte readByte()
{
	byte bRead = 0;
	for (int i = 0; i < 8; i++)
	{
		bRead = (bRead << 1) | (i2c_Rbit() & 1);
	}
	return bRead;
}

// Función que escribe un byte en una dirección concreta
void writeDirByte(byte dato, byte direccion)
{
START:
	i2c_start();

	writeByte(0xD0);
	if (i2c_Rbit() != 0)
	{
		goto START;
	}
	writeByte(direccion);
	if (i2c_Rbit() != 0)
	{
		goto START;
	}
	writeByte(dato);
	if (i2c_Rbit() != 0)
	{
		goto START;
	}
	// Stop
	i2c_stop();
}

// Funcion que lee una dirección determinada
byte readDir(int direccion)
{
read_start1:
	i2c_start();
	// byte de control(1010->memorias/000->dir. disp/0->W)
	writeByte(0xD0);
	if (i2c_Rbit() != 0)
	{
		goto read_start1;
	}

	writeByte(direccion);
	if (i2c_Rbit() != 0)
	{
		goto read_start1;
	}

read_start2:
	i2c_start();
	writeByte(0xD1);
	if (i2c_Rbit() != 0)
	{
		goto read_start2;
	}

	byte data = readByte();
	i2c_Ebit1();
	i2c_stop();
	return data;
}

ISR(TIMER1_OVF_vect)
{
	Serial3.write(0xFE);
	Serial3.write(128);
	byte segundos = readDir(0x00);
	byte minutos = readDir(0x01);
	byte hora = readDir(0x02);
	byte dia = readDir(0x04);
	byte mes = readDir(0x05);
	byte year = readDir(0x06);
	byte alarmM1 = readDir(0x08);
	byte alarmH1 = readDir(0x09);
	byte alarmM2 = readDir(0x0B);
	byte alarmH2 = readDir(0x0C);
	byte temp = readDir(0x11);
	mes = rtcHigh(mes) * 10 + rtcLow(mes);
	String meses[12] = {
			"JAN", "FEB", "MAR",
			"APR", "MAY", "JUN",
			"JUL", "AUG", "SEP",
			"OCT", "NOV", "DEC"};

	// Escribimos hora

	Serial3.print("     ");
	Serial3.print(rtcHigh(hora));
	Serial3.print(rtcLow(hora));
	Serial3.write(":");
	Serial3.print(rtcHigh(minutos));
	Serial3.print(rtcLow(minutos));
	Serial3.write(":");
	Serial3.print(rtcHigh(segundos));
	Serial3.print(rtcLow(segundos));
	Serial3.print("       ");

	// Linea 3
	Serial3.write(0xFE);
	Serial3.write(148);
	// alarma 1
	Serial3.print(rtcHigh(alarmH1)); // Digitos hora
	Serial3.print(rtcLow(alarmH1));
	Serial3.write(":");
	Serial3.print(rtcHigh(alarmM1)); // Digitos minutos
	Serial3.print(rtcLow(alarmM1));
	// Serial3.print();  // Marcador activación
	if (((B00000001)&readDir(0x0E)) == B00000001)
	{
		Serial3.write("*");
	}
	else
	{
		Serial3.write(" ");
	}
	Serial3.write("       DDMMMYY");

	// Escribimos Temperatura
	Serial3.write(0xFE);
	Serial3.write(192); // posicionarse en linea 2
	Serial3.write("ALARM         T=");
	if (temp & (1 << 7))
	{ // Comprobar el bit de signo (bit más significativo)
		Serial3.write("-");
		temp = ~(temp - 1); // Invertir todos los bits y restarle 1
	}
	else
	{
		Serial3.write("+");
	}
	Serial3.print(temp); // Digitos temperatura
	Serial3.print("C");

	// Linea 4
	Serial3.write(0xFE);
	Serial3.write(212);
	// Alarma 2
	Serial3.print(rtcHigh(alarmH2)); // Digitos hora
	Serial3.print(rtcLow(alarmH2));
	Serial3.write(":");
	Serial3.print(rtcHigh(alarmM2)); // Digitos minutos
	Serial3.print(rtcLow(alarmM2));
	// Serial3.print();  // Marcador activación
	if (((B00000010)&readDir(0x0E)) == B00000010)
	{
		Serial3.write("*");
	}
	else
	{
		Serial3.write(" ");
	}
	Serial3.write("       ");
	// Escribimos fecha
	Serial3.print(rtcHigh(dia));
	Serial3.print(rtcLow(dia));
	Serial3.print(meses[mes - 1]);
	Serial3.print(rtcHigh(year));
	Serial3.print(rtcLow(year));
}

ISR(TIMER3_COMPA_vect)
{
	PORTL = 0xFF;
	PORTA = 0;
	switch (digit)
	{
	case 0:
		PORTL = 0xFE;
		teclado(0);
		digit++;
		break;
	case 1:
		PORTL = 0xFD;
		teclado(1);
		digit++;
		break;
	case 2:
		PORTL = 0xFB;
		teclado(2);
		digit++;
		break;
	case 3:
		PORTL = 0xF7;
		digit = 0;
		break;
	}
}

ISR(INT0_vect)
{
	tone(37, 200, 10000);
}
