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

    void OnStateChange( GameState oldState, GameState newState );
    void Render( GameState state, float deltaTime, COORD cursorCoord );

    void SetOutputHandle( HANDLE handle );

    int GetScreenHeight() const;
    int GetScreenWidth() const;

    int GetLineCountAboveColumns() const;

    DifficultyLevel * GetDifficultyAtCoord( COORD coord ) const;

    bool ConvertViewSpaceToModelSpace( COORD coord, ModelCoordinate& ) const;

private:
    const HackingModel * const hackingModel;

    HANDLE outputHandle;

    std::vector<CHAR_INFO> displayBuffer;
    std::vector<std::string> characterBuffer;
    std::vector<std::vector<bool> > highlightBuffer;
    std::vector<std::vector<float>> delayBuffer;
    std::string stringBuffer;
    std::vector<std::string> hexAddresses;
    float cursorBlinkTimer;
    bool isCursorFilled;
    COORD lastTypingCoord;
    

    int GetTotalColumnWidth() const;
    int GetHexCodeLength() const;

    void ClearBuffer();

    void RefreshBuffer( GameState state, COORD cursorCoord );

    void RenderPreGame( COORD cursorCoord );
    void RenderDifficultyScreen( COORD cursorCoord );
    void RenderGameScreen( GameState state, COORD cursorCoord );

    void RenderText( COORD position, std::string text, bool isHighlighted );
    float RenderDelayedText( COORD position, std::string text, float startDelay, float endDelay );
    float RenderDelayedTextSlow( COORD position, std::string text, float startDelay );
    float RenderDelayedTextFast( COORD position, std::string text, float startDelay );

    bool IsCoordInString( const COORD & coord, const COORD & textPosition, int textLength ) const;

    bool IsCoordInArea( const COORD & position, const COORD & start, const COORD & end ) const;

    void SetHexAddresses();

    COORD ColumnPositionToCoord( int columnIndex, int rowIndex, int position ) const;
    COORD LetterPositionToCoord( ModelCoordinate letterPos ) const;
    void RenderLetterPositionOfCoord( COORD coord );
};

#endif