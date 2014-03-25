/*
 *  lib.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 04.11.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "lib.h"

namespace bg_zhechev_ventsislav {
	
	void toLower(basic_string<wchar_t>& s) {
		for (basic_string<wchar_t>::iterator p = s.begin(); p != s.end(); ++p) {
			const map<wchar_t, wchar_t>::const_iterator it = substMap.find(*p);
			if (it != substMap.end())
				*p = it->second;
			else
				*p = towlower(*p);
		}
	}
	
	void buildLowercasingTable(const string& fileName) {
		wifstream umlauts(fileName.c_str());
		if (umlauts.fail()) {
			wcout << "Cannot open the lowercasing table " << fileName.c_str() << " !" << endl;
			exit(1);
		}
		while (!umlauts.eof()) {
			wstring twoLetters;
			umlauts >> twoLetters;
			if (twoLetters.c_str()[0] != '#')
				substMap[twoLetters.c_str()[1]] = twoLetters.c_str()[3];
		}
#ifdef __debug_lc__
		for (map<wchar_t, wchar_t>::const_iterator it = substMap.begin(); it != substMap.end(); ++it)
			wcout << "“" << it->first << "”-->“" << it->second << "”" << endl;
#endif
		umlauts.close();
	}
	
	configMapType readConfig(const char* fileName) {
		configMapType config;
		ifstream file;
		file.open(fileName);
		if (file.fail()) { wcout << "!!!Cannot open " << fileName << " file!!!" << endl; exit(1); }
		
		while (!file.eof()) {
#ifdef __debug_cfg__
			wcout << ">>Reading ";
#endif
			char input[5000];
			file.getline(input, 100, ' ');
			if (input[0] == '\0') {
#ifdef __debug_cfg__
				wcout << endl;
#endif
				continue;
			}
			string optionName(input);
#ifdef __debug_cfg__
			wcout << "(" << optionName.c_str() << ") ";
#endif
			str_pos_type firstChar = optionName.find_first_not_of("\f\n\r\v\t");
			if (firstChar == string::npos) {
				wcerr << "Empty option name!" << endl;
				continue;
			}
			optionName = optionName.substr(firstChar);
#ifdef __debug_cfg__
			wcout << "«" << optionName.c_str() << "» -> ";
#endif
			file.getline(input, 5000);
			if (*(optionName.begin()) != '#') {
//#ifdef __debug_cfg__
//			{
//#endif
				if (input[0] == '\0') {
					wcerr << "Empty option: «" << optionName.c_str() << "»!" << endl;
					continue;
				}
				string valueName(input);
				firstChar = valueName.find_first_not_of(" \t");
				if (firstChar == string::npos) {
					wcerr << "Empty option: «" << optionName.c_str() << "»!" << endl;
					continue;
				}
				config[optionName] = valueName.substr(firstChar);
#ifdef __debug_cfg__
				wcout << "«" << valueName.substr(firstChar).c_str() << "»";
			} else
				wcout << "DISCARDED";
			wcout << endl;
#else
		}
#endif
		}
		
		return config;
	}

	wstring readChunk(wistream& input, wchar_t delim) {
		wstring output;
		
		wchar_t ln[25];
		input.get(ln, 25, delim);
		if (input.fail())
			return L"";
		output = ln;
		wchar_t lastChar = input.peek();
#ifdef __debug2b__
		wcerr << "??? “" << lastChar << "” ??? output: “" << output << "”" << endl;
#endif
		while (lastChar != delim) {
			input.get(ln, 25, delim);
			if (input.fail())
				return L"";
			output += ln;
			lastChar = input.peek();
#ifdef __debug2b__
			wcerr << "??? “" << lastChar << "” ??? output: “" << output << "”" << endl;
#endif
		}
		input.get();
		
		return output;
	}
	
}