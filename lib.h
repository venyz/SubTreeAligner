/*
 *  lib.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 04.11.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifndef LIB
#define LIB

#include <string>
#include <map>
#include <locale>
using namespace std;

namespace bg_zhechev_ventsislav {

	const wchar_t* SPACE = L" \f\n\r\t\v";	//a constant describing whitespace – for use in the parsing of bracketed representations of trees
	
	map<wchar_t, wchar_t> substMap;
	void toLower(basic_string<wchar_t>& s);
	void buildLowercasingTable(const string& fileName);
	
	bool USE_PHRASES;
	
	configMapType readConfig(const char* fileName);
	
	wstring readChunk(wistream& input, wchar_t delim);
	
}
#endif