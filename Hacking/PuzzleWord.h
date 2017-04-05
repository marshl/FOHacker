#ifndef PUZZLE_WORD_H_
#define PUZZLE_WORD_H_

#include <Windows.h>
#include <string>
#include <vector>

struct LetterPosition
{
    int position;

    int column;
    int x;
    int y;
};

class PuzzleWord
{
public:
    PuzzleWord( std::string _text ) : text( _text ), isRemoved( false ), position( -1 )
    {
        this->letterPositions.resize( _text.length(), {0, 0} );
    }

    const std::string& GetText() const;

    const int GetPosition() const;

    void SetPosition( int position, int columnCount, int columnHeight, int columnWidth );

    const LetterPosition& GetLetterPosition( int index ) const;

    LetterPosition& GetLetterPosition( int index );

private:
    std::string text;
    bool isRemoved;
    int position;

    std::vector<LetterPosition> letterPositions;
};

#endif
