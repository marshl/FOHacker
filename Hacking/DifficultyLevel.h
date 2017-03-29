#ifndef DIFFICULTY_LEVE_H_
#define DIFFICULTY_LEVEL_H_

#include <string>

class DifficultyLevel
{
public:
	DifficultyLevel(int rank, std::string name, int wordCount, int wordLength, int letterDifference, int startingAttemptCount);
	~DifficultyLevel();

	int GetRank() const;
	const std::string& GetName() const;
	int GetWordCount() const;
	int GetWordLength() const;
	int GetLetterDifference() const;
	int GetStartingAttemptCount() const;

private:

	int rank;
	std::string name;
	int wordCount;
	int wordLength;
	int letterDifference;
	int startingAttemptCount;
};

#endif