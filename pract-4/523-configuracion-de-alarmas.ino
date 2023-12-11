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

// Pulsadores
#define PRIGHT 30	 // PC[7] pulsador right
#define PDOWN 31	 // PC[6] pulsador down
#define PLEFT 32	 // PC[5] pulsador left
#define PSELECT 33 // PC[4] pulsador select/enter
#define PUP 34		 // PC[3] pulsador up
#define PSTART 37	 // PC[0] speaker

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

int map_7seg[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
String months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
String buffer = "";
int digit = 0;
int mode = 0;
boolean alarm1 = false;
boolean alarm2 = false;
int alarm_sec = 0;
int ext_mode = 0;
boolean text_mode = false;
String message = "";

// Función start para el bus I2C
void i2c_start()
{
	digitalWrite(ESC_SCL, LOW);	 // clock = 0
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, LOW);	 // SDA = 0

	// termina la función con SCL = 0 y SDA = 0
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
}

// Función stop para el bus I2C
void i2c_stop()
{
	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, LOW);	 // SDA = 0

	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	// termina la función con SCL = 0 y SDA = 0
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
}

// Función para escribir un 1 en el bus I2C (También se usará para enviar un 1 como ACK)
void i2c_w1()
{
	digitalWrite(ESC_SCL, LOW);	 // clock = 0
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, LOW);	 // clock = 0
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, LOW); // clock = 0
	digitalWrite(ESC_SDA, LOW); // SDA = 0

	// termina la función con SCL = 0 y SDA = 0
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
}

// Función para escribir un 0 en el bus I2C (También se usará para enviar un 0 como ACK)
void i2c_w0()
{
	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, LOW);	 // SDA = 0

	digitalWrite(ESC_SCL, LOW); // clock = 0
	digitalWrite(ESC_SDA, LOW); // clock = 0

	// termina la función con SCL = 0 y SDA = 0
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);
}

// Función para leer un bit del bus I2C (También se usará para leer el ACK)
int i2c_rbit()
{
	digitalWrite(ESC_SCL, LOW);	 // clock = 0
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, HIGH); // clock = 1
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	digitalWrite(ESC_SCL, HIGH);			// clock = 1
	int value = digitalRead(LEE_SDA); // leer el valor de SDA

	digitalWrite(ESC_SCL, LOW);	 // clock = 0
	digitalWrite(ESC_SDA, HIGH); // SDA = 1

	// termina la función con SCL = 0 y SDA = 0
	digitalWrite(ESC_SCL, LOW);
	digitalWrite(ESC_SDA, LOW);

	return value;
}

// Función para escribir un byte en el bus I2C
void i2c_wbyte(byte data)
{
	for (int i = 0; i < 8; i++)
	{
		if ((data & 0x80) != 0)
		{
			i2c_w1();
		}
		else
		{
			i2c_w0();
		}
		data = data << 1;
	}
}

// Función para leer un byte del bus I2C
byte i2c_rbyte()
{
	byte ibyte = 0;
	for (byte i = 0; i < 8; i++)
	{
		ibyte = (ibyte << 1) | (i2c_rbit() & 1);
	}
	return ibyte;
}

// Función para escribir un dato en una dirección de memoria
void i2c_rtc_write(int address, byte dato)
{
w1:
	i2c_start();
	i2c_wbyte(B11010000); // 0xD0
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

// Función para leer un dato de una dirección de memoria
byte i2c_rtc_read(int address)
{
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

// Función para establecer el cursor en una posición de la pantalla LCD
void set_cursor(int line, byte col)
{
	if (line == 1)
	{
		Serial3.write(0xFE);
		Serial3.write(128 + col);
	}
	else if (line == 2)
	{
		Serial3.write(0xFE);
		Serial3.write(192 + col);
	}
	else if (line == 3)
	{
		Serial3.write(0xFE);
		Serial3.write(148 + col);
	}
	else if (line == 4)
	{
		Serial3.write(0xFE);
		Serial3.write(212 + col);
	}
}

// Función para escribir la fecha en la pantalla LCD
void lcd_wdate()
{
	delay(100);
	set_cursor(3, 13);
	Serial3.write("DDMMMYY");
	// DIA
	delay(100);
	int day = i2c_rtc_read(4);
	set_cursor(4, 13);

	if (day < 10)
	{
		Serial3.print("0");
		set_cursor(4, 14);
		Serial3.print(day, HEX);
	}
	Serial3.print(day, HEX);
	// MES
	delay(100);
	set_cursor(4, 15);
	int nmonth = i2c_rtc_read(5) - 1;
	String month_hex = String(nmonth, HEX);
	String month = months[month_hex.toInt()];
	Serial3.print(month);
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

// Función para escribir la hora en la pantalla LCD
void lcd_whour()
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

// Función para escribir la temperatura en la pantalla LCD
void lcd_wtemp()
{
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

// Función para escribir la alarma 1 en la pantalla LCD
void lcd_walarm1()
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
	alarm_sec = i2c_rtc_read(7);
	alarm_sec = alarm_sec & 0x7F;
	alarm_sec = ((alarm_sec / 10) * 16) + (alarm_sec % 10);
}

// Función para escribir la alarma 2 en la pantalla LCD
void lcd_walarm2()
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

// Función para controlar el teclado
void keyboard(int column)
{
	int val = PINL >> 4;
	if (val == 15)
	{
		return;
	}
	while (PINL >> 4 != 15)
	{
	}
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
	}
}

// Función para leer el buffer de teclado
void read_buffer()
{
	if (buffer == "*#")
	{
		Serial.println("Entrando en el menu de configuracion...");
		mode = 1;
		Serial.write(12);
		buffer = "";
	}
	if (buffer == "#*")
	{
		Serial.println("Saliendo del menu de configuracion...");
		mode = 2;
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
		ext_mode = 1;
	}

	if (buffer == "#9")
	{
		ext_mode = 0;
	}
}

// ISR - Refresco de pantalla LCD
ISR(TIMER3_OVF_vect)
{
	if (text_mode == true)
	{
		Serial3.write(0xFE);
		Serial3.write(0x01); // Clear Screen
		delay(100);
		set_cursor(1, 0);
		Serial3.print(message);
		delay(2000);
		text_mode = false;
	}

	lcd_whour();
	lcd_wdate();
	lcd_wtemp();
	lcd_walarm1();
	lcd_walarm2();
}

// ISR - Funcionamiento del teclado
ISR(TIMER1_OVF_vect)
{
	if (ext_mode == 0)
	{
		switch (digit)
		{
			PORTL = DOFF;
		case 0:
			PORTL = B00001110;
			keyboard(digit);
			digit++;
			break;
		case 1:
			PORTL = B00001101;
			keyboard(digit);
			digit++;
			break;
		case 2:
			PORTL = B00001011;
			keyboard(digit);
			digit++;
			break;
		case 3:
			PORTL = B00000111;
			digit = 0;
			break;
		}
	}
	else if (ext_mode == 1)
	{
		switch (digit)
		{
			PORTL = DOFF;
		case 0:
			PORTL = B00001110;
			keyboard(digit);
			digit++;
			break;
		case 1:
			PORTL = B00001101;
			keyboard(digit);
			digit++;
			break;
		case 2:
			PORTL = B00001011;
			keyboard(digit);
			digit++;
			break;
		case 3:
			PORTL = B00000111;
			digit = 0;
			break;
		}
	}
	read_buffer();
}

// ISR - Interrupción de alarma
ISR(INT0_vect)
{
	if (alarm1 == true && alarm2 == false)
	{
		i2c_rtc_write(15, B11001000);
		tone(37, 350, 1000);
	}
	else if (alarm1 == false && alarm2 == true)
	{
		i2c_rtc_write(15, B1100110);
		tone(37, 350, 1000);
	}
	else if (alarm1 == true && alarm2 == true)
	{
		i2c_rtc_write(15, B11001010);
		tone(37, 350, 1000);
	}
}

// Timer 3
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

void prog_Timer1()
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
	// mode 14 --> 11 10
	// 1,6ms --> 0,0016 --> 1/0.0016 = 625 Hz

	// ICR1A = 7999;
	ICR1 = 7999;
	// OC1A = 3076;

	TCCR1A = B01000010;
	TCCR1B = B00011010;

	TIMSK1 |= (1 << TOIE1);
	sei();
}

// Función para controlar el menú
void menu()
{
m1:
	Serial.write(12);
	Serial.println("");
	Serial.println("\t\tMENU \n");
	Serial.println("1. Ajustar Hora");
	Serial.println("2. Ajustar Fecha");
	Serial.println("3. Ajustar Alarma 1");
	Serial.println("4. Ajustar Alarma 2");
	Serial.println("5. Leer mensaje de texto y almacenar");
	Serial.println("6. Leer mensaje de memoria y visualizar");
	Serial.println("");

	while (Serial.available() == 0 && mode == 1)
	{
	}

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

				i2c_rtc_write(2, horas);
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

				i2c_rtc_write(1, minutos);
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

				i2c_rtc_write(0, segundos);
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

				i2c_rtc_write(4, dia);
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

				i2c_rtc_write(5, mes);
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

				i2c_rtc_write(6, year);
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

				i2c_rtc_write(9, hour);

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

				i2c_rtc_write(8, minutes);

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

				i2c_rtc_write(7, segundos);

				goto a1;
			}

			if (suborden == '2')
			{
				// Activar alarma
				alarm1 = true;
				Serial.println("Alarma 1 activada");
				set_cursor(3, 5);
				Serial3.write("*");
				goto a1;
			}

			if (suborden == '3')
			{
				// Desactivar alarma
				alarm1 = false;
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

				i2c_rtc_write(12, hour);

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

				i2c_rtc_write(11, minutes);

				goto a2;
			}

			if (suborden == '2')
			{
				// Activar alarma
				alarm2 = true;
				Serial.println("Alarma 2 activada");
				set_cursor(4, 5);
				Serial3.write("*");
				goto a2;
			}

			if (suborden == '3')
			{
				// Desactivar alarma
				alarm2 = false;
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

			String message = Serial.readStringUntil(0x00);
		}
		if (orden == '6')
		{
			Serial.println("Introduzca un mensaje de texto");

			while (Serial.available() == 0)
			{
			} // Esperar

			message = Serial.readStringUntil(0x0D);

			Serial3.write(0xFE);
			Serial3.write(0x01); // Clear Screen
			delay(100);

			set_cursor(1, 0);
			Serial3.print(message);
			delay(7000);
			text_mode = false;
		}

	} // Serial.available()
}

void setup()
{
	Serial.begin(9600);
	Serial3.begin(9600);

	pinMode(LEE_SDA, INPUT);
	pinMode(LEE_SCL, INPUT);
	pinMode(ESC_SDA, OUTPUT);
	pinMode(ESC_SCL, OUTPUT);
	digitalWrite(ESC_SCL, HIGH);
	digitalWrite(ESC_SDA, HIGH);

	DDRA = 0xFF;
	PORTA = 0xFF;

	DDRL = 0x0F;
	PORTL = 0xFF;

	DDRC = B00000001;
	PORTC = B11111000;

	cli();
	EICRA |= (1 << ISC01) | (0 << ISC00);
	EIMSK |= (1 << INT0);
	sei();

	i2c_rtc_write(14, 31);

	i2c_rtc_write(13, 128);
	i2c_rtc_write(10, 128);

	delay(500);
	Serial.println("Pulse *# para acceder al menu de configuracion");
	Serial.println("Pulse #* para salir del menu de configuracion");
	prog_Timer3();
	prog_Timer1();
}

void loop()
{
	Serial3.write(0xFE);
	Serial3.write(0x0C);
	delay(500);
	if (mode == 1)
	{
		menu();
	}
}
