#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>

#define CTRL_KEY(k) ((k) & 0x1f)
//The c_lflag field is for “local flags”. A comment in macOS’s <termios.h> describes it as a “dumping ground for other state”. So perhaps it should be thought of as “miscellaneous flags”. The other flag fields are c_iflag (input flags), c_oflag (output flags), and c_cflag (control flags), all of which we will have to modify to enable raw mode.
/*** data ***/

struct editorCongfig() {
	struct termois orig_termios;
};

struct editorConfig E;

/*** terminal ***/

void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J",4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	perror(s);
	exit(1);
}

struct termios orig_termios;

void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1);
		die("tcsettattr");
}
void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr"));
	atexit(disableRawMode);

	struct termios raw = E.orig_termios;
	// ICANON is a "local" flag which is basically the CANONICAL mode enabler. 
	raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | IEXTEN | ICANON | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	
	// The TCSAFLUSH argument specifies when to apply the change: in this case, it waits for all pending output to be written to the terminal, and also discards any input that hasn’t been read.
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("csetattr");
}	

char editorReadKey() {
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN) die("read");
	}
	return c;
}

/*** input ***/

void editorProcessKeypress() {
	char c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J",4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
	}
}

/*** output ***/

void editorDrawRows() {
  int y;
  for (y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void editorRefreshScreen() {
       	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();
	write(STDIN_FILENO, "\x1b[H", 3);
}

/*** init ***/

int main() {
	enableRawMode();

	while(1){
	//	char c = '\0';
	//	if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read"); 
	//	if (iscntrl(c)) {
	//		printf("%d\r\n", c);
	//	}	
	//	else {
	//		printf("%d ('%c')\r\n", c, c);
	//	}
	//	if (c == CTRL_KEY('q')) break;
		editorRefreshScreen();
		editorProcessKeypress();
	}
	return 0;
}
