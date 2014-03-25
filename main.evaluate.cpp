/*
 *  main.evaluate.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 12.09.06.
 *  Copyright 2006 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include <iostream>
#include <ctime>
#include "Tree.h"
#include "TreePair.h"

namespace bg_zhechev_ventsislav {
	extern void buildLowercasingTable(const string& fileName);
	extern configMapType readConfig(const char*);
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
	
	bool logToStdOut = true;
	wstreambuf *saved_out = NULL;
	wifstream srcTreebank, trgTreebank;
	wofstream log;
	string srcTreebankFileName, trgTreebankFileName, logFileName = "";
	
	if (argc != 4 && argc != 3 && argc != 2) {
		wcerr << "Incorrect number of arguments: " << (argc - 1) << "! Usage:\n   " << argv[0] << " src_treebank trg_treebank [log_file]\n   " << argv[0] << " cfg_file" << endl;
		exit(1);
	} else if (argc == 4 || argc == 3) {
		srcTreebankFileName = argv[1];
		trgTreebankFileName = argv[2];
		if (argc == 4)
			logFileName = argv[3];
	} else if (argc == 2) {
		configMapType config = readConfig(argv[1]);
		configMapType::const_iterator it = config.find("source_treebank");
		if (it != config.end())
			srcTreebankFileName = it->second;
		else { wcout << "“source_treebank” option missing in config file!!!"; exit(1); }
		it = config.find("target_treebank");
		if (it != config.end())
			trgTreebankFileName = it->second;
		else { wcout << "“target_treebank” option missing in config file!!!"; exit(1); }
		it = config.find("log");
		if (it != config.end())
			logFileName = it->second;
		wstringstream ss;
	}
	
	srcTreebank.open(srcTreebankFileName.c_str());
	if (srcTreebank.fail()) { wcout << "!!!Cannot open " << srcTreebankFileName.c_str() << " file!!!" << endl; exit(1); }
	trgTreebank.open(trgTreebankFileName.c_str());
	if (trgTreebank.fail()) { wcout << "!!!Cannot open " << trgTreebankFileName.c_str() << " file!!!" << endl; exit(1); }
	if (logFileName != "") {
		logToStdOut = false;
		log.open(logFileName.c_str());
		if (log.fail()) { wcout << "!!!Cannot open " << logFileName.c_str() << " file!!!" << endl; exit(1);	}
		saved_out = wcout.rdbuf();
		wcout.rdbuf(log.rdbuf());
	}
	
	/*	chudInitialize();
	chudMarkPID(getpid(),1);
	chudAcquireRemoteAccess();
	chudStartRemotePerfMonitor("testthetester");*/
	
//#ifdef LOWERCASE
//	wcout << "LOWERCASE" << endl;
//	string case_ = "in";
//#else
//	string case_ = "";
//#endif
//	string version = "skip2_score2_span1";
//	string automaticFileName = "/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/old_multiword_logs_2.5/case_" + case_ + "sensitive/" + version + ".txt";
////	string automaticFileName = "/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/en-fr CMU/CMU HomeCentre/T2T.alignments.txt";
//	wifstream automatic;
//	automatic.open(automaticFileName.c_str());
//	if (automatic.fail()) {
//		wcout << "!!!Cannot open the automatic file!!! "<< automaticFileName.c_str() << endl;
//		exit(1);
//	}
//	wifstream manual("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/en-fr HomeCenter data.txt");
////	wifstream manual(("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/old_multiword_logs_2.5/case_" + case_ + "sensitive/" + version + ".txt").c_str());
////	wifstream manual("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/new_manual.txt");
//	if (manual.fail()) {
//		wcout << "!!!Cannot open the manual file!!!" << endl;
//		exit(1);
//	}
	
	TreePair srcTreePair, trgTreePair;
	NonTerminal::idBase.reset(500);
	Tree::binariseMode = false;
	Tree::pairedMode = true;
	
	int counter = 1, internalCounter = 1, internalStrictCounter = 1;
	TreePair::linked = true;
	double avgPrecision = 0, avgRecall = 0, avgFScore = 0, avgPrecisionInternal = 0, avgRecallInternal = 0, avgFScoreInternal = 0, avgPrecisionInternalStrict = 0, avgRecallInternalStrict = 0, avgFScoreInternalStrict = 0;
	
	int pairNo __attribute__ ((unused)) = 25;
	while (!srcTreebank.eof()) {
		srcTreebank >> srcTreePair;
		if (!TreePair::error) {
			trgTreebank >> trgTreePair;
			if (!TreePair::error) {
				++counter;
				++internalCounter;
				++internalStrictCounter;
//				if (counter <= pairNo) continue;
//				wcout << "TreePair №" << counter - 1 << ":" << endl;
				vector<pair<doubleType, doubleType> > result = TreePair::evaluate(srcTreePair, trgTreePair);
//				wcout << "Precision: " << result[1].first << "; Recall: " << result[1].second << endl;
				avgPrecision += result[0].first;
				avgRecall += result[0].second;
				if (result[0].first || result[0].second)
					avgFScore += (2.*result[0].first*result[0].second)/(result[0].first+result[0].second);
//				wcout << "?!?!?!?" << (2.*result[0].first*result[0].second)/(result[0].first+result[0].second) << endl;
				if (result[1].first == -1 && result[1].second == -1)
//				{
//					wcout << "««TreePair №" << counter - 1 << " has no lexicals!»»";
					--internalCounter;
//				}
				else {
					avgPrecisionInternal += result[1].first;
					avgRecallInternal += result[1].second;
					if (result[1].first || result[1].second)
						avgFScoreInternal += (2.*result[1].first*result[1].second)/(result[1].first+result[1].second);
				}

				if (result[2].first == -1 && result[2].second == -1)
					--internalStrictCounter;
				else {
					avgPrecisionInternalStrict += result[2].first;
					avgRecallInternalStrict += result[2].second;
					if (result[2].first || result[2].second)
						avgFScoreInternalStrict += (2.*result[2].first*result[2].second)/(result[2].first+result[2].second);
				}

//				autoTreePair.printBracketed();
//				wcout << autoTreePair;
//				wcout << endl;
				wcout << ".";
//				if (counter > pairNo) break;
			}
		}
	}
	
	wcout << endl;
	wcout.precision(4);
	
	wcout << "Evaluating " << srcTreebankFileName.c_str() << " against " << trgTreebankFileName.c_str() << endl;
	wcout << "Total number of tree pairs: " << --counter << endl;
	--internalCounter; --internalStrictCounter;
	wcout << "Average precision:    " << avgPrecision/counter << endl;
	wcout << "Average recall:       " << avgRecall/counter << endl;
//	wcout << "Average F-score:        " << avgFScore/counter << endl;
	wcout << "Average F-score:      " << (2*(avgPrecision/counter)*(avgRecall/counter)/((avgPrecision/counter)+(avgRecall/counter))) << endl << endl;

	wcout << "Number of these l:    " << internalCounter << endl;
	wcout << "Average precision l:  " << avgPrecisionInternal/internalCounter << endl;
	wcout << "Average recall l:     " << avgRecallInternal/internalCounter << endl;
//	wcout << "Average F-score:        " << avgFScoreInternal/counter << endl;
	wcout << "Average F-score l:    " << (2*(avgPrecisionInternal/internalCounter)*(avgRecallInternal/internalCounter)/((avgPrecisionInternal/internalCounter)+(avgRecallInternal/internalCounter))) << endl << endl;

	wcout << "Number of these si:   " << internalStrictCounter << endl;
	wcout << "Average precision si: " << avgPrecisionInternalStrict/internalStrictCounter << endl;
	wcout << "Average recall si:    " << avgRecallInternalStrict/internalStrictCounter << endl;
//	wcout << "Average F-score:        " << avgFScoreInternalStrict/counter << endl;
	wcout << "Average F-score si:   " << (2*(avgPrecisionInternalStrict/internalStrictCounter)*(avgRecallInternalStrict/internalStrictCounter)/((avgPrecisionInternalStrict/internalStrictCounter)+(avgRecallInternalStrict/internalStrictCounter))) << endl << endl;
	
	srcTreebank.close();
	trgTreebank.close();
	
	/*	chudStopRemotePerfMonitor();
	chudReleaseRemoteAccess();*/
	
	currentTime = time(0);
	tm* local = localtime(&currentTime);
	wcout << "Finished at " << asctime(local) << endl;
	sec = difftime(currentTime, startTime);
	wcout << "Elapsed: " << sec << "sec" << endl;
	
	if (!logToStdOut) {
		wcout.rdbuf(saved_out);
		log.close();
	}
	
	return 0;
}