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
#include "HackingModel.h"
#include "HackingView.h"

HackingController::HackingController(HackingModel* hackingModel, HackingView* hackingView)
{
	assert(hackingModel != nullptr);
	assert(hackingView != nullptr);


	this->hackingModel = hackingModel;
	this->hackingView = hackingView;

	this->inputHandle = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	this->outputHandle = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	this->hackingView->SetOutputHandle(outputHandle);

	this->cursorCoord = { 0, 0 };
}

HackingController::~HackingController()
{
}

void HackingController::Run()
{
	SetConsoleActiveScreenBuffer(this->inputHandle);
	DWORD flags;
	GetConsoleMode(this->inputHandle, &flags);

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

					this->hackingModel->OnMouseMoveEvent(this->cursorCoord);

					if (record->Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
					{
						this->hackingModel->OnClickEvent();
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

		this->hackingView->Render(this->cursorCoord);

		SetConsoleCursorPosition(outputHandle, GetOutputCursorCoord());
		Sleep(1000 / 24);
	}
}

COORD HackingController::GetOutputCursorCoord()
{
	COORD c = { TOTAL_COLUMN_WIDTH * COLUMN_COUNT + 1, TOTAL_SCREEN_HEIGHT - 1 };
	return c;
}