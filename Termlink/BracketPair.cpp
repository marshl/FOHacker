#include "BracketPair.h"

#include <cassert>

BracketPair::BracketPair(char openingSymbol, char closingSymbol, int column, int row, int startingPosition, int endingPosition)
{
	this->openingSymbol = openingSymbol;
	this->closingSymbol = closingSymbol;

	assert(column >= 0);
	this->column = column;

	assert(row >= 0);
	this->row = row;

	assert(startingPosition >= 0);
	this->startingPosition = startingPosition;

	assert(endingPosition > startingPosition);
	assert(endingPosition >= 0);
	this->endingPosition = endingPosition;

	this->isConsumed = false;

}

char BracketPair::GetOpeningSymbol() const
{
	return this->openingSymbol;
}

char BracketPair::GetClosingSymbol() const
{
	return this->closingSymbol;
}

int BracketPair::GetColumn() const
{
	return this->column;
}

int BracketPair::GetRow() const
{
	return this->row;
}

int BracketPair::GetStartingPosition() const
{
	return this->startingPosition;
}

int BracketPair::GetEndingPosition() const
{
	return this->endingPosition;
}

bool BracketPair::IsConsumed() const
{
	return this->isConsumed;
}

void BracketPair::Consume()
{
	this->isConsumed = true;
}