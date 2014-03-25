/*
 *  main.integerise.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 04.06.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "Index.h"

//int main (int argc, char * const argv[]) {
int main() {
	time_t startTime = time(0);
	time_t currentTime;
	unsigned long long sec;

#ifndef MACOSX
	locale::global(locale("en_US.UTF-8"));
#endif //#ifdef MACOSX
	
	/*	chudInitialize();
	chudMarkPID(getpid(),1);
	chudAcquireRemoteAccess();
	chudStartRemotePerfMonitor("testthetester");*/
		
	wifstream input;
	wofstream output;
	wofstream reference;
	wcout << "DATA_SET: " << DATA_SET << endl;
	if (0) {
		//HomeCenter data
		//	} else if (DATA_SET == "HomeCentre") {
		//		NonTerminal::idBase.reset(500);
		//		input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/new_manual.txt");
		//English-Spanish data
		//	} else if (DATA_SET == "en-sp") {
		//		input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/en-sp/pairs.txt");
		//English-German data
		//	} else if (DATA_SET == "en-de") {
		//		input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/en-de/10k/pairs.txt");
		//German News data
		} else if (DATA_SET == "de-news") {
			input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/de-news/de-news.de.DOP.grammar.txt");
			output.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/de-news/de-news.de.DOP.grammar.integers.txt");
			reference.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/de-news/de-news.de.DOP.integer.map.txt");
		} else {
			exit(15);
		}
	
	//	exit(66);
	
	if (input.fail()) {
		wcout << "!!!Cannot open the grammar file!!!" << endl;
		exit(1);
	}
	
	int counter = 1;
	
	Index index;
	
	labelReferenceMap integerMap;
	
	currentTime = time(0);
	
	int ruleNo = 10;
	while (!input.eof()) {
		++counter;
//		if (counter <= ruleNo) continue;
		if (counter % 1000 == 0)
			wcout << ".";
		wchar_t ln[500];
		input.getline(ln, 500);
		wstring line = ln;
#ifdef __debug1_int__
		wcout << "current line: " << line << endl;
#endif
		
		while (line.find('\t') != wstring::npos) {
			wstring curr = line.substr(0, line.find_first_of('\t'));
#ifdef __debug1_int__
			wcout << "   current token: " << curr << endl;
#endif
			if (!curr.empty()) {
				labelReferenceMap::const_iterator it = integerMap.find(curr);
				if (it == integerMap.end())
					it = integerMap.insert(make_pair(curr, index++)).first;
				output << it->second;
			}
			output << "\t";
			line = line.substr(line.find_first_of('\t') + 1);
		}
#ifdef __debug1_int__
		wcout << "   current token: " << line << endl;
#endif
		output << line << endl;
//		if (counter > ruleNo) break;
	}
	wcout << endl;
	ruleNo = 0;
	
	input.close();
	output.close();
	
	for (labelReferenceMap::const_iterator it = integerMap.begin(); it != integerMap.end(); ++it)
		reference << it->first << "\t" << it->second << endl;
	reference.close();
	
	/*	chudStopRemotePerfMonitor();
	chudReleaseRemoteAccess();*/
	
	currentTime = time(0);
	tm* local = localtime(&currentTime);
	sec = difftime(currentTime, startTime);
	wcout << endl << "Elapsed: " << sec << "sec" << endl;
	wcout << "Finished at " << asctime(local);
	
	return 0;
}