#ifndef BRACKET_PAIR_H_
#define BRACKET_PAIR_H_


class BracketPair
{
public:
    BracketPair( char openingSymbol, char closingSymbol, int column, int row, int startingPosition, int endingPosition );

    char GetOpeningSymbol() const;
    char GetClosingSymbol() const;

    int GetColumn() const;
    int GetRow() const;

    int GetStartingPosition() const;
    int GetEndingPosition() const;

    bool IsConsumed() const;
    void Consume();

private:

    char openingSymbol;
    char closingSymbol;

    int column;
    int row;

    int startingPosition;
    int endingPosition;

    bool isConsumed;
};

#endif