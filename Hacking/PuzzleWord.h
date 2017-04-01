#ifndef PUZZLE_WORD_H_
#define PUZZLE_WORD_H_

#include <Windows.h>
#include <string>
#include <boost/container/vector.hpp>

class PuzzleWord
{
public:
	PuzzleWord(std::string _text) : text(_text), isHighlighted(false), isRemoved(false), position(-1)
	{
		this->screenCoords.resize(_text.length(), { 0, 0 });
	}

	const std::string& GetText() const;

	bool IsHighlighted() const;

	void SetHighlight(bool isHighlighted);

	const int GetPosition() const;

	void SetPosition(int position);

	const COORD& GetScreenCoord(unsigned int index) const;
	COORD& GetScreenCoord(unsigned int index);

private:
	std::string text;
	bool isHighlighted;
	bool isRemoved;
	int position;
	boost::container::vector<COORD> screenCoords;
};

#endif
