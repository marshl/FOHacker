#include "PlayerAction.h"

#include <cassert>
#include <sstream>
#include <exception>

#include "PuzzleWord.h"

WordAttemptAction::WordAttemptAction( PuzzleWord const * puzzleWord )
{
    this->puzzleWord = puzzleWord;
}

int FailedAttemptAction::GetDisplayHeight() const
{
    return 3;
}

std::string FailedAttemptAction::GetDisplayText( int index ) const
{
    assert( index >= 0 && index < this->GetDisplayHeight() );

    switch ( index )
    {
    case 0:
    {
        int correctLetters = this->puzzleWord->GetText().size() - this->puzzleWord->GetDifferenceFromSolution();
        std::ostringstream outstr;
        outstr << correctLetters << "/" << this->puzzleWord->GetText().size() << " correct";
        return outstr.str();
    }
    case 1:
        return "Entry denied";
    case 2:
        return this->puzzleWord->GetText();
    default:
        return "";
    }
}

int SuccessfulAttemptAction::GetDisplayHeight() const
{
    return 2;
}

std::string SuccessfulAttemptAction::GetDisplayText( int index ) const
{
    assert( index >= 0 && index < this->GetDisplayHeight() );

    switch ( index )
    {
    case 0:
        return "Entry granted";
    case 1:
        return this->puzzleWord->GetText();
    default:
        return "";
    }
}
