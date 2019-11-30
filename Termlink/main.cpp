#include <cstdlib>
#include <ctime>

#include "strfunc.h"
#include "PuzzleWord.h"
#include "HackingController.h"
#include "HackingModel.h"
#include "HackingView.h"

int main(int argc, char* argv[])
{
	srand((unsigned int)time(nullptr));

	HackingModel* hackingModel = new HackingModel();
	HackingView* hackingView = new HackingView(hackingModel);
	HackingController* hackingController = new HackingController(hackingModel, hackingView);

	bool result = hackingController->Run();

	delete hackingController;
	delete hackingView;
	delete hackingModel;

	if (result && argc == 2)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		CreateProcess(argv[1],   // the path
			NULL,           // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
		);
	}

	return 0;
}
