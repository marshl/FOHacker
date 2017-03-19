#include "HackingModel.h"

#include <list>
#include <sstream>
#include <cassert>
#include <fstream>
#include <iostream>

#include "constants.h"
#include "strfunc.h"
#include "PuzzleWord.h"

HackingModel::HackingModel()
{
	this->attemptsRemaining = STARTING_ATTEMPT_COUNT;

	this->hexAddresses.resize(TOTAL_COLUMN_LINE_COUNT, std::string(HEX_CODE_LENGTH, '#'));

	this->currentHighlightedPuzzleWord = nullptr;

	this->SetPuzzleWords();
	this->PlacePuzzleWords();
	this->SetHexAddresses();
}

HackingModel::~HackingModel()
{
	for (unsigned int i = 0; i < this->puzzleWords.size(); ++i)
	{
		delete this->puzzleWords[i];
	}
}

int HackingModel::GetAttemptsRemaining() const
{
	return this->attemptsRemaining;
}

const std::string & HackingModel::GetHexAddress(int index) const
{
	return this->hexAddresses[index];
}

const unsigned int HackingModel::GetPuzzleWordCount() const
{
	return this->puzzleWords.size();
}

const unsigned int HackingModel::GetAttemptedWordCount() const
{
	return this->attemptedWords.size();
}

PuzzleWord * const HackingModel::GetPuzzleWord(int index) const
{
	return this->puzzleWords[index];
}

PuzzleWord * const HackingModel::GetSelectedPuzzleWord() const
{
	return this->currentHighlightedPuzzleWord;
}

void HackingModel::OnClickEvent()
{
	if (this->currentHighlightedPuzzleWord != nullptr)
	{
		int diff = StringDiff(this->currentHighlightedPuzzleWord->GetText(), this->solutionWord);

		if (diff == 0)
		{

		}
		else
		{
			this->attemptedWords.push_back(this->currentHighlightedPuzzleWord->GetText());
			--this->attemptsRemaining;
		}
	}
}

void HackingModel::OnMouseMoveEvent(COORD cursorCoord)
{
	this->currentHighlightedPuzzleWord = nullptr;

	for (unsigned int i = 0; i < puzzleWords.size(); ++i)
	{
		this->puzzleWords[i]->SetHighlight(false);
	}

	for (unsigned int i = 0; i < this->puzzleWords.size() && this->currentHighlightedPuzzleWord == nullptr; ++i)
	{
		PuzzleWord* word = this->puzzleWords[i];

		for (int j = 0; j < PUZZLE_WORD_LENGTH; ++j)
		{
			if (word->GetScreenCoord(j).X == cursorCoord.X
				&& word->GetScreenCoord(j).Y == cursorCoord.Y)
			{
				word->SetHighlight(true);
				this->currentHighlightedPuzzleWord = word;
				break;
			}
		}
	}
}


void HackingModel::SetHexAddresses()
{
	int address = rand() % 0xF000 + 0xFFF;
	for (int i = 0; i < TOTAL_COLUMN_LINE_COUNT; ++i)
	{
		address += sizeof(char) * TOTAL_COLUMN_WIDTH;
		std::ostringstream stream;
		stream << "0x" << std::hex << address;
		hexAddresses[i] = stream.str();
	}
}

void HackingModel::SetPuzzleWords()
{
	std::list<std::string> words;
	std::vector<std::string> triedWords;
	std::vector<std::string> goodWords;

	GetSampleWordList(triedWords);
	assert(triedWords.size() > WORD_COUNT);

	std::list<std::string>::iterator iter = words.begin();

	while (true)
	{
		if (words.size() == 0)
		{ // Ran out of usable words, reset and try again
			words.insert(words.end(), triedWords.begin(), triedWords.end());
			triedWords.clear();
			puzzleWords.clear();
			goodWords.clear();

			iter = words.begin();
			std::advance(iter, rand() % words.size());
			solutionWord = *iter;
		}

		// Get a random word
		iter = words.begin();
		std::advance(iter, rand() % words.size());
		std::string randWord = *iter;

		int diff = StringDiff(randWord, solutionWord);
		if (diff <= PUZZLE_DIFFERENCE)
		{ // If within tolerances, add to the list
			goodWords.push_back(randWord);
			if (goodWords.size() >= WORD_COUNT - 1)
			{// We only need N-1 because the last word is the solution word
				break;
			}
		}
		// Add word to used words and remove from searchable list
		triedWords.push_back(randWord);
		words.erase(iter);
	}

	puzzleWords.push_back(new PuzzleWord(solutionWord));
	for (unsigned int i = 0; i < WORD_COUNT - 1; ++i)
	{
		puzzleWords.push_back(new PuzzleWord(goodWords[i]));
	}
}

void HackingModel::PlacePuzzleWords()
{
	assert(TOTAL_COLUMNS_CHARACTER_COUNT > PUZZLE_WORD_LENGTH);

	for (unsigned int i = 0; i < puzzleWords.size(); )
	{
		// Theoretically, this might go infinite with the right conditions
		int place = rand() % (TOTAL_COLUMNS_CHARACTER_COUNT - PUZZLE_WORD_LENGTH);
		bool badPlacement = false;
		for (unsigned int j = 0; j < i; ++j)
		{
			if (i != j
				&& puzzleWords[j]->GetPosition() != -1
				&& std::abs(puzzleWords[j]->GetPosition() - place) < PUZZLE_WORD_LENGTH + 1)
			{
				badPlacement = true;
				break;
			}
		}

		if (!badPlacement)
		{
			puzzleWords[i]->SetPosition(place);
			++i;
		}
	}

	for (unsigned int i = 0; i < puzzleWords.size(); ++i)
	{
		int place = puzzleWords[i]->GetPosition();

		int x = place % COLUMN_CHARACTER_WIDTH;
		int y = (place - x) / COLUMN_CHARACTER_WIDTH;

		int column = 0;

		while (y >= COLUMN_HEIGHT)
		{
			++column;
			y -= COLUMN_HEIGHT;
		}

		// If the std::string goes over the end of a line
		if (x + PUZZLE_WORD_LENGTH > COLUMN_CHARACTER_WIDTH)
		{
			// Separate it into two
			std::string chunk1 = puzzleWords[i]->GetText().substr(0, COLUMN_CHARACTER_WIDTH - x + 1);
			std::string chunk2 = puzzleWords[i]->GetText().substr(COLUMN_CHARACTER_WIDTH - x + 1);

			for (int j = 0; j < COLUMN_CHARACTER_WIDTH - x + 1; ++j)
			{
				COORD& coord = puzzleWords[i]->GetScreenCoord(j);

				coord.X = x + HEX_CODE_LENGTH + column * TOTAL_COLUMN_WIDTH + j;
				coord.Y = y + LINES_BEFORE_COLUMNS;
			}

			// Render one half
			++y;
			if (y >= COLUMN_HEIGHT)
			{
				++column;
				y -= COLUMN_HEIGHT;
			}

			int k = 0;
			for (int j = COLUMN_CHARACTER_WIDTH - x + 1; j < PUZZLE_WORD_LENGTH; ++j)
			{
				COORD& coord = puzzleWords[i]->GetScreenCoord(j);
				coord.X = HEX_CODE_LENGTH + 1 + column * TOTAL_COLUMN_WIDTH + k;
				coord.Y = y + LINES_BEFORE_COLUMNS;
				++k;
			}
		}
		else
		{
			for (int j = 0; j < PUZZLE_WORD_LENGTH; ++j)
			{
				COORD& coord = puzzleWords[i]->GetScreenCoord(j);
				coord.X = x + 1 + HEX_CODE_LENGTH + column * TOTAL_COLUMN_WIDTH + j;
				coord.Y = y + LINES_BEFORE_COLUMNS;
			}
		}
	}
}

void HackingModel::GetSampleWordList(std::vector<std::string>& _out)
{
	std::ifstream fin("dictionary");
	if (!fin.is_open())
	{
		std::cout << "Error opening dictionary\n";
		exit(1);
	}

	std::string word;
	while (fin >> word)
	{
		if (word.length() == PUZZLE_WORD_LENGTH)
		{
			_out.push_back(word);
		}
	}
	fin.close();
}
