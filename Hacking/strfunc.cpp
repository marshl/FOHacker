#include "strfunc.h"

#include <cassert>

// Returns the number of characters that are different between 2 strings (position check)
// The strings must be the exact same length
int StringDiff(const std::string& _str1, const std::string& _str2)
{
	assert(("Strings must be the same length", _str1.length() == _str2.length()));

	int differences = 0;
	int l = _str1.length();
	for (int i = 0; i < l; ++i)
	{
		if (_str1[i] != _str2[i])
		{
			++differences;
		}
	}
	return differences;
}