#ifndef HACKING_CONTROLLER_H_
#define HACKING_CONTROLLER_H_

#include <Windows.h>
#include <vector>
#include <string>

#include "constants.h"

class HackingModel;
class HackingView;
class PuzzleWord;

class HackingController
{
public:
	HackingController(HackingModel* hackingModel, HackingView* hackingView);
	~HackingController();

	void Run();

private:
	HackingView* hackingView;
	HackingModel* hackingModel;

	CHAR_INFO* displayBuffer;

	std::vector<std::string> characterBuffer;
	std::vector<std::vector<bool> > highlightBuffer;

	int attemptsRemaining;
	std::string stringBuffer;

	COORD cursorCoord;

	std::vector<std::string> hexAddresses;
	std::string solutionWord;
	std::vector<PuzzleWord*> puzzleWords;
	std::vector<std::string> attemptedWords;

	PuzzleWord* currentHighlightedPuzzleWord;

	void RefreshBuffer();
	void SwapBuffers();
	void SetHexAddresses();
	void SetPuzzleWords();
	void PlacePuzzleWords();
	void OnClickEvent();
	void OnMouseMoveEvent();
	void GetSampleWordList(std::vector<std::string>& _out);
	COORD GetOutputCursorCoord();
};

#endif
