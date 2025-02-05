#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>

struct termios orig_termios;

void disableRawMode() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void enableRawMode() {
	tcgetattr(STDIN_FILENO, &orig_termios);
	atexit(disableRawMode);

	struct termios raw= orig_termios;
	// ICANON is a "local" flag which is basically the CANONICAL mode enabler. 
	raw.c_iflag &= ~(IXON);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG);
	
	// The TCSAFLUSH argument specifies when to apply the change: in this case, it waits for all pending output to be written to the terminal, and also discards any input that hasn’t been read.
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}	


int main() {
	enableRawMode();

	char c;
	while(read(STDIN_FILENO, &c, 1) == 1 && c!='q') {
		if(iscntrl(c)) {
			printf("%d\n", c);
		}	
		else {
			printf("%d ('%c')\n", c, c);
		}	
	}
	return 0;
}
