#ifndef HACKING_VIEW_H_
#define HACKING_VIEW_H_

#include <Windows.h>
#include <string>
#include <vector>

#include "PuzzleWord.h"
#include "GameState.h"

class HackingModel;
class DifficultyLevel;

class HackingView
{
public:
    HackingView( const HackingModel * const hackingModel );
    ~HackingView();

    void Render( GameState state, COORD cursorCoord );

    void SetOutputHandle( HANDLE handle );

    int GetScreenHeight() const;
    int GetScreenWidth() const;

    int GetLineCountAboveColumns() const;
    const std::vector<std::string> GetIntroText() const;

    DifficultyLevel * GetDifficultyAtCoord( COORD coord ) const;

    bool CoordToLetterPosition( COORD coord, LetterPosition& ) const;

    bool IsCoordInPuzzleWord( COORD coord, PuzzleWord* puzzleWord ) const;

private:
    const HackingModel * const hackingModel;

    std::vector<CHAR_INFO> displayBuffer;

    std::vector<std::string> characterBuffer;
    std::vector<std::vector<bool> > highlightBuffer;
    std::string stringBuffer;

    std::vector<std::string> hexAddresses;

    HANDLE outputHandle;

    int GetTotalColumnWidth() const;
    int GetHexCodeLength() const;

    void ClearBuffer();

    void RefreshBuffer( GameState state, COORD cursorCoord );

    void RenderDifficultyScreen( COORD cursorCoord );
    void RenderGameScreen( GameState state, COORD cursorCoord );

    void RenderText( COORD position, std::string text, bool isHighlighted );

    bool IsCoordInString( const COORD & coord, const COORD & textPosition, int textLength ) const;

    bool IsCoordInArea( const COORD & position, const COORD & start, const COORD & end ) const;

    void SetHexAddresses();

    COORD ColumnPositionToCoord( int columnIndex, int rowIndex, int position ) const;
    COORD LetterPositionToCoord( LetterPosition letterPos ) const;
    void RenderLetterPositionOfCoord( COORD coord );
};

#endif