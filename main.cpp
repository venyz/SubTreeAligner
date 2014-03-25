/*
 *  main.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 21.11.06.
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
	time_t treePairAverage = 0;

	wcout.unsetf(ios::fixed);
	wcout.precision(16);

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

#ifndef TRAINING
	bool readStdIn = false, writeToStdOut = true, logToStdLog = true;
	wstreambuf *saved_in = NULL, *saved_out = NULL, *saved_log = NULL;
	wifstream input;
	wofstream output, log;
	string inputFileName, srcWordsFileName, trgWordsFileName, phrasesFileName, outputFileName = "", logFileName = "";
	
	if (argc > 1 && argc != 5 && argc != 4 && argc != 2) {
		wcerr << "Incorrect number of arguments: " << (argc - 1) << "! Usage:\n   " << argv[0] << " [s2t_lex_prob t2s_lex_prob [s2t_phr_prob] input]\n   " << argv[0] << " [cfg_file]" << endl;
		exit(1);
	} else if (argc == 5 || argc == 4) {
		srcWordsFileName = argv[1];
		trgWordsFileName = argv[2];
		if (argc == 5) {
			phrasesFileName = argv[3];
			USE_PHRASES = true;
		} else
			USE_PHRASES = false;
		inputFileName = argv[argc - 1];
		TreePair::collectExpensiveStatistics = "all";
	} else if (argc == 2) {
		configMapType config = readConfig(argv[1]);
		configMapType::const_iterator it = config.find("input");
		if (it != config.end())
			inputFileName = it->second;
		else
			inputFileName = "-";
		it = config.find("source_alignments");
		if (it != config.end())
			srcWordsFileName = it->second;
		else { wcerr << "“source_alignments” option missing in config file!!!"; exit(1); }
		it = config.find("target_alignments");
		if (it != config.end())
			trgWordsFileName = it->second;
		else { wcerr << "“target_alignments” option missing in config file!!!"; exit(1); }
		it = config.find("phrase_alignments");
		if (it != config.end()) {
			phrasesFileName = it->second;
			USE_PHRASES = true;
		} else
			USE_PHRASES = false;
		it = config.find("output");
		if (it != config.end())
			outputFileName = it->second;
		it = config.find("log");
		if (it != config.end())
			logFileName = it->second;
		it = config.find("expensive_statistics");
		if (it != config.end())
			TreePair::collectExpensiveStatistics = it->second;
	}
	
	if (inputFileName != "-") {
		input.open(inputFileName.c_str());
		if (input.fail()) { wcerr << "!!!Cannot open input file ‘" << inputFileName.c_str() << "’!!!" << endl; exit(1); }
		saved_in = wcin.rdbuf();
		wcin.rdbuf(input.rdbuf());
	} else
		readStdIn = true;
	if (outputFileName != "") {
		writeToStdOut = false;
		output.open(outputFileName.c_str());
		if (output.fail()) { wcerr << "!!!Cannot open output file ‘" << outputFileName.c_str() << "’!!!" << endl; exit(1);	}
		saved_out = wcout.rdbuf();
		wcout.rdbuf(output.rdbuf());
	}
	if (logFileName != "") {
			logToStdLog = false;
		log.open(logFileName.c_str());
		if (log.fail()) { wcerr << "!!!Cannot open log file ‘" << logFileName.c_str() << "’!!!" << endl; exit(1);	}
		saved_log = wclog.rdbuf();
		wclog.rdbuf(log.rdbuf());
	}
	
	wclog << "Running align." << endl << endl;
	wclog << "Data set: " << DATA_SET << "; Configuration: ";
#ifdef DELAY_CONSTITUENT
	wclog << "skip2_";
#else
	wclog << "skip1_";
#endif
#ifdef NEW_PROBS
	wclog << "score2";
#else
	wclog << "score1";
#endif
	if (VERSION == "0.8a")
		wclog << "_span1";
	wclog << endl;
#ifdef RESCORE
	wclog << "Using the re-scoring module." << endl;
#endif
#ifdef LOWERCASE
	wclog << "Using lowercased data." << endl;
	string case_ = "in";
#else
	string case_ = "";
#endif
#ifdef SEP_PROB_TYPE
	wclog << "Using log-based probabilites for score calculation." << endl;
#endif
#ifdef LATTICE
	wclog << "Full-search enabled!!! Combinatorial algorithm in operation!" << endl;
#endif
	wclog << "Input file:  " << (inputFileName != "-" ? inputFileName : "standard input").c_str() << endl;
	wclog << "Output file: " << (outputFileName != "" ? outputFileName : "standard output").c_str() << endl;
	
	wclog << endl << "Initialising…" << flush;

#ifdef _OPENMP
	bool omp_dynamic = omp_get_dynamic();
	omp_set_dynamic(false);
#pragma omp parallel sections num_threads(3)
	{
#pragma omp section
#endif
		TreePair::readSourceWordAlignments(srcWordsFileName);
#ifdef _OPENMP
#pragma omp section
#endif
		TreePair::readTargetWordAlignments(trgWordsFileName);
#ifdef _OPENMP
#pragma omp section
#endif
		if (USE_PHRASES)
			TreePair::readPhraseAlignments(phrasesFileName);
#ifdef _OPENMP
	} //parallel sections
	omp_set_dynamic(omp_dynamic);
#endif
	
	Tree::binariseMode = false;
	//HomeCenter data
	if (DATA_SET == "HomeCentre") {
		NonTerminal::idBase.reset(500);
		Tree::pairedMode = true;
#ifndef CHECK_MAN_LINKS
		TreePair::linked = false;
#else
		TreePair::linked = true;
#endif
	} else {
		Tree::pairedMode = false;
		TreePair::linked = false;
	}
	
//	exit(66);
	
	TreePair treePair = TreePair();
	int counter = 1;

	currentTime = time(0);
	sec = difftime(currentTime, startTime);
	wclog << " initialisation done in " << sec << "sec" << endl << endl;
//	wcout.unsetf(ios::fixed);
//	wcout.precision(20);
	wclog.unsetf(ios::fixed);
	wclog.precision(10);

#ifdef LATTICE
	int totalCounter = 1, internalCounter = 1, internalStrictCounter = 1;
	double avgPrecision = 0, avgRecall = 0, avgFScore = 0, avgPrecisionInternal = 0, avgRecallInternal = 0, avgFScoreInternal = 0, avgPrecisionInternalStrict = 0, avgRecallInternalStrict = 0, avgFScoreInternalStrict = 0;
#endif //LATTICE
#ifdef PROFILE
	chudInitialize();
	chudMarkPID(getpid(),1);
	chudAcquireRemoteAccess();
	chudStartRemotePerfMonitor("testthetester");
#endif
	
	int pairNo __attribute__ ((unused)) = 100;
	while (!wcin.eof()) {
		wcin >> treePair;
		if (!TreePair::error) {
			++counter;
#ifdef HOMERUN
//			if (counter <= pairNo) continue;
			wcout << "TreePair №" << counter - 1 << ":" << endl;
#endif
			time_t treePairStart = time(0);
			treePair.link();
			treePairAverage += time(0) - treePairStart;
//			treePair.removeExtraNodes();
			treePair.printBracketed(wcout);
//			treePair.printXML(wcout);
//			wcout << treePair;
			treePair.collectTags();
			treePair.calculateStatistics();
//			wcout << ".";
			wcout << endl;
//			treePair.printTaggedSentences();
#ifdef LATTICE
			++totalCounter;
			++internalCounter;
			++internalStrictCounter;
			vector<pair<doubleType, doubleType> > result = treePair.evaluateLattice();
			//				wcout << "Precision: " << result[1].first << "; Recall: " << result[1].second << endl;
			if (result[0].first == -1 && result[0].second == -1)
				totalCounter--;
			else {
				avgPrecision += result[0].first;
				avgRecall += result[0].second;
				if (result[0].first || result[0].second)
					avgFScore += (2.*result[0].first*result[0].second)/(result[0].first+result[0].second);
			}
			//				wcout << "?!?!?!?" << (2.*result[0].first*result[0].second)/(result[0].first+result[0].second) << endl;
			
			if (result[1].first == -1 && result[1].second == -1)
				internalCounter--;
			else {
				avgPrecisionInternal += result[1].first;
				avgRecallInternal += result[1].second;
				if (result[1].first || result[1].second)
					avgFScoreInternal += (2.*result[1].first*result[1].second)/(result[1].first+result[1].second);
			}
			
			if (result[2].first == -1 && result[2].second == -1)
				internalStrictCounter--;
			else {
				avgPrecisionInternalStrict += result[2].first;
				avgRecallInternalStrict += result[2].second;
				if (result[2].first || result[2].second)
					avgFScoreInternalStrict += (2.*result[2].first*result[2].second)/(result[2].first+result[2].second);
			}
#endif //LATTICE
#ifdef HOMERUN
//			if (counter > pairNo) break;
#endif
		} else {
//			wcout << "TreePair No" << counter << ": !!!ERROR!!!" << endl;
		}
	}

#ifdef PROFILE
	chudStopRemotePerfMonitor();
	chudReleaseRemoteAccess();
#endif
	
	if (!readStdIn)
		wcin.rdbuf(saved_in);
	input.close();
	if (!writeToStdOut) {
		wcout.rdbuf(saved_out);
		output.close();
	}
	
	wclog << endl;
	TreePair::printTags(wclog);
	wclog.precision(6);
	TreePair::printStatistics(wclog);
	
#ifdef LATTICE
	wclog << endl;
	wclog.precision(4);
	
	wclog << "Total number of tree pairs: " << --totalCounter << endl;
	--internalCounter; --internalStrictCounter;
	wclog << "Average precision:    " << avgPrecision/totalCounter << endl;
	wclog << "Average recall:       " << avgRecall/totalCounter << endl;
	//	wclog << "Average F-score:        " << avgFScore/counter << endl;
	wclog << "Average F-score:      " << (2*(avgPrecision/totalCounter)*(avgRecall/totalCounter)/((avgPrecision/totalCounter)+(avgRecall/totalCounter))) << endl << endl;
	
	wclog << "Number of these:      " << internalCounter << endl;
	wclog << "Average precision l:  " << avgPrecisionInternal/internalCounter << endl;
	wclog << "Average recall l:     " << avgRecallInternal/internalCounter << endl;
	//	wclog << "Average F-score:        " << avgFScoreInternal/counter << endl;
	wclog << "Average F-score l:    " << (2*(avgPrecisionInternal/internalCounter)*(avgRecallInternal/internalCounter)/((avgPrecisionInternal/internalCounter)+(avgRecallInternal/internalCounter))) << endl << endl;
	
	wclog << "Number of these:      " << internalStrictCounter << endl;
	wclog << "Average precision si: " << avgPrecisionInternalStrict/internalStrictCounter << endl;
	wclog << "Average recall si:    " << avgRecallInternalStrict/internalStrictCounter << endl;
	//	wclog << "Average F-score:        " << avgFScoreInternalStrict/counter << endl;
	wclog << "Average F-score si:   " << (2*(avgPrecisionInternalStrict/internalStrictCounter)*(avgRecallInternalStrict/internalStrictCounter)/((avgPrecisionInternalStrict/internalStrictCounter)+(avgRecallInternalStrict/internalStrictCounter))) << endl << endl;
#endif //LATTICE
#else //TRAINING
	
	wifstream input("/Users/ventzi/Desktop/курсове - университет/ATTEMPT/data/HomeCentre/en-fr HomeCenter data.txt");
	TreePair treePair;
	while (!input.eof()) {
		input >> treePair;
		if (!TreePair::error) {
			//			wcout << "TreePair No" << counter++ << ":" << endl;
			//			wcout << treePair;
			treePair.saveContexts();
			//			wcout << endl;
		}
	//		if (counter > 200) break;
	}
//	input.close();
//	exit(66);
//	wcout << endl;

	int counter = 1;
	input.clear();
	input.seekg(0);
	while (!input.eof()) {
		input >> treePair;
		if (!TreePair::error) {
			//		wcout << "TreePair No" << counter++ << ":" << endl;
			treePair.storeUnlinkedCounts();
			//			wcout << treePair;
			//			wcout << endl;
		}
	}
/*	wcout << TreePair::contexts.size() << " contexts total." << endl;
for (map<pair<wstring, wstring>, int>::const_iterator it = TreePair::contexts.begin(); it != TreePair::contexts.end(); ++it)
wcout << "“" << it->first.first << "||" << it->first.second << "” -> " << it->second << endl;
wcout << endl;*/
//	exit(66);
 
//	wcout << endl << "The automatic alignment sucks!" << endl;
	input.clear();
	input.seekg(0);
	TreePair::linked = false;
	counter = 1;
	while (!input.eof()) {
		input >> treePair;
		if (!TreePair::error) {
			++counter;
//			if (counter <= 806) continue;
			wcout << "TreePair No" << counter - 1 << ":" << endl;
			treePair.linkTrees();
			wcout << treePair;
			wcout << endl;
//			if (counter > 200) break;
		}
	}
#endif //#ifdef TRAINING

	currentTime = time(0);
	tm* local = localtime(&currentTime);
	sec = difftime(currentTime, startTime);
	wclog << endl << "Elapsed: " << sec << "sec" << endl;
	wclog << "TreePair average link time:                                  " << ((1.0 * treePairAverage) / (counter - 1)) << "    sec" << endl;
	wclog << "Finished at " << asctime(local);
	
//	wstring temp;
//	wcin >> temp;
	
	if (!logToStdLog) {
		wclog.rdbuf(saved_log);
		log.close();
	}
	
	return 0;
}
