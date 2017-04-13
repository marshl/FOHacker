#include "MatchingBracket.h"

#include <cassert>

MatchingBracket::MatchingBracket( char openingSymbol, char closingSymbol, int column, int row, int startingPosition, int endingPosition )
{
    this->openingSymbol = openingSymbol;
    this->closingSymbol = closingSymbol;

    assert( column >= 0 );
    this->column = column;

    assert( row >= 0 );
    this->row = row;

    assert( startingPosition >= 0 );
    this->startingPosition = startingPosition;

    assert( endingPosition > startingPosition );
    assert( endingPosition >= 0 );
    this->endingPosition = endingPosition;

    this->isConsumed = false;

}

char MatchingBracket::GetOpeningSymbol() const
{
    return this->openingSymbol;
}

char MatchingBracket::GetClosingSymbol() const
{
    return this->closingSymbol;
}

int MatchingBracket::GetColumn() const
{
    return this->column;
}

int MatchingBracket::GetRow() const
{
    return this->row;
}

int MatchingBracket::GetStartingPosition() const
{
    return this->startingPosition;
}

int MatchingBracket::GetEndingPosition() const
{
    return this->endingPosition;
}

bool MatchingBracket::IsConsumed() const
{
    return this->isConsumed;
}

void MatchingBracket::Consume()
{
    this->isConsumed = true;
}