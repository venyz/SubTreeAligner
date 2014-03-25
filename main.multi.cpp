/*
 *  main.multi.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 17.11.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

//#include <ctime>
#include "MultiTreePair.h"

namespace bg_zhechev_ventsislav {
	extern void buildLowercasingTable(const string& fileName);
	extern configMapType readConfig(const char*);
}

int main (int argc, char * const argv[]) {
	time_t startTime = time(0);
	time_t currentTime;
	unsigned long long sec;
	doubleType treePairAverage = 0;
		
#ifndef MACOSX
	locale::global(locale("en_US.UTF-8"));
#else
#ifdef DATA_DIR
	string uml = DATA_DIR;
#else
	string uml = "/Users/ventzi/Desktop/курсове - университет/ATTEMPT/software/TreeTester all";
#endif
	uml += "/umlauts.txt";
	buildLowercasingTable(uml);
#endif //#ifdef MACOSX

	bool readStdIn = false, writeToStdOut = true, logToStdLog = true;
	wstreambuf *saved_in = NULL, *saved_out = NULL, *saved_log = NULL;
	wifstream input;
	wofstream output, log;
	string inputFileName, srcWordsFileName, trgWordsFileName, phrasesFileName, outputFileName = "", logFileName = "";
	string operationMode, inputType, outputType;
	
	if (argc > 1 && argc != 8 && argc != 7 && argc != 2) {
		wcout << "Incorrect number of arguments: " << (argc - 1) << "! Usage:\n   " << argv[0] << " [op_mode in_type out_type s2t_lex_prob t2s_lex_prob [s2t_phr_prob] input]\n   " << argv[0] << " [cfg_file]" << endl;
		exit(1);
	} else if (argc == 7 || argc == 8) {
		operationMode = string(argv[1]);
		inputType = string(argv[2]);
		outputType = string(argv[3]);
		srcWordsFileName = argv[4];
		trgWordsFileName = argv[5];
		if (argc == 8) {
			phrasesFileName = argv[6];
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
		it = config.find("operation_mode");
		if (it != config.end())
			operationMode = it->second;
		else
			operationMode = "str2str";
		it = config.find("input_type");
		if (it != config.end())
			inputType = it->second;
		else
			inputType = "plain";
		it = config.find("output_type");
		if (it != config.end())
			outputType = it->second;
		else
			outputType = "parse";
	}
	
	MultiTreePair::operation_mode = operationMode;
	MultiTreePair::input_type = inputType;
	MultiTreePair::output_type = outputType;

	if (outputFileName != "") {
		writeToStdOut = false;
		output.open(outputFileName.c_str());
		if (output.fail()) { wcerr << "!!!Cannot open output file ‘" << outputFileName.c_str() << "’!!!" << endl; exit(1);	}
		saved_out = wcout.rdbuf();
		wcout.rdbuf(output.rdbuf());
	}
	if (inputFileName != "-") {
		input.open(inputFileName.c_str());
		if (input.fail()) { wcerr << "!!!Cannot open input file ‘" << inputFileName.c_str() << "’!!!" << endl; exit(1); }
		saved_in = wcin.rdbuf();
		wcin.rdbuf(input.rdbuf());
	} else
		readStdIn = true;
	if (logFileName != "") {
		logToStdLog = false;
		log.open(logFileName.c_str());
		if (log.fail()) { wcerr << "!!!Cannot open log file ‘" << logFileName.c_str() << "’!!!" << endl; exit(1);	}
		saved_log = wclog.rdbuf();
		wclog.rdbuf(log.rdbuf());
	}
	
	wclog << "Running align_str2str." << endl << endl;
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
	wclog << "Using " << inputType.c_str() << " input. Output as “" << outputType.c_str() << "”." << endl;
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
	wclog << "Input  file: " << (inputFileName  != "-" ? inputFileName : "standard input").c_str() << endl;
	wclog << "Output file: " << (outputFileName != "" ? outputFileName : "standard output").c_str() << endl;
	wclog << "Log    file: " << (logFileName    != "" ? logFileName    : "standard log").c_str() << endl;
	
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
	
	//	exit(66);
	
//	if (!readStdIn && input.fail()) {
//		wcout << "!!!Cannot open the pairs file!!!" << endl;
//		exit(1);
//	}
			
/*	MultiTree tree;
	wstringstream inp(L"((I)) ((can't)) ((wait)) ((for)) ((the)) ((stock broker)) ((.))\nI can wait for lunch .\n1 2 3 4");
	inp >> tree;
	wcout << "The last test sentence has length " << tree.sentLength() << endl;
	wcout << tree << endl << endl;
	inp >> tree;
	wcout << "The last test sentence has length " << tree.sentLength() << endl;
	wcout << tree << endl << endl;
	inp >> tree;
	wcout << "The last test sentence has length " << tree.sentLength() << endl;
	wcout << tree << endl << endl;*/
//	tree = L"(S (NP (NN bla))(VP (AUX bli)(V blo)))";
//	wcout << "The last test sentence has length " << tree.sentLength() << endl;
//	wcout << tree << endl << endl;
	
	
/*	PCFGRulesMap PCFGRules;
	
	output.unsetf(ios::fixed);
	output.precision(16);*/
	
//	MultiTree sourceTree, targetTree;
	MultiTreePair treePair;
	int counter = 1;
	
	currentTime = time(0);
	sec = difftime(currentTime, startTime);
	wclog << " initialisation done in " << sec << "sec" << endl << endl;
	
	
#ifdef PROFILE
	chudInitialize();
	chudMarkPID(getpid(),1);
	chudAcquireRemoteAccess();
	chudStartRemotePerfMonitor("testthetester");
#endif
	
	struct timeval segStartTime;
	struct timeval segCurTime;
	
//	wcerr << "ALIGN_STANDALONE is " << getenv("ALIGN_STANDALONE") << endl;
//	if (string(getenv("ALIGN_STANDALONE")) == "1") {
//		exit(98);
//	}

	int treePairNo __attribute__ ((unused)) = 5;
	while (!wcin.eof()) {
		wcin >> treePair;
		if (!MultiTreePair::error) {
			/*		input >> tree;
		if (tree.sentLength() > 0) {*/
			char * env = getenv("ALIGN_STANDALONE");
			if (env != NULL && string(env) == "1") {
				if (counter % 100 == 0)
					wcerr << L"; [" << counter << L"]" << endl;
				else if (counter % 10 == 0)
					wcerr << L"." << flush;
			}
			
			wclog << "TreePair №" << counter << ":" << endl;
#ifdef HOMERUN
			//			if (counter < treePairNo) continue;
//			if (counter > treePairNo) break;
#endif
			++counter;
			
			gettimeofday(&segStartTime, 0);

//			time_t treePairStart = time(0);
//			wcout << treePair;
			treePair.link();
//			treePairAverage += time(0) - treePairStart;
			
			gettimeofday(&segCurTime, 0);
			
			time_t sec = segCurTime.tv_sec - segStartTime.tv_sec - ((segCurTime.tv_usec - segStartTime.tv_usec) < 0 ? 1 : 0);
			suseconds_t susec = (segCurTime.tv_usec - segStartTime.tv_usec) < 0 ? (1000000 + segCurTime.tv_usec - segStartTime.tv_usec) : (segCurTime.tv_usec - segStartTime.tv_usec);
			
			treePairAverage += sec + susec/1000000.;
			
			wclog << "Sentence pair processed in " << sec << L"." << setw(6) << susec << L" sec" << endl;
			
//			wcout << treePair;
			treePair.printBracketed(wcout);
//			treePair.printXML(wcout);
			//			wcout << ".";
			wcout << endl;
			//			treePair.removeExtraNodes();
			//			treePair.printTaggedSentences();
			//			treePair.printSentences();
			//			treePair.freezeLinks();
			//			wcout << endl;
			treePair.collectTags();
			treePair.calculateStatistics();
		} else {
//			wcout << "Tree №" << counter << ": !!!ERROR!!!" << endl;
		}
	}

#ifdef PROFILE
	chudStopRemotePerfMonitor();
	chudReleaseRemoteAccess();
#endif

	wcout << endl;
	TreePair::printTags(wclog);
	wcout.precision(6);
	TreePair::printStatistics(wclog);
	
	if (!readStdIn) {
		wcin.rdbuf(saved_in);
		input.close();
	}
	if (!writeToStdOut) {
		wcout.rdbuf(saved_out);
		output.close();
	}
	
	currentTime = time(0);
	tm* local = localtime(&currentTime);
	sec = difftime(currentTime, startTime);
	wclog << endl << "Elapsed: " << sec << "sec" << endl;
	wclog << "TreePair average link time:                                  " << ((1. * treePairAverage) / (counter - 1)) << "    sec" << endl;
	wclog << "Finished at " << asctime(local);
	
	if (!logToStdLog) {
		wclog.rdbuf(saved_log);
		log.close();
	}
	
	return 0;
}