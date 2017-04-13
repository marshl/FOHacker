#pragma once
class MatchingBracket
{
public:
    MatchingBracket( char openingSymbol, char closingSymbol, int column, int row, int startingPosition, int endingPosition );

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

