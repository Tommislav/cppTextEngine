#include <iostream>
#include <string>
#include "textengine.h"

using namespace std;

bool writerSkip = false;


struct Writer {
	string text;
	int pos;
	int len;
	timer tim;
	int startX;
	int startY;
	int x;
	int y;
	int w;
	int h;
	int layer;
	Color col;
	Writer() : pos(0), len(0), startX(0), startY(0), x(0), y(0), w(0), h(0), layer(0), col(Color::def) {}
};


bool wrAtEnd(Writer &writer) {
	return writer.pos == writer.len;
}

void wrParseCommand(Writer &writer, char command, char instr) {
	if (command == 'n') { // newline
		writer.x = writer.startX;
		writer.y ++;
		}
	if (command == 'p') { // pause
		if (instr == '0') { writer.tim.pause = 100; }
		else if (instr == '1') { writer.tim.pause = 500; }
		else if (instr == '2') { writer.tim.pause = 1000; }
		else if (instr == '3') { writer.tim.pause = 1500; }
	}
	if (command == 's') { // speed 
		if (instr == '0') { writer.tim.wait = 5; }
		else if (instr == '1') { writer.tim.wait = 30; } 
		else if (instr == '2') { writer.tim.wait = 50; } 
		else if (instr == '3') { writer.tim.wait = 150; } 
		else if (instr == '9') { writer.tim.wait = -1; }
	}
	if (command == 'c') { // color
		if (instr == 'R') { writer.col = Color::red; }
		else if (instr == 'G') { writer.col = Color::green; }
		else if (instr == 'B') { writer.col = Color::blue; }
		else if (instr == 'W') { writer.col = Color::white; }
		else if (instr == 'P') { writer.col = Color::purple; }
		else if (instr == 'D') { writer.col = Color::def; }
	}
	if (command == '>') { // option marker
		printAt(instr, writer.x, writer.y, Color::purple);
		writer.x ++;
	}
}



bool wrPutChar(Writer &writer, int millisec) {
	if (wrAtEnd(writer)) {return false;}
	if (millisec > -1) {
		if (!writer.tim.countDown(millisec, writerSkip)) {
			return false;
		}
	}

	char c = writer.text[writer.pos];
	if (c == '^') {
		wrParseCommand(writer, 'n', ' ');
		writer.pos++;
		return wrPutChar(writer, millisec);
	}
	else if (c == '#') {
		wrParseCommand(writer, writer.text[writer.pos+1], writer.text[writer.pos+2]);
		writer.pos += 3;
		return wrPutChar(writer, millisec);
	}

	printAt(c, writer.x, writer.y, writer.col);
	writer.pos ++;
	writer.x ++;

	if (writer.tim.wait == -1) { wrPutChar(writer, 0); }
	return true;
}

void wrResize(Writer &writer) {
	termSize size = getTermSize();
	size.width = 80;
	writer.w = size.width;
	writer.h = size.height;
	int lastSpace = 0;
	int lastSpaceX = writer.startX;
	int x = writer.startX;
	for (int i=0; i<writer.len; i++) {
		char c = writer.text[i];

		if (c == '#') {
			if (writer.text[i+1] == 'n') { x = 0; }
			i+=2;
			continue;
		}

		if (c == '^') { writer.text[i] = ' '; }
		if (c == ' ') { 
			lastSpace = i;
			lastSpaceX = x;
		 }
		
		x++;
		if (x >= size.width) {
			writer.text[lastSpace] = '^';
			x -= lastSpaceX;
		}
	}
	// re-write all resized text
	int oldPos = writer.pos;
	writer.pos = 0;
	for (int i=0; i<oldPos; i++) {
		wrPutChar(writer, -1);
	}
}

void wrSetText(Writer &writer, string text, int startX, int startY) {
	writer.text = text;
	writer.pos = 0;
	writer.len = text.size();
	writer.startX = writer.x = startX;
	writer.startY = writer.y = startY;
	writer.tim.wait = 30;
	wrResize(writer);
}

