#include "HackingController.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <list>
#include <boost/algorithm/clamp.hpp>

#include "strfunc.h"
#include "constants.h"
#include "PuzzleWord.h"

HackingController::HackingController(HackingModel* hackingModel, HackingView* hackingView)
{
	assert(hackingModel != nullptr);
	assert(hackingView != nullptr);

	this->hackingModel = hackingModel;
	this->hackingView = hackingView;

	this->displayBuffer = new CHAR_INFO[TOTAL_SCREEN_HEIGHT * TOTAL_SCREEN_WIDTH];

	this->characterBuffer.resize(TOTAL_SCREEN_HEIGHT, std::string(TOTAL_SCREEN_WIDTH, ' '));
	this->highlightBuffer.resize(TOTAL_SCREEN_HEIGHT, std::vector<bool>(TOTAL_SCREEN_WIDTH, false));

	this->attemptsRemaining = STARTING_ATTEMPT_COUNT;

	this->stringBuffer.resize(TOTAL_COLUMNS_CHARACTER_COUNT, '#');

	this->cursorCoord = { 0, 0 };

	this->hexAddresses.resize(TOTAL_COLUMN_LINE_COUNT, std::string(HEX_CODE_LENGTH, '#'));

	this->currentHighlightedPuzzleWord = nullptr;
}

HackingController::~HackingController()
{
	delete this->displayBuffer;
}

void HackingController::Run()
{
	for (int i = 0; i < TOTAL_COLUMNS_CHARACTER_COUNT; ++i)
	{
		stringBuffer[i] = FILLER_CHARACTERS[rand() % FILLER_CHARACTER_COUNT];
	}

	SetPuzzleWords();
	PlacePuzzleWords();
	SetHexAddresses();

	HANDLE inputHandle = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	HANDLE outputHandle = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	SetConsoleActiveScreenBuffer(inputHandle);
	DWORD flags;
	GetConsoleMode(inputHandle, &flags);

	DWORD fdwMode = flags & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
	assert(SetConsoleMode(inputHandle, fdwMode));

	bool done = false;

	while (!done)
	{
		const int INPUT_BUFFER_SIZE = 255;
		INPUT_RECORD inputBuffer[INPUT_BUFFER_SIZE];
		DWORD eventsRead;

		if (ReadConsoleInput(inputHandle, inputBuffer, INPUT_BUFFER_SIZE, &eventsRead))
		{
			unsigned int index = 0;
			while (index < eventsRead)
			{
				INPUT_RECORD* record = &inputBuffer[index];
				if (record->EventType == MOUSE_EVENT)
				{
					cursorCoord = record->Event.MouseEvent.dwMousePosition;
					cursorCoord.X = boost::algorithm::clamp((int)cursorCoord.X, 0, TOTAL_SCREEN_WIDTH - 1);
					cursorCoord.Y = boost::algorithm::clamp((int)cursorCoord.Y, 0, TOTAL_SCREEN_HEIGHT - 1);

					OnMouseMoveEvent();

					if (record->Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
					{
						OnClickEvent();
					}
				}

				if (record->EventType == KEY_EVENT)
				{
					WORD keyEvent = record->Event.KeyEvent.wVirtualKeyCode;

					if (keyEvent == VK_ESCAPE)
					{
						done = true;
					}
				}
				++index;
			}
		}

		RefreshBuffer();
		SwapBuffers();

		SMALL_RECT AREA_RECT = { ORIGIN_COORD.X, ORIGIN_COORD.Y, TOTAL_SCREEN_WIDTH, TOTAL_SCREEN_HEIGHT };
		WriteConsoleOutput(outputHandle, this->displayBuffer, TOTAL_SIZE_COORD, ORIGIN_COORD, &AREA_RECT);
		SetConsoleCursorPosition(outputHandle, GetOutputCursorCoord());
		Sleep(1000 / 24);
	}

	for (unsigned int i = 0; i < puzzleWords.size(); ++i)
	{
		delete puzzleWords[i];
	}
}

void HackingController::SwapBuffers()
{
	for (unsigned int y = 0; y < TOTAL_SCREEN_HEIGHT; ++y)
	{
		for (unsigned int x = 0; x < TOTAL_SCREEN_WIDTH; ++x)
		{
			int index = y * TOTAL_SCREEN_WIDTH + x;
			displayBuffer[index].Char.AsciiChar = this->characterBuffer[y][x];
			displayBuffer[index].Attributes = this->highlightBuffer[y][x] ? HIGHLIGHTED_CHAR_ATTRIBUTES : NORMAL_CHAR_ATTRIBUTES;
		}
	}
}

void HackingController::RefreshBuffer()
{
	for (int i = 0; i < INTRO_LINE_COUNT; ++i)
	{
		this->characterBuffer[i].replace(0, introLines[i].size(), introLines[i]);
	}

	std::ostringstream outstr;
	outstr << attemptsRemaining << " ATTEMPT(S) LEFT:";
	std::string str = outstr.str();

	this->characterBuffer[INTRO_LINE_COUNT + 1].replace(0, str.size(), str);

	int index = 0;
	for (int x = 0; x < COLUMN_COUNT; ++x)
	{
		for (int y = 0; y < COLUMN_HEIGHT; ++y)
		{
			assert(index < TOTAL_COLUMN_LINE_COUNT);

			this->characterBuffer[y + LINES_BEFORE_COLUMNS].replace(
				x*TOTAL_COLUMN_WIDTH,
				HEX_CODE_LENGTH,
				hexAddresses[index]);

			this->characterBuffer[y + LINES_BEFORE_COLUMNS].replace(
				x*TOTAL_COLUMN_WIDTH + HEX_CODE_LENGTH + 1,
				COLUMN_CHARACTER_WIDTH,
				stringBuffer.substr(index * COLUMN_CHARACTER_WIDTH, COLUMN_CHARACTER_WIDTH));

			++index;
		}
	}

	for (unsigned int i = 0; i < puzzleWords.size(); ++i)
	{
		for (int j = 0; j < PUZZLE_WORD_LENGTH; ++j)
		{
			COORD coord = puzzleWords[i]->GetScreenCoord(j);
			this->characterBuffer[coord.Y][coord.X] = puzzleWords[i]->GetText()[j];
		}
	}

	for (unsigned int y = 0; y < TOTAL_SCREEN_HEIGHT; ++y)
	{
		for (unsigned int x = 0; x < TOTAL_SCREEN_WIDTH; ++x)
		{
			this->highlightBuffer[y][x] = false;
		}
	}

	for (unsigned int i = 0; i < attemptedWords.size(); ++i)
	{

	}

	for (unsigned int i = 0; i < puzzleWords.size(); ++i)
	{
		PuzzleWord* puzzleWord = puzzleWords[i];

		if (puzzleWord->IsHighlighted())
		{
			for (int j = 0; j < PUZZLE_WORD_LENGTH; ++j)
			{
				COORD& pos = puzzleWord->GetScreenCoord(j);
				this->highlightBuffer[pos.Y][pos.X] = true;
			}
		}
	}

	if (currentHighlightedPuzzleWord != nullptr)
	{
		std::ostringstream outstr;
		outstr << "> " << currentHighlightedPuzzleWord->GetText();

		this->characterBuffer[TOTAL_SCREEN_HEIGHT - 1].replace(TOTAL_COLUMN_WIDTH * COLUMN_COUNT + 1,
			outstr.str().length(),
			outstr.str());
	}
	else
	{
		std::ostringstream outstr;
		outstr << "> " << std::string(PUZZLE_WORD_LENGTH, ' ');

		this->characterBuffer[TOTAL_SCREEN_HEIGHT - 1].replace(TOTAL_COLUMN_WIDTH * COLUMN_COUNT + 1,
			outstr.str().length(),
			outstr.str());
	}

	this->highlightBuffer[cursorCoord.Y][cursorCoord.X] = true;
}

void HackingController::SetHexAddresses()
{
	int address = rand() % 0xF000 + 0xFFF;
	for (int i = 0; i < TOTAL_COLUMN_LINE_COUNT; ++i)
	{
		address += sizeof(char) * TOTAL_COLUMN_WIDTH;
		std::ostringstream stream;
		stream << "0x" << std::hex << address;
		hexAddresses[i] = stream.str();
	}
}

void HackingController::SetPuzzleWords()
{
	std::list<std::string> words;
	std::vector<std::string> triedWords;
	std::vector<std::string> goodWords;

	GetSampleWordList(triedWords);
	assert(triedWords.size() > WORD_COUNT);

	std::list<std::string>::iterator iter = words.begin();

	while (true)
	{
		if (words.size() == 0)
		{ // Ran out of usable words, reset and try again
			words.insert(words.end(), triedWords.begin(), triedWords.end());
			triedWords.clear();
			puzzleWords.clear();
			goodWords.clear();

			iter = words.begin();
			std::advance(iter, rand() % words.size());
			solutionWord = *iter;
		}

		// Get a random word
		iter = words.begin();
		std::advance(iter, rand() % words.size());
		std::string randWord = *iter;

		int diff = StringDiff(randWord, solutionWord);
		if (diff <= PUZZLE_DIFFERENCE)
		{ // If within tolerances, add to the list
			goodWords.push_back(randWord);
			if (goodWords.size() >= WORD_COUNT - 1)
			{// We only need N-1 because the last word is the solution word
				break;
			}
		}
		// Add word to used words and remove from searchable list
		triedWords.push_back(randWord);
		words.erase(iter);
	}

	puzzleWords.push_back(new PuzzleWord(solutionWord));
	for (unsigned int i = 0; i < WORD_COUNT - 1; ++i)
	{
		puzzleWords.push_back(new PuzzleWord(goodWords[i]));
	}
}

void HackingController::PlacePuzzleWords()
{
	assert(TOTAL_COLUMNS_CHARACTER_COUNT > PUZZLE_WORD_LENGTH);

	for (unsigned int i = 0; i < puzzleWords.size(); )
	{
		// Theoretically, this might go infinite with the right conditions
		int place = rand() % (TOTAL_COLUMNS_CHARACTER_COUNT - PUZZLE_WORD_LENGTH);
		bool badPlacement = false;
		for (unsigned int j = 0; j < i; ++j)
		{
			if (i != j
				&& puzzleWords[j]->GetPosition() != -1
				&& std::abs(puzzleWords[j]->GetPosition() - place) < PUZZLE_WORD_LENGTH + 1)
			{
				badPlacement = true;
				break;
			}
		}

		if (!badPlacement)
		{
			puzzleWords[i]->SetPosition(place);
			++i;
		}
	}

	for (unsigned int i = 0; i < puzzleWords.size(); ++i)
	{
		int place = puzzleWords[i]->GetPosition();

		int x = place % COLUMN_CHARACTER_WIDTH;
		int y = (place - x) / COLUMN_CHARACTER_WIDTH;

		int column = 0;

		while (y >= COLUMN_HEIGHT)
		{
			++column;
			y -= COLUMN_HEIGHT;
		}

		// If the std::string goes over the end of a line
		if (x + PUZZLE_WORD_LENGTH > COLUMN_CHARACTER_WIDTH)
		{
			// Separate it into two
			std::string chunk1 = puzzleWords[i]->GetText().substr(0, COLUMN_CHARACTER_WIDTH - x + 1);
			std::string chunk2 = puzzleWords[i]->GetText().substr(COLUMN_CHARACTER_WIDTH - x + 1);

			for (int j = 0; j < COLUMN_CHARACTER_WIDTH - x + 1; ++j)
			{
				COORD& coord = puzzleWords[i]->GetScreenCoord(j);

				coord.X = x + HEX_CODE_LENGTH + column * TOTAL_COLUMN_WIDTH + j;
				coord.Y = y + LINES_BEFORE_COLUMNS;
			}

			// Render one half
			++y;
			if (y >= COLUMN_HEIGHT)
			{
				++column;
				y -= COLUMN_HEIGHT;
			}

			int k = 0;
			for (int j = COLUMN_CHARACTER_WIDTH - x + 1; j < PUZZLE_WORD_LENGTH; ++j)
			{
				COORD& coord = puzzleWords[i]->GetScreenCoord(j);
				coord.X = HEX_CODE_LENGTH + 1 + column * TOTAL_COLUMN_WIDTH + k;
				coord.Y = y + LINES_BEFORE_COLUMNS;
				++k;
			}
		}
		else
		{
			for (int j = 0; j < PUZZLE_WORD_LENGTH; ++j)
			{
				COORD& coord = puzzleWords[i]->GetScreenCoord(j);
				coord.X = x + 1 + HEX_CODE_LENGTH + column * TOTAL_COLUMN_WIDTH + j;
				coord.Y = y + LINES_BEFORE_COLUMNS;
			}
		}
	}
}

void HackingController::OnClickEvent()
{
	// Determine if the cursorCoord was over a word

	if (currentHighlightedPuzzleWord != nullptr)
	{
		int diff = StringDiff(currentHighlightedPuzzleWord->GetText(), solutionWord);

		if (diff == 0)
		{

		}
		else
		{
			attemptedWords.push_back(currentHighlightedPuzzleWord->GetText());
			--attemptsRemaining;
		}
	}
}

void HackingController::OnMouseMoveEvent()
{
	currentHighlightedPuzzleWord = nullptr;

	for (unsigned int i = 0; i < puzzleWords.size(); ++i)
	{
		puzzleWords[i]->SetHighlight(false);
	}

	for (unsigned int i = 0; i < puzzleWords.size() && currentHighlightedPuzzleWord == nullptr; ++i)
	{
		PuzzleWord* word = puzzleWords[i];

		for (int j = 0; j < PUZZLE_WORD_LENGTH; ++j)
		{
			if (word->GetScreenCoord(j).X == cursorCoord.X
				&& word->GetScreenCoord(j).Y == cursorCoord.Y)
			{
				word->SetHighlight(true);
				currentHighlightedPuzzleWord = word;
				break;
			}
		}
	}
}

void HackingController::GetSampleWordList(std::vector<std::string>& _out)
{
	std::ifstream fin("dictionary");
	if (!fin.is_open())
	{
		std::cout << "Error opening dictionary\n";
		exit(1);
	}

	std::string word;
	while (fin >> word)
	{
		if (word.length() == PUZZLE_WORD_LENGTH)
		{
			_out.push_back(word);
		}
	}
	fin.close();
}

COORD HackingController::GetOutputCursorCoord()
{
	COORD c = { TOTAL_COLUMN_WIDTH * COLUMN_COUNT + 1, TOTAL_SCREEN_HEIGHT - 1 };
	return c;
}