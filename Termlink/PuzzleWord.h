#ifndef PUZZLE_WORD_H_
#define PUZZLE_WORD_H_

#include <string>
#include <vector>
#include "strfunc.h"

struct ModelCoordinate
{
	int column;
	int x;
	int y;
};

class PuzzleWord
{
public:
	PuzzleWord(std::string text, std::string solution);

	const std::string& GetText() const;

	const int GetPosition() const;

	void SetPosition(int position, int columnCount, int columnHeight, int columnWidth);

	const ModelCoordinate& GetLetterPosition(int index) const;

	ModelCoordinate& GetLetterPosition(int index);

	bool GetIsAttempted() const;

	void SetIsAttempted(bool attempted);

	int GetDifferenceFromSolution() const;

	bool IsRemoved() const;

	void Remove();

private:
	std::string text;
	bool isAttempted;
	int position;
	int differenceFromSolution;
	bool isRemoved;

	std::vector<ModelCoordinate> letterPositions;
};

#endif
