/*  Copyright (C) 1999, 2010  Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// TODO refactor
#include <cstdarg>

#include "printf.hpp"

void clean_vga_buffer(char color) {
  for(unsigned int i = 0; i < VGA_MODE_WIDTH*VGA_MODE_HEIGHT; i++) {
    (*(video + 2*i)) = (char)' ';
    (*(video + 2*i + 1)) = color;
  }
}

/*  Convert the integer D to a string and save the string in BUF. If
    BASE is equal to 'd', interpret that D is decimal, and if BASE is
    equal to 'x', interpret that D is hexadecimal. */
void
itoa (char *buf, long long base, long long d)
{
  char *p = buf;
  char *p1, *p2;
  unsigned long long ud = d;
  long long divisor = 10;
     
  /*  If %d is specified and D is minus, put `-' in the head. */
  if (base == 'd' && d < 0)
    {
      *p++ = '-';
      buf++;
      ud = -d;
    }
  else if (base == 'x')
    divisor = 16;
  else if (base == 'b')
    divisor = 2;
     
  /*  Divide UD by DIVISOR until UD == 0. */
  do
    {
      long long remainder = ud % divisor;
     
      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
  while (ud /= divisor);
     
  /*  Terminate BUF. */
  *p = 0;
     
  /*  Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
    {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
    }
}

/*  Put the character C on the screen. */
static void
putchar (int c)
{
  if (c == '\n' || c == '\r')
    {
    newline:
      xpos = 0;
      ypos++;
      if (ypos >= LINES)
        ypos = 0;
      return;
    }

  *(video + (xpos + ypos * COLUMNS) * 2) = c & 0xff;
  *(video + (xpos + ypos * COLUMNS) * 2 + 1) = ATTRIBUTE;

  xpos++;
  if (xpos >= COLUMNS)
    goto newline;
}

void prints(char *string, int length) {
  for(int i = 0; i < length; i++) {
    putchar(string[i]);
  }
}

/*  Format a string and print it on the screen, just like the libc
    function printf. */
void
printf (const char *format, ...)
{
  //char **arg = (char **) &format;
  int c;
  char buf[64];
  for(int i = 0; i < 64; i++)
    buf[i] = 0;
  char null_msg[] = "(null)\0";

  va_list args;
  va_start(args, format);

  //arg++;

  while ((c = *format++) != 0)
    {
      if (c != '%')
        putchar (c);
      else
        {
          char *p, *p2;
          long long pad0 = 0, pad = 0;

          c = *format++;
          if (c == '0')
            {
              pad0 = 1;
              c = *format++;
            }

          if (c >= '0' && c <= '9')
            {
              pad = c - '0';
              c = *format++;
            }
     
          switch (c)
            {
            case 'd':
            case 'u':
            case 'x':
            case 'b':
              //itoa (buf, c, *((long long*) arg++));
              itoa(buf, c, va_arg(args, long long));
              //asm("hlt\n");
              p = buf;
              goto string;
              break;
     
            case 's':
              //p = *arg++;
              p = va_arg(args, char*);
              if (! p)
                p = null_msg;
     
            string:
              for (p2 = p; *p2; p2++);
              for (; p2 < p + pad; p2++)
                putchar (pad0 ? '0' : ' ');
              while (*p)
                putchar (*p++);
              break;
     
            default:
              //putchar (*((long long*) arg++));
              putchar(*(long long*) va_arg(args, long long));
              break;
            }
        }
    }

  va_end(args);
}
