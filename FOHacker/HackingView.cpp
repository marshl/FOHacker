#include "HackingView.h"

#include <cassert>
#include <sstream>
#include <algorithm>

#include "HackingModel.h"
#include "PuzzleWord.h"
#include "DifficultyLevel.h"
#include "PlayerAction.h"

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

void HackingView::Render( GameState state, COORD cursorCoord )
{
    this->ClearBuffer();
    this->RefreshBuffer( state, cursorCoord );

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

void HackingView::RefreshBuffer( GameState state, COORD cursorCoord )
{
    switch ( state )
    {
    case GameState::DIFFICULTY_SELECTION:
    {
        this->RenderDifficultyScreen( cursorCoord );
        break;
    }
    case GameState::PLAYING_GAME:
    case GameState::GAME_COMPLETE:
    {
        this->RenderGameScreen( state, cursorCoord );
        break;
    }
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

void HackingView::RenderGameScreen( GameState state, COORD cursorCoord )
{
    for ( int i = 0; i < ( int )this->GetIntroText().size(); ++i )
    {
        this->RenderText( {0, (short)i}, this->GetIntroText()[i], false );
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
            ModelCoordinate letterPos = puzzleWord->GetLetterPosition( j );
            COORD coord = this->LetterPositionToCoord( letterPos );
            char character = puzzleWord->IsRemoved() ? '.' : puzzleWord->GetText()[j];
            this->characterBuffer[coord.Y][coord.X] = character;
        }
    }

    int attemptedWordOffset = 0;
    for ( int i = this->hackingModel->GetPlayerActionCount() - 1; i >= 0; --i )
    {
        PlayerAction const * playerAction = this->hackingModel->GetPlayerAction( i );
        COORD coord = {(short)( this->GetTotalColumnWidth() * this->hackingModel->GetColumnCount() + 1 ),(short)( this->GetScreenHeight() - 2 - attemptedWordOffset )};

        for ( int j = 0; j < playerAction->GetDisplayHeight(); ++j )
        {
            this->RenderText( coord, ">" + playerAction->GetDisplayText( j ), false );
            --coord.Y;
            ++attemptedWordOffset;
            if ( attemptedWordOffset > this->hackingModel->GetColumnHeight() - 2 )
            {
                i = -1;
                break;
            }
        }
    }

    if ( state == GameState::PLAYING_GAME )
    {
        ModelCoordinate letterPos;
        if ( this->ConvertViewSpaceToModelSpace( cursorCoord, letterPos ) )
        {
            const BracketPair * selectedBracket = this->hackingModel->GetBracketPairAtCoord( letterPos );

            if ( selectedBracket != nullptr && !selectedBracket->IsConsumed() )
            {
                for ( int position = selectedBracket->GetStartingPosition(); position <= selectedBracket->GetEndingPosition(); ++position )
                {
                    COORD coord = this->ColumnPositionToCoord( selectedBracket->GetColumn(), selectedBracket->GetRow(), position );
                    this->highlightBuffer[coord.Y][coord.X] = true;
                }
            }

            PuzzleWord const * selectedPuzzleWord = this->hackingModel->GetPuzzleWordAtCoord( letterPos );

            if ( selectedPuzzleWord != nullptr )
            {
                for ( int j = 0; j < this->hackingModel->GetCurrentDifficulty()->GetWordLength(); ++j )
                {
                    COORD& pos = this->LetterPositionToCoord( selectedPuzzleWord->GetLetterPosition( j ) );
                    this->highlightBuffer[pos.Y][pos.X] = true;
                }
            }

            std::ostringstream outstr;

            outstr << ">";
            if ( selectedPuzzleWord != nullptr )
            {
                outstr << selectedPuzzleWord->GetText();
            }
            else if ( selectedBracket != nullptr )
            {
                const std::string& filler = this->hackingModel->GetFillerText( selectedBracket->GetColumn(), selectedBracket->GetRow() );
                outstr << filler.substr( selectedBracket->GetStartingPosition(), selectedBracket->GetEndingPosition() - selectedBracket->GetStartingPosition() + 1 );
            }
            else
            {
                const std::string& filler = this->hackingModel->GetFillerText( letterPos.column, letterPos.y );
                outstr << filler[letterPos.x];
            }

            this->characterBuffer[this->GetScreenHeight() - 1].replace( this->GetTotalColumnWidth() * this->hackingModel->GetColumnCount() + 1,
                outstr.str().length(),
                outstr.str() );

            this->highlightBuffer[cursorCoord.Y][cursorCoord.X] = true;
        }
        else
        {
            std::ostringstream outstr;
            outstr << ">";

            this->characterBuffer[this->GetScreenHeight() - 1].replace( this->GetTotalColumnWidth()* this->hackingModel->GetColumnCount() + 1,
                outstr.str().length(),
                outstr.str() );
        }
    }
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

COORD HackingView::ColumnPositionToCoord( int columnIndex, int rowIndex, int position ) const
{
    int xpos = position + columnIndex * this->GetTotalColumnWidth() + this->GetHexCodeLength() + 1;
    int ypos = rowIndex + this->GetLineCountAboveColumns();
    return{(short)xpos, (short)ypos};
}

COORD HackingView::LetterPositionToCoord( ModelCoordinate letterPos ) const
{
    return this->ColumnPositionToCoord( letterPos.column, letterPos.y, letterPos.x );
}

bool HackingView::ConvertViewSpaceToModelSpace( COORD coord, ModelCoordinate& letterPosition ) const
{
    if ( coord.Y < this->GetLineCountAboveColumns() || coord.Y >= this->hackingModel->GetColumnHeight() + this->GetLineCountAboveColumns() )
    {
        return false;
    }

    for ( int columnNumber = 0; columnNumber < this->hackingModel->GetColumnCount(); ++columnNumber )
    {
        if ( coord.X > this->GetHexCodeLength() + columnNumber * this->GetTotalColumnWidth()
            && coord.X < this->GetTotalColumnWidth() * ( columnNumber + 1 ) - 1
            )
        {
            letterPosition.x = coord.X - ( this->GetHexCodeLength() + columnNumber * this->GetTotalColumnWidth() ) - 1;
            letterPosition.y = coord.Y - this->GetLineCountAboveColumns();
            letterPosition.column = columnNumber;

            return true;
        }
    }

    return false;
}

void HackingView::RenderLetterPositionOfCoord( COORD coord )
{
    std::ostringstream ostream;
    ModelCoordinate currentLetterPos;
    bool hl = this->ConvertViewSpaceToModelSpace( coord, currentLetterPos );
    ostream << currentLetterPos.x << " " << currentLetterPos.y << " " << currentLetterPos.column;
    this->RenderText( {0,0}, ostream.str(), hl );
}
