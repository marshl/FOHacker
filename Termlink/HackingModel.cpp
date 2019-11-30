#include "HackingModel.h"

#include <list>
#include <sstream>
#include <cassert>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <regex>
#include <sstream>

#include "strfunc.h"
#include "PuzzleWord.h"
#include "DifficultyLevel.h"
#include "PlayerAction.h"

HackingModel::HackingModel()
{
	this->currentDifficulty = nullptr;

	this->InitialiseDifficultyLevels();
}

HackingModel::~HackingModel()
{
	for (int i = 0; i < (int)this->puzzleWords.size(); ++i)
	{
		delete this->puzzleWords[i];
	}

	for (int i = 0; i < (int)this->difficultyLevels.size(); ++i)
	{
		delete this->difficultyLevels[i];
	}

	for (int i = 0; i < (int)this->playerActionList.size(); ++i)
	{
		delete this->playerActionList[i];
	}

	for (int i = 0; i < (int)this->bracketPairs.size(); ++i)
	{
		delete this->bracketPairs[i];
	}
}

int HackingModel::GetAttemptsRemaining() const
{
	return this->attemptsRemaining;
}

int HackingModel::GetPuzzleWordCount() const
{
	return this->puzzleWords.size();
}

PuzzleWord const* HackingModel::GetSolutionWord() const
{
	return this->solutionWord;
}

PuzzleWord const* HackingModel::GetPuzzleWord(int index) const
{
	return this->puzzleWords[index];
}

PuzzleWord const* HackingModel::GetPuzzleWordAtPosition(int columnIndex, int rowIndex, int positionInRow) const
{
	for (int wordIndex = 0; wordIndex < (int)this->puzzleWords.size(); ++wordIndex)
	{
		PuzzleWord* const puzzleWord = this->puzzleWords[wordIndex];

		for (int letterIndex = 0; letterIndex < (int)puzzleWord->GetText().size(); ++letterIndex)
		{
			const ModelCoordinate& letterPos = puzzleWord->GetLetterPosition(letterIndex);

			if (columnIndex == letterPos.column && rowIndex == letterPos.y && positionInRow == letterPos.x)
			{
				return puzzleWord;
			}
		}
	}

	return nullptr;
}

PuzzleWord const* HackingModel::GetPuzzleWordAtCoord(const ModelCoordinate& letterPos) const
{
	return this->GetPuzzleWordAtPosition(letterPos.column, letterPos.y, letterPos.x);
}

BracketPair const* HackingModel::GetBracketPairAtCoord(const ModelCoordinate& letterPos) const
{
	for (int i = 0; i < (int)this->bracketPairs.size(); ++i)
	{
		BracketPair const* bracket = this->bracketPairs[i];
		if (!bracket->IsConsumed() && bracket->GetColumn() == letterPos.column && bracket->GetRow() == letterPos.y && bracket->GetStartingPosition() == letterPos.x)
		{
			return bracket;
		}
	}

	return nullptr;
}

DifficultyLevel* HackingModel::GetCurrentDifficulty() const
{
	return this->currentDifficulty;
}

DifficultyLevel* HackingModel::GetDifficultyLevelWithIndex(int index) const
{
	return this->difficultyLevels[index];
}

int HackingModel::GetDifficultyCount() const
{
	return this->difficultyLevels.size();
}

void HackingModel::SetDifficultyLevel(DifficultyLevel* difficulty)
{
	this->currentDifficulty = difficulty;

	this->SetPuzzleWords();
	this->PlacePuzzleWords();
	this->RandomiseFillerLetters();
	this->SetupBracketPairs();

	this->attemptsRemaining = this->currentDifficulty->GetStartingAttemptCount();
}

int HackingModel::GetColumnWidth() const
{
	return 12;
}

int HackingModel::GetColumnHeight() const
{
	return 17;
}

int HackingModel::GetColumnCount() const
{
	return 2;
}

int HackingModel::GetTotalLineCount() const
{
	return this->GetColumnHeight() * this->GetColumnCount();
}

int HackingModel::GetTotalColumnCharacterCount() const
{
	return this->GetColumnCount() * this->GetColumnWidth() * this->GetColumnHeight();;
}

int HackingModel::GetMaximumWordLength() const
{
	int maximumLength = 0;
	for (int i = 0; i < (int)this->difficultyLevels.size(); ++i)
	{
		if (this->GetDifficultyLevelWithIndex(i)->GetWordLength() > maximumLength)
		{
			maximumLength = this->GetDifficultyLevelWithIndex(i)->GetWordLength();
		}
	}

	return maximumLength;
}

const std::string& HackingModel::GetFillerText(int columnIndex, int rowIndex) const
{
	return this->fillerCharacters[columnIndex][rowIndex];
}

int HackingModel::GetBracketPairCount() const
{
	return this->bracketPairs.size();
}

BracketPair const* HackingModel::GetBracketPair(int matchingBracketIndex) const
{
	assert(matchingBracketIndex >= 0 && matchingBracketIndex < this->GetBracketPairCount());
	return this->bracketPairs[matchingBracketIndex];
}

bool HackingModel::AttemptWord(PuzzleWord const* puzzleWord)
{
	assert(this->solutionWord != nullptr);
	assert(std::find(this->puzzleWords.begin(), this->puzzleWords.end(), puzzleWord) != this->puzzleWords.end());

	// Find the internal version of that word so it can be modified
	PuzzleWord* localWord = *std::find(this->puzzleWords.begin(), this->puzzleWords.end(), puzzleWord);

	if (localWord == this->solutionWord)
	{
		this->playerActionList.push_back(new SuccessfulAttemptAction(puzzleWord));
		return true;
	}
	else
	{
		localWord->SetIsAttempted(true);
		this->playerActionList.push_back(new FailedAttemptAction(puzzleWord, this->attemptsRemaining, this->GetCurrentDifficulty()->GetStartingAttemptCount()));
		--this->attemptsRemaining;
		return false;
	}
}

void HackingModel::AttemptBracketPair(BracketPair const* bracketPair)
{
	assert(!bracketPair->IsConsumed());
	assert(bracketPair != nullptr);
	assert(std::find(this->bracketPairs.begin(), this->bracketPairs.end(), bracketPair) != this->bracketPairs.end());

	BracketPair* localBracket = *std::find(this->bracketPairs.begin(), this->bracketPairs.end(), bracketPair);

	float randVal = (float)rand() / (float)RAND_MAX;

	std::string fillerText = this->GetFillerText(localBracket->GetColumn(), localBracket->GetRow());
	std::string bracketText = fillerText.substr(localBracket->GetStartingPosition(), localBracket->GetEndingPosition() - localBracket->GetStartingPosition() + 1);

	std::vector<PuzzleWord*> possibleDuds;
	for (int i = 0; i < (int)this->puzzleWords.size(); ++i)
	{
		PuzzleWord* puzzleWord = this->puzzleWords[i];
		if (puzzleWord != this->solutionWord && !puzzleWord->IsRemoved())
		{
			possibleDuds.push_back(puzzleWord);
		}
	}

	if (possibleDuds.size() == 0 || randVal < 0.25f)
	{
		this->attemptsRemaining = this->GetCurrentDifficulty()->GetStartingAttemptCount();
		this->playerActionList.push_back(new ReplenishBracketAction(bracketText));
	}
	else
	{
		std::random_shuffle(possibleDuds.begin(), possibleDuds.end());
		possibleDuds.front()->Remove();

		this->playerActionList.push_back(new DudBracketAction(bracketText));
	}

	localBracket->Consume();
}

PlayerAction const* HackingModel::GetPlayerAction(int index) const
{
	assert(index >= 0 && index < (int)this->playerActionList.size());
	return this->playerActionList[index];
}

int HackingModel::GetPlayerActionCount() const
{
	return this->playerActionList.size();
}

void HackingModel::SetPuzzleWords()
{
	std::vector<std::string> allWords;
	std::vector<std::string> solutionWords;
	std::vector<std::string> triedWords;

	this->GetSampleWordList(allWords);

	assert((int)allWords.size() >= this->GetCurrentDifficulty()->GetWordCount());

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
			for each (const std::string & solutionWord in solutionWords)
			{
				int difference = StringDiff(solutionWord, randWord);
				if (difference > this->GetCurrentDifficulty()->GetLetterDifference() || difference == 0)
				{
					continue;
				}

				solutionWords.push_back(randWord);
				wordAdded = true;

				if (solutionWords.size() == this->GetCurrentDifficulty()->GetWordCount())
				{
					std::random_shuffle(solutionWords.begin(), solutionWords.end());

					std::string solution = solutionWords.front();

					for each (std::string word in solutionWords)
					{
						this->puzzleWords.push_back(new PuzzleWord(word, solution));
					}

					this->solutionWord = this->puzzleWords.front();

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
	assert(this->GetTotalColumnCharacterCount() > this->GetCurrentDifficulty()->GetWordLength());

	for (unsigned int puzzleWordIndex = 0; puzzleWordIndex < puzzleWords.size(); )
	{
		PuzzleWord* puzzleWord = this->puzzleWords[puzzleWordIndex];

		// Theoretically, this might go infinite with the right conditions
		int place = rand() % (this->GetTotalColumnCharacterCount() - this->GetCurrentDifficulty()->GetWordLength());
		bool badPlacement = false;
		// Check for position collisions with words that have already been placed
		for (unsigned int placedWordIndex = 0; placedWordIndex < puzzleWordIndex; ++placedWordIndex)
		{
			if (std::abs(puzzleWords[placedWordIndex]->GetPosition() - place) < this->GetCurrentDifficulty()->GetWordLength() + 1)
			{
				badPlacement = true;
				break;
			}
		}

		if (!badPlacement)
		{
			puzzleWord->SetPosition(place, this->GetColumnCount(), this->GetColumnHeight(), this->GetColumnWidth());
			++puzzleWordIndex;
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
		if (word.length() == this->GetCurrentDifficulty()->GetWordLength())
		{
			std::transform(word.begin(), word.end(), word.begin(), ::toupper);
			_out.push_back(word);
		}
	}

	fin.close();
}

void HackingModel::InitialiseDifficultyLevels()
{
	this->difficultyLevels.push_back(new DifficultyLevel(0, "Very Easy", 9, 5, 2, 4));
	this->difficultyLevels.push_back(new DifficultyLevel(1, "Easy", 9, 7, 3, 4));
	this->difficultyLevels.push_back(new DifficultyLevel(2, "Average", 8, 9, 3, 4));
	this->difficultyLevels.push_back(new DifficultyLevel(3, "Hard", 7, 11, 4, 4));
	this->difficultyLevels.push_back(new DifficultyLevel(4, "Very Hard", 7, 13, 4, 4));
}

void HackingModel::RandomiseFillerLetters()
{
	const std::string FILLER_CHARACTERS = "$-/\\?|=+&^%!@#&*{}[]()<>,.'\"";
	const int FILLER_CHARACTER_COUNT = FILLER_CHARACTERS.length();

	this->fillerCharacters.resize(this->GetColumnCount());

	for (int columnIndex = 0; columnIndex < this->GetColumnCount(); ++columnIndex)
	{
		this->fillerCharacters[columnIndex].resize(this->GetColumnHeight());
		for (int rowIndex = 0; rowIndex < this->GetColumnHeight(); ++rowIndex)
		{
			std::string filler(this->GetColumnWidth(), '#');
			for (int fillerIndex = 0; fillerIndex < this->GetColumnWidth(); ++fillerIndex)
			{
				filler[fillerIndex] = FILLER_CHARACTERS[rand() % FILLER_CHARACTER_COUNT];
			}

			this->fillerCharacters[columnIndex][rowIndex] = filler;
		}
	}
}

void HackingModel::SetupBracketPairs()
{
	for (int columnIndex = 0; columnIndex < this->GetColumnCount(); ++columnIndex)
	{
		for (int rowIndex = 0; rowIndex < this->GetColumnHeight(); ++rowIndex)
		{
			std::vector<BracketPair*> brackets = this->GetBracketPairsOnLine(columnIndex, rowIndex);
			this->bracketPairs.insert(this->bracketPairs.end(), brackets.begin(), brackets.end());
		}
	}

#if _DEBUG
	for (int i = 0; i < (int)this->bracketPairs.size(); ++i)
	{
		for (int j = i + 1; j < (int)this->bracketPairs.size(); ++j)
		{
			BracketPair const* a = this->bracketPairs[i];
			BracketPair const* b = this->bracketPairs[j];
			if (a->GetColumn() == b->GetColumn() && a->GetRow() == b->GetRow() && a->GetStartingPosition() == b->GetStartingPosition())
			{
				assert(false);
			}
		}
	}
#endif

	for (int i = 0; i < (int)this->bracketPairs.size(); )
	{
		BracketPair const* bracket = this->bracketPairs[i];
		if (this->GetPuzzleWordAtPosition(bracket->GetColumn(), bracket->GetRow(), bracket->GetStartingPosition()) != nullptr
			|| this->GetPuzzleWordAtPosition(bracket->GetColumn(), bracket->GetRow(), bracket->GetEndingPosition()) != nullptr)
		{
			delete this->bracketPairs[i];
			this->bracketPairs.erase(this->bracketPairs.begin() + i);
		}
		else
		{
			++i;
		}
	}
}

std::vector<BracketPair*> HackingModel::GetBracketPairsOnLine(int columnIndex, int rowIndex)
{
	std::vector<BracketPair*> result;

	const std::string validStartingChars = "<{[(";
	const std::string validEndingChars = ">}])";

	assert(validStartingChars.size() == validEndingChars.size());

	std::string fillerText = this->GetFillerText(columnIndex, rowIndex);

	for (int charIndex = 0; charIndex < (int)validStartingChars.size(); ++charIndex)
	{
		size_t openingOffset = 0;
		while (openingOffset < fillerText.size() && (openingOffset = fillerText.find(validStartingChars[charIndex], openingOffset)) != std::string::npos)
		{
			size_t closingOffset;
			if ((closingOffset = fillerText.find(validEndingChars[charIndex], openingOffset + 1)) != std::string::npos)
			{
				char openingSymbol = fillerText[openingOffset];
				char closingSymbol = fillerText[closingOffset];

				BracketPair* match = new BracketPair(openingSymbol, closingSymbol, columnIndex, rowIndex, openingOffset, closingOffset);
				result.push_back(match);
			}

			openingOffset += 1;
		}
	}

	return result;
}