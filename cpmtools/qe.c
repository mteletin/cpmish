#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <cpm.h>

#define WIDTH 80
#define HEIGHT 23

typedef uint8_t uc;

uc physical_screen[HEIGHT][WIDTH];
uc logical_screen[HEIGHT][WIDTH];
#define logical_screen_start (logical_screen[0])
#define logical_screen_end (logical_screen[HEIGHT])
uc cursorx, cursory;

uc status_line_length;

void con_init(void)
{
	memset(physical_screen, ' ', sizeof(physical_screen));
	memset(logical_screen, ' ', sizeof(logical_screen));
	cpm_conout(26);
}

void con_newline(void)
{
	cursorx = 0;
	cursory++;
}

void con_goto(uc x, uc y)
{
	cursorx = x;
	cursory = y;
}

void con_putc(uc c)
{
	if (cursory >= HEIGHT)
		return;

	if (c < 32)
	{
		con_putc('^');
		c += '@';
	}

	logical_screen[cursory][cursorx] = c;
	cursorx++;
	if (cursorx == WIDTH)
		con_newline();
}

void con_puts(const uc* s)
{
	for (;;)
	{
		uc c = *s++;
		if (!c)
			break;
		con_putc(c);
	}
}

void scr_goto(uc x, uc y)
{
	if (!x && !y)
		cpm_conout(30);
	else
	{
		cpm_conout(27);
		cpm_conout('=');
		cpm_conout(y + ' ');
		cpm_conout(x + ' ');
	}
}

void con_refresh(void)
{
	static uc* lptr;
	static uc* pptr;

	lptr = logical_screen[0];
	pptr = physical_screen[0];
	while (lptr != logical_screen_end)
	{
		uint16_t offset;
		uc x, y;

		for (;;)
		{
			if (*lptr != *pptr)
				break;
			lptr++;
			pptr++;
			if (lptr == logical_screen_end)
				goto done;
		}

		offset = lptr - logical_screen[0];
		y = offset / WIDTH;
		x = offset % WIDTH;

		scr_goto(x, y);
		for (;;)
		{
			uc c = *lptr;
			if (c == *pptr)
				break;
			lptr++;
			*pptr++ = c;

			cpm_conout(c);
		}
	}
done:;

	scr_goto(cursorx, cursory);
}

void set_status_line(const char* message)
{
	uc length = 0;
	scr_goto(0, HEIGHT);
	for (;;)
	{
		uc c = *message++;
		if (!c)
			break;
		cpm_conout(c);
		length++;
	}
	while (length < status_line_length)
	{
		cpm_conout(' ');
		length++;
	}
	status_line_length = length;
	scr_goto(cursorx, cursory);
}

void main(int argc, const char* argv[])
{
	uc i;

	con_init();
	for (i=0; i<10; i++)
	{
		con_goto(70, i);
		con_puts("This is some text");
	}
	con_goto(0, HEIGHT-1);
	con_puts("This is the last line");
	con_refresh();
	set_status_line("This is status!");
	cpm_conin();
}

