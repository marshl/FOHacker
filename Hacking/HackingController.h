#ifndef HACKING_CONTROLLER_H_
#define HACKING_CONTROLLER_H_

#include <Windows.h>
#include <vector>
#include <string>

class HackingModel;
class HackingView;
class PuzzleWord;

class HackingController
{
public:
    HackingController( HackingModel* hackingModel, HackingView* hackingView );
    ~HackingController();

    void Run();

private:
    HackingView* hackingView;
    HackingModel* hackingModel;

    COORD cursorCoord;

    HANDLE inputHandle;
    HANDLE outputHandle;
};

#endif
