/*
 *  main.print_sentences.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 24.10.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include <iostream>
#include <ctime>
#include "Tree.h"

int main (int argc, char * const argv[]) {
//int main() {
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
	bool files_loaded = false;
        
	if (argc > 1 && argc != 2) {
		wcout << "Incorrect number of arguments: " << (argc - 1) << "! Usage:\n   " << argv[0] << " [input]\n";
		exit(1);
	} else if (argc == 2) {
		files_loaded = true;
		input.open(argv[1]);
	}
	        
	if (!files_loaded) {
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
		input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/de-news/weaver/de-news.de.bitpar.clean");
	} else {
		exit(15);
	}
	}
	
	//	exit(66);
	
	if (input.fail()) {
		wcout << "!!!Cannot open the input file!!!" << endl;
		exit(1);
	}
	
	Tree tree;
	int counter = 1;
	
	currentTime = time(0);
	
	
//	int treeNo = 10;
	while (!input.eof()) {
		input >> tree;
		if (tree.sentLength() > 0) {
			++counter;
			//			if (counter <= treeNo) continue;
//			wcout << "Tree №" << counter - 1 << ":" << endl;
			//			treePair.printXML();
			wcout << tree.sentence() << endl;
//			tree.collectPCFGRules(PCFGRules, internalOnlyLabels, output);
//			wcout << endl;
			//			treePair.removeExtraNodes();
			//			treePair.printTaggedSentences();
			//			treePair.printSentences();
			//			treePair.freezeLinks();
			//			treePair.printBracketed();
			//			wcout << endl;
//			if (counter > treeNo) break;
		} else {
//			wcout << "Tree №" << counter << ": !!!ERROR!!!" << endl;
			wcout << "()";
		}
	}
	wcout << endl;
	
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