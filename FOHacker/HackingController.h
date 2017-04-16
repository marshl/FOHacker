#ifndef HACKING_CONTROLLER_H_
#define HACKING_CONTROLLER_H_

#include <Windows.h>
#include <vector>
#include <string>

#include "GameState.h"

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

    GameState currentState;

    COORD cursorCoord;

    HANDLE inputHandle;
    HANDLE outputHandle;

    void OnClickEvent();

    void ChangeState( GameState newState );

    bool isDone;
};

#endif
