#include "PuzzleWord.h"

#include <cassert>

PuzzleWord::PuzzleWord( std::string text, std::string solution ) : text( text ), isAttempted( false ), position( -1 ), differenceFromSolution( 0 )
{
    this->letterPositions.resize( text.length(), {0, 0} );

    this->differenceFromSolution = StringDiff( text, solution );
}

const std::string & PuzzleWord::GetText() const
{
    return this->text;
}

const int PuzzleWord::GetPosition() const
{
    return this->position;
}

void PuzzleWord::SetPosition( int position, int columnCount, int columnHeight, int columnWidth )
{
    this->position = position;

    for ( int i = 0; i < ( int )this->letterPositions.size(); ++i )
    {
        LetterPosition& letterPos = this->letterPositions[i];

        letterPos.position = position + i;
        letterPos.column = letterPos.position / ( columnHeight * columnWidth );
        letterPos.x = ( letterPos.position % ( columnHeight * columnWidth ) ) % columnWidth;
        assert( letterPos.x >= 0 && letterPos.x < columnWidth );
        letterPos.y = ( letterPos.position % ( columnHeight * columnWidth ) ) / columnWidth;
        assert( letterPos.y >= 0 && letterPos.y < columnHeight );
    }
}

const LetterPosition & PuzzleWord::GetLetterPosition( int index ) const
{
    assert( index >= 0 && index < ( int )this->text.size() );
    return this->letterPositions[index];
}

LetterPosition & PuzzleWord::GetLetterPosition( int index )
{
    assert( index >= 0 && index < ( int )this->text.size() );
    return this->letterPositions[index];
}

bool PuzzleWord::GetIsAttempted() const
{
    return this->isAttempted;
}

void PuzzleWord::SetIsAttempted( bool attempted )
{
    this->isAttempted = attempted;
}

int PuzzleWord::GetDifferenceFromSolution() const
{
    return this->differenceFromSolution;
}
