/*
 *  LinkSet.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 24.12.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifndef __LINKSET
#define __LINKSET

#include "LinkHypothesis.h"
#ifdef SEP_PROB_TYPE
#include "ProbabilityType.h"
#endif

namespace bg_zhechev_ventsislav {
	class LinkSet {
		const LinkSet* baseSet;
		int base;
		linkAddressPair* last;
		bool subSet;
		//	doubleType* probability;
		//	bool probabilityIsCalculated;
	public:
		LinkSet();
		LinkSet(linkAddressPair* lastLink);
		LinkSet(const LinkSet* baseSet, int baseID, linkAddressPair* lastLink);
		~LinkSet();
		
		LinkSet& operator=(const LinkSet& source);
		
		const setOfLinks* Links() const ;
		inline const int& Base() const { return base; };
		inline linkAddressPair* Last() const { return last; };
		
		inline void setSubSet() { subSet = true; };
		inline bool isSubSet() const { return subSet; };
		
		doubleType probabilityMass(const hypothesesMap& hypotheses) const ;
	};
}

#endif