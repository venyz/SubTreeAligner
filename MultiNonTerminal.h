/*
 *  MultiNonTerminal.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 03.11.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifndef __MULTINONTERMINAL
#define __MULTINONTERMINAL

#include "NonTerminal.h"

namespace bg_zhechev_ventsislav {
	class MultiNonTerminal : public NonTerminal {
		MultiNonTerminal(wstring, int, NonTerminal*, int, bigNumber) {}
		MultiNonTerminal(int, wstring, int, NonTerminal*, int, bigNumber) {}
//		MultiNonTerminal(const NonTerminal&);
		
//		void setMother(NonTerminal*) {}
//		NonTerminal* getMother() const { return NULL; }
		
		void setSubtrees(bigNumber) {}
		bigNumber Subtrees() const { return 0; }
		
		multiNTVector mothers;
	public:
		MultiNonTerminal(const wstring& label, int position, int span = 0) : NonTerminal(label, position, NULL, span), mothers(multiNTVector()) {}
		MultiNonTerminal(const NonTerminal& source) : NonTerminal(source), mothers(multiNTVector()) {
#ifdef __debug8__
			const NonTerminal* mum = getMother();
			wcerr << "1. Setting mother ID as " << (mum == NULL ? 0 : getMother()->ID()) << "(" << mum << ") for MultiNonTerminal ID " << ID() << "(" << this << ")" << endl;
#endif
		}
		inline void addMother(const MultiNonTerminal* mother) { mothers.push_back(mother); }
		inline const multiNTVector& getMothers() const { return mothers; }
		inline void clearMothers() { mothers.clear(); }
	};
}

#endif