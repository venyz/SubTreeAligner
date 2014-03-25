/*
 *  main.extract_grammar.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 02.06.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include <ctime>
#include "DOPTree.h"

namespace bg_zhechev_ventsislav {
	extern void buildLowercasingTable(const string& fileName);
}

int main (int argc, char * const argv[]) {
	time_t startTime = time(0);
	time_t currentTime;
	unsigned long long sec;

#ifndef MACOSX
	locale::global(locale("en_US.UTF-8"));
#else
	//This points to the file, where lowercase versions for diacritisised characters can be found. The version of GCC that Xcode uses has broken locale system.
#ifdef DATA_DIR
	string uml = DATA_DIR;
#else
	string uml = "/Users/ventzi/Desktop/курсове - университет/ATTEMPT/software/TreeTester all";
#endif
	uml += "/umlauts.txt";
	buildLowercasingTable(uml);
#endif //#ifdef MACOSX
		
	/*	chudInitialize();
	chudMarkPID(getpid(),1);
	chudAcquireRemoteAccess();
	chudStartRemotePerfMonitor("testthetester");*/
		
	wcout << "DATA_SET: " << DATA_SET << endl;

	wifstream input;
	wifstream internal;
	wofstream output;

	bool files_loaded = false;
	if (argc > 1 && argc != 4 && argc != 3) {
		wcout << "Incorrect number of arguments: " << (argc - 1) << "! Usage:\n   " << argv[0] << " [input [internal] output]\n";
		exit(1);
	} else if (argc == 4 || argc == 3) {
		files_loaded = true;
		input.open(argv[1]);
		if (argc == 4)
			internal.open(argv[2]);
		output.open(argv[argc - 1]);
	}
	
	if (!files_loaded) {
	if (1) {
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
		internal.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/de-news/de-news.de.DOP.internal.only.txt");
		output.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/de-news/de-news.de.DOP.grammar.txt");
	} else {
		exit(15);
	}
	}
	
	//	exit(66);
	
	if (input.fail()) {
		wcout << "!!!Cannot open the pairs file!!!" << endl;
		exit(1);
	}
	
	if (internal.fail()) {
		wcout << "!!!Cannot open the internals file!!!" << endl;
		exit(1);
	}
	
	DOPTree tree;
	int counter = 1;
	
//	PCFGRulesMap PCFGRules;
	
	output.unsetf(ios::fixed);
	output.precision(16);

//	currentTime = time(0);
	
//	DOPTree::internalOnlyLabels = wstringSet();
	if (internal.is_open())
		while (!internal.eof()) {
			wstring label;
			internal >> label;
			DOPTree::internalOnlyLabels.insert(label);
			wcout << "Got internalOnlyLabel " << label << endl;
		}
//	internalOnlyLabels.insert(L"#EOS#");
	
	tree = L"(NP (D the)(NPadj (Adj green)(N house)))";
	tree.collectPCFGRules(wcout);
	tree = L"(NP (D the)(NPadj (Adj small)(N house)))";
	tree.collectPCFGRules(wcout);
	
	
	int treeNo = 10;
	while (0 && !input.eof()) {
		input >> tree;
		if (tree.sentLength() > 0) {
			++counter;
			//			if (counter <= treeNo) continue;
//			wcout << "Tree №" << counter - 1 << ":" << endl;
			//			treePair.printXML();
//			wcout << tree;
			wcout << ".";
			tree.collectPCFGRules(output);
//			wcout << endl;
			//			treePair.removeExtraNodes();
			//			treePair.printTaggedSentences();
			//			treePair.printSentences();
			//			treePair.freezeLinks();
			//			treePair.printBracketed();
			//			wcout << endl;
			if (counter > treeNo) break;
		} else {
			wcout << "Tree №" << counter << ": !!!ERROR!!!" << endl;
		}
	}
	wcout << endl;
	
	input.close();
	
	DOPTree::outputPCFGRules(wcout);
/*	for (PCFGRulesMap::const_iterator it = PCFGRules.begin(); it != PCFGRules.end(); ++it) {
		for (PCFGRulesMap::value_type::second_type::second_type::const_iterator rit = it->second.second.begin(); rit != it->second.second.end(); ++rit) {
			wcout << it->first << "\t" << rit->first << "\t" << ((1.0*rit->second)/it->second.first) << endl;
//			output << it->first << "\t" << rit->first << "\t" << ((1.0*rit->second)/it->second.first) << endl;
		}
	}*/
	output.close();
	
	/*	chudStopRemotePerfMonitor();
	chudReleaseRemoteAccess();*/
	
	currentTime = time(0);
	tm* local = localtime(&currentTime);
	sec = difftime(currentTime, startTime);
	wcout << endl << "Elapsed: " << sec << "sec" << endl;
	wcout << "Finished at " << asctime(local);
	
	return 0;
}