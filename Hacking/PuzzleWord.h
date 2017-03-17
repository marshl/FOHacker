#ifndef PUZZLE_WORD_H_
#define PUZZLE_WORD_H_

#include <Windows.h>
#include <string>

#include "constants.h"

class PuzzleWord
{
public:
	PuzzleWord(std::string _text) : text(_text), isHighlighted(false), isRemoved(false), position(-1) { }

	const std::string& GetText() const;

	bool IsHighlighted() const;

	void SetHighlight(bool isHighlighted);

	const int GetPosition() const;

	void SetPosition(int position);

	const COORD& GetScreenCoord(int index) const;
	COORD& GetScreenCoord(int index);

private:
	std::string text;
	bool isHighlighted;
	bool isRemoved;
	int position;
	COORD screenCoords[PUZZLE_WORD_LENGTH];
};

#endif
