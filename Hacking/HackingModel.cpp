#include "HackingModel.h"

#include <list>
#include <sstream>
#include <cassert>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <regex>
#include <sstream>

#include "strfunc.h"
#include "PuzzleWord.h"
#include "DifficultyLevel.h"

HackingModel::HackingModel()
{
    this->currentDifficulty = nullptr;

    this->InitialiseDifficultyLevels();
}

HackingModel::~HackingModel()
{
    for ( unsigned int i = 0; i < this->puzzleWords.size(); ++i )
    {
        delete this->puzzleWords[i];
    }

    for ( unsigned int i = 0; i < this->difficultyLevels.size(); ++i )
    {
        delete this->difficultyLevels[i];
    }
}

int HackingModel::GetAttemptsRemaining() const
{
    return this->attemptsRemaining;
}

const int HackingModel::GetPuzzleWordCount() const
{
    return this->puzzleWords.size();
}

const int HackingModel::GetAttemptedWordCount() const
{
    return this->attemptedWords.size();
}

PuzzleWord * const HackingModel::GetPuzzleWord( int index ) const
{
    return this->puzzleWords[index];
}

DifficultyLevel * HackingModel::GetCurrentDifficulty() const
{
    return this->currentDifficulty;
}

DifficultyLevel * HackingModel::GetDifficultyLevelWithIndex( int index ) const
{
    return this->difficultyLevels[index];
}

int HackingModel::GetDifficultyCount() const
{
    return this->difficultyLevels.size();
}

void HackingModel::SetDifficultyLevel( DifficultyLevel * difficulty )
{
    this->currentDifficulty = difficulty;

    this->SetPuzzleWords();
    this->PlacePuzzleWords();
    this->RandomiseFillerLetters();
    this->SetupMatchingBrackets();

    this->attemptsRemaining = this->currentDifficulty->GetStartingAttemptCount();
}

const int HackingModel::GetColumnWidth() const
{
    return 12;
}

const int HackingModel::GetColumnHeight() const
{
    return 17;
}

const int HackingModel::GetColumnCount() const
{
    return 2;
}

const int HackingModel::GetTotalLineCount() const
{
    return this->GetColumnHeight() * this->GetColumnCount();
}

const int HackingModel::GetTotalColumnCharacterCount() const
{
    return this->GetColumnCount() * this->GetColumnWidth() * this->GetColumnHeight();;
}

const int HackingModel::GetMaximumWordLength() const
{
    int maximumLength = 0;
    for ( int i = 0; i < ( int )this->difficultyLevels.size(); ++i )
    {
        if ( this->GetDifficultyLevelWithIndex( i )->GetWordLength() > maximumLength )
        {
            maximumLength = this->GetDifficultyLevelWithIndex( i )->GetWordLength();
        }
    }

    return maximumLength;
}

const std::string& HackingModel::GetFillerText( int columnIndex, int rowIndex ) const
{
    return this->fillerCharacters[columnIndex][rowIndex];
}

void HackingModel::SetPuzzleWords()
{
    std::vector<std::string> allWords;
    std::vector<std::string> solutionWords;
    std::vector<std::string> triedWords;

    this->GetSampleWordList( allWords );

    assert( (int)allWords.size() >= this->GetCurrentDifficulty()->GetWordCount() );

    std::random_shuffle( allWords.begin(), allWords.end() );

    while ( true )
    {
        std::string randWord = allWords.back();
        bool wordAdded = false;

        // Add the word if there are no others
        if ( solutionWords.size() == 0 )
        {
            solutionWords.push_back( randWord );
            wordAdded = true;
        }
        else
        {
            for each( const std::string& solutionWord in solutionWords )
            {
                int difference = StringDiff( solutionWord, randWord );
                if ( difference > this->GetCurrentDifficulty()->GetLetterDifference() || difference == 0 )
                {
                    continue;
                }

                solutionWords.push_back( randWord );
                wordAdded = true;

                if ( solutionWords.size() == this->GetCurrentDifficulty()->GetWordCount() )
                {
                    for each( std::string word in solutionWords )
                    {
                        this->puzzleWords.push_back( new PuzzleWord( word ) );
                    }
                    return;
                }

                // More words are needed, conitnue the loop
                break;
            }
        }

        triedWords.push_back( randWord );
        allWords.pop_back();

        if ( allWords.size() == 0 )
        {
            allWords.insert( allWords.begin(), triedWords.begin(), triedWords.end() );
            triedWords.clear();
            solutionWords.clear();
            std::random_shuffle( allWords.begin(), allWords.end() );
        }
    }
}

void HackingModel::PlacePuzzleWords()
{
    assert( this->GetTotalColumnCharacterCount() > this->GetCurrentDifficulty()->GetWordLength() );

    for ( unsigned int puzzleWordIndex = 0; puzzleWordIndex < puzzleWords.size(); )
    {
        PuzzleWord* puzzleWord = this->GetPuzzleWord( puzzleWordIndex );

        // Theoretically, this might go infinite with the right conditions
        int place = rand() % ( this->GetTotalColumnCharacterCount() - this->GetCurrentDifficulty()->GetWordLength() );
        bool badPlacement = false;
        // Check for position collisions with words that have already been placed
        for ( unsigned int placedWordIndex = 0; placedWordIndex < puzzleWordIndex; ++placedWordIndex )
        {
            if ( std::abs( puzzleWords[placedWordIndex]->GetPosition() - place ) < this->GetCurrentDifficulty()->GetWordLength() + 1 )
            {
                badPlacement = true;
                break;
            }
        }

        if ( !badPlacement )
        {
            puzzleWord->SetPosition( place, this->GetColumnCount(), this->GetColumnHeight(), this->GetColumnWidth() );
            ++puzzleWordIndex;
        }
    }
}

void HackingModel::GetSampleWordList( std::vector<std::string>& _out )
{
    std::ifstream fin( "dictionary" );
    if ( !fin.is_open() )
    {
        std::cout << "Error opening dictionary\n";
        exit( 1 );
    }

    std::string word;
    while ( fin >> word )
    {
        if ( word.length() == this->GetCurrentDifficulty()->GetWordLength() )
        {
            std::transform( word.begin(), word.end(), word.begin(), ::toupper );
            _out.push_back( word );
        }
    }

    fin.close();
}

void HackingModel::InitialiseDifficultyLevels()
{
    this->difficultyLevels.push_back( new DifficultyLevel( 0, "Very Easy", 9, 5, 2, 4 ) );
    this->difficultyLevels.push_back( new DifficultyLevel( 1, "Easy", 9, 7, 3, 4 ) );
    this->difficultyLevels.push_back( new DifficultyLevel( 2, "Average", 8, 9, 3, 4 ) );
    this->difficultyLevels.push_back( new DifficultyLevel( 3, "Hard", 7, 11, 4, 4 ) );
    this->difficultyLevels.push_back( new DifficultyLevel( 4, "Very Hard", 7, 13, 4, 4 ) );
}

void HackingModel::RandomiseFillerLetters()
{
    const std::string FILLER_CHARACTERS = "$-/?|=+&^%!@#&*{}[]()<>,.'\"";
    const int FILLER_CHARACTER_COUNT = FILLER_CHARACTERS.length();

    this->fillerCharacters.resize( this->GetColumnCount() );

    for ( int columnIndex = 0; columnIndex < this->GetColumnCount(); ++columnIndex )
    {
        this->fillerCharacters[columnIndex].resize( this->GetColumnHeight() );
        for ( int rowIndex = 0; rowIndex < this->GetColumnHeight(); ++rowIndex )
        {
            std::string filler( this->GetColumnWidth(), '#' );
            for ( int fillerIndex = 0; fillerIndex < this->GetColumnWidth(); ++fillerIndex )
            {
                filler[fillerIndex] = FILLER_CHARACTERS[rand() % FILLER_CHARACTER_COUNT];
            }

            this->fillerCharacters[columnIndex][rowIndex] = filler;
        }
    }
}

void HackingModel::SetupMatchingBrackets()
{
    for ( int columnIndex = 0; columnIndex < this->GetColumnCount(); ++columnIndex )
    {
        for ( int rowIndex = 0; rowIndex < this->GetColumnHeight(); ++rowIndex )
        {
            std::vector<MatchingBracket> brackets = this->GetMatchingBracketsForLine( columnIndex, rowIndex );
            this->matchingBrackets.insert( this->matchingBrackets.end(), brackets.begin(), brackets.end() );
        }
    }
}

std::vector<MatchingBracket> HackingModel::GetMatchingBracketsForLine( int columnIndex, int rowIndex )
{
    std::vector<MatchingBracket> result;

    const std::string validStartingChars = "<{[(";
    const std::string validEndingChars = ">}])";

    assert( validStartingChars.size() == validEndingChars.size() );

    std::string fillerText = this->GetFillerText( columnIndex, rowIndex );

    for ( int charIndex = 0; charIndex < (int)validStartingChars.size(); ++charIndex )
    {
        size_t closingOffset = fillerText.length();
        while ( closingOffset >= 1 && ( closingOffset = fillerText.rfind( validEndingChars[charIndex], closingOffset - 1 ) ) != std::string::npos )
        {
            size_t openingOffset = closingOffset;
            size_t tempOffset = openingOffset;
            while ( openingOffset != 0 && ( tempOffset = fillerText.rfind( validStartingChars[charIndex], openingOffset - 1 ) ) != std::string::npos )
            {
                openingOffset = tempOffset;
                char openingSymbol = fillerText[openingOffset];
                char closingSymbol = fillerText[closingOffset];

                MatchingBracket match( openingSymbol, closingSymbol, columnIndex, rowIndex, openingOffset, closingOffset );
                result.push_back( match );
            }
        }
    }

    return result;
}