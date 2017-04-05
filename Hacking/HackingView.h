#ifndef HACKING_VIEW_H_
#define HACKING_VIEW_H_

#include <Windows.h>
#include <string>
#include <vector>

class HackingModel;

class HackingView
{
public:
    HackingView( const HackingModel * const hackingModel );
    ~HackingView();

    void Render( COORD cursorCoord );

    void SetOutputHandle( HANDLE handle );

    int GetScreenHeight() const;
    int GetScreenWidth() const;

    short GetLineCountAboveColumns() const;
    const std::vector<std::string> GetIntroText() const;

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

    void RefreshBuffer( COORD cursorCoord );

    void RenderDifficultyScreen( COORD cursorCoord );
    void RenderGameScreen( COORD cursorCoord );

    void RenderText( COORD position, std::string text, bool isHighlighted );

    bool IsCoordInString( const COORD & coord, const COORD & textPosition, int textLength ) const;

    bool IsCoordInArea( const COORD & position, const COORD & start, const COORD & end ) const;


    void SetHexAddresses();
};

#endif