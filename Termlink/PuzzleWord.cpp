#include "PuzzleWord.h"

#include <cassert>

PuzzleWord::PuzzleWord(std::string text, std::string solution) : text(text), isAttempted(false), position(-1), differenceFromSolution(0), isRemoved(false)
{
	this->letterPositions.resize(text.size(), { 0, 0 });

	this->differenceFromSolution = StringDiff(text, solution);
}

const std::string& PuzzleWord::GetText() const
{
	return this->text;
}

const int PuzzleWord::GetPosition() const
{
	return this->position;
}

void PuzzleWord::SetPosition(int position, int columnCount, int columnHeight, int columnWidth)
{
	this->position = position;

	for (int i = 0; i < (int)this->letterPositions.size(); ++i)
	{
		ModelCoordinate& letterPos = this->letterPositions[i];

		letterPos.column = (position + i) / (columnHeight * columnWidth);
		letterPos.x = ((position + i) % (columnHeight * columnWidth)) % columnWidth;
		assert(letterPos.x >= 0 && letterPos.x < columnWidth);
		letterPos.y = ((position + i) % (columnHeight * columnWidth)) / columnWidth;
		assert(letterPos.y >= 0 && letterPos.y < columnHeight);
	}
}

const ModelCoordinate& PuzzleWord::GetLetterPosition(int index) const
{
	assert(index >= 0 && index < (int)this->text.size());
	return this->letterPositions[index];
}

ModelCoordinate& PuzzleWord::GetLetterPosition(int index)
{
	assert(index >= 0 && index < (int)this->text.size());
	return this->letterPositions[index];
}

bool PuzzleWord::GetIsAttempted() const
{
	return this->isAttempted;
}

void PuzzleWord::SetIsAttempted(bool attempted)
{
	this->isAttempted = attempted;
}

int PuzzleWord::GetDifferenceFromSolution() const
{
	return this->differenceFromSolution;
}

bool PuzzleWord::IsRemoved() const
{
	return this->isRemoved;
}

void PuzzleWord::Remove()
{
	assert(this->differenceFromSolution != 0);
	this->isRemoved = true;
}
