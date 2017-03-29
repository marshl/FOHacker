#include "DifficultyLevel.h"

#include <cassert>

#include "constants.h"

DifficultyLevel::DifficultyLevel(int rank, std::string name, int wordCount, int wordLength, int letterDifference, int startingAttemptCount)
{
	this->rank = rank;
	this->name = name;

	assert(wordCount > 0);
	this->wordCount = wordCount;

	assert(wordLength > 0);
	assert(wordLength < LONGEST_PUZZLE_WORD);
	this->wordLength = wordLength;

	assert(letterDifference > 0 && letterDifference <= wordLength);
	this->letterDifference = letterDifference;

	assert(startingAttemptCount > 0);
	assert(startingAttemptCount < wordCount);
	this->startingAttemptCount = startingAttemptCount;
}

DifficultyLevel::~DifficultyLevel()
{
}

int DifficultyLevel::GetRank() const
{
	return this->rank;
}

const std::string & DifficultyLevel::GetName() const
{
	return this->name;
}

int DifficultyLevel::GetWordCount() const
{
	return this->wordCount;
}

int DifficultyLevel::GetWordLength() const
{
	return this->wordLength;
}

int DifficultyLevel::GetLetterDifference() const
{
	return this->letterDifference;
}

int DifficultyLevel::GetStartingAttemptCount() const
{
	return this->startingAttemptCount;
}
