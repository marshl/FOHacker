#include "HackingView.h"

#include <cassert>
#include <sstream>
#include <algorithm>

#include "HackingModel.h"
#include "PuzzleWord.h"
#include "DifficultyLevel.h"


HackingView::HackingView(const HackingModel * const hackingModel) : hackingModel(hackingModel)
{
	assert(hackingModel != nullptr);

	this->displayBuffer.resize(this->GetScreenHeight() * this->GetScreenWidth());

	this->characterBuffer.resize(this->GetScreenHeight(), std::string(this->GetScreenWidth(), ' '));
	this->highlightBuffer.resize(this->GetScreenHeight(), std::vector<bool>(this->GetScreenWidth(), false));

	this->stringBuffer.resize(this->hackingModel->GetTotalColumnCharacterCount(), '#');

	SetConsoleScreenBufferSize(this->outputHandle, { (short)this->GetScreenWidth(), (short)this->GetScreenHeight() });

	this->SetHexAddresses();
}


HackingView::~HackingView()
{
}

void HackingView::Render(COORD cursorCoord)
{
	this->ClearBuffer();
	this->RefreshBuffer(cursorCoord);

	const WORD NORMAL_CHAR_ATTRIBUTES = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	const WORD HIGHLIGHTED_CHAR_ATTRIBUTES = BACKGROUND_GREEN | BACKGROUND_INTENSITY;

	for (int y = 0; y < this->GetScreenHeight(); ++y)
	{
		for (int x = 0; x < this->GetScreenWidth(); ++x)
		{
			const int index = y * this->GetScreenWidth() + x;
			displayBuffer[index].Char.AsciiChar = this->characterBuffer[y][x];
			displayBuffer[index].Attributes = this->highlightBuffer[y][x] ? HIGHLIGHTED_CHAR_ATTRIBUTES : NORMAL_CHAR_ATTRIBUTES;
		}
	}

	const COORD ORIGIN_COORD = { 0,0 };
	SMALL_RECT AREA_RECT = { ORIGIN_COORD.X, ORIGIN_COORD.Y, (short)this->GetScreenWidth(), (short)this->GetScreenHeight() };
	const COORD screenSize = { (short)this->GetScreenWidth(), (short)this->GetScreenHeight() };
	WriteConsoleOutput(outputHandle, this->displayBuffer.data(), screenSize, ORIGIN_COORD, &AREA_RECT);
}

void HackingView::SetOutputHandle(HANDLE handle)
{
	this->outputHandle = handle;
}

int HackingView::GetScreenHeight() const
{
	return this->hackingModel->GetColumnHeight() + this->GetLineCountAboveColumns();
}

int HackingView::GetScreenWidth() const
{
	const int COLUMN_CHARACTER_WIDTH = 12;
	const int HEX_CODE_LENGTH = 6;

	const int ANSWER_AREA_WIDTH = this->hackingModel->GetMaximumWordLength() + 1;
	const int TOTAL_COLUMN_WIDTH = HEX_CODE_LENGTH + 1 + COLUMN_CHARACTER_WIDTH + 1;
	const int TOTAL_SCREEN_WIDTH = 80;// TOTAL_COLUMN_WIDTH * COLUMN_COUNT + ANSWER_AREA_WIDTH;

	return 80;
}

int HackingView::GetTotalColumnWidth() const
{
	return this->hackingModel->GetColumnWidth() + 1 + this->GetHexCodeLength() + 1;
}

int HackingView::GetHexCodeLength() const
{
	return 6;
}

short HackingView::GetLineCountAboveColumns() const
{
	return 3;
}

const std::vector<std::string> HackingView::GetIntroText() const
{
	return
		std::vector<std::string>{
		"ROBCO INDUSTRIES (TM) TERMALINK PROTOCOL",
			"!!!! WARNING: LOCKOUT IMMINENT !!!!",
	};
}

void HackingView::ClearBuffer()
{
	this->characterBuffer.clear();
	this->characterBuffer.resize(this->GetScreenHeight(), std::string(this->GetScreenWidth(), ' '));
	this->highlightBuffer.clear();
	this->highlightBuffer.resize(this->GetScreenHeight(), std::vector<bool>(this->GetScreenWidth(), false));
}

void HackingView::RefreshBuffer(COORD cursorCoord)
{
	if (this->hackingModel->GetCurrentDifficulty() == nullptr)
	{
		this->RenderDifficultyScreen(cursorCoord);
	}
	else
	{
		this->RenderGameScreen(cursorCoord);
	}
}

void HackingView::RenderDifficultyScreen(COORD cursorCoord)
{
	for (int i = 0; i < this->hackingModel->GetDifficultyCount(); ++i)
	{
		const DifficultyLevel * const difficulty = this->hackingModel->GetDifficultyLevelWithIndex(i);

		COORD textPosition = { (short)(this->GetScreenWidth() - difficulty->GetName().size()) / 2, (short)i * 2 + 5 };
		bool highlighted = this->IsCoordInString(cursorCoord, textPosition, difficulty->GetName().size());

		this->RenderText(textPosition, difficulty->GetName(), highlighted);
	}

	this->highlightBuffer[cursorCoord.Y][cursorCoord.X] = true;
}

void HackingView::RenderGameScreen(COORD cursorCoord)
{
	for (int i = 0; i < (int)this->GetIntroText().size(); ++i)
	{
		this->characterBuffer[i].replace(0, this->GetIntroText()[i].size(), this->GetIntroText()[i]);
	}

	std::ostringstream outstr;
	outstr << this->hackingModel->GetAttemptsRemaining() << " ATTEMPT(S) LEFT:";
	this->RenderText({ 0, this->GetLineCountAboveColumns() + 1 }, outstr.str(), false);

	for (int x = 0; x < this->hackingModel->GetColumnCount(); ++x)
	{
		for (int y = 0; y < this->hackingModel->GetColumnWidth(); ++y)
		{
			const std::string& hexCode = this->hexAddresses[x * this->hackingModel->GetColumnHeight() + y];
			COORD coord = { (short)(x * this->GetTotalColumnWidth()), (short)(y + this->GetLineCountAboveColumns()) };
			this->RenderText(coord, hexCode, false);
		}
	}

	for (int i = 0; i < this->hackingModel->GetPuzzleWordCount(); ++i)
	{
		for (int j = 0; j < this->hackingModel->GetCurrentDifficulty()->GetWordLength(); ++j)
		{
			const PuzzleWord * const puzzleWord = this->hackingModel->GetPuzzleWord(i);
			COORD coord = puzzleWord->GetScreenCoord(j);
			this->characterBuffer[coord.Y][coord.X] = puzzleWord->GetText()[j];
		}
	}

	for (int y = 0; y < this->GetScreenHeight(); ++y)
	{
		for (int x = 0; x < this->GetScreenWidth(); ++x)
		{
			this->highlightBuffer[y][x] = false;
		}
	}

	for (int i = 0; i < this->hackingModel->GetAttemptedWordCount(); ++i)
	{

	}

	for (int i = 0; i < this->hackingModel->GetPuzzleWordCount(); ++i)
	{
		PuzzleWord* puzzleWord = this->hackingModel->GetPuzzleWord(i);

		if (puzzleWord->IsHighlighted())
		{
			for (int j = 0; j < this->hackingModel->GetCurrentDifficulty()->GetWordLength(); ++j)
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

		this->characterBuffer[this->GetScreenHeight() - 1].replace(this->GetTotalColumnWidth() * this->hackingModel->GetColumnCount() + 1,
			outstr.str().length(),
			outstr.str());
	}
	else
	{
		std::ostringstream outstr;
		outstr << "> ";

		this->characterBuffer[this->GetScreenHeight() - 1].replace(this->GetTotalColumnWidth()* this->hackingModel->GetColumnCount() + 1,
			outstr.str().length(),
			outstr.str());
	}

	this->highlightBuffer[cursorCoord.Y][cursorCoord.X] = true;
}

void HackingView::RenderText(COORD position, std::string text, bool isHighlighted)
{
	this->characterBuffer[position.Y].replace(position.X, text.size(), text);
	for (unsigned int i = 0; i < text.size(); ++i)
	{
		this->highlightBuffer[position.Y][position.X + i] = isHighlighted;
	}
}

bool HackingView::IsCoordInString(const COORD & coord, const COORD & textPosition, int textLength) const
{
	return coord.Y == textPosition.Y && coord.X >= textPosition.X && coord.X < textPosition.X + textLength;
}

bool HackingView::IsCoordInArea(const COORD & position, const COORD & start, const COORD & end) const
{
	return position.Y >= start.Y && position.X <= end.Y && position.X >= start.X && position.X <= end.X;
}


void HackingView::SetHexAddresses()
{
	this->hexAddresses.resize(this->hackingModel->GetTotalLineCount(), std::string(this->GetHexCodeLength(), '#'));

	int address = rand() % 0xF000 + 0xFFF;
	for (int i = 0; i < this->hackingModel->GetTotalLineCount(); ++i)
	{
		address += sizeof(char) * this->GetTotalColumnWidth();
		std::ostringstream stream;
		stream << "0x" << std::hex << address;
		hexAddresses[i] = stream.str();
		std::transform(hexAddresses[i].begin() + 2, hexAddresses[i].end(), hexAddresses[i].begin() + 2, ::toupper);
	}
}
