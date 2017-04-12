#ifndef PLAYER_ACTION_H_
#define PLAYER_ACTION_H_

#include <string>

class PuzzleWord;

class PlayerAction
{
public:
    virtual int GetDisplayHeight() const = 0;

    virtual std::string GetDisplayText( int index ) const = 0;
};

class WordAttemptAction : public PlayerAction
{
public:
    WordAttemptAction( PuzzleWord const * puzzleWord );

protected:
    PuzzleWord const *  puzzleWord;
};

class FailedAttemptAction : public WordAttemptAction
{
public:

    FailedAttemptAction( PuzzleWord  const *  puzzleWord, int attemptNumber, int totalAttemptCount ) : WordAttemptAction( puzzleWord )
    {
        this->attemptNumber = attemptNumber;
        this->totalAttemptCount = totalAttemptCount;
    }

    int GetDisplayHeight() const;
    std::string GetDisplayText( int index ) const;

private:
    int attemptNumber;
    int totalAttemptCount;
};

class SuccessfulAttemptAction : public WordAttemptAction
{
public:
    SuccessfulAttemptAction( PuzzleWord const * puzzleWord ) : WordAttemptAction( puzzleWord )
    {

    }

    int GetDisplayHeight() const;

    std::string GetDisplayText( int index ) const;
};


#endif