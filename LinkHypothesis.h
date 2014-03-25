/*
 *  LinkHypothesis.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 12.03.07.
 *  Copyright 2007–2010 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifndef __LINKHYPOTHESIS
#define __LINKHYPOTHESIS

#ifdef SEP_PROB_TYPE
#include "ProbabilityType.h"
#endif

namespace bg_zhechev_ventsislav {
	class LinkHypothesis {

		ProbabilityType prob;
		bool calculated;
		unsigned short int linkStatus;	//0 – undecided; 1 – linked; 2 – blocked
		
		LinkHypothesis(const LinkHypothesis&) {}
	public:
		LinkHypothesis() : linkStatus(0), prob(1.), calculated(false) {}
		
		inline void operator*=(const ProbabilityType& factor) { prob *= factor; }
		inline bool operator==(const ProbabilityType& source) { return prob == source; }
		inline bool operator!() const { return !prob; }
		
		inline void setProbability(doubleType prob) { this->prob = prob; calculated = true; }
		inline const ProbabilityType& probability() const { return prob; }
		
		inline bool isDecided() const { return linkStatus != 0; }
		inline bool isLinked() const { return linkStatus == 1; }
		inline bool isBlocked() const { return linkStatus == 2; }
		inline wstring status() const { switch (linkStatus) { case 0: return L"undecided"; case 1: return L"linked"; case 2: return L"blocked"; default: return L"unknown"; } }
		
		inline void link() { linkStatus = 1; }
		inline void block() { linkStatus = 2; }
		
#ifdef RESCORE
		inline void setCalculated(bool value) { calculated = value; }
#endif
	};
}

#endif