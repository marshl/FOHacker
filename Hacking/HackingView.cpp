#include "HackingView.h"

#include <cassert>
#include <sstream>

#include "constants.h"
#include "HackingModel.h"
#include "PuzzleWord.h"


HackingView::HackingView(HackingModel* hackingModel)
{
	assert(hackingModel != nullptr);
	this->hackingModel = hackingModel;

	this->displayBuffer = new CHAR_INFO[TOTAL_SCREEN_HEIGHT * TOTAL_SCREEN_WIDTH];

	this->characterBuffer.resize(TOTAL_SCREEN_HEIGHT, std::string(TOTAL_SCREEN_WIDTH, ' '));
	this->highlightBuffer.resize(TOTAL_SCREEN_HEIGHT, std::vector<bool>(TOTAL_SCREEN_WIDTH, false));

	this->stringBuffer.resize(TOTAL_COLUMNS_CHARACTER_COUNT, '#');

	for (int i = 0; i < TOTAL_COLUMNS_CHARACTER_COUNT; ++i)
	{
		stringBuffer[i] = FILLER_CHARACTERS[rand() % FILLER_CHARACTER_COUNT];
	}

}


HackingView::~HackingView()
{
	delete this->displayBuffer;
}

void HackingView::Render(COORD cursorCoord)
{
	this->RefreshBuffer(cursorCoord);
	this->SwapBuffers();

	SMALL_RECT AREA_RECT = { ORIGIN_COORD.X, ORIGIN_COORD.Y, TOTAL_SCREEN_WIDTH, TOTAL_SCREEN_HEIGHT };
	WriteConsoleOutput(outputHandle, this->displayBuffer, TOTAL_SIZE_COORD, ORIGIN_COORD, &AREA_RECT);
}

void HackingView::SetOutputHandle(HANDLE handle)
{
	this->outputHandle = handle;
}


void HackingView::SwapBuffers()
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

void HackingView::RefreshBuffer(COORD cursorCoord)
{
	for (int i = 0; i < INTRO_LINE_COUNT; ++i)
	{
		this->characterBuffer[i].replace(0, introLines[i].size(), introLines[i]);
	}

	std::ostringstream outstr;
	outstr << this->hackingModel->GetAttemptsRemaining() << " ATTEMPT(S) LEFT:";
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
				this->hackingModel->GetHexAddress(index));

			this->characterBuffer[y + LINES_BEFORE_COLUMNS].replace(
				x*TOTAL_COLUMN_WIDTH + HEX_CODE_LENGTH + 1,
				COLUMN_CHARACTER_WIDTH,
				stringBuffer.substr(index * COLUMN_CHARACTER_WIDTH, COLUMN_CHARACTER_WIDTH));

			++index;
		}
	}

	for (unsigned int i = 0; i < this->hackingModel->GetPuzzleWordCount(); ++i)
	{
		for (int j = 0; j < PUZZLE_WORD_LENGTH; ++j)
		{
			const PuzzleWord * const puzzleWord = this->hackingModel->GetPuzzleWord(i);
			COORD coord = puzzleWord->GetScreenCoord(j);
			this->characterBuffer[coord.Y][coord.X] = puzzleWord->GetText()[j];
		}
	}

	for (unsigned int y = 0; y < TOTAL_SCREEN_HEIGHT; ++y)
	{
		for (unsigned int x = 0; x < TOTAL_SCREEN_WIDTH; ++x)
		{
			this->highlightBuffer[y][x] = false;
		}
	}

	for (unsigned int i = 0; i < this->hackingModel->GetAttemptedWordCount(); ++i)
	{

	}

	for (unsigned int i = 0; i < this->hackingModel->GetPuzzleWordCount(); ++i)
	{
		PuzzleWord* puzzleWord = this->hackingModel->GetPuzzleWord(i);

		if (puzzleWord->IsHighlighted())
		{
			for (int j = 0; j < PUZZLE_WORD_LENGTH; ++j)
			{
				COORD& pos = puzzleWord->GetScreenCoord(j);
				this->highlightBuffer[pos.Y][pos.X] = true;
			}
		}
	}

	if (this->hackingModel->GetSelectedPuzzleWord() != nullptr)
	{
		std::ostringstream outstr;
		outstr << "> " << this->hackingModel->GetSelectedPuzzleWord()->GetText();

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