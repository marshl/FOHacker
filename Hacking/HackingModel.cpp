#include "HackingModel.h"

#include <list>
#include <sstream>
#include <cassert>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "constants.h"
#include "strfunc.h"
#include "PuzzleWord.h"
#include "DifficultyLevel.h"

HackingModel::HackingModel()
{
	this->attemptsRemaining = STARTING_ATTEMPT_COUNT;


	this->currentHighlightedPuzzleWord = nullptr;
	this->currentDifficulty = nullptr;

	this->SetPuzzleWords();
	this->PlacePuzzleWords();
	this->SetHexAddresses();

	this->InitialiseDifficultyLevels();

	for (int i = 0; i < TOTAL_COLUMN_LINE_COUNT; ++i)
	{
		std::string filler('#', COLUMN_CHARACTER_WIDTH);
		for (int j = 0; j < COLUMN_CHARACTER_WIDTH; ++j)
		{
			filler[i] = FILLER_CHARACTERS[rand() % FILLER_CHARACTER_COUNT];
		}

		this->fillerCharacters.push_back(filler);
	}
}

HackingModel::~HackingModel()
{
	for (unsigned int i = 0; i < this->puzzleWords.size(); ++i)
	{
		delete this->puzzleWords[i];
	}

	for (unsigned int i = 0; i < this->difficultyLevels.size(); ++i)
	{
		delete this->difficultyLevels[i];
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

void HackingModel::OnClickEvent(const COORD& cursorCoord)
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

void HackingModel::OnMouseMoveEvent(const COORD& cursorCoord)
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

const DifficultyLevel * const HackingModel::GetCurrentDifficulty() const
{
	return this->currentDifficulty;
}

const DifficultyLevel * const HackingModel::GetDifficultyLevelWithIndex(int index) const
{
	return this->difficultyLevels[index];
}

void HackingModel::SetDifficultyLevel(DifficultyLevel* difficultyLevel)
{
	this->currentDifficulty = difficultyLevel;
}


void HackingModel::SetHexAddresses()
{
	this->hexAddresses.resize(TOTAL_COLUMN_LINE_COUNT, std::string(HEX_CODE_LENGTH, '#'));

	int address = rand() % 0xF000 + 0xFFF;
	for (int i = 0; i < TOTAL_COLUMN_LINE_COUNT; ++i)
	{
		address += sizeof(char) * TOTAL_COLUMN_WIDTH;
		std::ostringstream stream;
		stream << "0x" << std::hex << address;
		hexAddresses[i] = stream.str();
		std::transform(hexAddresses[i].begin() + 2, hexAddresses[i].end(), hexAddresses[i].begin() + 2, ::toupper);
	}
}

void HackingModel::SetPuzzleWords()
{
	std::vector<std::string> allWords;
	std::vector<std::string> solutionWords;
	std::vector<std::string> triedWords;

	this->GetSampleWordList(allWords);

	assert(allWords.size() >= WORD_COUNT);

	std::random_shuffle(allWords.begin(), allWords.end());

	while (true)
	{
		std::string randWord = allWords.back();
		bool wordAdded = false;

		// Add the word if there are no others
		if (solutionWords.size() == 0)
		{
			solutionWords.push_back(randWord);
			wordAdded = true;
		}
		else
		{
			for each(const std::string& solutionWord in solutionWords)
			{
				int difference = StringDiff(solutionWord, randWord);
				if (difference > PUZZLE_DIFFERENCE || difference == 0)
				{
					continue;
				}

				solutionWords.push_back(randWord);
				wordAdded = true;

				if (solutionWords.size() == WORD_COUNT)
				{
					for each(std::string word in solutionWords)
					{
						this->puzzleWords.push_back(new PuzzleWord(word));
					}
					return;
				}

				// More words are needed, conitnue the loop
				break;
			}
		}

		triedWords.push_back(randWord);
		allWords.pop_back();

		if (allWords.size() == 0)
		{
			allWords.insert(allWords.begin(), triedWords.begin(), triedWords.end());
			triedWords.clear();
			solutionWords.clear();
			std::random_shuffle(allWords.begin(), allWords.end());
		}
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
		// Check for position collisions with words that have already been placed
		for (unsigned int j = 0; j < i; ++j)
		{
			if (std::abs(puzzleWords[j]->GetPosition() - place) < PUZZLE_WORD_LENGTH + 1)
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
			std::transform(word.begin(), word.end(), word.begin(), ::toupper);
			_out.push_back(word);
		}
	}
	fin.close();
}

void HackingModel::InitialiseDifficultyLevels()
{
	this->difficultyLevels.push_back(new DifficultyLevel(0, "Very Easy", 9, 5, 2));
	this->difficultyLevels.push_back(new DifficultyLevel(1, "Easy", 9, 7, 3));
	this->difficultyLevels.push_back(new DifficultyLevel(2, "Average", 8, 9, 3));
	this->difficultyLevels.push_back(new DifficultyLevel(3, "Hard", 7, 11, 4));
	this->difficultyLevels.push_back(new DifficultyLevel(4, "Very Hard", 7, 13, 4));
}