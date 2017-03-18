#include "HackingView.h"

#include <cassert>

HackingView::HackingView(HackingModel* hackingModel)
{
	assert(hackingModel != nullptr);
	this->hackingModel = hackingModel;
}


HackingView::~HackingView()
{
}
