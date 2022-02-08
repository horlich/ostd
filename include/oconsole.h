/*
 * oconsole.h
 *
 *  Created on: 20.08.2019
 *      Author: kanzlei
 */

#ifndef OCONSOLE_H_
#define OCONSOLE_H_

#include <string>
#include <vector>
#include <termios.h>
#include <iostream>
#include <dirent.h>
//#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include "otextutil.h"
//#include "oexception.h"
//#include "ocharutil.h"


namespace OConsole {



class ShellPrompt {
private:
	std::string prompt = "program>";
	std::string terminate = "quit";

protected:
	virtual void parseCommand(const std::vector<std::string>& args) = 0;

	void setPrompt(std::string str) { prompt = str; }

	std::string getPrompt() const { return prompt; }

	void setTerminate(std::string str) { terminate = str; }

	std::string getTerminate() const { return terminate; }

public:
	virtual ~ShellPrompt(){}

	void initShell();
};



std::string getWorkingDirectory();


/* Environment-Variablen abfragen: */
std::string getEnv(const std::string& key);


/* Siehe 'man setenv' */
int setEnv(const std::string& key, const std::string& value, bool overwrite);





/****************************|  Termios  |*********************************/

const int FUNKTIONSTASTEN_MIN	= 500;
const int KEY_ARROW_LEFT 		= 500;
const int KEY_ARROW_RIGHT	 	= 501;
const int KEY_ARROW_UP 			= 502;
const int KEY_ARROW_DOWN 		= 503;
const int KEY_ENTF 				= 504;
const int KEY_EINFG				= 505;
const int KEY_POS1				= 506;
const int KEY_ENDE				= 507;
const int KEY_BILD_UP			= 508;
const int KEY_BILD_DOWN			= 509;
const int KEY_F_1			   	= 510;
const int KEY_F_2			   	= 511;
const int KEY_F_3			   	= 512;
const int KEY_F_4			   	= 513;
const int KEY_F_5			   	= 514;
const int KEY_F_6			   	= 515;
const int KEY_F_7			   	= 516;
const int KEY_F_8			   	= 517;
const int KEY_F_9			   	= 518;
const int KEY_F_10				= 519;
const int KEY_F_11				= 520;
const int KEY_F_12				= 521;



class Termios {
	//
	/*--------------------------------------------------------------------------------------------------------
	 * Wrapper für termios.h
	 * Siehe 'man termios'
	 * Siehe dazu: https://stackoverflow.com/questions/7469139/what-is-the-equivalent-to-getch-getche-in-linux
	 * Siehe auch die Manpages von ioctl, ioctl_tty und ioctl_console!
	 * WICHTIG: das Programm muß die Locale setzen, damit Umlaute erkannt werden:
	 * #include <locale.h>
	 * setlocale(LC_ALL, "");
	 *-------------------------------------------------------------------------------------------------------- */
private:
	struct termios original, current;

public:
	Termios(); /* Konstruktor merkt sich den Zustand des Terminals *
	            * und speichert ihn als Urzustand in original      */

	/*---------------------  Flags:  --------------------*/

	bool lflagEcho() const;

	void lflagEcho(bool b); /* Defaultwert == true */

	bool lflagEchoe() const;

	void lflagEchoe(bool b); /* Defaultwert == true */

	void disableBuffer();

	void setRaw(); /* siehe 'man cfmakeraw' */

	/* Eingestellte Änderungen anwenden.
	 * Wirft OException::OperationFailedException: */
	void implementNow() const;

	/* Am Schluß muß reset() und implementNow() ausgeführt
	 * werden, weil sich die Konsole die Einstellungen auch
	 * nach Programmende noch merkt:                      */
	void reset();
};


/* Erzeugt aus einem WideChar einen Multibyte-String (z.B. für Stream): */
std::string wcharToString(wchar_t wc);


/* gibt einen WideChar als Multibyte-String aus    *
 * und gibt die Anzahl der gedruckten Bytes zurück */
int printWchar(wchar_t wc, std::ostream& os = std::cout);


/* bildet getch() aus curses.h nach.                     *
 * Bei unbekannter Funktionstaste wird -1 zurückgegeben: */
wchar_t xgetch();





/****************************|  Screen  |*********************************/

class Screen {
public:

	/* Anzahl der aktuellen cols und rows des Terminals: */
	static TextUtil::Dimension getDimension();

	/* Funktioniert nur in einer Console.                      */
	/* Wirft bei Fehler OException::IllegalOperationException: */
	static bool isUnicodeTerminal();
};

} // Ende namespace OConsole



#endif /* OCONSOLE_H_ */
