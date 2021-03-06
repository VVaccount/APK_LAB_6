#include <dos.h>
#include <conio.h>
#include <stdio.h>
int number_of_repeats = 0;									// ���������� ���������� �������� ������ � ���� 0x60
int success_flag = 0;								// ���� �������� �������� ������ � ���� 0x60
int blinking_flag = 0;						// ���� ������� "CAPS LOCK"
int exit_flag = 0;								// ���� "ESC"
void blink(void);							// ������� ������� ������������
void set_indicator(unsigned char mask);			// ������� ���������� ������������
void interrupt(*old_int_9)(...);					// ������ ���������� 9-�� ����������
void interrupt new_int_9(...);					// ����� ���������� 9-�� ����������
// ������� ������� ������������
void blink()
{
	set_indicator(0x04);							// ��������� "CAPS LOCK"
	delay(100);
	set_indicator(0x00);								// ���������� ���� �����������
	delay(100);
}
// ������� ���������� ������������
void set_indicator(unsigned char mask)
{
	success_flag = 0;
	while (!success_flag)							// ���� ���� �������� �������� ������ � ���� 0x60 �������
	{
		while ((inp(0x64) & 0x02) != 0x00);		// �������� ������������ �������� ������ ����������
		outp(0x60, 0xED);						// � ���� ������������ ����������� ����
	}
	success_flag = 0;
	while (!success_flag)							// ���� ���� �������� �������� ������ � ���� 0x60 �������
	{
		while ((inp(0x64) & 0x02) != 0x00);		// �������� ������������ �������� ������ ����������
		outp(0x60, mask);						//� ���� ������������ ������� �����, ����������� ��� ������ 
	}
}
// ����� ���������� 9-�� ����������
void interrupt new_int_9(...)
{
	unsigned char value_60h = 0;
	value_60h = inp(0x60);							// �������� �� ����� 0x60
	if (value_60h == 0x01) {
		exit_flag = 1;				// ��������� ����� ������, ���� ����� "ESC"  
	}
	if (value_60h == 0x0B && blinking_flag == 0) { 
		blinking_flag = 1;			// ��������� ����� �������, ���� ����� "0"
	}
	if (value_60h == 0xFE && blinking_flag == 1) {	// ���� ��������� ������ (��� 0xFE) 
		success_flag = 0;							// ����� ����� �������� �������� ������ � ���� 0x60
		if (number_of_repeats > 2)				// ���� 3 ��������� �������
		{
			printf("Error occured\n");
			exit_flag = 1;						// ��������� ����� "ESC"
			outp(0x20, 0x20);					// ����� ����������� ����������
			return;
		}
		number_of_repeats++;					// ���������� ��������� ������� �������������
	}
	else {
		number_of_repeats = 0;					// ����� ���������� ��������� �������
		success_flag = 1;						// ��������� ����� �������� �������� ������ � ���� 0x60
	}
	clrscr();
	printf("Press \"0\" to blink CAPS LOCK\n");
	printf("Press \"ESC\" to exit\n");
	printf("Current value of 0x60 port is %x", value_60h);
	outp(0x20, 0x20);							// ����� ����������� ����������
}
void main()
{
	clrscr();
	printf("Press \"0\" to blink CAPS LOCK\n");
	printf("Press \"ESC\" to exit\n");
	disable();
	old_int_9 = getvect(9);						// ����������� ����� ������� �����������
	setvect(9, new_int_9);						// � �������� �� �����  
	enable();
	while (!exit_flag) // ���� �� ����� "ESC"
	{											
		if (blinking_flag) {					// ���� ����� "0" 
			for (int blink_times = 0; blink_times < 5; blink_times++) {
				blink();							// ������ "CAPS LOCK"
			}
			blinking_flag = 0;						// ����� ����� �������
		}
	}
	disable();
	setvect(9, old_int_9);						// ����������������� ������ ����������
	enable();
	return;
}