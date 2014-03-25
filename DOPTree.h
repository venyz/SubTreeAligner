/*
 *  DOPTree.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 08.12.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifndef __DOPTREE
#define __DOPTREE

#include "Tree.h"

namespace bg_zhechev_ventsislav {
	class DOPTree : public Tree {
		bigNumIndex externalNodes;
		extRulesMap externalPCFGRules;
	public:
		const linkAddressPair addExtraNodes(const wstring& topCatString, const wstring& eosCatString, const wstring& eosWordString, bool linked = false);
		DOPTree();
		DOPTree(wchar_t input[]);
		DOPTree(wstring input);
		DOPTree(const Tree& source);
		void reset();
		
		friend wistream& operator>>(wistream& in, DOPTree& tree);
		DOPTree& operator=(const DOPTree& source);
		void convert(const Tree& src, const Tree& trg, linksMap& links, bool reverse = false);
		bool isExternal(const bigNumber nodeID) const ;
		
		static void resetPCFGRulesStorage();
	private:
		static PCFGRulesMap PCFGRules;
#ifdef DOT_EXTRACTION
		static PCFGRulesMap PCFGRules2;
#endif
	public:
		static wstringSet internalOnlyLabels;
#ifdef DOT_EXTRACTION
		static wstringSet internalOnlyLabels2;
		static PCFGRulesMap extRulesDepot;
#endif
	private:
		void collectPCFGRules(wostream& output, const_nterm_iterator it, bool internal
#ifdef DOT_EXTRACTION
													, bool target
#endif
													)
#ifndef DOT_EXTRACTION
		const
#endif
		;
	public:
#ifdef DOT_EXTRACTION
		const extRulesMap&
#else
		void
#endif
		collectPCFGRules(wostream& output
#ifdef DOT_EXTRACTION
										 , bool target
#endif
										 )
#ifndef DOT_EXTRACTION
		const
#endif
		;
		static void outputPCFGRules(wostream& output
#ifdef DOT_EXTRACTION
																, bool target
#endif
																);
#ifdef DOT_EXTRACTION
		void convertAndStoreExtRules(const extRulesMap& rules) const ;
#endif
	};
}

#endif