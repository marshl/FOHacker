#include <cstdlib>
#include <ctime>

#include "strfunc.h"
#include "PuzzleWord.h"
#include "HackingController.h"
#include "HackingModel.h"
#include "HackingView.h"

int main( int argc, char* argv[] )
{
    srand( (unsigned int)time( nullptr ) );

    HackingModel* hackingModel = new HackingModel();
    HackingView* hackingView = new HackingView( hackingModel );
    HackingController* hackingController = new HackingController( hackingModel, hackingView );

    hackingController->Run();

    delete hackingController;
    delete hackingView;
    delete hackingModel;

    return 0;
}
