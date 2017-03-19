#ifndef HACKING_VIEW_H_
#define HACKING_VIEW_H_

#include <Windows.h>
#include <string>
#include <vector>

class HackingModel;

class HackingView
{
public:
	HackingView(HackingModel* hackingModel);
	~HackingView();

	void Render(COORD cursorCoord);

	void SetOutputHandle(HANDLE handle);

private:
	HackingModel* hackingModel;


	CHAR_INFO* displayBuffer;

	std::vector<std::string> characterBuffer;
	std::vector<std::vector<bool> > highlightBuffer;

	std::string stringBuffer;

	HANDLE outputHandle;

	void SwapBuffers();

	void RefreshBuffer(COORD cursorCoord);
};

#endif