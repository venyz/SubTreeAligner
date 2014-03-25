/*
 *  main.print_tagged.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 01.05.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include <iostream>
#include <ctime>
#include "Tree.h"
#include "TreePair.h"

//int main (int argc, char * const argv[]) {
int main() {
	time_t startTime = time(0);
	time_t currentTime;
	unsigned long long sec;
	time_t treePairAverage = 0;

#ifndef MACOSX
	locale::global(locale("en_US.UTF-8"));
#endif //#ifdef MACOSX
	
	/*	chudInitialize();
	chudMarkPID(getpid(),1);
	chudAcquireRemoteAccess();
	chudStartRemotePerfMonitor("testthetester");*/
		
	Tree::binariseMode = false;
	wifstream input;
	wcout << "DATA_SET: " << DATA_SET << endl;
	//HomeCenter data
	if (DATA_SET == "HomeCentre") {
		NonTerminal::idBase.reset(500);
		Tree::pairedMode = true;
		input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/new_manual.txt");
//		input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/en-fr HomeCenter data.txt");
		//English-Spanish data
	} else if (DATA_SET == "en-sp") {
		Tree::pairedMode = false;
		input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/en-sp/pairs.txt");
		//English-German data
	} else if (DATA_SET == "en-de") {
		Tree::pairedMode = false;
		input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/en-de/10k/pairs.txt");
		//German News data
	} else if (DATA_SET == "de-news") {
		Tree::pairedMode = false;
		input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/de-news/pairs.txt");
	} else {
		exit(15);
	}
	
	//	exit(66);
	
	if (input.fail()) {
		wcout << "!!!Cannot open the pairs file!!!" << endl;
		exit(1);
	}
	
	TreePair treePair;
	int counter = 1;
	
	TreePair::linked = true;
	
	currentTime = time(0);
	
//	int pairNo = 1; //118
//	vector<TreePair> pairs;
	while (!input.eof()) {
		input >> treePair;
		if (!TreePair::error) {
//			pairs.push_back(treePair);
//			wcout << "size of treePair: " << sizeof(treePair) << endl;
//			exit(0);
			++counter;
			//			if (counter <= pairNo) continue;
//			wcout << "TreePair №" << counter - 1 << ":" << endl;
			//			treePair.printXML();
			//			wcout << treePair;
			//			wcout << ".";
//			wcout << endl;
			treePair.removeExtraNodes();
//			treePair.printTaggedSentences();
			treePair.printSentences();
//			treePair.freezeLinks();
//			treePair.printBracketed();
//			wcout << endl;
			//			if (counter > pairNo) break;
		} else {
			wcout << "TreePair №" << counter << ": !!!ERROR!!!" << endl;
		}
	}
	wcout << endl;
	
//	wcout << "Size of pairs: " <<  << endl;
	
	input.close();
	
	/*	chudStopRemotePerfMonitor();
	chudReleaseRemoteAccess();*/
	
	currentTime = time(0);
	tm* local = localtime(&currentTime);
	sec = difftime(currentTime, startTime);
	wcout << endl << "Elapsed: " << sec << "sec" << endl;
	wcout << "Finished at " << asctime(local);
	
	return 0;
}