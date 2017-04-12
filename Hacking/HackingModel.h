#ifndef HACKING_MODEL_H_
#define HACKING_MODEL_H_

#include <vector>
#include <string>
#include <Windows.h>
#include "PuzzleWord.h"
#include "MatchingBracket.h"

class DifficultyLevel;
class PlayerAction;

class HackingModel
{
public:
    HackingModel();
    ~HackingModel();

    int GetAttemptsRemaining() const;

    const int GetPuzzleWordCount() const;
    const int GetAttemptedWordCount() const;

    PuzzleWord * const GetSolutionWord() const;

    PuzzleWord * const GetPuzzleWord( int index ) const;

    PuzzleWord * const GetPuzzleWordAtPosition( int columnIndex, int rowIndex, int positionInRow ) const;

    PuzzleWord * const GetPuzzleWordAtLetterPosition( const LetterPosition& letterPos ) const;

    PuzzleWord * GetAttemptedWord(int index) const;

    DifficultyLevel * GetCurrentDifficulty() const;

    DifficultyLevel * GetDifficultyLevelWithIndex( int index ) const;
    int GetDifficultyCount() const;

    void SetDifficultyLevel( DifficultyLevel * difficulty );

    const int GetColumnWidth() const;
    const int GetColumnHeight() const;
    const int GetColumnCount() const;
    const int GetTotalLineCount() const;
    const int GetTotalColumnCharacterCount() const;

    const int GetMaximumWordLength() const;

    const std::string& GetFillerText( int columnIndex, int rowIndex ) const;

    const int GetMatchingBracketCount() const;
    const MatchingBracket& GetMatchingBracket( int matchingBracketIndex ) const;

    bool AttemptWord( PuzzleWord * const puzzleWord );

    PlayerAction const * GetPlayerAction( int index ) const;
    int GetPlayerActionCount() const;

private:
    int attemptsRemaining;

    std::vector<PuzzleWord * > puzzleWords;
    PuzzleWord* solutionWord;
    std::vector<PuzzleWord * > attemptedWords;
    std::vector<DifficultyLevel*> difficultyLevels;
    std::vector<std::vector<std::string>> fillerCharacters;
    std::vector<MatchingBracket> matchingBrackets;
    std::vector<PlayerAction*> playerActionList;

    DifficultyLevel * currentDifficulty;

    void SetPuzzleWords();
    void PlacePuzzleWords();
    void GetSampleWordList( std::vector<std::string>& _out );
    void InitialiseDifficultyLevels();
    void RandomiseFillerLetters();

    void SetupMatchingBrackets();

    std::vector<MatchingBracket> GetMatchingBracketsForLine( int columnIndex, int rowIndex );
};

#endif
