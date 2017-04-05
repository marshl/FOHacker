#include "PuzzleWord.h"

#include <cassert>

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

    for ( int i = 0; i < this->letterPositions.size(); ++i )
    {
        LetterPosition& letterPos = this->letterPositions[i];

        letterPos.position = position + i;
        letterPos.column = letterPos.position / ( columnHeight * columnWidth );
        letterPos.x = (letterPos.position % ( columnHeight * columnWidth )) % columnWidth;
        assert( letterPos.x >= 0 && letterPos.x < columnWidth );
        letterPos.y = ( letterPos.position % ( columnHeight * columnWidth ) ) / columnWidth;
        assert( letterPos.y >= 0 && letterPos.y < columnHeight );
    }
}

const LetterPosition & PuzzleWord::GetLetterPosition( int index ) const
{
    assert( index >= 0 && index < this->text.size() );
    return this->letterPositions[index];
}

LetterPosition & PuzzleWord::GetLetterPosition( int index )
{
    assert( index >= 0 && index < this->text.size() );
    return this->letterPositions[index];
}
