#include <unistd.h>  // for read
#include <stdlib.h>  // for atexit
#include <termios.h> // for tcget/setatribs and termios struct
#include <stdio.h>   // for printff
#include <string.h>  // for memcpy
#include <ctype.h>   // for iscntrl

// A File Global Structure to remember the beginning Terminal Options
static struct termios orig;

void disableRawMode()
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

void enableRawMode()
{
  struct termios raw;
  tcgetattr(STDIN_FILENO, &orig);
  atexit(disableRawMode);

  (void)memcpy(&raw, &orig, sizeof(struct termios));

  // Disable OPOST -> output conversion of \n to \r\n
  raw.c_oflag &= ~(OPOST);
  // Disable IXON -> Software_flow_control -> Ctrl-S, Ctrl-Q
  // Disable ICRNL -> conversion of all hex values 13s(Carraige Retruns) to 10s
  // Disable INPCK, ISTRIP, INPCK, BRKINT (traditional for raw mode)
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // Disable 'ECHO' the letter printing as you type
  // Disable 'ICANON' so that you do not need to click 'enter' for input.
  // Dsiable 'ISIG' -> Signals like Ctrl-Z Ctrl-C.
  // DISABLE IEXTEN for Ctrl-V
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  raw.c_cflag |= (CS8);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main()
{
  enableRawMode();
  char c;
  while((1 == read(STDIN_FILENO, &c, 1)) &&(c != 'q'))
  {
    if (iscntrl(c))
    {
      printf("%d\r\n", c);
    }
    else
    {
      printf("%d ('%c')\r\n", c, c);
    }
  }
  return 0;
}
