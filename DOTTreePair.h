/*
 *  DOTTreePair.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 08.12.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifndef __DOTTREEPAIR
#define __DOTTREEPAIR

#include "TreePair.h"
#include "DOPTree.h"

namespace bg_zhechev_ventsislav {
	class DOTTreePair : public TreePair {
		DOPTree source, target;
		static PCFGRulesMap externalPCFGRules;
	public:
		static wstring topCatString;
		static wstring eosCatString;
		static wstring eosWordString;

		static bool error;
		
		friend wistream& operator>>(wistream& in, DOTTreePair& treePair);
		
		void reset();
		DOTTreePair& operator=(const TreePair& source);
		
		void printXML(wostream& out) const;
		void printBracketed(wostream& out, bool is_XML = false) const;
		void printSentences(wostream& out) const;
		
		void collectPCFGRules(wostream& srcGrammar, wostream& trgGrammar, wostream& alignments);
		static void outputPCFGRules(wostream& srcGrammar, wostream& trgGrammar, wostream& externals);
	};
}

#endif