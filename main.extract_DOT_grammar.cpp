/*
 *  main.extract_DOT_grammar.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 09.12.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "DOTTreePair.h"

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
	
	bool readStdIn = false, logToStdOut = true;
	wstreambuf *saved_in = NULL, *saved_out = NULL;
	wifstream input, srcInternals, trgInternals;
	wofstream srcGrammar, trgGrammar, externals, alignments, log;
	string inputFileName, srcInternalsFileName, trgInternalsFileName, srcGrammarFileName, trgGrammarFileName, externalsFileName, alignmentsFileName, logFileName = "";
	
	if (argc != 8 && argc != 6 && argc != 2) {
		wcerr << "Incorrect number of arguments: " << (argc - 1) << "! Usage:\n   " << argv[0] << " input [src_intern trg_intern] src_gram trg_gram extern align\n   " << argv[0] << " cfg_file" << endl;
		exit(1);
	} else if (argc == 8 || argc == 6) {
		inputFileName = argv[1];
		if (argc == 8) {
			srcInternalsFileName = argv[2];
			trgInternalsFileName = argv[3];
		}
		srcGrammarFileName = argv[argc - 4];
		trgGrammarFileName = argv[argc - 3];
		externalsFileName = argv[argc - 2];
		alignmentsFileName = argv[argc - 1];
	} else if (argc == 2) {
		configMapType config = readConfig(argv[1]);
		configMapType::const_iterator it = config.find("input");
		if (it != config.end())
			inputFileName = it->second;
		else
			inputFileName = "-";
		it = config.find("source_internal_only_labels");
		if (it != config.end())
			srcInternalsFileName = it->second;
		else
			srcInternalsFileName = "";
		it = config.find("target_internal_only_labels");
		if (it != config.end())
			trgInternalsFileName = it->second;
		else
			trgInternalsFileName = "";
		it = config.find("source_grammar");
		if (it != config.end())
			srcGrammarFileName = it->second;
		else { wcout << "“source_grammar” option missing in config file!!!"; exit(1); }
		it = config.find("target_grammar");
		if (it != config.end())
			trgGrammarFileName = it->second;
		else { wcout << "“target_grammar” option missing in config file!!!"; exit(1); }
		it = config.find("externals");
		if (it != config.end())
			externalsFileName = it->second;
		else { wcout << "“externals” option missing in config file!!!"; exit(1); }
		it = config.find("alignments");
		if (it != config.end())
			alignmentsFileName = it->second;
		else { wcout << "“alignments” option missing in config file!!!"; exit(1); }
		it = config.find("log");
		if (it != config.end())
			logFileName = it->second;
		wstringstream ss;
		it = config.find("top_category");
		if (it != config.end()) {
			ss << it->second.c_str();
			ss >> DOTTreePair::topCatString;
		}
		it = config.find("eos_category");
		if (it != config.end()) {
			ss << it->second.c_str();
			ss >> DOTTreePair::eosCatString;
		}
		it = config.find("eos_word");
		if (it != config.end()) {
			ss << it->second.c_str();
			ss >> DOTTreePair::eosWordString;
		}
	}
	
	if (inputFileName != "-") {
		input.open(inputFileName.c_str());
		if (input.fail()) { wcout << "!!!Cannot open " << inputFileName.c_str() << " file!!!" << endl; exit(1); }
		saved_in = wcin.rdbuf();
		wcin.rdbuf(input.rdbuf());
	} else
		readStdIn = true;
	if (srcInternalsFileName != "") {
		srcInternals.open(srcInternalsFileName.c_str());
		if (srcInternals.fail()) { wcout << "!!!Cannot open " << srcInternalsFileName.c_str() << " file!!!" << endl; exit(1); }
	}
	if (trgInternalsFileName != "") {
		trgInternals.open(trgInternalsFileName.c_str());
		if (trgInternals.fail()) { wcout << "!!!Cannot open " << trgInternalsFileName.c_str() << " file!!!" << endl; exit(1); }
	}
	srcGrammar.open(srcGrammarFileName.c_str());
	if (srcGrammar.fail()) { wcout << "!!!Cannot open " << srcGrammarFileName.c_str() << " file!!!" << endl; exit(1); }
	trgGrammar.open(trgGrammarFileName.c_str());
	if (trgGrammar.fail()) { wcout << "!!!Cannot open " << trgGrammarFileName.c_str() << " file!!!" << endl; exit(1); }
	externals.open(externalsFileName.c_str());
	if (externals.fail()) { wcout << "!!!Cannot open " << externalsFileName.c_str() << " file!!!" << endl; exit(1); }
	alignments.open(alignmentsFileName.c_str());
	if (alignments.fail()) { wcout << "!!!Cannot open " << alignmentsFileName.c_str() << " file!!!" << endl; exit(1);	}
	if (logFileName != "") {
		logToStdOut = false;
		log.open(logFileName.c_str());
		if (log.fail()) { wcout << "!!!Cannot open " << logFileName.c_str() << " file!!!" << endl; exit(1);	}
		saved_out = wcout.rdbuf();
		wcout.rdbuf(log.rdbuf());
	}
	
	DOTTreePair treePair;
	int counter = 1;
	
	//	output.unsetf(ios::fixed);
	//	output.precision(16);
	
	//	currentTime = time(0);
	
	//	wcout << "DATA_SET: " << DATA_SET << endl;
	
	//	DOPTree::internalOnlyLabels = wstringSet();
	if (srcInternals.is_open()) {
		while (!srcInternals.eof()) {
			wstring label;
			srcInternals >> label;
			DOPTree::internalOnlyLabels.insert(label);
			wcout << "Got internalOnlyLabel " << label << endl;
		}
		srcInternals.close();
	}
	if (trgInternals.is_open()) {
		while (!trgInternals.eof()) {
			wstring label;
			srcInternals >> label;
			DOPTree::internalOnlyLabels2.insert(label);
			wcout << "Got internalOnlyLabel " << label << endl;
		}
		trgInternals.close();
	}
	//	internalOnlyLabels.insert(L"#EOS#");
	
	/*	wstringstream temp_inp;
	 temp_inp << "(NP-1 (D-2 the)(NPadj-3 (Adj-4 green)(N-5 house)))" << endl << "(NP-1 (D-2 la)(NPadj-3 (N-4 maison)(Adj-5 verte)))" << endl << "1 1 2 2 3 3 4 5 5 4" << endl << endl << "(NP-1 (D-2 the)(NPadj-3 (Adj-4 small)(N-5 house)))" << endl << "(NP-1 (D-2 le)(NPadj-3 (Adj-4 petit)(N-5 domicile)))" << endl << "1 1 2 2 3 3 4 4 5 5" << endl << endl;
	 
	 temp_inp >> treePair;
	 treePair.collectPCFGRules(srcGrammar, trgGrammar);
	 treePair.printBracketed();
	 wcout << endl;
	 temp_inp >> treePair;
	 treePair.collectPCFGRules(srcGrammar, trgGrammar);
	 treePair.printBracketed();
	 wcout << endl;*/
	
#ifdef PROFILE
	chudInitialize();
	chudMarkPID(getpid(),1);
	chudAcquireRemoteAccess();
	chudStartRemotePerfMonitor("testthetester");
#endif
	
#ifdef HOMERUN
	int treePairNo __attribute__ ((unused)) = 450;
#endif
	while (!wcin.eof()) {
		wcin >> treePair;
		if (!DOTTreePair::error) {
//			wcout << "ﬂﬂﬂﬂﬂﬂﬂﬂﬂ " << DOTTreePair::error << " ÌÌÌÌÌÌÌÌÌ " << endl;
			++counter;
#ifdef HOMERUN
//			if (counter <= treePairNo) continue;
//			wcout << "TreePair №" << counter - 1 << ":" << endl;
#endif
			//			treePair.printXML();
			//			wcout << tree;
			treePair.collectPCFGRules(srcGrammar, trgGrammar, alignments);
			wcout << ".";
			//			wcout << endl;
			//			treePair.removeExtraNodes();
			//			treePair.printTaggedSentences();
			//			treePair.printSentences();
			//			treePair.freezeLinks();
			//			treePair.printBracketed();
			//			wcout << endl;
#ifdef HOMERUN
//			if (counter > treePairNo) break;
#endif
		} else {
//			wcout << "Tree №" << counter << ": !!!ERROR!!!" << endl;
		}
	}
#ifdef PROFILE
	chudStopRemotePerfMonitor();
	chudReleaseRemoteAccess();
#endif
	
	wcout << endl;
	
	if (!readStdIn)
		wcin.rdbuf(saved_in);
	input.close();
	
	DOTTreePair::outputPCFGRules(srcGrammar, trgGrammar, externals);
	srcGrammar.close();
	trgGrammar.close();
	externals.close();
	alignments.close();
	
	//	output.close();
	
	/*	chudStopRemotePerfMonitor();
	 chudReleaseRemoteAccess();*/
	
	currentTime = time(0);
	tm* local = localtime(&currentTime);
	sec = difftime(currentTime, startTime);
	wcout << endl << "Elapsed: " << sec << "sec" << endl;
	wcout << "Finished at " << asctime(local);
	
	if (!logToStdOut) {
		wcout.rdbuf(saved_out);
		log.close();
	}
	
	return 0;
}