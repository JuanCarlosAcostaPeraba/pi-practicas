
// Declaración de variables
#define LEE_SCL 40 // puerto de entrada para leer el estado de la línea SCL
#define LEE_SDA 41 // puerto de entrada para leer el estado de la línea SDA
#define ESC_SCL 4	 // puerto de salida para escribir el valor de la línea SCL-out
#define ESC_SDA 39 // puerto de salida para escribir el valor de la línea SDA-out

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
char keyboard_map[][3] = {{'1', '2', '3'},
													{'4', '5', '6'},
													{'7', '8', '9'},
													{'*', '0', '#'}};

int tabla_7seg[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
String meses[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
String buffer = "";
int digit = 0;
int modo = 0;
boolean alarma1 = false;
boolean alarma2 = false;
int segundosAlarma = 0;
int modo_ext = 0;
boolean modo_texto = false;
String mensaje = "";

//---------------------------------FUNCIONES I2C BASICAS------------------------------------------------------------------------------------

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

void i2c_w1()
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

void i2c_w0()
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

int i2c_rbit()
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

//----------------------------------------------FUNCIONES I2C DE MEDIO NIVEL----------------------------------------------

void i2c_wbyte(byte dato)
{
	/*
	Funcion que escribe un byte
	parametros
	byte dato--> Dato que se va a escribir
	*/

	for (int i = 0; i < 8; i++)
	{
		if ((dato & 128) != 0)
		{
			i2c_w1();
		}
		else
		{
			i2c_w0();
		}
		dato = dato << 1;
	}
}

byte i2c_rbyte()
{
	/*
	Funcion que lee 8 bits, (1 byte), desde la posicion actual del puntero
	*/

	byte ibyte = 0;
	for (byte i = 0; i < 8; i++)
	{
		ibyte = (ibyte << 1) | (i2c_rbit() & 1);
	}
	return ibyte;
}

//--------------------------------------FUNCIONE I2C DE ALTO NIVEL--------------------------------------------------------------------------

void rtc_write(int address, byte dato)
{
	/*
	Funcion que escribe un dato de tipo byte entre 0 y 255 en una direccion elegida por el usuario entre 0 y 8190
	parametros:
	int address --> Direccion de memoria en la que se va a escribir el dato(0 ,8190)
	byte dato --> Dato que se va a escribir en memoria
	*/

w1:
	i2c_start();
	i2c_wbyte(B11010000); // ult. byte escritura //CONTROL BYTE
	if (i2c_rbit() != 0)
	{
		goto w1;
	}

	i2c_wbyte(address); // ADDRESS LOW BYTE
	if (i2c_rbit() != 0)
	{
		goto w1;
	}

	i2c_wbyte(dato); // DATA BYTE //dato a escribir
	if (i2c_rbit() != 0)
	{
		goto w1;
	}
	i2c_stop();
}

byte i2c_rtc_read(int address)
{
	/*
	Funcion que lee un dato de tipo byte desde memoria(0,8190) y lo devuelve
	parametros:
	int address --> Direccion de memoria en la que se va a leer el dato
	*/
w2:
	i2c_start();

	i2c_wbyte(B11010000); // Control byte
	if (i2c_rbit() != 0)
	{
		goto w2;
	}

	i2c_wbyte(address); // ADDRESS LOW BYTE
	if (i2c_rbit() != 0)
	{
		goto w2;
	}

// Romper escritura
w3:
	i2c_start();

	i2c_wbyte(B11010001); // Control byte para leer (ult. byte a 1)
	if (i2c_rbit() != 0)
	{
		goto w3;
	}

	byte data = i2c_rbyte();
	i2c_w1();
	i2c_stop();
	return data;
}

//-------------------------------FUNCION PARA MOVEL EL CURSOR EN LA PANTALLA LCD----------------------------
void set_cursor(int line, byte col)
{
	if (line == 1)
	{
		Serial3.write(0xFE);
		Serial3.write(128 + col);
	}

	if (line == 2)
	{
		Serial3.write(0xFE);
		Serial3.write(192 + col);
	}

	if (line == 3)
	{
		Serial3.write(0xFE);
		Serial3.write(148 + col);
	}

	if (line == 4)
	{
		Serial3.write(0xFE);
		Serial3.write(212 + col);
	}
}

//------------------------------FUNCIONES PARA ESCRIBIR DATOS EN LA PANTALLA LCD---------------------------------------------

// FECHA
void escribir_fecha()
{
	delay(100);
	set_cursor(3, 13);
	Serial3.write("DDMMMYY");

	// DIA
	delay(100);
	int dia = i2c_rtc_read(4);
	set_cursor(4, 13);

	if (dia < 10)
	{
		Serial3.print("0");
		set_cursor(4, 14);
		Serial3.print(dia, HEX);
	}
	Serial3.print(dia, HEX);

	// MES
	delay(100);
	set_cursor(4, 15);
	int nmes = i2c_rtc_read(5) - 1;
	String mes_hex = String(nmes, HEX);
	String mes = meses[mes_hex.toInt()];
	Serial3.print(mes);

	// AÑO
	delay(100);
	set_cursor(4, 18);
	int year = i2c_rtc_read(6);
	if (year < 10)
	{
		Serial3.print("0");
		set_cursor(4, 19);
		Serial3.print(year, HEX);
	}

	Serial3.print(year, HEX);
}

// HORA
void escribir_hora()
{
	// HORAS
	set_cursor(1, 5);
	int hours = i2c_rtc_read(2);

	if (hours < 10)
	{
		Serial3.write("0");
		set_cursor(1, 6);
		Serial3.print(hours, HEX);
	}
	else
	{
		Serial3.print(hours, HEX);
	}
	set_cursor(1, 7);
	Serial3.write(":");

	// MINUTOS
	set_cursor(1, 8);
	int mins = i2c_rtc_read(1);

	if (mins < 10)
	{
		Serial3.write("0");
		set_cursor(1, 9);
		Serial3.print(mins, HEX);
	}
	else
	{
		Serial3.print(mins, HEX);
	}

	set_cursor(1, 10);
	Serial3.write(":");

	// SEGUNDOS

	int secs = i2c_rtc_read(0);
	set_cursor(1, 11);

	if (secs < 10)
	{
		Serial3.write("0");
		set_cursor(1, 12);
		Serial3.print(secs, HEX);
	}
	else
	{
		Serial3.print(secs, HEX);
	}
}

// TEMPERATURA
void escribir_temp(int temperatura)
{
	rtc_write(17, temperatura);

	set_cursor(2, 14);
	Serial3.write("T=");

	int temp = i2c_rtc_read(17);

	set_cursor(2, 16);
	if (temp >= 0)
	{
		Serial3.print("+");
	}
	else
	{
		Serial3.print("-");
	}

	set_cursor(2, 17);
	Serial3.print(temp);

	set_cursor(2, 19);
	Serial3.print("C");
}

// ALARMA 1
void escribir_alarma_1()
{
	set_cursor(2, 0);
	Serial3.write("ALARM");

	// HORAS
	set_cursor(3, 0);
	int hours = i2c_rtc_read(9);

	if (hours < 10)
	{
		Serial3.write("0");
		set_cursor(3, 1);
		Serial3.print(hours, HEX);
	}
	else
	{
		Serial3.print(hours, HEX);
	}
	set_cursor(3, 2);
	Serial3.write(":");

	// MINUTOS
	set_cursor(3, 3);
	int mins = i2c_rtc_read(8);

	if (mins < 10)
	{
		Serial3.write("0");
		set_cursor(3, 4);
		Serial3.print(mins, HEX);
	}
	else
	{
		Serial3.print(mins, HEX);
	}

	segundosAlarma = i2c_rtc_read(7);
	segundosAlarma = segundosAlarma & 0x7F;

	segundosAlarma = ((segundosAlarma / 10) * 16) + (segundosAlarma % 10);

	/*Serial.print(int(segundosAlarma % 10));
	Serial.print(":");
			Serial.println(int(segundosAlarma /10)% 10);*/
}

// ALARMA 2
void escribir_alarma_2()
{
	// HORAS
	set_cursor(4, 0);
	int hours = i2c_rtc_read(12);

	if (hours < 10)
	{
		Serial3.write("0");
		set_cursor(4, 1);
		Serial3.print(hours, HEX);
	}
	else
	{
		Serial3.print(hours, HEX);
	}
	set_cursor(4, 2);
	Serial3.write(":");

	// MINUTOS
	set_cursor(4, 3);
	int mins = i2c_rtc_read(11);

	if (mins < 10)
	{
		Serial3.write("0");
		set_cursor(4, 4);
		Serial3.print(mins, HEX);
	}
	else
	{
		Serial3.print(mins, HEX);
	}
}

void comprobarAlarma()
{
}

//---------------------------------FUNCIONES PARA EL FUNCIONAMIENTO DEL TECLADO MATRICIAL--------------------------------
void teclado(int column)
{
	// Serial.println("Entra en teclado");
	int val = PINL >> 4;
	if (val == 15)
	{
		return;
	}
	while (PINL >> 4 != 15)
	{
	} // Espera activa, se espera a que se pulse un boton del teclado 4x3
	switch (val)
	{
	case 7:
		buffer = buffer + keyboard_map[0][column];
		break;
	case 11:
		buffer = buffer + keyboard_map[1][column];
		break;
	case 13:
		buffer = buffer + keyboard_map[2][column];
		break;
	case 14:
		buffer = buffer + keyboard_map[3][column];
		break;

	} // switch

} // teclado()

void separarBuffer()
{
	if (buffer == "*#")
	{
		Serial.println("Entrando en el menu de configuracion...");
		modo = 1;
		Serial.write(12);
		buffer = "";
	}
	if (buffer == "#*")
	{
		Serial.println("Saliendo del menu de configuracion...");
		modo = 2;
		buffer = "";
		Serial.write(12);
		Serial.println("Pulse *# para acceder al menu de configuracion");
		Serial.println("Pulse #* para salir del menu de configuracion");
	}

	if (buffer.length() == 2)
	{
		buffer = "";
	}

	if (buffer == "#3")
	{
		modo_ext = 1;
	}

	if (buffer == "#9")
	{
		modo_ext = 0;
	}
}

//----------------------------------------ISRs----------------------------------------------------------------------------

// ISR REFRESCADO DE PANTALLA LCD
ISR(TIMER3_OVF_vect)
{
	if (modo_texto == true)
	{
		Serial3.write(0xFE);
		Serial3.write(0x01); // Clear Screen
		delay(100);
		set_cursor(1, 0);
		Serial3.print(mensaje);
		delay(2000);
		modo_texto = false;
	}

	escribir_hora();
	escribir_fecha();
	escribir_temp(23);
	escribir_alarma_1();
	escribir_alarma_2();
}

// ISR FUNCIONAMIENTO DEL TECLADO MATRICIAL
ISR(TIMER1_OVF_vect)
{
	// Serial.println("Entrando en la ISR...");

	if (modo_ext == 0)
	{
		switch (digit)
		{
			PORTL = DOFF;
		case 0:
			PORTA = 0x6F;
			PORTL = B00001110;
			teclado(digit);
			digit++;
			break;
		case 1:
			PORTA = 0x6F;
			PORTL = B00001101;
			teclado(digit);
			digit++;
			break;
		case 2:
			PORTA = 0x6F;
			PORTL = B00001011;
			teclado(digit);
			digit = 0;
			break;
		} // switch()
	}

	else if (modo_ext == 1)
	{
		switch (digit)
		{
			PORTL = DOFF;
		case 0:
			// PORTA = tabla_7seg[int(segundosAlarma % 10)];
			PORTA = 0x3F;
			PORTL = B00001110;
			teclado(digit);
			digit++;
			break;
		case 1:
			// PORTA = tabla_7seg[int(segundosAlarma /10)% 10];
			PORTA = 0x3F;
			PORTL = B00001101;
			teclado(digit);
			digit++;
			break;
		case 2:
			PORTA = 0x3F;
			PORTL = B00001011;
			teclado(digit);
			digit = 0;
			break;
		} // switch()
	}

	separarBuffer();
}

// ISR INTERRUPCION ALARMA
ISR(INT0_vect)
{
	if (alarma1 == true && alarma2 == false)
	{
		// activar solo Alarma1;
		rtc_write(15, B11001000);
		tone(37, 350, 1000);
	}

	if (alarma1 == true && alarma2 == true)
	{
		// activar Alarma1 y Alarma2
		rtc_write(15, B11001010);
		tone(37, 350, 1000);
	}

	if (alarma1 == false && alarma2 == true)
	{
		// activar solo Alarma2
		rtc_write(15, B1100110);
		tone(37, 350, 1000);
	}
}

//----------------------------------PROGRAMACION DE LOS TIMERS------------------------------
// PROGRAMACION TIMER 3
void prog_Timer3()
{

	cli();
	// RESET TIMERS
	TCCR3A = 0;
	TCCR3B = 0;
	TCCR3C = 0;
	TCNT3 = 0;

	// PROGRAMACION (MODO = Fast PWM, TOP= ORC3A)

	// OCR3A = 15624;
	OCR3A = 15624;

	TCCR3A = B00000011;
	TCCR3B = B00011101;

	TIMSK3 |= (1 << TOIE3);
	sei();
}

// PROGRAMAACION TIMER 1
/*  void prog_Timer1(){

	cli();
	//RESET TIMERS
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1C = 0;
	TCNT1= 0;

	//PROGRAMACION (MODO = 4, CTC, TOP= ORC1A)

	//OCR1A = 1249;
	OCR1A = 1249;

	TCCR1A = B00000000;
	TCCR1B = B00001011;

	TIMSK1 |=  (1<<OCIE1A);
	sei();

	}*/

void prog_Timer1_Examen()
{

	cli();
	// RESET TIMERS
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1C = 0;
	TCNT1 = 0;

	// PROGRAMACION (MODO 14 = Fast PWM, TOP= ICR1)
	// T=4ms, N=8 --> 010
	// f = 1/0.004 = 250 Hz
	// modo 14 --> 11 10
	// 1,6ms --> 0,0016 --> 1/0.0016 = 625 Hz

	// ICR1A = 7999;
	ICR1 = 7999;
	// OC1A = 3076;

	TCCR1A = B01000010;
	TCCR1B = B00011010;

	TIMSK1 |= (1 << TOIE1);
	sei();
}

//--------------------------------FUNCION MENU PARA LA CONFIGURACION DEL RELOJ-------------------------------------------
void menu()
{
/*
TO DO LIST:
Mejorar la programacion de la temperatura (Preguntar por signo, como funciona temperatura)  (CON MASCARAS EN BINARIO, IGUAL QUE LAS ALARMAS...A BUSCARSE LA VIDA)
Arreglar Alarmas --> no vuelven a sonar cuando se cambia la hora

*/
m1:
	Serial.write(12);
	Serial.println("");
	Serial.println("     MENU \n");
	Serial.println("1. Ajustar Hora");
	Serial.println("2. Ajustar Fecha");
	Serial.println("3. Ajustar Alarma 1");
	Serial.println("4. Ajustar Alarma 2");
	Serial.println("5. Leer mensaje de texto y almacenar");
	Serial.println("6. Leer mensaje de memoria y visualizar");
	Serial.println("");

	while (Serial.available() == 0 && modo == 1)
	{
	} // Esperar

	if (Serial.available() > 0)
	{
		char orden = Serial.read();
		if (orden == '1')
		{
		s1:
			Serial.println("");
			Serial.println("1. Hora: ");
			Serial.println("2. Minutos: ");
			Serial.println("3. Segundos: ");
			Serial.println("4. Exit: ");
			Serial.println("");

			while (Serial.available() == 0)
			{
			}
			char suborden = Serial.read();
			if (suborden == '1')
			{
				Serial.println("Introducir Horas(0-23)");
			h1:
				while (Serial.available() == 0)
				{
				} // Esperar el dato

				int horas = Serial.parseInt();

				if (horas < 0 || horas > 23)
				{
					Serial.println("Hora incorrecta , vuelva a introducir la hora:");
					goto h1;
				}

				horas = ((horas / 10) * 16) + (horas % 10);

				Serial.print("Se han introducido las horas:  ");
				Serial.println(horas, HEX);

				rtc_write(2, horas);
				Serial.println("Se ha cambiado la hora con exito");

				goto s1;
			}
			if (suborden == '2')
			{
				Serial.println("Introducir Minutos(0-59)");
			min1:
				while (Serial.available() == 0)
				{
				} // Esperar el dato

				int minutos = Serial.parseInt();
				if (minutos < 0 || minutos > 59)
				{
					Serial.println("Minutos incorrectos , vuelva a introducir los minutos:");
					goto min1;
				}

				minutos = ((minutos / 10) * 16) + (minutos % 10);

				Serial.print("Se han introducido los minutos:  ");
				Serial.println(minutos, HEX);

				rtc_write(1, minutos);
				Serial.println("Se han cambiado los minutos con exito");
				goto s1;
			}
			if (suborden == '3')
			{
				Serial.println("Introducir Segundos(0-59)");
			seg1:
				while (Serial.available() == 0)
				{
				} // Esperar el dato

				int segundos = Serial.parseInt();
				if (segundos < 0 || segundos > 59)
				{
					Serial.println("Segundos incorrectos , vuelva a introducir los segundos:");
					goto seg1;
				}

				segundos = ((segundos / 10) * 16) + (segundos % 10);

				Serial.print("Se han introducido los segundos:  ");
				Serial.println(segundos, HEX);

				rtc_write(0, segundos);
				Serial.println("Se han cambiado los segundos con exito");
				goto s1;
			}
			if (suborden == '4')
			{
				Serial.println("Exit");
				goto m1;
			}
		}

		if (orden == '2')
		{
		f1:
			Serial.println("");
			Serial.println("1. Dia: ");
			Serial.println("2. Mes: ");
			Serial.println("3. Año: ");
			Serial.println("4. Exit: ");
			Serial.println("");

			while (Serial.available() == 0)
			{
			}
			char suborden = Serial.read();
			if (suborden == '1')
			{
				Serial.println("Introducir Dia(0-31)");
			dia1:
				while (Serial.available() == 0)
				{
				} // Esperar el dato

				int dia = Serial.parseInt();
				if (dia < 0 || dia > 31)
				{
					Serial.println("Dia incorrecto , vuelva a introducir el dia:");
					goto dia1;
				}

				dia = ((dia / 10) * 16) + (dia % 10);

				Serial.print("Se han introducido el dia:  ");
				Serial.println(dia, HEX);

				rtc_write(4, dia);
				Serial.println("Se ha cambiado el dia con exito");
				goto f1;
			}
			if (suborden == '2')
			{
				Serial.println("Introducir Mes(0-12)");
			mes1:
				while (Serial.available() == 0)
				{
				} // Esperar el dato

				int mes = Serial.parseInt();
				if (mes < 0 || mes > 12)
				{
					Serial.println("Mes incorrecto , vuelva a introducir el mes:");
					goto mes1;
				}

				Serial.print("Se han introducido el mes:  ");
				Serial.println(mes, HEX);

				rtc_write(5, mes);
				Serial.println("Se ha cambiado el mes con exito");
				goto f1;
			}
			if (suborden == '3')
			{
				Serial.println("Introducir Año(0-99)");
			year1:
				while (Serial.available() == 0)
				{
				} // Esperar el dato

				int year = Serial.parseInt();
				if (year < 0 || year > 99)
				{
					Serial.println("Año incorrecto , vuelva a introducir el año:");
					goto year1;
				}

				year = ((year / 10) * 16) + (year % 10);

				Serial.print("Se han introducido el año:  ");
				Serial.println(year, HEX);

				rtc_write(6, year);
				Serial.println("Se ha cambiado el año con exito");
				goto f1;
			}
			if (suborden == '4')
			{
				Serial.println("Exit");
				goto m1;
			}
		}

		if (orden == '3')
		{
		a1:
			Serial.println("");
			Serial.println("1. Configurar Alarma 1 ");
			Serial.println("2. Activar Alarma ");
			Serial.println("3. Desactivar Alarma ");
			Serial.println("4. Exit: ");
			Serial.println("");

			while (Serial.available() == 0)
			{
			}
			char suborden = Serial.read();

			if (suborden == '1')
			{
				Serial.println("CONFIGURANDO ALARMA...");
				Serial.println("Introducir la hora");
			hora2:
				while (Serial.available() == 0)
				{
				} // Esperar el dato

				int hour = Serial.parseInt();
				if (hour < 0 || hour > 23)
				{
					Serial.println("Hora incorrecta , vuelva a introducir la hora:");
					goto hora2;
				}

				hour = ((hour / 10) * 16) + (hour % 10);

				Serial.print("Se ha introducido la hora:  ");
				Serial.println(hour, HEX);

				rtc_write(9, hour);

				Serial.println("Introducir Minutos: ");
			min2:
				while (Serial.available() == 0)
				{
				} // Esperar el dato

				int minutes = Serial.parseInt();
				if (minutes < 0 || minutes > 59)
				{
					Serial.println("Minutos incorrectos , vuelva a introducir los minutos:");
					goto min2;
				}

				minutes = ((minutes / 10) * 16) + (minutes % 10);

				Serial.print("Se han introducido los minutos:  ");
				Serial.println(minutes, HEX);

				rtc_write(8, minutes);

				Serial.println("Introducir segundos");
			seg2:
				while (Serial.available() == 0)
				{
				} // Esperar el dato

				int segundos = Serial.parseInt();
				if (segundos < 0 || segundos > 59)
				{
					Serial.println("Segundos incorrectos , vuelva a introducir los segundos:");
					goto seg2;
				}

				segundos = ((segundos / 10) * 16) + (segundos % 10);

				Serial.print("Se han introducido los segundos:  ");
				Serial.println(segundos, HEX);

				rtc_write(7, segundos);

				goto a1;
			}

			if (suborden == '2')
			{
				// Activar alarma
				alarma1 = true;
				Serial.println("Alarma 1 activada");
				set_cursor(3, 5);
				Serial3.write("*");
				goto a1;
			}

			if (suborden == '3')
			{
				// Desactivar alarma
				alarma1 = false;
				Serial.println("Alarma 1 desactivada");
				set_cursor(3, 5);
				Serial3.write(" ");
				goto a1;
			}

			if (suborden == '4')
			{
				// Exit
				Serial.println("Exit");
				goto m1;
			}
		}

		if (orden == '4')
		{

		a2:
			Serial.println("");
			Serial.println("1. Configurar Alarma 2 ");
			Serial.println("2. Activar Alarma ");
			Serial.println("3. Desactivar Alarma ");
			Serial.println("4. Exit: ");
			Serial.println("");

			while (Serial.available() == 0)
			{
			}
			char suborden = Serial.read();

			if (suborden == '1')
			{
				Serial.println("CONFIGURANDO ALARMA...");
				Serial.println("Introducir la hora");
			hora3:
				while (Serial.available() == 0)
				{
				} // Esperar el dato

				int hour = Serial.parseInt();
				if (hour < 0 || hour > 23)
				{
					Serial.println("Hora incorrecta , vuelva a introducir la hora:");
					goto hora3;
				}

				hour = ((hour / 10) * 16) + (hour % 10);

				Serial.print("Se ha introducido la hora:  ");
				Serial.println(hour, HEX);

				rtc_write(12, hour);

				Serial.println("Introducir Minutos: ");
			min3:
				while (Serial.available() == 0)
				{
				} // Esperar el dato

				int minutes = Serial.parseInt();
				if (minutes < 0 || minutes > 59)
				{
					Serial.println("Minutos incorrectos , vuelva a introducir los minutos:");
					goto min3;
				}

				minutes = ((minutes / 10) * 16) + (minutes % 10);

				Serial.print("Se han introducido los minutos:  ");
				Serial.println(minutes, HEX);

				rtc_write(11, minutes);

				goto a2;
			}

			if (suborden == '2')
			{
				// Activar alarma
				alarma2 = true;
				Serial.println("Alarma 2 activada");
				set_cursor(4, 5);
				Serial3.write("*");
				goto a2;
			}

			if (suborden == '3')
			{
				// Desactivar alarma
				alarma2 = false;
				Serial.println("Alarma 2 desactivada");
				set_cursor(4, 5);
				Serial3.write(" ");
				goto a2;
			}

			if (suborden == '4')
			{
				// Exit
				Serial.println("Exit");
				goto m1;
			}
		}
		if (orden == '5')
		{
			Serial.println("Introduzca un mensaje de texto");

			while (Serial.available() == 0)
			{
			} // Esperar

			String mensaje = Serial.readStringUntil(0x00);
		}
		if (orden == '6')
		{
			Serial.println("Introduzca un mensaje de texto");

			while (Serial.available() == 0)
			{
			} // Esperar

			mensaje = Serial.readStringUntil(0x0D);

			Serial3.write(0xFE);
			Serial3.write(0x01); // Clear Screen
			delay(100);

			set_cursor(1, 0);
			Serial3.print(mensaje);
			delay(7000);
			modo_texto = false;
		}

	} // Serial.available()
}

//------------------------------------------------SETUP---------------------------------------------------------------
void setup()
{
	// put your setup code here, to run once:
	// habilitar canal TX0/RX0, canal de comunicaciones serie con el virtual terminal.
	Serial.begin(9600);
	pinMode(LEE_SDA, INPUT);
	pinMode(LEE_SCL, INPUT);
	pinMode(ESC_SDA, OUTPUT);
	pinMode(ESC_SCL, OUTPUT);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH);

	// PORTA: Segmentos a-f
	DDRA = 0xFF;	// PORTA de salida
	PORTA = 0xFF; // activamos segmentos a-g

	// PORTL[7:4]: filas del teclado
	DDRL = 0x0F;	// input;
	PORTL = 0xFF; // pull-up activos, cátodos/columnas teclado desactivadas

	// PORTC: Pulsadores y altavoz
	DDRC = B00000001;	 // PC7:1 input: PC0: output-speaker
	PORTC = B11111000; // pull-up activos menos el speaker que es de salida

	// PORTC= 0xF8;   // pull-up activos menos el speaker que es de salida

	// habilitar canal TX3/RX3, canal de comunicaciones serie con la pantalla LCD (MILFORD 4x20 BKP)
	Serial3.begin(9600); // canal 3, 9600 baudios,
											 //  8 bits, no parity, 1 stop bit

	cli();
	EICRA |= (1 << ISC01) | (0 << ISC00); // SE HABILITA EL MODO DE DISPARO POR FLANCO DE SUBIDA
	EIMSK |= (1 << INT0);									// ó EOMSK |= EIMSK B00000001;
	sei();

	// Habilitar las interrupciones de la alarma equivalente a escribir B00011111
	rtc_write(14, 31);

	rtc_write(13, 128); // B10000000  //Match horas y minutos alarma 2
	rtc_write(10, 128); // B10000000 // Match horas , minutos y segundos alarma 1

	delay(500);
	Serial.println("Pulse *# para acceder al menu de configuracion");
	Serial.println("Pulse #* para salir del menu de configuracion");
	prog_Timer3();
	prog_Timer1_Examen();

} // setup()

//----------------------------------------------------------LOOP-----------------------------------------------------------------------------
void loop()
{

	// MILFORD LCD on/off
	Serial3.write(0xFE);
	Serial3.write(0x0C); // Display on
	delay(500);

	if (modo == 1)
	{
		menu();
	}
}
