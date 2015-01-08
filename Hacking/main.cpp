#include <cstdio>
#include <cstdlib>
#include <Windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <list>

template <typename T> T clamp(const T& value, const T& low, const T& high) 
{
  return value < low ? low : (value > high ? high : value); 
}

using std::string;
using std::vector;
using std::list;

const int LONGEST_PUZZLE_WORD = 15;
//const unsigned int upperLimit = 23;

const string introLines[] =
{
	"ROBCO INDUSTRIES (TM) TERMALINK PROTOCOL",
	"!!!! WARNING: LOCKOUT IMMINENT !!!!",
};
const int INTRO_LINE_COUNT = sizeof(introLines) / sizeof(string);

const int COLUMN_CHARACTER_WIDTH = 12;
const int HEX_CODE_LENGTH = 6;

const int COLUMN_COUNT = 2;
const int ANSWER_AREA_WIDTH = LONGEST_PUZZLE_WORD + 1;
const int TOTAL_COLUMN_WIDTH = HEX_CODE_LENGTH + 1 + COLUMN_CHARACTER_WIDTH + 1;
const int TOTAL_SCREEN_WIDTH = TOTAL_COLUMN_WIDTH * COLUMN_COUNT + ANSWER_AREA_WIDTH;

const int COLUMN_HEIGHT = 17;
const int LINES_BEFORE_COLUMNS = INTRO_LINE_COUNT + 3;
const int TOTAL_SCREEN_HEIGHT = COLUMN_HEIGHT + LINES_BEFORE_COLUMNS;

const int TOTAL_COLUMN_LINE_COUNT = COLUMN_HEIGHT * COLUMN_COUNT;

const COORD TOTAL_SIZE_COORD = { TOTAL_SCREEN_WIDTH, TOTAL_SCREEN_HEIGHT };
const COORD ORIGIN_COORD = { 0, 0 };
SMALL_RECT AREA_RECT = { ORIGIN_COORD.X, ORIGIN_COORD.Y, TOTAL_SCREEN_WIDTH, TOTAL_SCREEN_HEIGHT };

CHAR_INFO* displayBuffer = new CHAR_INFO[TOTAL_SCREEN_HEIGHT * TOTAL_SCREEN_WIDTH];

vector<string> characterBuffer( TOTAL_SCREEN_HEIGHT, string( TOTAL_SCREEN_WIDTH, ' ') );
vector<vector<bool> > highlightBuffer( TOTAL_SCREEN_HEIGHT, std::vector<bool>( TOTAL_SCREEN_WIDTH, false ) );

const WORD NORMAL_CHAR_ATTRIBUTES = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
const WORD HIGHLIGHTED_CHAR_ATTRIBUTES = BACKGROUND_GREEN | BACKGROUND_INTENSITY;

const int STARTING_ATTEMPT_COUNT = 5;
int attemptsRemaining = STARTING_ATTEMPT_COUNT;
int startingPointer;

const char* FILLER_CHARACTERS = "$-/?|=+&^%!@#&*{}[]()<>,.'\"";
const int FILLER_CHARACTER_COUNT = strlen(FILLER_CHARACTERS);

const int TOTAL_COLUMNS_CHARACTER_COUNT = COLUMN_COUNT * COLUMN_CHARACTER_WIDTH * COLUMN_HEIGHT;
string stringBuffer( TOTAL_COLUMNS_CHARACTER_COUNT, '#' );

COORD cursorCoord = {0, 0};

vector<string> hexAddresses( TOTAL_COLUMN_LINE_COUNT, std::string( HEX_CODE_LENGTH, '#' ) );

const int PUZZLE_WORD_LENGTH = 8;
struct PuzzleWord
{
	string text;
	bool isHighlighted;
	int position;
	COORD screenCoords[PUZZLE_WORD_LENGTH];
};

const int puzzleDifference = 4;
string solutionWord;
vector<PuzzleWord*> puzzleWords;

vector<string> attemptedWords;

PuzzleWord* currentHighlightedPuzzleWord;

int StringDiff( const string& _str1, const string& _str2 );
int StringDiff2( const string& _str1, const string& _str2 );
void RefreshBuffer();
void SwapBuffers();
void SetHexAddresses();
void SetPuzzleWords();
void PlacePuzzleWords();
void OnClickEvent();
void OnMouseMoveEvent();

int main( int argc, char* argv[] )
{
	srand( (unsigned int)time( nullptr ) );

	/*for ( int i = 0; i < TOTAL_SCREEN_HEIGHT; ++i )
	{
		characterBuffer[i].resize( TOTAL_SCREEN_WIDTH );
		characterBuffer[i].replace( 0, TOTAL_SCREEN_WIDTH, TOTAL_SCREEN_WIDTH, '_' );
	}*/

	for ( int i = 0; i < TOTAL_COLUMNS_CHARACTER_COUNT; ++i )
	{
		stringBuffer[i] = FILLER_CHARACTERS[ rand() % FILLER_CHARACTER_COUNT ];
	}

	SetPuzzleWords();
	PlacePuzzleWords();
	SetHexAddresses();

	HANDLE inputHandle = CreateFile( "CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
	HANDLE outputHandle = CreateFile( "CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );

	assert( inputHandle != INVALID_HANDLE_VALUE );
	assert( outputHandle != INVALID_HANDLE_VALUE );

	SetConsoleActiveScreenBuffer( inputHandle );
	DWORD flags;
	GetConsoleMode( inputHandle, &flags );

	DWORD fdwMode = flags & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
	assert( SetConsoleMode(inputHandle, fdwMode) );
	
	bool done = false;

	while ( done == false )
	{
		const int inputBufferSize = 255;
		INPUT_RECORD inputBuffer[inputBufferSize];
		DWORD eventsRead;

		if ( ReadConsoleInput( inputHandle, inputBuffer, inputBufferSize, &eventsRead ) )
		{
			unsigned int index = 0;
			while ( index < eventsRead )
			{
				INPUT_RECORD* record = &inputBuffer[index];
				if ( record->EventType == MOUSE_EVENT )
				{
					cursorCoord = record->Event.MouseEvent.dwMousePosition;
					cursorCoord.X = clamp( (int)cursorCoord.X, 0, TOTAL_SCREEN_WIDTH-1 );
					cursorCoord.Y = clamp( (int)cursorCoord.Y, 0, TOTAL_SCREEN_HEIGHT-1 );
					
					OnMouseMoveEvent();

					if ( record->Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED )
					{
						OnClickEvent();
					}
				}

				if ( record->EventType == KEY_EVENT )
				{
					WORD keyEvent = record->Event.KeyEvent.wVirtualKeyCode;

					if ( keyEvent == VK_ESCAPE )
					{
						done = true;
					}
				}
				++index;
			}
		}

		RefreshBuffer();
		SwapBuffers();
		WriteConsoleOutput( outputHandle, displayBuffer, TOTAL_SIZE_COORD, ORIGIN_COORD, &AREA_RECT );

		Sleep( 1000 / 24 );
	}

	for ( unsigned int i = 0; i < puzzleWords.size(); ++i )
	{
		delete puzzleWords[i];
	}
	delete displayBuffer;
	return 0;
}

int StringDiff( const string& _str1, const string& _str2 )
{
	assert( _str1.length() == _str2.length() );

	int differences = 0;
	int l = _str1.length();
	for ( int i = 0; i < l; ++i )
	{
		if ( _str1[i] != _str2[i] )
		{
			++differences;
		}
	}
	return differences;
}

int StringDiff2( const string& _str1, const string& _str2 )
{
	int differences = 0;
	for ( int i = 0; i < PUZZLE_WORD_LENGTH; ++i )
	{
		if ( _str1[i] != _str2[i] )
		{
			++differences;
		}
	}
	return differences;
}

void SwapBuffers()
{
	for ( unsigned int y = 0; y < TOTAL_SCREEN_HEIGHT; ++y )
	{
		for ( unsigned int x = 0; x < TOTAL_SCREEN_WIDTH; ++x )
		{
			int index = y * TOTAL_SCREEN_WIDTH + x;
			displayBuffer[index].Char.AsciiChar = characterBuffer[y][x];
			displayBuffer[index].Attributes = highlightBuffer[y][x] ? HIGHLIGHTED_CHAR_ATTRIBUTES : NORMAL_CHAR_ATTRIBUTES;
		}
	}
}

void RefreshBuffer()
{
	for ( int i = 0; i < INTRO_LINE_COUNT; ++i )
	{
		characterBuffer[i].replace( 0, introLines[i].size(), introLines[i] );
	}

	std::ostringstream outstr;
	outstr << attemptsRemaining << " ATTEMPT(S) LEFT:";
	string str = outstr.str();

	characterBuffer[INTRO_LINE_COUNT + 1].replace( 0, str.size(), str );

	int index = 0;
	for ( int x = 0; x < COLUMN_COUNT; ++x )
	{
		for ( int y = 0; y < COLUMN_HEIGHT; ++y )
		{
			assert(index < TOTAL_COLUMN_LINE_COUNT );

			characterBuffer[y + LINES_BEFORE_COLUMNS].replace( 
				x*TOTAL_COLUMN_WIDTH,
				HEX_CODE_LENGTH, 
				hexAddresses[index] );

			characterBuffer[y + LINES_BEFORE_COLUMNS].replace(
				x*TOTAL_COLUMN_WIDTH + HEX_CODE_LENGTH+1,
				COLUMN_CHARACTER_WIDTH,
				stringBuffer.substr( index * COLUMN_CHARACTER_WIDTH, COLUMN_CHARACTER_WIDTH ) );

			++index;
		}
	}

	for ( unsigned int i = 0; i < puzzleWords.size(); ++i )
	{
		for ( int j = 0; j < PUZZLE_WORD_LENGTH; ++j )
		{
			COORD coord = puzzleWords[i]->screenCoords[j];
			characterBuffer[coord.Y][coord.X] = puzzleWords[i]->text[j];
		}
	}

	for ( unsigned int y = 0; y < TOTAL_SCREEN_HEIGHT; ++y )
	{
		for ( unsigned int x = 0; x < TOTAL_SCREEN_WIDTH; ++x )
		{
			highlightBuffer[y][x] = false;
		}
	}

	for ( unsigned int i = 0; i < attemptedWords.size(); ++i )
	{

	}

	for ( unsigned int i = 0; i < puzzleWords.size(); ++i )
	{
		PuzzleWord* puzzleWord = puzzleWords[i];

		if ( puzzleWord->isHighlighted )
		{
			for ( int j = 0; j < PUZZLE_WORD_LENGTH; ++j )
			{
				COORD& pos = puzzleWord->screenCoords[j];
				highlightBuffer[pos.Y][pos.X] = true;
			}
		}
	}

	if ( currentHighlightedPuzzleWord != nullptr )
	{
		std::ostringstream outstr;
		outstr << "> " << currentHighlightedPuzzleWord->text;

		characterBuffer[ TOTAL_SCREEN_HEIGHT - 1 ].replace( TOTAL_COLUMN_WIDTH * COLUMN_COUNT + 1,
			outstr.str().length(),
			outstr.str() );
	}
	else
	{
		std::ostringstream outstr;
		outstr << "> " << string( PUZZLE_WORD_LENGTH, ' ' );

		characterBuffer[ TOTAL_SCREEN_HEIGHT - 1 ].replace( TOTAL_COLUMN_WIDTH * COLUMN_COUNT + 1,
			outstr.str().length(),
			outstr.str() );
	}

	highlightBuffer[cursorCoord.Y][cursorCoord.X] = true;
}

void SetHexAddresses()
{
	int address = rand() % 0xF000 + 0xFFF;
	for ( int i = 0; i < TOTAL_COLUMN_LINE_COUNT; ++i )
	{
		address += sizeof(char) * TOTAL_COLUMN_WIDTH;
		std::ostringstream stream;
		stream << "0x" << std::hex << address;
		hexAddresses[i] = stream.str();
	}
}

void SetPuzzleWords()
{
	std::ifstream fin( "dictionary" );
	if ( !fin.is_open() )
	{
		std::cout << "Error opening dictionary\n";
		exit( 1 );
	}

	list<string> words;
	string word;
	
	while ( fin >> word )
	{
		if ( word.length() == PUZZLE_WORD_LENGTH )
		{
			words.push_back( word );
		}
	}

	fin.close();

	int randIndex = rand() % words.size();

	//string solutionWord = words[randIndex];
	list<string>::iterator iter = words.begin();
	std::advance( iter, randIndex );
	solutionWord = *iter;

	vector<string> triedWords;
	vector<string> goodWords;
	//std::cout << "Trying " << solutionWord << "\n";
	for ( int i = 0; i < 10; )
	{
		//std::cout << "i:" << i << std::endl;
		if ( words.size() == 0 )
		{ // Ran out of usable words, reset and try again
			words.insert( words.end(), triedWords.begin(), triedWords.end() );
			triedWords.clear();
			puzzleWords.clear();

			//std::cout << "Ran out of usable words: Resetting\n";
			randIndex = rand() % words.size();
			//solutionWord = words[randIndex];
			iter = words.begin();
			std::advance( iter, randIndex );
			solutionWord = *iter;
			//std::cout << "Trying " << solutionWord << "\n";
			i = 0;
		}

		randIndex = rand() % words.size();
		//string randWord = words[randIndex];

		iter = words.begin();
		std::advance( iter, randIndex );
		string randWord = *iter;

		int diff = StringDiff2( randWord, solutionWord );
		if ( diff <= puzzleDifference )
		{ // If within tolerances, add to the list
			//std::cout << "Good word " << randWord << "\n";
			goodWords.push_back( randWord );
			++i;
		}
		// Add word to used words and remove from searchable list
		triedWords.push_back( randWord );
		//words.erase( words.begin() + randIndex );
		words.erase( iter );
	}

	//for ( list<string>::iterator iter = goodWords.begin(); iter != goodWords.end(); ++iter )
	for ( unsigned int i = 0; i < goodWords.size(); ++i )
	{
		PuzzleWord* puzzleWord = new PuzzleWord();
		//puzzleWord->text = *iter;
		puzzleWord->text = goodWords[i];
		puzzleWord->position = -1;
		puzzleWord->isHighlighted = false;
		puzzleWords.push_back( puzzleWord );
	}
	
}

void PlacePuzzleWords()
{
	assert( TOTAL_COLUMNS_CHARACTER_COUNT > PUZZLE_WORD_LENGTH );

	for ( unsigned int i = 0; i < puzzleWords.size(); )
	{
		// Theoretically, this might go infinite with the right conditions

		int place = rand() % ( TOTAL_COLUMNS_CHARACTER_COUNT - PUZZLE_WORD_LENGTH );
		bool badPlacement = false;
		for ( unsigned int j = 0; j < i; ++j )
		{
			
			if ( i != j 
			  && puzzleWords[j]->position != -1 
			  && std::abs( puzzleWords[j]->position - place ) < PUZZLE_WORD_LENGTH + 1 )
			{
				badPlacement = true;
				break;
			}
		}
		
		if ( !badPlacement )
		{
			puzzleWords[i]->position = place;
			++i;
		}
	}

	
	for ( unsigned int i = 0; i < puzzleWords.size(); ++i )
	{
		int place = puzzleWords[i]->position;

		int x = place % COLUMN_CHARACTER_WIDTH;
		int y = (place - x) / COLUMN_CHARACTER_WIDTH;

		int column = 0;

		while ( y >= COLUMN_HEIGHT )
		{
			++column;
			y -= COLUMN_HEIGHT;
		}

		// If the string goes over the end of a line
		if ( x + PUZZLE_WORD_LENGTH > COLUMN_CHARACTER_WIDTH )
		{
			// Separate it into two
			string chunk1 = puzzleWords[i]->text.substr( 0, COLUMN_CHARACTER_WIDTH - x + 1 );
			string chunk2 = puzzleWords[i]->text.substr( COLUMN_CHARACTER_WIDTH - x + 1 );
			
			for ( int j = 0; j < COLUMN_CHARACTER_WIDTH - x + 1; ++j )
			{
				COORD& coord = puzzleWords[i]->screenCoords[j];

				coord.X = x + HEX_CODE_LENGTH + column * TOTAL_COLUMN_WIDTH + j;
				coord.Y = y + LINES_BEFORE_COLUMNS;
			}

			// Render one half
			++y;
			if ( y >= COLUMN_HEIGHT )
			{
				++column;
				y -= COLUMN_HEIGHT;
			}

			int k = 0;
			for ( int j = COLUMN_CHARACTER_WIDTH - x + 1; j < PUZZLE_WORD_LENGTH; ++j )
			{
				COORD& coord = puzzleWords[i]->screenCoords[j];
				coord.X = HEX_CODE_LENGTH + 1 + column * TOTAL_COLUMN_WIDTH + k;
				coord.Y = y + LINES_BEFORE_COLUMNS;
				++k;
			}
		}
		else
		{
			for ( int j = 0; j < PUZZLE_WORD_LENGTH; ++j )
			{
				COORD& coord = puzzleWords[i]->screenCoords[j];
				coord.X = x + 1 + HEX_CODE_LENGTH + column * TOTAL_COLUMN_WIDTH + j;
				coord.Y = y + LINES_BEFORE_COLUMNS;
			}
		}
	}
}

void OnClickEvent()
{
	// Determine if the cursorCoord was over a word

	if ( currentHighlightedPuzzleWord != nullptr )
	{
		int diff = StringDiff2( currentHighlightedPuzzleWord->text, solutionWord );

		if ( diff == 0 )
		{

		}
		else
		{
			attemptedWords.push_back( currentHighlightedPuzzleWord->text );
			--attemptsRemaining;
		}
	}
}

void OnMouseMoveEvent()
{
	currentHighlightedPuzzleWord = nullptr;

	for ( unsigned int i = 0; i < puzzleWords.size(); ++i )
	{
		puzzleWords[i]->isHighlighted = false;
	}

	for ( unsigned int i = 0; i < puzzleWords.size() && currentHighlightedPuzzleWord == nullptr; ++i )
	{
		PuzzleWord* word = puzzleWords[i];

		for ( int j = 0; j < PUZZLE_WORD_LENGTH; ++j )
		{
			if ( word->screenCoords[j].X == cursorCoord.X
			  && word->screenCoords[j].Y == cursorCoord.Y )
			{
				word->isHighlighted = true;
				currentHighlightedPuzzleWord = word;
				break;
			}
		}
	}
}