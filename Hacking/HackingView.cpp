#include "HackingView.h"

#include <cassert>
#include <sstream>
#include <algorithm>

#include "HackingModel.h"
#include "PuzzleWord.h"
#include "DifficultyLevel.h"


HackingView::HackingView( const HackingModel * const hackingModel ) : hackingModel( hackingModel )
{
    assert( hackingModel != nullptr );

    this->displayBuffer.resize( this->GetScreenHeight() * this->GetScreenWidth() );

    this->characterBuffer.resize( this->GetScreenHeight(), std::string( this->GetScreenWidth(), ' ' ) );
    this->highlightBuffer.resize( this->GetScreenHeight(), std::vector<bool>( this->GetScreenWidth(), false ) );

    this->stringBuffer.resize( this->hackingModel->GetTotalColumnCharacterCount(), '#' );

    SetConsoleScreenBufferSize( this->outputHandle, {( short )this->GetScreenWidth(), ( short )this->GetScreenHeight()} );

    this->SetHexAddresses();
}


HackingView::~HackingView()
{
}

void HackingView::Render( COORD cursorCoord )
{
    this->ClearBuffer();
    this->RefreshBuffer( cursorCoord );

    const WORD NORMAL_CHAR_ATTRIBUTES = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    const WORD HIGHLIGHTED_CHAR_ATTRIBUTES = BACKGROUND_GREEN | BACKGROUND_INTENSITY;

    for ( int y = 0; y < this->GetScreenHeight(); ++y )
    {
        for ( int x = 0; x < this->GetScreenWidth(); ++x )
        {
            const int index = y * this->GetScreenWidth() + x;
            displayBuffer[index].Char.AsciiChar = this->characterBuffer[y][x];
            displayBuffer[index].Attributes = this->highlightBuffer[y][x] ? HIGHLIGHTED_CHAR_ATTRIBUTES : NORMAL_CHAR_ATTRIBUTES;
        }
    }

    const COORD ORIGIN_COORD = {0,0};
    SMALL_RECT AREA_RECT = {ORIGIN_COORD.X, ORIGIN_COORD.Y, ( short )this->GetScreenWidth(), ( short )this->GetScreenHeight()};
    const COORD screenSize = {( short )this->GetScreenWidth(), ( short )this->GetScreenHeight()};
    WriteConsoleOutput( outputHandle, this->displayBuffer.data(), screenSize, ORIGIN_COORD, &AREA_RECT );
}

void HackingView::SetOutputHandle( HANDLE handle )
{
    this->outputHandle = handle;
}

int HackingView::GetScreenHeight() const
{
    return this->hackingModel->GetColumnHeight() + this->GetLineCountAboveColumns();
}

int HackingView::GetScreenWidth() const
{
    const int answerAreaWidth = this->hackingModel->GetMaximumWordLength() + 3;
    return this->GetTotalColumnWidth() * this->hackingModel->GetColumnCount() + answerAreaWidth;
}

int HackingView::GetTotalColumnWidth() const
{
    return this->hackingModel->GetColumnWidth() + 1 + this->GetHexCodeLength() + 1;
}

int HackingView::GetHexCodeLength() const
{
    return 6;
}

int HackingView::GetLineCountAboveColumns() const
{
    return 3 + this->GetIntroText().size();
}

const std::vector<std::string> HackingView::GetIntroText() const
{
    return
        std::vector<std::string>{
        "ROBCO INDUSTRIES (TM) TERMALINK PROTOCOL",
            "!!!! WARNING: LOCKOUT IMMINENT !!!!",
    };
}

DifficultyLevel * HackingView::GetDifficultyAtCoord( COORD coord ) const
{
    for ( int i = 0; i < this->hackingModel->GetDifficultyCount(); ++i )
    {
        DifficultyLevel * difficulty = this->hackingModel->GetDifficultyLevelWithIndex( i );

        COORD textPosition = {(short)( this->GetScreenWidth() - difficulty->GetName().size() ) / 2, (short)i * 2 + 5};
        if ( this->IsCoordInString( coord, textPosition, difficulty->GetName().size() ) )
        {
            return difficulty;
        }
    }

    return nullptr;
}

void HackingView::ClearBuffer()
{
    this->characterBuffer.clear();
    this->characterBuffer.resize( this->GetScreenHeight(), std::string( this->GetScreenWidth(), ' ' ) );
    this->highlightBuffer.clear();
    this->highlightBuffer.resize( this->GetScreenHeight(), std::vector<bool>( this->GetScreenWidth(), false ) );
}

void HackingView::RefreshBuffer( COORD cursorCoord )
{
    if ( this->hackingModel->GetCurrentDifficulty() == nullptr )
    {
        this->RenderDifficultyScreen( cursorCoord );
    }
    else
    {
        this->RenderGameScreen( cursorCoord );
    }
}

void HackingView::RenderDifficultyScreen( COORD cursorCoord )
{
    for ( int i = 0; i < this->hackingModel->GetDifficultyCount(); ++i )
    {
        const DifficultyLevel * const difficulty = this->hackingModel->GetDifficultyLevelWithIndex( i );

        COORD textPosition = {(short)( this->GetScreenWidth() - difficulty->GetName().size() ) / 2, (short)i * 2 + 5};
        bool highlighted = this->IsCoordInString( cursorCoord, textPosition, difficulty->GetName().size() );

        this->RenderText( textPosition, difficulty->GetName(), highlighted );
    }

    this->highlightBuffer[cursorCoord.Y][cursorCoord.X] = true;
}

void HackingView::RenderGameScreen( COORD cursorCoord )
{
    for ( int i = 0; i < ( int )this->GetIntroText().size(); ++i )
    {
        this->characterBuffer[i].replace( 0, this->GetIntroText()[i].size(), this->GetIntroText()[i] );
    }

    std::ostringstream outstr;
    outstr << this->hackingModel->GetAttemptsRemaining() << " ATTEMPT(S) LEFT:";
    this->RenderText( {0, ( short )this->GetIntroText().size() + 1}, outstr.str(), false );

    for ( int columnIndex = 0; columnIndex < this->hackingModel->GetColumnCount(); ++columnIndex )
    {
        for ( int rowIndex = 0; rowIndex < this->hackingModel->GetColumnHeight(); ++rowIndex )
        {
            const std::string& hexCode = this->hexAddresses[columnIndex * this->hackingModel->GetColumnHeight() + rowIndex];
            COORD hexCoord = {(short)( columnIndex * this->GetTotalColumnWidth() ), (short)( rowIndex + this->GetLineCountAboveColumns() )};
            this->RenderText( hexCoord, hexCode, false );

            const std::string& fillerText = this->hackingModel->GetFillerText( columnIndex, rowIndex );
            COORD fillerCoord = {hexCoord.X + ( short )this->GetHexCodeLength() + 1, hexCoord.Y};
            this->RenderText( fillerCoord, fillerText, false );
        }
    }

    for ( int i = 0; i < this->hackingModel->GetPuzzleWordCount(); ++i )
    {
        const PuzzleWord * const puzzleWord = this->hackingModel->GetPuzzleWord( i );
        for ( int j = 0; j < this->hackingModel->GetCurrentDifficulty()->GetWordLength(); ++j )
        {
            LetterPosition letterPos = puzzleWord->GetLetterPosition( j );
            COORD coord = this->LetterPositionToCoord( letterPos );
            this->characterBuffer[coord.Y][coord.X] = puzzleWord->GetText()[j];
        }
    }

    for ( int y = 0; y < this->GetScreenHeight(); ++y )
    {
        for ( int x = 0; x < this->GetScreenWidth(); ++x )
        {
            this->highlightBuffer[y][x] = false;
        }
    }

    for ( int i = 0; i < this->hackingModel->GetAttemptedWordCount(); ++i )
    {

    }

    PuzzleWord* selectedPuzzleWord = nullptr;
    for ( int i = 0; i < this->hackingModel->GetPuzzleWordCount(); ++i )
    {
        PuzzleWord* puzzleWord = this->hackingModel->GetPuzzleWord( i );

        if ( this->IsCoordInPuzzleWord( cursorCoord, puzzleWord ) )
        {
            selectedPuzzleWord = puzzleWord;
            for ( int j = 0; j < this->hackingModel->GetCurrentDifficulty()->GetWordLength(); ++j )
            {
                COORD& pos = this->LetterPositionToCoord( puzzleWord->GetLetterPosition( j ) );
                this->highlightBuffer[pos.Y][pos.X] = true;
            }
        }
    }

    if (selectedPuzzleWord != nullptr )
    {
        std::ostringstream outstr;
        outstr << "> " << selectedPuzzleWord->GetText();

        this->characterBuffer[this->GetScreenHeight() - 1].replace( this->GetTotalColumnWidth() * this->hackingModel->GetColumnCount() + 1,
            outstr.str().length(),
            outstr.str() );
    }
    else
    {
        std::ostringstream outstr;
        outstr << "> ";

        this->characterBuffer[this->GetScreenHeight() - 1].replace( this->GetTotalColumnWidth()* this->hackingModel->GetColumnCount() + 1,
            outstr.str().length(),
            outstr.str() );
    }

    this->highlightBuffer[cursorCoord.Y][cursorCoord.X] = true;
}

void HackingView::RenderText( COORD position, std::string text, bool isHighlighted )
{
    this->characterBuffer[position.Y].replace( position.X, text.size(), text );
    for ( unsigned int i = 0; i < text.size(); ++i )
    {
        this->highlightBuffer[position.Y][position.X + i] = isHighlighted;
    }
}

bool HackingView::IsCoordInString( const COORD & coord, const COORD & textPosition, int textLength ) const
{
    return coord.Y == textPosition.Y && coord.X >= textPosition.X && coord.X < textPosition.X + textLength;
}

bool HackingView::IsCoordInArea( const COORD & position, const COORD & start, const COORD & end ) const
{
    return position.Y >= start.Y && position.X <= end.Y && position.X >= start.X && position.X <= end.X;
}


void HackingView::SetHexAddresses()
{
    this->hexAddresses.resize( this->hackingModel->GetTotalLineCount(), std::string( this->GetHexCodeLength(), '#' ) );

    int address = rand() % 0xF000 + 0xFFF;
    for ( int i = 0; i < this->hackingModel->GetTotalLineCount(); ++i )
    {
        address += sizeof( char ) * this->GetTotalColumnWidth();
        std::ostringstream stream;
        stream << "0x" << std::hex << address;
        hexAddresses[i] = stream.str();
        std::transform( hexAddresses[i].begin() + 2, hexAddresses[i].end(), hexAddresses[i].begin() + 2, ::toupper );
    }
}

COORD HackingView::LetterPositionToCoord( LetterPosition letterPos ) const
{
    int xpos = letterPos.x + letterPos.column * this->GetTotalColumnWidth() + this->GetHexCodeLength() + 1;
    int ypos = letterPos.y + this->GetLineCountAboveColumns();
    return{(short)xpos, (short)ypos};
}

bool HackingView::IsCoordInPuzzleWord( COORD coord, PuzzleWord * puzzleWord )
{
    for ( int i = 0; i < (int)puzzleWord->GetText().size(); ++i )
    {
        COORD c = this->LetterPositionToCoord( puzzleWord->GetLetterPosition( i ) );
        if ( c.X == coord.X && c.Y == coord.Y )
        {
            return true;
        }
    }

    return false;
}
