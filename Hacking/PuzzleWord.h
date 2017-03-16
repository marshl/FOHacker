#ifndef PUZZLE_WORD_H_
#define PUZZLE_WORD_H_

#include <Windows.h>
#include <string>

#include "constants.h"

class PuzzleWord
{
public:
	PuzzleWord(std::string _text) : text(_text), isHighlighted(false), isRemoved(false), position(-1) { }
	std::string text;
	bool isHighlighted;
	bool isRemoved;
	int position;
	COORD screenCoords[PUZZLE_WORD_LENGTH];
};

#endif
