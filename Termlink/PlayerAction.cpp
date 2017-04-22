#include "PlayerAction.h"

#include <cassert>
#include <sstream>
#include <exception>

#include "PuzzleWord.h"

WordAttemptAction::WordAttemptAction( PuzzleWord const * puzzleWord )
{
    this->puzzleWord = puzzleWord;
}

FailedAttemptAction::FailedAttemptAction( PuzzleWord  const *  puzzleWord, int attemptNumber, int totalAttemptCount ) : WordAttemptAction( puzzleWord )
{
    this->attemptNumber = attemptNumber;
    this->totalAttemptCount = totalAttemptCount;
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

SuccessfulAttemptAction::SuccessfulAttemptAction( PuzzleWord const * puzzleWord ) : WordAttemptAction( puzzleWord )
{

}

int SuccessfulAttemptAction::GetDisplayHeight() const
{
    return 5;
}

std::string SuccessfulAttemptAction::GetDisplayText( int index ) const
{
    assert( index >= 0 && index < this->GetDisplayHeight() );

    switch ( index )
    {
    case 0:
        return "is accessed.";
    case 1:
        return "while system";
    case 2:
        return "Please wait";
    case 3:
        return "Exact match!";
    case 4:
        return this->puzzleWord->GetText();
    default:
        return "";
    }
}

BracketAction::BracketAction( std::string bracketText )
{
    this->bracketText = bracketText;
}

DudBracketAction::DudBracketAction( std::string bracketText ) : BracketAction(bracketText)
{
}

int DudBracketAction::GetDisplayHeight() const
{
    return 2;
}

std::string DudBracketAction::GetDisplayText( int index ) const
{
    switch ( index )
    {
    case 0:
        return "Dud removed.";
    case 1:
        return this->bracketText;
    default:
        return "";
    }
}

ReplenishBracketAction::ReplenishBracketAction( std::string bracketText ) : BracketAction(bracketText)
{
}

int ReplenishBracketAction::GetDisplayHeight() const
{
    return 3;
}

std::string ReplenishBracketAction::GetDisplayText( int index ) const
{
    switch ( index )
    {
    case 0:
        return "replenished.";
    case 1:
        return "Allowance";
    case 2:
        return this->bracketText;
    default:
        return "";
    }
}
