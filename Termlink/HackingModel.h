#ifndef HACKING_MODEL_H_
#define HACKING_MODEL_H_

#include <vector>
#include <string>
#include <Windows.h>
#include "PuzzleWord.h"
#include "BracketPair.h"

class DifficultyLevel;
class PlayerAction;

class HackingModel
{
public:
	HackingModel();
	~HackingModel();

	int GetAttemptsRemaining() const;

	int GetPuzzleWordCount() const;

	PuzzleWord const* GetSolutionWord() const;

	PuzzleWord const* GetPuzzleWord(int index) const;

	PuzzleWord const* GetPuzzleWordAtPosition(int columnIndex, int rowIndex, int positionInRow) const;

	PuzzleWord const* GetPuzzleWordAtCoord(const ModelCoordinate& letterPos) const;

	BracketPair const* GetBracketPairAtCoord(const ModelCoordinate& letterPos) const;

	DifficultyLevel* GetCurrentDifficulty() const;

	DifficultyLevel* GetDifficultyLevelWithIndex(int index) const;
	int GetDifficultyCount() const;

	void SetDifficultyLevel(DifficultyLevel* difficulty);

	int GetColumnWidth() const;
	int GetColumnHeight() const;
	int GetColumnCount() const;
	int GetTotalLineCount() const;
	int GetTotalColumnCharacterCount() const;

	int GetMaximumWordLength() const;

	const std::string& GetFillerText(int columnIndex, int rowIndex) const;

	int GetBracketPairCount() const;
	BracketPair const* GetBracketPair(int matchingBracketIndex) const;

	bool AttemptWord(PuzzleWord const* puzzleWord);
	void AttemptBracketPair(BracketPair const* bracketPair);

	PlayerAction const* GetPlayerAction(int index) const;
	int GetPlayerActionCount() const;

private:
	int attemptsRemaining;

	std::vector<PuzzleWord* > puzzleWords;
	PuzzleWord* solutionWord;
	std::vector<DifficultyLevel*> difficultyLevels;
	std::vector<std::vector<std::string>> fillerCharacters;
	std::vector<BracketPair*> bracketPairs;
	std::vector<PlayerAction*> playerActionList;

	DifficultyLevel* currentDifficulty;

	void SetPuzzleWords();
	void PlacePuzzleWords();
	void GetSampleWordList(std::vector<std::string>& _out);
	void InitialiseDifficultyLevels();
	void RandomiseFillerLetters();

	void SetupBracketPairs();

	std::vector<BracketPair*> GetBracketPairsOnLine(int columnIndex, int rowIndex);
};

#endif
