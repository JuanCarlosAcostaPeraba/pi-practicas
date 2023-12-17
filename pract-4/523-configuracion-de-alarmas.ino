/* Práctica Lab4: Reloj despertador
 *
 * Descripción: Proyecto base con esquema, definiciones y programa demo
 * Programa demo: uso básico pantalla LCD
 *
 * Fichero: 	22-23_lab4_base.pdsprj
 * Creado: 		17 noviembre 2022
 * Autor:			Gustavo Vega Santana
 */

// Declaración de variables
#define ESC_SCL 4	 // puerto de salida para escribir el valor en la lí­nea SCL-out =>IO4 =>PG5
#define ESC_SDA 39 // puerto de salida para escribir el valor en la lí­nea SDA-out =>IO39=>PG2
#define LEE_SCL 40 // puerto de entrada para leer el estado de la lí­nea SCL       =>IO40=>PG1
#define LEE_SDA 41 // puerto de entrada para leer el estado de la lí­nea SDA       =>IO41=>PG0

#define pright 30	 // pulsador right
#define pdown 31	 // "" down
#define pleft 32	 // "" left
#define pcenter 33 // "" center
#define pup 34		 // "" up
#define speaker 37 // speaker

#define D4 0xFE		// 1111 1110 unidades
#define D3 0xFD		// 1111 1101 decenas
#define D2 0xFB		// 1111 1011 centenas
#define D1 0xF7		// 1111 0111 millares
#define DOFF 0xFF // 1111 1111 apagado: todos los cátados comunes a "1"
#define DON 0xC0	// 1111 0000   todos los cátados comunes a "0"

// Definición de las teclas del nuevo teclado
char teclado_map[][3] = {{'1', '2', '3'},
												 {'4', '5', '6'},
												 {'7', '8', '9'},
												 {'*', '0', '#'}};

boolean alarm1 = false;
boolean alarm2 = false;
String mes[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
								"JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
int digit = 0;
int mode = 0;
String buffer = "";
boolean modo = false;
boolean modoext = false;

// Setup
void setup()
{
	Serial.begin(9600);
	pinMode(LEE_SDA, INPUT);
	pinMode(LEE_SCL, INPUT);
	pinMode(ESC_SDA, OUTPUT);
	pinMode(ESC_SCL, OUTPUT);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH);

	DDRA = 0xFF;	// PORTA de salida
	PORTA = 0xFF; // activamos segmentos a-g

	DDRL = 0x0F;	// input;
	PORTL = 0xFF; // pull-up activos, c?todos/columnas teclado desactivadas

	DDRC = 0x01;	// PC7:1 input: PC0: output-speaker
	PORTC = 0xFE; // pull-up activos menos el speaker que es de salida

	Serial3.begin(9600); // canal 3, 9600 baudios,

	cli();
	TCCR3A = 0; // Registro funcional del modo
	TCCR3B = 0; // Registro funcional del modo
	TCNT3 = 0;	// Registro funcional del contador

	ICR3 = 31249; // TOP definido
	OCR3A = 12499;

	TCCR3A = B10000010; // modo 15
	TCCR3B = B00011100; // modo 15, N = 256
	TIMSK3 |= (1 << OCIE3A);
	sei();
	//---------------------------------------------------
	cli();
	TCCR1A = 0; // Registro funcional del modo
	TCCR1B = 0; // Registro funcional del modo
	TCNT1 = 0;	// Registro funcional del contador

	OCR1A = 1249; // TOP definido

	TCCR1A = B00000100; // modo 4
	TCCR1B = B00001011; // modo 4, N = 64
	TIMSK1 = B00000001; // habilitar interrupcion por overflow
	sei();
	//---------------------------------------------------
	cli();
	EICRA |= (1 << ISC01) | (0 << ISC00);
	EIMSK |= (1 << INT0);
	sei();

	write_memoria(0x0E, B00011111);
	write_memoria(0x0D, B10000000);
	write_memoria(0x0A, B10000000);
}

//-------------------------------------------------------------------------------------
//--------------funciones basicas------------------------------------------------
//--------------------------------------------------------------------------------------

void i2c_start()
{ // funcion naive para start
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, LOW); // Flanco de bajada =>Esto causa el start
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW); // Se pone para asegurarse de deja CLK a 0
}

void i2c_stop()
{ // funcion naive para stop
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, LOW);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH); // Flanco de subida =>Esto causa el stop
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH); // Si se quita sigue funcionando!
}

void i2c_Ebit1()
{ // funcion naive enviar un 1
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH); // Si se quita sigue funcionando!
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, HIGH);
}

void i2c_Ebit0()
{ // funcion naive enviar un 0
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, LOW);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, LOW); // Si se quita sigue funcionando!
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
}

void i2c_Ebit(bool val)
{ // funcion naive enviar un bit
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, val);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, val);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, val); // Si se quita sigue funcionando!
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, val);
}

int i2c_Rbit()
{ // funcion naive leer un bit
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH);
	digitalWrite(ESC_SCL, HIGH);
	int val = digitalRead(LEE_SDA); // Aqui se produce la lectura y se guarda en val!
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, HIGH);
	return val; // Aqui se devuelve val!
}

//---------------------------------------------------------------------------------------------
//------------------funciones medias----------------------------------------------------
//---------------------------------------------------------------------------------------------

void escribir_byte(byte dato)
{ // funcion donde escribimos un byte
	for (int i = 0; i < 8; i++)
	{
		if ((dato & 128) != 0)
		{ // vamos haciendo el and con 10000000 para ir printando cada bit
			i2c_Ebit1();
		}
		else
		{
			i2c_Ebit0(); // aqui marcamos si el resultado es 1 ponemos 1 y 0 en caso contrario
		}
		dato = dato << 1;
	}
}

byte leer_byte()
{ // en esta leemos un byte
	byte dato = 0;
	for (byte i = 0; i < 8; i++)
	{ // usando la misma idea de ir leyendo bit por bit
		dato = (dato << 1) | (i2c_Rbit() & 1);
	}
	return dato;
}

byte leer_memoria(int address)
{ // leemos la memoria siguiendo el esquema
r1:
	i2c_start();
	escribir_byte(B11010000); // con esto ponemos el byte de control
	if (i2c_Rbit() != 0)
	{
		goto r1;
	}
	escribir_byte(address); // y aqui vamos a lower
	if (i2c_Rbit() != 0)
	{
		goto r1;
	} // todo esto era solo para poner el puntero, no vamosa escribir nada
r2:
	i2c_start();
	escribir_byte(B11010001); // aqui es donde empezamos a leer de verdad
	if (i2c_Rbit() != 0)
	{
		goto r2;
	}
	byte byteVal = leer_byte(); // y en esta linea vemos como usamos la funcion anterior
	i2c_Ebit1();								// no ack
	i2c_stop();
	return byteVal;
}

void write_memoria(int address, byte data)
{ // en esta es donde escribimos el byte en una direccion dada
w1:
	i2c_start();
	escribir_byte(B11010000); // B10100000 como byte de control
	if (i2c_Rbit() != 0)
		goto w1;
	escribir_byte(address);
	if (i2c_Rbit() != 0)
		goto w1;
	escribir_byte(data); // y aqui finalmente es donde escribimos el byte
	if (i2c_Rbit() != 0)
		goto w1;
	i2c_stop();
}

void escribir_mem_eeprom(int address, byte data)
{
w1:
	i2c_start();
	escribir_byte(0xA0); // A0 como byte de control
	if (i2c_Rbit() != 0)
		goto w1;
	escribir_byte(address >> 8);
	if (i2c_Rbit() != 0)
		goto w1;
	escribir_byte(address & 0x00FF);
	if (i2c_Rbit() != 0)
		goto w1;
	escribir_byte(data); // y aqui finalmente es donde escribimos el byte
	if (i2c_Rbit() != 0)
		goto w1;
	i2c_stop();
}

byte leer_memoriaext(int address)
{ // leemos la memoria siguiendo el esquema
r1:
	i2c_start();
	escribir_byte(B10100000); // con esto ponemos el byte de control
	if (i2c_Rbit() != 0)
	{
		goto r1;
	}
	escribir_byte(address >> 8); // desplazamos para coger lo mas significativo
	if (i2c_Rbit() != 0)
	{
		goto r1;
	}
	escribir_byte(address & 0x00FF); // y aqui vamos a lower
	if (i2c_Rbit() != 0)
	{
		goto r1;
	} // todo esto era solo para poner el puntero, no vamosa escribir nada
r2:
	i2c_start();
	escribir_byte(B10100001); // aqui es donde empezamos a leer de verdad
	if (i2c_Rbit() != 0)
	{
		goto r2;
	}
	byte byteVal = leer_byte(); // y en esta linea vemos como usamos la funcion anterior
	i2c_Ebit1();								// no ack
	i2c_stop();
	return byteVal;
}
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------

void leer_hora()
{
	String hora = String(leer_memoria(2), HEX);
	if (hora.toInt() < 10)
	{
		hora = "0" + hora;
	}
	Serial3.print(hora);
}

void leer_minutos()
{
	String minutos = String(leer_memoria(1), HEX);
	if (minutos.toInt() < 10)
	{
		minutos = "0" + minutos;
	}
	Serial3.print(minutos);
}

void leer_segundos()
{
	String segundos = String(leer_memoria(0), HEX);
	if (segundos.toInt() < 10)
	{
		segundos = "0" + segundos;
	}
	Serial3.print(segundos);
}

void leer_temp()
{
	String temp = String(leer_memoria(17), DEC);
	if (temp.toInt() < 0)
	{
		temp = "T=-" + temp + "C";
	}
	else
	{
		temp = "T=+" + temp + "C";
	}
	Serial3.print(temp);
}

void leer_dia()
{
	String dia = String(leer_memoria(4), HEX);
	if (dia.toInt() < 10)
	{
		dia = "0" + dia;
	}
	Serial3.print(dia);
}

void leer_mes()
{
	int numes = leer_memoria(5) - 1;
	String mesh = String(numes, HEX);
	String mess = mes[mesh.toInt()];
	Serial3.print(mess);
}

void leer_year()
{
	String year = String(leer_memoria(6), HEX);
	if (year.toInt() < 10)
	{
		year = "0" + year;
	}
	Serial3.print(year);
}

void leer_alarm1()
{
	String alarma1 = String(leer_memoria(9), HEX);
	if (alarma1.toInt() < 10)
	{
		alarma1 = "0" + alarma1;
	}
	cursor(3, 0);
	Serial3.print(alarma1);
	cursor(3, 2);
	Serial3.print(":");
	alarma1 = String(leer_memoria(8), HEX);
	if (alarma1.toInt() < 10)
	{
		alarma1 = "0" + alarma1;
	}
	cursor(3, 3);
	Serial3.print(alarma1);
	if (alarm1 == true)
	{
		Serial3.write("*");
	}
	else
	{
		Serial3.write(" ");
	}
}

void leer_alarm2()
{
	String alarma2 = String(leer_memoria(0x0C), HEX);
	if (alarma2.toInt() < 10)
	{
		alarma2 = "0" + alarma2;
	}
	cursor(4, 0);
	Serial3.print(alarma2);
	cursor(4, 2);
	Serial3.print(":");
	alarma2 = String(leer_memoria(0x0B), HEX);
	if (alarma2.toInt() < 10)
	{
		alarma2 = "0" + alarma2;
	}
	cursor(4, 3);
	Serial3.print(alarma2);
	if (alarm2 == true)
	{
		Serial3.write("*");
	}
	else
	{
		Serial3.write(" ");
	}
}

void leer_7()
{
	String siete = String(leer_memoriaext(7));
	if (siete.toInt() < 10)
	{
		siete = "0" + siete;
	}
	Serial3.print(siete);
}

void leer_8()
{
	String ocho = String(leer_memoriaext(8));
	if (ocho.toInt() < 10)
	{
		ocho = "0" + ocho;
	}
	Serial3.print(ocho);
}

void leer_9()
{
	String nueve = String(leer_memoriaext(9));
	if (nueve.toInt() < 10)
	{
		nueve = "0" + nueve;
	}
	Serial3.print(nueve);
}

void leer_10()
{
	String diez = String(leer_memoriaext(0xA));
	if (diez.toInt() < 10)
	{
		diez = "0" + diez;
	}
	Serial3.print(diez);
}

String leer_horatemp()
{
	String horatemp = String(leer_memoria(2), HEX);
	if (horatemp.toInt() < 10)
	{
		horatemp = "0" + horatemp;
	}
	String minutostemp = String(leer_memoria(1), HEX);
	if (minutostemp.toInt() < 10)
	{
		minutostemp = "0" + minutostemp;
	}
	String segundostemp = String(leer_memoria(0), HEX);
	if (segundostemp.toInt() < 10)
	{
		segundostemp = "0" + segundostemp;
	}
	String temptemp = String(leer_memoria(17), DEC);
	if (temptemp.toInt() < 0)
	{
		temptemp = "-" + temptemp;
	}
	else
	{
		temptemp = "+" + temptemp;
	}
	String horat = horatemp + ":" + minutostemp + ":" + segundostemp + "#" + temptemp + "#";
	return horat;
}

void escribir_horatemp(String horatemperatura)
{
	for (int n = 0; n < horatemperatura.length(); n++)
	{
		escribir_mem_eeprom(n, horatemperatura[n]);
	}
}

void cursor(int fila, byte columna)
{
	if (fila == 1)
	{
		Serial3.write(0xFE);
		Serial3.write(128 + columna);
	}
	if (fila == 2)
	{
		Serial3.write(0xFE);
		Serial3.write(192 + columna);
	}
	if (fila == 3)
	{
		Serial3.write(0xFE);
		Serial3.write(148 + columna);
	}
	if (fila == 4)
	{
		Serial3.write(0xFE);
		Serial3.write(212 + columna);
	}
}

void teclado(int columna)
{
	int val = PINL >> 4;
	if (val == 15)
	{
		return;
	}
	while ((PINL >> 4) != 15)
	{
	}
	switch (val)
	{
	case 7:
		buffer = buffer + teclado_map[0][columna]; // 0111 se pulsa la primera fila
		break;
	case 11:
		buffer = buffer + teclado_map[1][columna]; // 1011 se pulsa la segunda fila
		break;
	case 13:
		buffer = buffer + teclado_map[2][columna]; // 1101 se pulsa la tercera fila
		break;
	case 14:
		buffer = buffer + teclado_map[3][columna]; // 1110 se pulsa la cuarta fila
		break;
	}
}

void comprobar_teclado()
{
	if (buffer == "#*")
	{ // si se han pulsado 3 numeros y el #
		modo = false;
		buffer = "";
		Serial.write(12);
	}
	else if (buffer == "*#")
	{ // si se han pulsado 2 numeros y el #
		modo = true;
		buffer = "";
	}
	else if (buffer == "*2")
	{ // si se han pulsado 2 numeros y el #
		modoext = true;
		buffer = "";
	}
	else if (buffer == "*0")
	{ // si se han pulsado 2 numeros y el #
		modoext = false;
		buffer = "";
	}
}

void menu()
{
menu:
	Serial.println("1.- Ajustar hora");
	Serial.println("2.- Ajustar fecha");
	Serial.println("3.- Ajustar alarma 1");
	Serial.println("4.- Ajustar alarma 2");
	Serial.println("5.- Guardar hora y temperatura");
	Serial.println("----");
	Serial.println("Entrar la opción: ");
	while (Serial.available() == 0 && modo == true)
	{
	}
	int mode1 = Serial.parseInt();
	Serial.println(mode1);
	Serial.println("");
	if (mode1 == 1)
	{
		Serial.println("1.- Hora");
		Serial.println("2.- Minuto");
		Serial.println("3.- Segundo");
		Serial.println("4.- Exit");
		while (Serial.available() == 0)
		{
		}
		int mode2 = Serial.parseInt();
		if (mode2 == 1)
		{
			Serial.println("Introducir hora: ");
			while (Serial.available() == 0)
			{
			}
			int hora = Serial.parseInt();
			hora = ((hora / 10) * 16) + (hora % 10);
			write_memoria(2, hora);
		}
		if (mode2 == 2)
		{
			Serial.println("Introducir min: ");
			while (Serial.available() == 0)
			{
			}
			int minuto = Serial.parseInt();
			minuto = ((minuto / 10) * 16) + (minuto % 10);
			write_memoria(1, minuto);
		}
		if (mode2 == 3)
		{
			Serial.println("Introducir seg: ");
			while (Serial.available() == 0)
			{
			}
			int segundo = Serial.parseInt();
			segundo = ((segundo / 10) * 16) + (segundo % 10);
			write_memoria(0, segundo);
		}
		if (mode2 == 4)
		{
			goto menu;
		}
	}
	if (mode1 == 2)
	{
		Serial.println("1.- Dia");
		Serial.println("2.- Mes");
		Serial.println("3.- Year");
		Serial.println("4.- Exit");
		while (Serial.available() == 0)
		{
		}
		int mode3 = Serial.parseInt();
		if (mode3 == 1)
		{
			Serial.println("Introducir dia: ");
			while (Serial.available() == 0)
			{
			}
			int dia = Serial.parseInt();
			dia = ((dia / 10) * 16) + (dia % 10);
			write_memoria(4, dia);
		}
		if (mode3 == 2)
		{
			Serial.println("Introducir mes: ");
			while (Serial.available() == 0)
			{
			}
			int mes = Serial.parseInt();
			write_memoria(5, mes);
		}
		if (mode3 == 3)
		{
			Serial.println("Introducir year: ");
			while (Serial.available() == 0)
			{
			}
			int year = Serial.parseInt();
			year = ((year / 10) * 16) + (year % 10);
			write_memoria(6, year);
		}
		if (mode3 == 4)
		{
			goto menu;
		}
	}
	if (mode1 == 3)
	{
	alarma1:
		Serial.println("1.- Hora");
		Serial.println("2.- Minuto");
		Serial.println("3.- ON");
		Serial.println("4.- OFF");
		Serial.println("5.- Exit");
		while (Serial.available() == 0)
		{
		}
		int mode4 = Serial.parseInt();
		if (mode4 == 1)
		{
			Serial.println("Introducir hora: ");
			while (Serial.available() == 0)
			{
			}
			int hora = Serial.parseInt();
			hora = ((hora / 10) * 16) + (hora % 10);
			Serial.print(hora);
			write_memoria(9, hora);
			goto alarma1;
		}
		if (mode4 == 2)
		{
			Serial.println("Introducir min: ");
			while (Serial.available() == 0)
			{
			}
			int minuto = Serial.parseInt();
			minuto = ((minuto / 10) * 16) + (minuto % 10);
			write_memoria(8, minuto);
			goto alarma1;
		}
		if (mode4 == 3)
		{
			alarm1 = true;
			goto menu;
		}
		if (mode4 == 4)
		{
			alarm1 = false;
			goto menu;
		}
		if (mode4 == 5)
		{
			goto menu;
		}
	}
	if (mode1 == 4)
	{
	alarma2:
		Serial.println("1.- Hora");
		Serial.println("2.- Minuto");
		Serial.println("3.- ON");
		Serial.println("4.- OFF");
		Serial.println("5.- Exit");
		while (Serial.available() == 0)
		{
		}
		int mode5 = Serial.parseInt();
		if (mode5 == 1)
		{
			Serial.println("Introducir hora: ");
			while (Serial.available() == 0)
			{
			}
			int hora = Serial.parseInt();
			hora = ((hora / 10) * 16) + (hora % 10);
			write_memoria(0x0C, hora);
			goto alarma2;
		}
		if (mode5 == 2)
		{
			Serial.println("Introducir min: ");
			while (Serial.available() == 0)
			{
			}
			int minuto = Serial.parseInt();
			minuto = ((minuto / 10) * 16) + (minuto % 10);
			write_memoria(0x0B, minuto);
			goto alarma2;
		}
		if (mode5 == 3)
		{
			cursor(3, 5);
			Serial3.write("*");
			alarm2 = true;
			goto menu;
		}
		if (mode5 == 4)
		{
			cursor(3, 5);
			Serial3.write(" ");
			alarm2 = false;
			goto menu;
		}
		if (mode5 == 5)
		{
			goto menu;
		}
	}
	if (mode1 == 5)
	{
		String hhmmsstt = leer_horatemp();
		escribir_horatemp(hhmmsstt);
	}
}
ISR(TIMER3_COMPA_vect)
{
	Serial3.write(0xFE); // inicio de comunicacion con al lcd
	Serial3.write(0x01); // clear pantalla lcd
	Serial3.write(0xFE);
	Serial3.write(0x00);
	if (!modoext)
	{
		cursor(1, 5);
		leer_hora();
		Serial3.write(":");
		leer_minutos();
		Serial3.write(":");
		leer_segundos();
		cursor(2, 0);
		Serial3.write("ALARM");
	}
	else
	{
		cursor(1, 0);
		leer_7();
		cursor(1, 3);
		leer_8();
		cursor(1, 6);
		leer_9();
		cursor(1, 9);
		leer_10();
	}

	cursor(2, 13);
	leer_temp();

	cursor(3, 13);
	Serial3.write("DDMMMYY");

	cursor(4, 13);
	leer_dia();
	leer_mes();
	leer_year();

	leer_alarm1();

	leer_alarm2();
}

ISR(TIMER1_OVF_vect)
{
	PORTL = DOFF;
	switch (digit)
	{
	case 0:
		PORTL = D4;
		teclado(digit);
		digit++;
		break;
	case 1:
		PORTL = D3;
		teclado(digit);
		digit++;
		break;
	case 2:
		PORTL = D2;
		teclado(digit);
		digit = 0;
		break;
	}
	comprobar_teclado();
}

ISR(INT0_vect)
{
	if (alarm1 == true && alarm2 == false)
	{
		write_memoria(0x0F, B11001000);
		tone(37, 250, 1000);
	}
	if (alarm1 == false && alarm2 == true)
	{
		write_memoria(0x0F, B11001000);
		tone(37, 250, 1000);
	}
	if (alarm1 == true && alarm2 == true)
	{
		write_memoria(0x0F, B11001000);
		tone(37, 250, 1000);
	}
}
void loop()
{
	if (modo == true)
	{
		menu();
	}
}
