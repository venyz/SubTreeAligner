/*
 *  main.statistics.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 11.04.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include <ctime>
#include "Tree.h"
#include "TreePair.h"

namespace bg_zhechev_ventsislav {
	extern void buildLowercasingTable(const string& fileName);
//	extern configMapType readConfig(const char*);
}

int main (int argc, char * const argv[]) {
	time_t startTime = time(0);
	time_t currentTime;
	unsigned long long sec;
	time_t treePairAverage = 0;

#ifndef MACOSX
	locale::global(locale("en_US.UTF-8"));
#else
#ifndef EXTRACT_CHUNKS
	//This points to the file, where lowercase versions for diacritisised characters can be found. The version of GCC that Xcode uses has broken locale system.
#ifdef DATA_DIR
	string uml = DATA_DIR;
#else
	string uml = "/Users/ventzi/Desktop/курсове - университет/ATTEMPT/software/TreeTester all";
#endif
	uml += "/umlauts.txt";
	buildLowercasingTable(uml);
#endif //#ifndef EXTRACT_CHUNKS
#endif //#ifdef MACOSX
	
	/*	chudInitialize();
	chudMarkPID(getpid(),1);
	chudAcquireRemoteAccess();
	chudStartRemotePerfMonitor("testthetester");*/
	
	Tree::binariseMode = false;
	wifstream input;

	bool files_loaded = false;
#ifndef EXTRACT_CHUNKS
	if (argc > 1 && (argc != 5 && argc != 4)) {
		wcout << "Incorrect number of arguments: " << (argc - 1) << "! Usage:\n   " << argv[0] << " [s2t_lex_prob t2s_lex_prob {s2t_phr_prob} input]\n";
#else
	if (argc > 1 && argc != 2) {
		wcout << "Incorrect number of arguments: " << (argc - 1) << "! Usage:\n   " << argv[0] << " [input]\n";
#endif
		exit(1);
#ifndef EXTRACT_CHUNKS
	} else if (argc == 5 || argc == 4) {
		TreePair::readSourceWordAlignments(argv[1]);
		TreePair::readTargetWordAlignments(argv[2]);
		if (argc == 5) {
			USE_PHRASES = true;
			TreePair::readPhraseAlignments(argv[3]);
		} else
			USE_PHRASES = false;
#else
	} else if (argc == 2) {
#endif
		input.open(argv[argc - 1]);
		files_loaded = true;
	}

#ifdef LOWERCASE
	wcout << "LOWERCASE" << endl;
	string case_ = "in";
#else
	string case_ = "";
#endif

	Tree::pairedMode = true;
		
	if (!files_loaded) {
	//HomeCenter data
	NonTerminal::idBase.reset(500);
#ifndef EXTRACT_CHUNKS
	TreePair::readSourceWordAlignments("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/old_multiword/case_" + case_ + "sensitive/giza/en/model/lex.0-0.f2n");
	TreePair::readTargetWordAlignments("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/old_multiword/case_" + case_ + "sensitive/giza/fr/model/lex.0-0.f2n");
	TreePair::readPhraseAlignments("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/old_multiword/case_" + case_ + "sensitive/giza/en/model/phrase-table.0-0");
#endif
//	TreePair::SENTENCE_COUNT = 810;
//	input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/en-fr HomeCenter data.txt");
//	input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/new_manual.txt");
//	input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/en-es/case_insensitive/skip1_score1_span1.txt");
	input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/de-news/case_insensitive/skip1_score1.txt");
//	input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/en-de/10k/case_insensitive_simple/skip1_score1.txt");
//	input.open("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/old_multiword/case_insensitive/skip1_score1_span1.txt");

	//	exit(66);
	}
	if (input.fail()) {
		wcout << "!!!Cannot open the pairs file!!!" << endl;
		exit(1);
	}
	
	TreePair treePair;
	int counter = 1;
	
	TreePair::linked = true;

//	USE_PHRASES = false;

	currentTime = time(0);
	sec = difftime(currentTime, startTime);
	wcout << "Init done in " << sec << "sec" << endl;
	wcout.unsetf(ios::fixed);
	wcout.precision(30);
	
//	int pairNo = 655; //118
	while (!input.eof()) {
		input >> treePair;
		if (!TreePair::error) {
			++counter;
//			if (counter <= pairNo) continue;
//			wcout << "TreePair №" << counter - 1 << ":" << endl; // << treePair << endl;
#ifndef EXTRACT_CHUNKS
			treePair.link();
//			treePair.printBracketed();
			treePair.collectTags();
			treePair.calculateStatistics();
			wcout << "." << flush;
#else
#if defined(SAMT_RULES)
			wcout << "SENT_ID" << counter - 2 << endl;
			treePair.printSAMTRules(wcout);
#else
			wcout << "SENT_ID" << counter - 2 << endl;
			treePair.collectChunks();
			TreePair::printChunks(wcout);
			TreePair::chunks.clear();
#endif
#endif
//			wcout << endl;
//			if (counter > pairNo) break;
		} else {
//			wcout << "TreePair №" << counter << ": !!!ERROR!!!" << endl;
		}
	}
	wcout << endl;
#ifndef EXTRACT_CHUNKS
	TreePair::printTags(wcout);
	wcout.precision(6);
	TreePair::printStatistics(wcout);
#else
//	TreePair::printChunks();
#endif
		
	input.close();
	
	/*	chudStopRemotePerfMonitor();
	chudReleaseRemoteAccess();*/
	
	currentTime = time(0);
	tm* local = localtime(&currentTime);
	wcout << "Finished at " << asctime(local) << endl;
	sec = difftime(currentTime, startTime);
	wcout << "Elapsed: " << sec << "sec" << endl;
	wcout << "TreePair average. " << ((1.0 * treePairAverage) / (counter - 1)) << "sec" << endl;
	
	return 0;
}