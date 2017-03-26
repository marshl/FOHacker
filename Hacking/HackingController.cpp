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
	CloseHandle(this->inputHandle);
	CloseHandle(this->outputHandle);
}

void HackingController::Run()
{
	bool done = false;

	SetConsoleActiveScreenBuffer(this->inputHandle);
	DWORD flags;
	GetConsoleMode(this->inputHandle, &flags);

	DWORD fdwMode = flags & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
	if (!SetConsoleMode(inputHandle, fdwMode))
	{
		done = true;
	}

	CONSOLE_CURSOR_INFO cursorInfo;
	cursorInfo.bVisible = FALSE;
	cursorInfo.dwSize = 100;
	SetConsoleCursorInfo(this->outputHandle, &cursorInfo);

	while (!done)
	{
		const int INPUT_BUFFER_SIZE = 255;
		INPUT_RECORD eventBuffer[INPUT_BUFFER_SIZE];
		DWORD eventsRead;

		if (ReadConsoleInput(inputHandle, eventBuffer, INPUT_BUFFER_SIZE, &eventsRead))
		{
			for (unsigned int eventIndex = 0; eventIndex < eventsRead; ++eventIndex)
			{
				INPUT_RECORD* record = &eventBuffer[eventIndex];
				if (record->EventType == MOUSE_EVENT)
				{
					cursorCoord = record->Event.MouseEvent.dwMousePosition;
					cursorCoord.X = boost::algorithm::clamp((int)cursorCoord.X, 0, TOTAL_SCREEN_WIDTH - 1);
					cursorCoord.Y = boost::algorithm::clamp((int)cursorCoord.Y, 0, TOTAL_SCREEN_HEIGHT - 1);

					this->hackingModel->OnMouseMoveEvent(this->cursorCoord);

					if (record->Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
					{
						this->hackingModel->OnClickEvent(this->cursorCoord);
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
			}
		}

		this->hackingView->Render(this->cursorCoord);
		Sleep(1000 / 24);
	}
}
