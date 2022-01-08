/*
 * otextutil.h
 *
 *  Created on: 10.09.2019
 *      Author: kanzlei
 */

#ifndef OTEXTUTIL_H_
#define OTEXTUTIL_H_

#include <iostream>
#include <sstream>
#include <vector>

#include "ocharutil.h"
#include "debug.h"


namespace TextUtil {



struct Dimension {
	int height = -1;
	int width  = -1;
	Dimension(int h = 0, int w = 0) : height(h), width(w){}

	bool operator==(const Dimension& d) const {
		return ((height == d.height) && (width == d.width));
	}

	bool operator!=(const Dimension& d) const {
		return ((height != d.height) || (width != d.width));
	}

	Dimension operator+(const Dimension& d) const;

	Dimension operator-(const Dimension& d) const;
};

std::ostream& operator<<(std::ostream&, const Dimension&);








class ItemLine : public StringUtil::StringVisitor {
	//
	std::stringstream textbuf;
	int buflen  = 0;
	int maxcols = 0;

	VisitResult visit(const CharUtil::U32Char& ch) override;

public:
	/* Beschränkt schon die Speicherung der Texte mit maxcol. *
	 * maxCols == 0 bedeutet: keine Beschränkung:             */
	ItemLine(int maxCols = 0);

	virtual ~ItemLine() = default;

	int getMaxCols() const;

	/* Übernimmt den Text, bis ein Newline kommt oder MaxLength erreicht ist. */
	void setText(const std::string& str);

	std::string getText() const;

	/* Beschränkt erst den Ausdruck des Textes mit maxcol. *
	 * maxCols == 0 bedeutet: keine Beschränkung:          */
	std::ostream& print(std::ostream& os, int maxcols = 0);
};





using MenuDimension = std::pair<int, int>;

class ItemRange {
	std::vector<ItemLine*> ilVec;
	int maxcols = 0; /* Der Wert 0 heißt: keine Beschränkung */
	MenuDimension visibleRange;
	bool rangeHasMoved = true; /* seit dem letzten print-Befehl... */

public:
	ItemRange(int visibleRows, int maxCols = 0);

	virtual ~ItemRange();

	int countItems() const;

	void moveRange(int rows);

	/* TODO: ItemRange zur Laufzeit verändern:
	 * void expandRange(int deltaRows, int deltaCols); */

	bool hasIndex(int index) const;

	inline bool rangeMoved() const { return rangeHasMoved; }

	const ItemLine* addLine(const std::string& str);

	std::ostream& printLines(std::ostream& os);

	/* Wenn index sichtbar ist, wird 0 zurückgegeben.    *
	 * Ist er unter dem Rahmen, wird eine negative Zahl, *
	 * ist er über dem Rahmen, wird eine positive        *
	 * Zahl zurückgegeben:                               */
	int isVisible(int index) const;

	/* Wenn der Index außerhalb von visibleRange ist, *
	 * wird -1 zurückgegeben.                         */
	int getRangeLine(int index);

	int getVecIndex(int rangeline);

	const ItemLine* getItem(int index) const;

	/* Gibt die RangeLine von index zurück: */
	int setVisible(int index);

	void deleteItems();
};










class LineExplorer {
	/*
	 * Liest von einem Stream eine Zeile von maximal
	 * lineLength Breite ein. Zeilenumbruch, wenn
	 * möglich, beim letzten Whitespace vor Ereichen
	 * der maximalen Zeilenlänge.
	 *
	 * */
private:
	int linesize;
	const int TOKENPOS 			= 2;
	const int WSPOS    			= 1;
	const int BEGINPOS 			= 0;
	int lastPos 				= BEGINPOS;
	std::streampos lineStart	= 0;
	std::streampos lineLength 	= 0;
	std::streampos startNext 	= 0;
	bool eoffound 				= false;
	char *lineBuf 				= nullptr;

public:
	LineExplorer(int lineLength);

	virtual ~LineExplorer();

	inline bool eof() const { return eoffound; }

	inline bool isEmtpy() const { return lineLength == 0; }

	inline std::streampos getLinestart() const { return lineStart; }

	inline std::streampos getLinelength() const { return lineLength; }

	std::string readLine(std::istream& is);
};








class TextScroller {
	/* Gibt von einem eingelesenen Text ab beginLine maxlines aus
	 * Auf diese Weise kann ein Text rauf und runter gescrollt
	 * werden                                                  */
private:
	std::stringstream* sstream = nullptr;
	std::vector<std::string> lines;
	int	scrollpos = 0; // Die erste angezeigte Zeile
	Dimension panel;

	/* Finde die beginLine, bei der die letzte Zeile
	 * des Textes in der letzten Zeile des Panels
	 * erscheint. */
	int getMaxScrollpos() const;

	/* Kopier- und Zuweisschutz: */
	TextScroller(const TextScroller&);
	TextScroller& operator=(const TextScroller&);

public:
//	TextScroller(){ MESSAGE.println("TextScroller()"); }

	TextScroller(Dimension screen);

	virtual ~TextScroller();

	inline Dimension getPanel() const { return panel; }

	inline std::stringstream* getStream() { return sstream; }

	inline int getScrollPos() const { return scrollpos; }

	/* sstream und lines werden neu initialisiert: */
	void reset();

	/* sstream wird damit unbrauchbar (nullptr) */
	void prepareScrolling();

	std::string readLines(int beginLine = 0);

	inline int linesTotal() const { return lines.size(); }

	inline bool empty() const { return lines.empty(); }

	bool showsFirstLine() const;

	bool showsLastLine() const;
};






} // Ende namespace TextUtil




#endif /* OTEXTUTIL_H_ */
