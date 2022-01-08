/*
 * oconsole.cpp
 *
 *  Created on: 20.08.2019
 *      Author: kanzlei
 */

#include "oconsole.h"

using namespace std;



namespace OConsole {



void ShellPrompt::initShell() {
	std::vector<std::string> commands;
	int bufsize = 300;
	char buf[bufsize];

	for (;;) {
		commands.clear();
		cout << this->prompt << ' ';
		cin.getline(buf,bufsize);
		string arg(buf);
		int args = StringUtil::parseArgs(arg, &commands);

		// Terminate?
		if ((args == 1) && (commands.at(0) == this->terminate)) break;

		parseCommand(commands); // Unterklassen führen aus...
	}
}


std::string getWorkingDirektory() {
	char *success;
	const int BUFSIZE_MIN = 200;
	int bsize = BUFSIZE_MIN;
	char *buf;
	for(;;) {
		buf = (char*) malloc (bsize+1);
		success = ::getcwd(buf, bsize);
		if (success == nullptr) {
			if (errno == ERANGE) { /* Buffer zu klein...  */
				bsize += BUFSIZE_MIN;
				free(buf);
				continue;
			} else {
				free(buf);
				throw std::ios_base::failure("Fehler beim Ermitteln des Arbeitsverzeichnisses");
			}
		} else {
			break; /* Endlosschleife beenden */
		}
	}
	std::string ret(buf);
	free(buf);
	return ret;
}


std::string getEnv(const std::string& key) { return std::getenv(key.c_str()); }


int setEnv(const std::string& key, const std::string& value, bool overwrite) {
	int i = overwrite ? 1 : 0;
	return setenv(key.c_str(), value.c_str(), i);
}

/***************************************|  Termios  |******************************************/

Termios::Termios() {
	int sucess = tcgetattr(STDIN_FILENO, &original); /* originale i/o settings speichern. original wird nie geändert! */
	if (sucess < 0) {
		std::stringstream buf;
		buf << "Fehler bei der Ausführung von tcgetattr:" << strerror(errno);
		throw OException::OperationFailedException(buf.str());
	}
	current = original;
}


bool Termios::lflagEcho() const { return (current.c_lflag & ECHO); }

void Termios::lflagEcho(bool b) {
	if (b) {
		current.c_lflag |= ECHO;
	} else {
		current.c_lflag &= ~ECHO;
	}
//	_setAttrNow(&current);
}


bool Termios::lflagEchoe() const { return (current.c_lflag & ECHOE); }


void Termios::lflagEchoe(bool b) {
	if (b) {
		current.c_lflag |= ECHOE;
	} else {
		current.c_lflag &= ~ECHOE;
	}
}

void Termios::disableBuffer()	{
	current.c_lflag &= ~ICANON; /* disable buffered i/o */
	current.c_cc[VMIN] = 1; /* Jedes Byte einzeln einlesen */
	current.c_cc[VTIME] = 0;
}


void Termios::implementNow() const {
	if (tcsetattr(STDIN_FILENO, TCSANOW, &current) < 0) {
		std::stringstream buf;
		buf << "OConsole::Termios: Fehler bei der Ausführung von tcsetattr:" << strerror(errno);
		throw OException::OperationFailedException(buf.str());
	}
}


void Termios::setRaw() {
	current.c_cc[VMIN]  = 1;
	current.c_cc[VTIME] = 0;
	current.c_cflag &= ~CSTOPB;
	current.c_cflag &= ~CRTSCTS;
	cfmakeraw(&current);
}

void Termios::reset() {	current = original; }


int _getKeyCode(const char* str) {
	if (strcmp(str, "[C") 	== 0) return KEY_ARROW_RIGHT;
	if (strcmp(str, "[D") 	== 0) return KEY_ARROW_LEFT;
	if (strcmp(str, "[A") 	== 0) return KEY_ARROW_UP;
	if (strcmp(str, "[B") 	== 0) return KEY_ARROW_DOWN;
	if (strcmp(str, "[3~")	== 0) return KEY_ENTF;
	if (strcmp(str, "[2~")	== 0) return KEY_EINFG;
	if (strcmp(str, "[H")	== 0) return KEY_POS1;
	if (strcmp(str, "[1~")	== 0) return KEY_POS1; /* Am Textbildschirm */
	if (strcmp(str, "[F")	== 0) return KEY_ENDE;
	if (strcmp(str, "[4~")	== 0) return KEY_ENDE; /* Am Textbildschirm */
	if (strcmp(str, "[5~")	== 0) return KEY_BILD_UP;
	if (strcmp(str, "[6~")	== 0) return KEY_BILD_DOWN;
	if (strcmp(str, "OP")	== 0) return KEY_F_1;
	if (strcmp(str, "[[A")	== 0) return KEY_F_1; /* Am Textbildschirm */
	if (strcmp(str, "OQ") 	== 0) return KEY_F_2;
	if (strcmp(str, "[[B") 	== 0) return KEY_F_2; /* Am Textbildschirm */
	if (strcmp(str, "OR") 	== 0) return KEY_F_3;
	if (strcmp(str, "[[C") 	== 0) return KEY_F_3; /* Am Textbildschirm */
	if (strcmp(str, "OS") 	== 0) return KEY_F_4;
	if (strcmp(str, "[[D") 	== 0) return KEY_F_4; /* Am Textbildschirm */
	if (strcmp(str, "[15") 	== 0) return KEY_F_5;
	if (strcmp(str, "[[E") 	== 0) return KEY_F_5; /* Am Textbildschirm */
	if (strcmp(str, "[17") 	== 0) return KEY_F_6;
	if (strcmp(str, "[18") 	== 0) return KEY_F_7;
	if (strcmp(str, "[19") 	== 0) return KEY_F_8;
	if (strcmp(str, "[20") 	== 0) return KEY_F_9;
	if (strcmp(str, "[21") 	== 0) return KEY_F_10;
	if (strcmp(str, "[23") 	== 0) return KEY_F_11;
	if (strcmp(str, "[24") 	== 0) return KEY_F_12;
	return -1; /* Taste nicht erkannt */
}

std::string wcharToString(wchar_t wc) {
	const int MAXDEST = 8;
	wchar_t buf[] = {wc, L'\0'};
	char dest[MAXDEST];
	wcstombs(dest, buf, MAXDEST-1);
	return std::string(dest);
}


int printWchar(wchar_t wc, std::ostream& os) {
	const int MAXDEST = 8;
	wchar_t buf[] = {wc, L'\0'};
	char dest[MAXDEST];
	wcstombs(dest, buf, MAXDEST-1);
	int ret;
	for (ret = 0; ret < MAXDEST; ret++) {
		char c = dest[ret];
		if (c == '\0') break;
		os.put(c);
	}
	return ret;
}


/* bildet getch() aus curses.h nach: */
wchar_t getch() {
	Termios term;
	term.disableBuffer();
	term.implementNow();
	const int BUFSIZE = 5;
	/* Bei den Nicht-ASCII-Zeichen und den Sondertasten sendet *
	* der Tastendruck mehrere Byte, deshalb muß in einen      *
	* String eingelesen werden:                               */
	char buf[BUFSIZE];
	int len;
	/* bei std::cin ist das ungepufferte Einlesen nicht möglich, *
	* deshalb read():                                           */
	len = read(STDIN_FILENO, buf, BUFSIZE-1);
	buf[len] = '\0';
	wchar_t ret;
	if (len == 1) { /* ASCII-Zeichen */
	   ret = buf[0];
	} else if (buf[0] == '\E') { /* Escape-Zeichen */
	   /* Substring von buf ohne das führende \E herstellen: */
	   char cmp[len];
	   for (int i = 0; i < len; i++) {
		   cmp[i] = buf[i+1];
	   }
	   ret = _getKeyCode(cmp); /* Wird cmp (die Taste) nicht erkannt, wird -1 zurückgegeben */
	} else { /* Multibyte-Zeichen */
	   wchar_t wbuf[5];
	   /* Multibyte-Zeichen in WideChar umwandeln: */
	   mbstowcs(wbuf, buf, 4);
	   ret = wbuf[0];
	}
	term.reset();
	term.implementNow();
	return ret;
}







/****************************|  Screen  |*********************************/


TextUtil::Dimension Screen::getDimension() {
	/* Siehe 'man ioctl_tty: ' */
	struct winsize {
		unsigned short ws_row;
		unsigned short ws_col;
		unsigned short ws_xpixel;   /* unused */
		unsigned short ws_ypixel;   /* unused */
	} dim;

	ioctl(STDIN_FILENO, TIOCGWINSZ, &dim);
	return TextUtil::Dimension(dim.ws_col, dim.ws_row);
}


bool Screen::isUnicodeTerminal() {
	/* Siehe 'man ioctl_console'!                     */
    /* KDGKBMODE ist im Header <linux/kd.h> definiert */
	long mode;
	if (ioctl(STDIN_FILENO, KDGKBMODE, &mode) < 0) {
		std::stringstream buf;
		buf << "OConsole::Screen::isUnicodeTerminal: Keyboard Mode konnte nicht abgefragt werden, weil " << strerror(errno);
		throw OException::IllegalOperationException(buf.str());
	}
	return (mode == K_UNICODE); /* Definitionen in man ioctl_console */
}


}  // Ende namespace OConsole

