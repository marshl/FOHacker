#ifndef HACKING_VIEW_H_
#define HACKING_VIEW_H_

class HackingModel;

class HackingView
{
public:
	HackingView(HackingModel* hackingModel);
	~HackingView();

	private:
	HackingModel* hackingModel;
};

#endif