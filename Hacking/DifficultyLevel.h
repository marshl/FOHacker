#ifndef DIFFICULTY_LEVE_H_
#define DIFFICULTY_LEVEL_H_

#include <string>

class DifficultyLevel
{
public:
	DifficultyLevel(int rank, std::string name, int wordCount, int wordLength, int letterDifference);
	~DifficultyLevel();

	int GetRank() const;
	const std::string& GetName() const;
	int GetWordCount() const;
	int GetWordLength() const;
	int GetLetterDifference() const;

private:

	int rank;
	std::string name;
	int wordCount;
	int wordLength;
	int letterDifference;
};

#endif