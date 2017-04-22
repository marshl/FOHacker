#ifndef STRFUNCS_H_
#define STRFUNCS_H_

#include <string>

// Returns the number of characters that are different between 2 strings (position check)
// The strings must be the exact same length
int StringDiff( const std::string& _str1, const std::string& _str2 );

#endif