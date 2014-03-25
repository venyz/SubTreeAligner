/*
 *  LinkSet.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 24.12.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "LinkSet.h"

LinkSet::LinkSet() {
	baseSet = NULL;
	base = -1;
	last = NULL;
	subSet = false;
//	probabilityIsCalculated = false;
}

LinkSet::LinkSet(linkAddressPair* lastLink) {
	baseSet = NULL;
	base = -1;
	last = lastLink;
	subSet = false;
//	probabilityIsCalculated = false;
}

LinkSet::LinkSet(const LinkSet* bsSet, int baseID, linkAddressPair* lastLink) : baseSet(bsSet), base(baseID), last(lastLink) {
	subSet = false;
//	probabilityIsCalculated = false;
}

LinkSet::~LinkSet() {
	if (base == -1 && last != NULL)
		delete(last);
//	if (probability != NULL)
//		delete probability;
}

LinkSet& LinkSet::operator=(const LinkSet& source) {
	baseSet = source.baseSet;
	base = source.base;
	last = source.last;
	subSet = source.subSet;
//	probabilityIsCalculated = source.probabilityIsCalculated;
	return *this;
}

const setOfLinks* LinkSet::Links() const {
	setOfLinks* links = new setOfLinks();
	if (baseSet != NULL) {
		const setOfLinks* subLinks = baseSet->Links();
		links->assign(subLinks->begin(), subLinks->end());
		delete subLinks;
	}
	links->push_back(last);
	return links;
}

doubleType LinkSet::probabilityMass(const hypothesesMap& hypotheses) const {
/*	if (probabilityIsCalculated)
		return *probability;
	probabilityIsCalculated = true;*/
	doubleType probability = 0.;
	if (baseSet != NULL)
		probability += baseSet->probabilityMass(hypotheses);
	probability += hypotheses.find(*last)->second->probability()
#ifdef SEP_PROB_TYPE
	.getDouble()
#endif
	;

	return probability;
}