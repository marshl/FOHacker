#ifndef HACKING_MODEL_H_
#define HACKING_MODEL_H_

#include <vector>
#include <string>
#include <Windows.h>

class PuzzleWord;
class DifficultyLevel;

class HackingModel
{
public:
    HackingModel();
    ~HackingModel();

    int GetAttemptsRemaining() const;

    const int GetPuzzleWordCount() const;
    const int GetAttemptedWordCount() const;

    PuzzleWord * const GetPuzzleWord( int index ) const;
    PuzzleWord * const GetSelectedPuzzleWord() const;

    void OnClickEvent( const COORD& cursorCoord );
    void OnMouseMoveEvent( const COORD& cursorCoord );

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

private:
    int attemptsRemaining;

    std::string solutionWord;
    std::vector<PuzzleWord*> puzzleWords;
    std::vector<std::string> attemptedWords;
    std::vector<DifficultyLevel*> difficultyLevels;
    std::vector<std::string> fillerCharacters;

    DifficultyLevel * currentDifficulty;

    PuzzleWord* currentHighlightedPuzzleWord;

    void SetPuzzleWords();
    void PlacePuzzleWords();
    void GetSampleWordList( std::vector<std::string>& _out );
    void InitialiseDifficultyLevels();
};

#endif
