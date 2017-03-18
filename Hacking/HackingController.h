#ifndef HACKING_CONTROLLER_H_
#define HACKING_CONTROLLER_H_

class HackingModel;
class HackingView;

class HackingController
{
public:
	HackingController(HackingModel* hackingModel, HackingView* hackingView);
	~HackingController();

	void Run();

private:
	HackingView* hackingView;
	HackingModel* hackingModel;
};

#endif
