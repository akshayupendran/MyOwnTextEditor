/************************
*        HEADERS        *
************************/
#include <errno.h>   // for errorno and eagain
#include <unistd.h>  // for read
#include <stdlib.h>  // for atexit
#include <termios.h> // for tcget/setatribs and termios struct
#include <stdio.h>   // for printff
#include <string.h>  // for memcpy
#include <ctype.h>   // for iscntrl

/************************
*           BSS         *
************************/
// A File Global Structure to remember the beginning Terminal Options
static struct termios orig_termios;

/***********
*  DEFINES *
***********/
#define CTRL_KEY(k) ((k) & 0x1f)

/**************************
* FUNCTION DECLATARATIONS *
**************************/
void die(const char *s);
void disableRawMode();
void enableRawMode();

/*****************
* TERMINAL METHODS *
*****************/
void die(const char *s)
{
  perror(s);
  exit(1);
}

void disableRawMode()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode()
{
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;
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
  raw.c_cc[VMIN] = 0;
  // Warning in Bash for windows VTIME Does not work
  raw.c_cc[VTIME] = 1; //1/10 of a second / 100ms

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey()
{
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
  {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}
/*** output ***/
void editorRefreshScreen()
{
  write(STDOUT_FILENO, "\x1b[2J", 4);
}

/*** input ***/
void editorProcessKeypress()
{
  char c = editorReadKey();
  switch (c)
  {
    case CTRL_KEY('q'):
      exit(0);
      break;
  }
}

/*** init ***/
int main()
{
  enableRawMode();
  while(1)
  {
    editorRefreshScreen();
    editorProcessKeypress();
  }
  return 0;
}
