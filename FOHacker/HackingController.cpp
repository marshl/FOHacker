#include "HackingController.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <list>
#include <ctime>
#include <boost/algorithm/clamp.hpp>

#include "strfunc.h"
#include "PuzzleWord.h"
#include "HackingModel.h"
#include "HackingView.h"

HackingController::HackingController( HackingModel* hackingModel, HackingView* hackingView )
{
    assert( hackingModel != nullptr );
    assert( hackingView != nullptr );

    this->hackingModel = hackingModel;
    this->hackingView = hackingView;

    this->inputHandle = CreateFile( "CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
    this->outputHandle = CreateFile( "CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );

    this->hackingView->SetOutputHandle( outputHandle );

    this->cursorCoord = {0, 0};

    this->currentState = GameState::NONE;

    this->isDone = false;
}

HackingController::~HackingController()
{
    CloseHandle( this->inputHandle );
    CloseHandle( this->outputHandle );
}

void HackingController::Run()
{
    this->SetupWindow();
    this->ChangeState( GameState::PRE_GAME );

    DWORD lastMouseState = 0;
    DWORD eventCount;

    LARGE_INTEGER previousTime;
    QueryPerformanceCounter( &previousTime );

    while ( !this->isDone )
    {
        const int INPUT_BUFFER_SIZE = 255;
        INPUT_RECORD eventBuffer[INPUT_BUFFER_SIZE];
        DWORD eventsRead;

        if ( GetNumberOfConsoleInputEvents( this->inputHandle, &eventCount ) &&
            eventCount > 0 &&
            ReadConsoleInput( this->inputHandle, eventBuffer, INPUT_BUFFER_SIZE, &eventsRead ) )
        {
            for ( unsigned int eventIndex = 0; eventIndex < eventsRead; ++eventIndex )
            {
                INPUT_RECORD& inputRecord = eventBuffer[eventIndex];
                if ( inputRecord.EventType == MOUSE_EVENT )
                {
                    cursorCoord = inputRecord.Event.MouseEvent.dwMousePosition;
                    cursorCoord.X = boost::algorithm::clamp( (int)cursorCoord.X, 0, this->hackingView->GetScreenWidth() - 1 );
                    cursorCoord.Y = boost::algorithm::clamp( (int)cursorCoord.Y, 0, this->hackingView->GetScreenHeight() - 1 );

                    if ( lastMouseState & FROM_LEFT_1ST_BUTTON_PRESSED && !inputRecord.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED )
                    {
                        this->OnClickEvent();
                    }

                    lastMouseState = inputRecord.Event.MouseEvent.dwButtonState;
                }

                if ( inputRecord.EventType == KEY_EVENT )
                {
                    WORD keyEvent = inputRecord.Event.KeyEvent.wVirtualKeyCode;

                    if ( keyEvent == VK_ESCAPE )
                    {
                        this->isDone = true;
                    }
                }
            }
        }

        LARGE_INTEGER frequency;
        LARGE_INTEGER currentTime;

        QueryPerformanceFrequency( &frequency );
        QueryPerformanceCounter( &currentTime );

        float deltaTime = (float)( currentTime.QuadPart - previousTime.QuadPart ) / (float)frequency.QuadPart;
        previousTime = currentTime;

        bool renderResult = this->hackingView->Render( this->currentState, deltaTime, this->cursorCoord );
        if ( renderResult )
        {
            switch ( this->currentState )
            {
            case GameState::DIFFICULTY_SELECTION_PRE_RENDER:
                this->ChangeState( GameState::DIFFICULTY_SELECTION );
            }
        }


        Sleep( 1000 / 24 );
    }
}


void HackingController::SetupWindow()
{
    SetConsoleActiveScreenBuffer( this->inputHandle );
    DWORD flags;
    GetConsoleMode( this->inputHandle, &flags );

    DWORD fdwMode = flags & ~( ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT );
    if ( !SetConsoleMode( inputHandle, fdwMode ) )
    {
        this->isDone = true;
    }

    SetConsoleTitle( "Hacking" );

    // Remove blinking cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.bVisible = FALSE;
    cursorInfo.dwSize = 100; // Yes, the size needs to be set
    SetConsoleCursorInfo( this->outputHandle, &cursorInfo );

    // Resize window to fix View size
    SMALL_RECT tmp = {0, 0, ( short )this->hackingView->GetScreenWidth(), ( short )this->hackingView->GetScreenHeight()};
    SetConsoleWindowInfo( this->outputHandle, TRUE, &tmp );

    // Remove scrollbars
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo( this->outputHandle, &csbi );
    COORD scrollbar = {
        csbi.srWindow.Right - csbi.srWindow.Left + 1,
        csbi.srWindow.Bottom - csbi.srWindow.Top + 1
    };
    SetConsoleScreenBufferSize( this->outputHandle, scrollbar );

    // Remove minimuze/maximise buttons, and prevent resizing
    HWND hwnd = GetConsoleWindow();
    long dwStyle;
    dwStyle = GetWindowLong( hwnd, GWL_STYLE );
    dwStyle ^= WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
    SetWindowLong( hwnd, GWL_STYLE, dwStyle );

    // Set font to be larger and more distinct
    HANDLE outputHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    CONSOLE_FONT_INFOEX consoleFontInfo;
    consoleFontInfo.cbSize = sizeof( consoleFontInfo );
    consoleFontInfo.nFont = 0;
    consoleFontInfo.dwFontSize.X = 0;
    consoleFontInfo.dwFontSize.Y = 18;
    consoleFontInfo.FontFamily = FF_DONTCARE;
    consoleFontInfo.FontWeight = FW_BOLD;
    wcscpy_s( consoleFontInfo.FaceName, L"Consolas" );
    SetCurrentConsoleFontEx( outputHandle, FALSE, &consoleFontInfo );
}

void HackingController::OnClickEvent()
{
    switch ( this->currentState )
    {
    case GameState::PRE_GAME:
    {
        this->ChangeState( GameState::DIFFICULTY_SELECTION_PRE_RENDER );
        break;
    }
    case GameState::DIFFICULTY_SELECTION:
    {
        DifficultyLevel * cursorDifficulty = this->hackingView->GetDifficultyAtCoord( this->cursorCoord );
        if ( cursorDifficulty != nullptr )
        {
            this->hackingModel->SetDifficultyLevel( cursorDifficulty );
            this->currentState = GameState::PLAYING_GAME;
        }

        break;
    }
    case GameState::PLAYING_GAME:
    {
        ModelCoordinate letterPos;
        if ( this->hackingView->ConvertViewSpaceToModelSpace( this->cursorCoord, letterPos ) )
        {
            PuzzleWord const * puzzleWord = this->hackingModel->GetPuzzleWordAtCoord( letterPos );

            if ( puzzleWord != nullptr && !puzzleWord->IsRemoved())
            {
                if ( this->hackingModel->AttemptWord( puzzleWord ) )
                {
                    this->currentState = GameState::GAME_COMPLETE;
                }
            }

            BracketPair const * bracketPair = this->hackingModel->GetBracketPairAtCoord( letterPos );

            if ( bracketPair != nullptr )
            {
                this->hackingModel->AttemptBracketPair( bracketPair );
            }
        }

        break;
    }
    case GameState::GAME_COMPLETE:
    {
        this->isDone = true;
    }
    }
}

void HackingController::ChangeState( GameState newState )
{
    this->hackingView->OnStateChange( this->currentState, newState );

    this->currentState = newState;
}
