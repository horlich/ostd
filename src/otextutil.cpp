/*
 * otextutil.cpp
 *
 *  Created on: 10.09.2019
 *      Author: kanzlei
 */


#include "otextutil.h"

using namespace std;
using namespace TextUtil;



std::ostream& TextUtil::operator<<(std::ostream& os, const Dimension& d) {
	os.put('(');
	os << d.height;
	os.put(',');
	os << d.width;
	os.put(')');
	return os;
}



constexpr Dimension Dimension::operator+(const Dimension& d) const {
	Dimension ret;
	ret.height = height + d.height;
	ret.width  = width + d.width;
	return ret;
}

constexpr Dimension Dimension::operator-(const Dimension& d) const {
	/* noch nicht getestet!!! */
	Dimension ret;
	ret.height = height - d.height;
	ret.width  = width - d.width;
	return ret;
}














/*
 *
 *                   I T E M L I N E :
 *
 * */


ItemLine::ItemLine(int maxCols) : maxcols(maxCols)	 {}


int ItemLine::getMaxCols() const { return maxcols; }


std::string ItemLine::getText() const { return textbuf.str(); }


std::ostream& ItemLine::print(std::ostream& os, int maxcols) {
	if (maxcols <= 0) {
		os << textbuf.str();
	} else {
		textbuf.clear();
		textbuf.seekg(0);
		for (int i = 0; i < maxcols; i++) {
			int ch = textbuf.get();
			if (! textbuf.good()) break;
			os.put(ch);
		}
	}
	return os;
}


ItemLine::VisitResult ItemLine::visit(wchar_t ch) {
	if (ch == '\n') return VisitResult::BREAK;
	if (ch == '\t') {
		textbuf.put(' ');
	} else {
		textbuf << ch;
	}
	if ((maxcols > 0) && (++buflen >= getMaxCols())) return VisitResult::BREAK;
	return VisitResult::CONTINUE;
}


void ItemLine::setText(const std::string& str) {
	textbuf.str("");
	parseString(str);
}









ItemRange::ItemRange(int visibleRows, int maxCols) :
	maxcols(maxCols),
	visibleRange(0, visibleRows-1) {}


ItemRange::~ItemRange() { deleteItems(); }


int ItemRange::countItems() const { return ilVec.size(); }


void ItemRange::moveRange(int rows) {
	if (rows < 0) {
		/* Verhindern, daß visibleRange.first < 0 */
		 if ((rows + visibleRange.first) < 0) rows = visibleRange.first;
	} else {
		/* Verhindern, daß visibleRange.second >= countItems() */
		if ((visibleRange.second + rows + 1) > countItems()) rows = countItems() - 1 - visibleRange.second;
	}
	if (rows == 0) return;
	visibleRange.first  += rows;
	visibleRange.second += rows;
	rangeHasMoved = true;
}


bool ItemRange::hasIndex(int index) const { return ((index >= 0) && (index < countItems())); }


const ItemLine* ItemRange::addLine(const std::string& str) {
	ItemLine* ret = new ItemLine(maxcols);
	ilVec.push_back(ret);
	ret->setText(str);
	return ret;
}

std::ostream& ItemRange::printLines(std::ostream& os) {
	if (ilVec.empty()) return os;
	int last = countItems() - 1;
	if (visibleRange.second < last) last = visibleRange.second;
	for (int i = visibleRange.first; i <= last; i++) {
		if (i != visibleRange.first) os.put('\n');
		ilVec.at(i)->print(os, maxcols);
	}
	rangeHasMoved = false;
	return os;
}

/* Wenn index sichtbar ist, wird 0 zurückgegeben.    *
 * Ist er unter dem Rahmen, wird eine negative Zahl, *
 * ist er über dem Rahmen, wird eine positive        *
 * Zahl zurückgegeben:                               */
int ItemRange::isVisible(int index) const {
	if (index < visibleRange.first) return index - visibleRange.first;
	if (index > visibleRange.second) return index - visibleRange.second;
	return 0;
}

/* Wenn der Index außerhalb von visibleRange ist, *
 * wird -1 zurückgegeben.                         */
int ItemRange::getRangeLine(int index) {
	if (isVisible(index) != 0) return -1;
	return index - visibleRange.first;
}

int ItemRange::getVecIndex(int rangeline) {
	int ret = visibleRange.first + rangeline;
	return (hasIndex(ret)) ? ret : -1;
}

const ItemLine* ItemRange::getItem(int index) const {
	if (! hasIndex(index)) throw OException::IndexOutOfBoundsException(__PRETTY_FUNCTION__, "Ungültiger Index");
	return ilVec.at(index);
}

/* Gibt die RangeLine von index zurück: */
int ItemRange::setVisible(int index) {
	if (! hasIndex(index)) return -1;
	int mv = isVisible(index);
//	MESSAGE.stream() << __PRETTY_FUNCTION__ << ": mv=" << mv << endl;
	if (mv != 0) moveRange(mv);
	return getRangeLine(index);
}

void ItemRange::deleteItems() {
	for (auto it = ilVec.begin(); it != ilVec.end(); it++) delete (*it);
	ilVec.clear();
}













LineExplorer::LineExplorer(int lsz) : linesize(lsz) {
	lineBuf = new wchar_t[lsz + 1];
	lineBuf[lsz] = 0; // Endzeichen setzen!
}


LineExplorer::~LineExplorer() {
	delete [] lineBuf;
}




std::string LineExplorer::readLine(std::wistream& is) {
	if (startNext > 0) {
		is.seekg(startNext);
	} else if (startNext < 0) {
		/* Dieser Fall darf eigentlich nicht eintreten.
		 * Ein fehlerhafter Stream sollte bereits abgefangen
		 * worden sein.                                               */
		startNext = 0;
	}
	lastPos = BEGINPOS;
	bool lineHasWs = false;
	lineStart = is.tellg();
	int currlength = 0;
//	CharUtil::U32CharCounter ucc;
//   int charCounter = 0;
	while (is.good()) {
		char ch = is.get();
		currlength++;
		/* currlength: 		Länge des Strings von lineStart bis inclusive ch.
		 * currlength: 		Die Position des (unbekannten) Zeichens NACH ch.
		 * currlength - 1: 	Position von ch.
		 * currlength - 2: 	Position des Zeichens VOR ch.                      */
//		float u32length = ucc.put(ch); // Zähle Multibyte-Zeichen (z.B. Umlaute)
		bool overload = currlength > linesize;
		if (is.eof()) {
			lineLength = currlength - 1; // EOF-Zeichen ignorieren.
			eoffound = true;
			startNext = 0; // Sollte nicht mehr verwendet werden!
			break;
		} else if (ch == '\n') {
			lineLength = currlength - 1; // '\n'-Zeichen ignorieren
			startNext = is.tellg();
			break;
		} else if (isspace(ch)) {
			lineHasWs = true;
			/* Erstes WS-Zeichen nach einem Token? */
			if (lastPos == TOKENPOS) lineLength = currlength - 1;
			if (overload) {
				/* Bei Overload noch den nächsten Tokenbeginn
				 * definieren: */
				while (is.good()) {
					char c1 = is.get();
					if (is.eof()) {
						startNext = 0; // Sollte nicht mehr verwendet werden!
						eoffound = true;
						break;
					}
					if (! isspace(c1)) {
						startNext = (int)is.tellg() - 1;
						break;
					}
				}
			}
			// Nächsten Durchgang vorbereiten:
			lastPos = WSPOS;
		} else {
			/* Erstes Non-WS-Zeichen nach WS-Sequenz? */
			if (lastPos == WSPOS) startNext = (int)is.tellg() - 1;
			// Nächsten Durchgang vorbereiten:
			lastPos = TOKENPOS;
		}
		if (overload) {
			if ((lineHasWs == false) && (lastPos == TOKENPOS)) {
				/* Zeile enthält nur ein einziges überlanges Token */
				lineLength = currlength - 1;
				startNext = (int)is.tellg() -1;
			}
			break;
		}
	}
	/* Lesen beendet. Jetzt String erzeugen: */
	if (lineLength==0) return string("");
	is.seekg(lineStart);
	is.read(lineBuf,lineLength);
	lineBuf[lineLength] = 0; // Endzeichen setzen
	StringUtil::WC_Converter conv;
	return conv.to_bytes(lineBuf);
}



/*
 *
 *                      T E X T S C R O L L E R :
 *
 * */


TextScroller::TextScroller(Dimension screen) : sstream(new std::wstringstream), panel(screen) {}


TextScroller::~TextScroller() {
	delete sstream;
}


int TextScroller::getMaxScrollpos() const {
	if (panel.height >= (int)lines.size()) return 0;
	return lines.size() - panel.height;
}


void TextScroller::reset() {
//	MESSAGE.println("TextScroller::reset()");
	delete sstream;
	sstream = new std::wstringstream;
	lines.clear();
	scrollpos = 0;
}


void TextScroller::prepareScrolling() {
	lines.clear();
	// Speicherbedarf von lines schätzen:
	int alloc = (int)sstream->tellp() / 10;
	lines.reserve(alloc);
	LineExplorer lpilot(panel.width);
	int i = 0; // Debug!
	while (sstream->good()) {
		string str = lpilot.readLine(*sstream);
//		MESSAGE.println(str);
		if (lpilot.eof() && lpilot.isEmtpy()) break;
		lines.push_back(str);
		if (i++ > 20) break; // Debug!
	}
	delete sstream;
	sstream = nullptr;
}



std::string TextScroller::readLines(int beginLine) {
	string ret;
	ret.reserve(panel.width+3);
	// Schaun, ob die Zeile überhaupt existiert:
	int maxpos = getMaxScrollpos();
	if (beginLine > maxpos) beginLine = maxpos;
	scrollpos = beginLine;
	int vecindex = beginLine;
	int vecsize = lines.size();
	for (int i = 0; i < panel.height; i++) {
		if (vecindex >= vecsize) break;
		if (i > 0) ret.push_back('\n');
		ret.append(lines.at(vecindex++));
	}
	return ret;
}


bool TextScroller::showsFirstLine() const {
	return scrollpos == 0;
}

bool TextScroller::showsLastLine() const {
	return scrollpos == getMaxScrollpos();
}






