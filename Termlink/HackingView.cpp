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

    this->cursorBlinkTimer = 0.0f;
    this->timeSinceDelayedRenderStart = 0.0f;
    this->isCursorFilled = false;
    this->lastTypingCoord = {-1,-1};

    this->displayBuffer.resize( this->GetScreenHeight() * this->GetScreenWidth() );

    this->ClearBuffer();

    SetConsoleScreenBufferSize( this->outputHandle, {( short )this->GetScreenWidth(), ( short )this->GetScreenHeight()} );

    this->SetHexAddresses();
}


HackingView::~HackingView()
{
}

void HackingView::OnStateChange( GameState oldState, GameState newState )
{
    this->timeSinceDelayedRenderStart = 0.0f;
    this->lastTypingCoord = {-1,-1};
}

bool HackingView::Render( GameState state, float deltaTime, COORD cursorCoord )
{
    this->ClearBuffer();
    this->cursorBlinkTimer += deltaTime;
    this->timeSinceDelayedRenderStart += deltaTime;
    if ( cursorBlinkTimer > 0.4f )
    {
        this->cursorBlinkTimer = 0.0f;
        this->isCursorFilled = !this->isCursorFilled;
    }

    bool result = this->RefreshBuffer( state, cursorCoord );

    for ( int y = 0; y < this->GetScreenHeight(); ++y )
    {
        for ( int x = 0; x < this->GetScreenWidth(); ++x )
        {
            const int index = y * this->GetScreenWidth() + x;

            if ( this->delayBuffer[y][x] > this->timeSinceDelayedRenderStart && this->delayBuffer[y][x] - deltaTime < this->timeSinceDelayedRenderStart )
            {
                this->lastTypingCoord = {(short)x + 1, (short)y};
            }

            displayBuffer[index].Char.AsciiChar = this->delayBuffer[y][x] <= this->timeSinceDelayedRenderStart ? this->characterBuffer[y][x] : ' ';
            displayBuffer[index].Attributes = this->delayBuffer[y][x] <= this->timeSinceDelayedRenderStart && this->highlightBuffer[y][x] ? HIGHLIGHTED_CHAR_ATTRIBUTES : NORMAL_CHAR_ATTRIBUTES;
        }
    }

    const COORD ORIGIN_COORD = {0,0};
    SMALL_RECT AREA_RECT = {ORIGIN_COORD.X, ORIGIN_COORD.Y, ( short )this->GetScreenWidth(), ( short )this->GetScreenHeight()};
    const COORD screenSize = {( short )this->GetScreenWidth(), ( short )this->GetScreenHeight()};
    WriteConsoleOutput( outputHandle, this->displayBuffer.data(), screenSize, ORIGIN_COORD, &AREA_RECT );

    return result;
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
    return 5;
}

DifficultyLevel * HackingView::GetDifficultyAtCoord( COORD coord ) const
{
    for ( int i = 0; i < this->hackingModel->GetDifficultyCount(); ++i )
    {
        DifficultyLevel * difficulty = this->hackingModel->GetDifficultyLevelWithIndex( i );

        COORD textPosition = {(short)1, (short)i * 2 + 3};
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

    this->delayBuffer.clear();
    this->delayBuffer.resize( this->GetScreenHeight(), std::vector<float>( this->GetScreenWidth(), 0.0f ) );
}

bool HackingView::RefreshBuffer( GameState state, COORD cursorCoord )
{
    switch ( state )
    {
    case GameState::PRE_GAME:
    {
        return this->RenderPreGame( cursorCoord );
    }
    case GameState::DIFFICULTY_SELECTION_PRE_RENDER:
    {
        return this->RenderDifficultyScreen( cursorCoord, true );
    }
    case GameState::DIFFICULTY_SELECTION:
    {
        return this->RenderDifficultyScreen( cursorCoord, false );
    }
    case GameState::PLAYING_GAME:
    case GameState::PLAYING_GAME_PRE_RENDER:
    case GameState::GAME_COMPLETE:
    case GameState::GAME_OVER:
    {
        return this->RenderGameScreen( state, cursorCoord );
    }
    case GameState::LOCKED_OUT:
        return this->RenderLockoutScreen( state, cursorCoord );
    case GameState::LOGIN:
        return this->RenderLoginScreen( state, cursorCoord );
    default:
        return false;
    }
}

bool HackingView::RenderPreGame( COORD cursorCoord )
{
    float delay = 0.0f;

    delay = this->RenderDelayedTextFast( {0,0}, "WELCOME TO ROBCO INDUSTRIES (TM) TERMALINK", delay );

    delay = this->RenderDelayedText( {0,2}, ">", delay, delay );
    delay += 1.0f;
    delay = this->RenderDelayedTextSlow( {1,2}, "SET TERMINAL/INQUIRE", delay );
    delay += 0.5f;
    delay = this->RenderDelayedTextFast( {0,4}, "RIT-V300", delay );

    delay = this->RenderDelayedText( {0,6}, ">", delay, delay );
    delay += 1.0f;
    delay = this->RenderDelayedTextSlow( {1,6}, "SET FILE/PROTECTION-OWNER:RWED ACCOUNTS.F", delay );
    delay += 0.5f;

    delay = this->RenderDelayedText( {0,7}, ">", delay, delay );
    delay += 1.0f;
    delay = this->RenderDelayedTextSlow( {1,7}, "SET HALT RESTART/MAINT", delay );
    delay += 0.5f;
    delay = this->RenderDelayedTextFast( {0,9}, "Initializing Robco Industries(TM) MF Boot Agent v.2.3.0", delay );
    delay = this->RenderDelayedTextFast( {0,10}, "RETROS BIOS", delay );
    delay = this->RenderDelayedTextFast( {0,11}, "RBIOS-4.-2.08.00 52EE5.E7.E8", delay );
    delay = this->RenderDelayedTextFast( {0,12}, "Copyright 2201-2203 Robco Ind.", delay );
    delay = this->RenderDelayedTextFast( {0,13}, "Uppermem: 64 KB", delay );
    delay = this->RenderDelayedTextFast( {0,14}, "Root (5A8)", delay );
    delay = this->RenderDelayedTextFast( {0,15}, "Maintenance Mode", delay );

    delay = this->RenderDelayedText( {0,17}, ">", delay, delay );
    delay += 1.0f;
    delay = this->RenderDelayedTextSlow( {1,17}, "RUN DEBUG/ACCOUNTS.F", delay );

    if ( this->lastTypingCoord.X != -1 && this->lastTypingCoord.Y != -1 )
    {
        this->RenderText( this->lastTypingCoord, " ", this->isCursorFilled );
    }

    return this->timeSinceDelayedRenderStart >= delay;
}

bool HackingView::RenderDifficultyScreen( COORD cursorCoord, bool preRendering )
{
    float delay = 0.0f;

    if ( preRendering )
    {
        delay = this->RenderDelayedTextFast( {0,0}, "SELECT THE PASSWORD STRENGTH:", delay );
    }
    else
    {
        this->RenderText( {0,0}, "SELECT THE PASSWORD STRENGTH:", false );
    }


    for ( int i = 0; i < this->hackingModel->GetDifficultyCount(); ++i )
    {
        const DifficultyLevel * const difficulty = this->hackingModel->GetDifficultyLevelWithIndex( i );

        COORD textPosition = {(short)1, (short)i * 2 + 3};
        std::string text = ">" + difficulty->GetName();
        bool highlighted = this->IsCoordInString( cursorCoord, textPosition, text.size() );

        if ( preRendering )
        {
            delay = this->RenderDelayedTextFast( textPosition, text, delay );
        }
        else
        {
            this->RenderText( textPosition, text, highlighted );
        }
    }

    this->highlightBuffer[cursorCoord.Y][cursorCoord.X] = true;

    return this->timeSinceDelayedRenderStart >= delay;
}

bool HackingView::RenderGameScreen( GameState state, COORD cursorCoord )
{
    float delay = 0.0f;

    if ( state == GameState::PLAYING_GAME_PRE_RENDER )
    {
        delay = this->RenderDelayedTextFast( {0,0}, "ROBCO INDUSTRIES (TM) TERMALINK PROTOCOL", delay );
    }
    else
    {
        this->RenderText( {0,0}, "ROBCO INDUSTRIES (TM) TERMALINK PROTOCOL", false );
    }

    if ( this->hackingModel->GetAttemptsRemaining() == 1 )
    {
        // Make the lockout indicator blink
        float f = remainder( this->timeSinceDelayedRenderStart, 1.0f );
        if ( f > 0.0f )
        {
            this->RenderText( {0, 1}, "!!! WARNING: LOCKOUT IMMINENT !!!", false );
        }
    }
    else
    {
        if ( state == GameState::PLAYING_GAME_PRE_RENDER )
        {
            delay = this->RenderDelayedTextFast( {0,1}, "ENTER PASSWORD NOW", delay );
        }
        else
        {
            this->RenderText( {0,1}, "ENTER PASSWORD NOW", false );
        }

    }

    std::ostringstream outstr;
    outstr << this->hackingModel->GetAttemptsRemaining() << " ATTEMPT(S) LEFT:";

    if ( state == GameState::PLAYING_GAME_PRE_RENDER )
    {
        delay = this->RenderDelayedTextFast( {0, 3}, outstr.str(), delay );
    }
    else
    {
        this->RenderText( {0, 3}, outstr.str(), false );
    }

    for ( int i = 0; i < this->hackingModel->GetAttemptsRemaining(); ++i )
    {
        short xOffset = (short)( outstr.str().size() + 1 + i * 2 );
        COORD coord = {xOffset, 3};
        if ( state == GameState::PLAYING_GAME_PRE_RENDER )
        {
            delay = this->RenderDelayedTextFast( coord, " ", delay, true );
        }
        else
        {
            this->RenderText( coord, " ", true );
        }
    }

    for ( int columnIndex = 0; columnIndex < this->hackingModel->GetColumnCount(); ++columnIndex )
    {
        for ( int rowIndex = 0; rowIndex < this->hackingModel->GetColumnHeight(); ++rowIndex )
        {
            const std::string& hexCode = this->hexAddresses[columnIndex * this->hackingModel->GetColumnHeight() + rowIndex];
            COORD hexCoord = {(short)( columnIndex * this->GetTotalColumnWidth() ), (short)( rowIndex + this->GetLineCountAboveColumns() )};
            if ( state == GameState::PLAYING_GAME_PRE_RENDER )
            {
                delay = this->RenderDelayedTextVeryFast( hexCoord, hexCode, delay );
            }
            else
            {
                this->RenderText( hexCoord, hexCode, false );
            }

            const std::string& fillerText = this->hackingModel->GetFillerText( columnIndex, rowIndex );
            COORD fillerCoord = {hexCoord.X + ( short )this->GetHexCodeLength() + 1, hexCoord.Y};
            if ( state == GameState::PLAYING_GAME_PRE_RENDER )
            {
                delay = this->RenderDelayedTextVeryFast( fillerCoord, fillerText, delay );
            }
            else
            {
                this->RenderText( fillerCoord, fillerText, false );
            }
        }
    }

    for ( int i = 0; i < this->hackingModel->GetPuzzleWordCount(); ++i )
    {
        const PuzzleWord * const puzzleWord = this->hackingModel->GetPuzzleWord( i );
        for ( int j = 0; j < this->hackingModel->GetCurrentDifficulty()->GetWordLength(); ++j )
        {
            ModelCoordinate letterPos = puzzleWord->GetLetterPosition( j );
            COORD coord = this->LetterPositionToCoord( letterPos );
            std::string character( 1, puzzleWord->IsRemoved() ? '.' : puzzleWord->GetText()[j] );

            if ( state == GameState::PLAYING_GAME_PRE_RENDER )
            {
                float spotDelay = this->delayBuffer[coord.Y][coord.X];
                this->RenderDelayedTextVeryFast( coord, character, spotDelay );
            }
            else
            {
                this->RenderText( coord, character, false );
            }
        }
    }

    COORD highlightDisplayCoord = {(short)( this->GetTotalColumnWidth() * this->hackingModel->GetColumnCount() ), (short)( this->GetScreenHeight() - 1 )};

    int attemptedWordOffset = 1;
    for ( int i = this->hackingModel->GetPlayerActionCount() - 1; i >= 0; --i )
    {
        PlayerAction const * playerAction = this->hackingModel->GetPlayerAction( i );
        COORD coord = {highlightDisplayCoord.X, highlightDisplayCoord.Y - (short)attemptedWordOffset};

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

            if ( selectedPuzzleWord != nullptr && !selectedPuzzleWord->IsRemoved() )
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
                if ( selectedPuzzleWord->IsRemoved() )
                {
                    outstr << ".";
                }
                else
                {
                    outstr << selectedPuzzleWord->GetText();
                }
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

            delay = this->RenderDelayedTextFast( highlightDisplayCoord, outstr.str(), delay );
            this->highlightBuffer[cursorCoord.Y][cursorCoord.X] = true;
        }
        else
        {
            std::ostringstream outstr;
            outstr << ">";
            this->RenderText( highlightDisplayCoord, outstr.str(), false );
        }
    }

    if ( state == GameState::GAME_OVER )
    {
        // Roll the lines up to the top of the screen until none can be seen
        int rowOffset = (int)( this->timeSinceDelayedRenderStart * 16.0f );
        for ( int i = 0; i < this->GetScreenHeight(); ++i )
        {
            if ( i + rowOffset >= this->GetScreenHeight() )
            {
                this->characterBuffer[i] = std::string( this->GetScreenWidth(), ' ' );
            }
            else
            {
                this->characterBuffer[i] = this->characterBuffer[i + rowOffset];
            }
        }

        // Rendering is done when the last line goes above the top of the screen
        return rowOffset > this->GetScreenHeight();
    }
    else
    {
        // Rendering is done when the last delayed character is rendered
        return delay <= this->timeSinceDelayedRenderStart;
    }
}

bool HackingView::RenderLockoutScreen( GameState state, COORD cursorCoord )
{
    std::string msg = "TERMINAL LOCKED";
    this->RenderText( {(short)( this->GetScreenWidth() / 2 - msg.size() / 2 ), (short)( this->GetScreenHeight() / 2 - 2 )}, msg, false );

    msg = "PLEASE CONTACT AN ADMINISTRATOR";
    this->RenderText( {(short)( this->GetScreenWidth() / 2 - msg.size() / 2 ), (short)( this->GetScreenHeight() / 2 )}, msg, false );

    return true;
}

bool HackingView::RenderLoginScreen( GameState state, COORD cursorCoord )
{
    float delay = 0.0f;

    delay = this->RenderDelayedTextFast( {0, 0}, "WELCOME TO ROBCO INDUSTRIES (TM) TERMALINK", delay );

    delay = this->RenderDelayedTextFast( {0, 3}, "> ", delay );
    delay += 1.0f;

    delay += this->RenderDelayedTextSlow( {2, 3}, "LOGON ADMIN", delay );
    delay += 0.5f;

    delay = this->RenderDelayedTextFast( {0, 5}, "ENTER PASSWORD NOW", delay );
    
    delay = this->RenderDelayedTextSlow( {0, 7}, ">", delay );
    delay += 1.0f;

    delay = this->RenderDelayedTextSlow( {2, 7}, std::string( this->hackingModel->GetCurrentDifficulty()->GetWordLength(), '*' ), delay );

    return this->timeSinceDelayedRenderStart >= delay;
}

void HackingView::RenderText( COORD position, std::string text, bool isHighlighted )
{
    this->characterBuffer[position.Y].replace( position.X, text.size(), text );
    for ( unsigned int i = 0; i < text.size(); ++i )
    {
        this->highlightBuffer[position.Y][position.X + i] = isHighlighted;
    }
}

float HackingView::RenderDelayedText( COORD position, std::string text, float startDelay, float endDelay, bool highlighted )
{
    this->RenderText( position, text, highlighted );

    for ( int i = 0; i < (int)text.size(); ++i )
    {
        this->delayBuffer[position.Y][position.X + i] = startDelay + ( text.size() == 1 ? 0.0f : ( ( endDelay - startDelay ) / ( (float)text.size() - 1 ) * (float)i ) );
    }

    return endDelay;
}

float HackingView::RenderDelayedTextSlow( COORD position, std::string text, float startDelay, bool highlighted )
{
    return this->RenderDelayedText( position, text, startDelay, startDelay + (float)text.size() * 0.055f, highlighted );
}

float HackingView::RenderDelayedTextFast( COORD position, std::string text, float startDelay, bool highlighted )
{
    return this->RenderDelayedText( position, text, startDelay, startDelay + (float)text.size() * 0.015f, highlighted );
}

float HackingView::RenderDelayedTextVeryFast( COORD position, std::string text, float startDelay, bool highlighted )
{
    return this->RenderDelayedText( position, text, startDelay, startDelay + (float)text.size() * 0.005f, highlighted );
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
