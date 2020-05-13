#include <dos.h>
#include <conio.h>
#include <stdio.h>
int number_of_repeats = 0;									// Количество повторений передачи данных в порт 0x60
int success_flag = 0;								// Флаг успешной передачи данных в порт 0x60
int blinking_flag = 0;						// Флаг мигания "CAPS LOCK"
int exit_flag = 0;								// Флаг "ESC"
void blink(void);							// Функция мигания индикаторами
void set_indicator(unsigned char mask);			// Функция управления индикаторами
void interrupt(*old_int_9)(...);					// Старый обработчик 9-го прерывания
void interrupt new_int_9(...);					// Новый обработчик 9-го прерывания
// Функция мигания индикаторами
void blink()
{
	set_indicator(0x04);							// индикатор "CAPS LOCK"
	delay(100);
	set_indicator(0x00);								// выключение всех индикаторов
	delay(100);
}
// Функция управления индикаторами
void set_indicator(unsigned char mask)
{
	success_flag = 0;
	while (!success_flag)							// Пока флаг успешной передачи данных в порт 0x60 сброшен
	{
		while ((inp(0x64) & 0x02) != 0x00);		// Ожидание освобождения входного буфера клавиатуры
		outp(0x60, 0xED);						// В порт записывается управляющий байт
	}
	success_flag = 0;
	while (!success_flag)							// Пока флаг успешной передачи данных в порт 0x60 сброшен
	{
		while ((inp(0x64) & 0x02) != 0x00);		// Ожидание освобождения входного буфера клавиатуры
		outp(0x60, mask);						//В порт записывается битовая маска, указывающая как мигать 
	}
}
// Новый обработчик 9-го прерывания
void interrupt new_int_9(...)
{
	unsigned char value_60h = 0;
	value_60h = inp(0x60);							// Значение из порта 0x60
	if (value_60h == 0x01) {
		exit_flag = 1;				// Установка флага выхода, если нажат "ESC"  
	}
	if (value_60h == 0x0B && blinking_flag == 0) { 
		blinking_flag = 1;			// Установка флага мигания, если нажат "0"
	}
	if (value_60h == 0xFE && blinking_flag == 1) {	// Если произошла ошибка (код 0xFE) 
		success_flag = 0;							// Сброс флага успешной передачи данных в порт 0x60
		if (number_of_repeats > 2)				// Если 3 неудачных попытки
		{
			printf("Error occured\n");
			exit_flag = 1;						// Установка флага "ESC"
			outp(0x20, 0x20);					// Сброс контроллера прерываний
			return;
		}
		number_of_repeats++;					// Количество неудачных попыток увеличивается
	}
	else {
		number_of_repeats = 0;					// Сброс количества неудачных попыток
		success_flag = 1;						// Установка флага успешной передачи данных в порт 0x60
	}
	clrscr();
	printf("Press \"0\" to blink CAPS LOCK\n");
	printf("Press \"ESC\" to exit\n");
	printf("Current value of 0x60 port is %x", value_60h);
	outp(0x20, 0x20);							// Сброс контроллера прерываний
}
void main()
{
	clrscr();
	printf("Press \"0\" to blink CAPS LOCK\n");
	printf("Press \"ESC\" to exit\n");
	disable();
	old_int_9 = getvect(9);						// Сохраняется адрес старого обработчика
	setvect(9, new_int_9);						// и меняется на новый  
	enable();
	while (!exit_flag) // Пока не нажат "ESC"
	{											
		if (blinking_flag) {					// Если нажат "0" 
			for (int blink_times = 0; blink_times < 5; blink_times++) {
				blink();							// мигает "CAPS LOCK"
			}
			blinking_flag = 0;						// Сброс флага мигания
		}
	}
	disable();
	setvect(9, old_int_9);						// Восстанавливается старый обработчик
	enable();
	return;
}