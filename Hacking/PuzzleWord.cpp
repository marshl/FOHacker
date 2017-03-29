#include "PuzzleWord.h"

#include <cassert>

const std::string & PuzzleWord::GetText() const
{
	return this->text;
}

bool PuzzleWord::IsHighlighted() const
{
	return this->isHighlighted;
}

void PuzzleWord::SetHighlight(bool isHighlighted)
{
	this->isHighlighted = isHighlighted;
}

const int PuzzleWord::GetPosition() const
{
	return this->position;
}

void PuzzleWord::SetPosition(int position)
{
	this->position = position;
}

const COORD & PuzzleWord::GetScreenCoord(unsigned int index) const
{
	assert(index >= 0 && index < this->text.size());
	return this->screenCoords[index];
}

COORD & PuzzleWord::GetScreenCoord(unsigned int index)
{
	assert(index >= 0 && index < this->text.size());
	return this->screenCoords[index];
}
