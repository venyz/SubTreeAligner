/*
 *  MultiTreePair.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 19.11.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifndef __MULTITREEPAIR
#define __MULTITREEPAIR

#include "TreePair.h"
#include "MultiTree.h"

namespace bg_zhechev_ventsislav {
	class MultiTreePair : public TreePair {
		MultiTree source, target;
	public:
		static string operation_mode;
		static string input_type;
		static string output_type;
		friend wostream& operator<<(wostream& out, MultiTreePair& treePair);
		friend wistream& operator>>(wistream& in, MultiTreePair& treePair);
	private:
		static void parsedToTagged(wistream& in, wostream& out);
	public:
		void printBracketed(wostream& out, bool is_XML = false) const ;
		void printXML(wostream& out) const ;
		
	private:
		void initLexicalInfo();
		void createSubStringLists();
#ifdef __debug7__
		void printLexMaps() const ;
#endif
		void prepareLexicalLinks();
		void printInducedLinks(bool printStatus, bool trees) const ;
#ifdef RESCORE
		bool cleanLexProbabilites(const linkAddressPair& link);
		void rescoreLinks();
#endif //RESCORE
		virtual void buildIndices();
		void eliminateCrossingForLink(const linkAddressPair& link);
		void eliminateLinksForIncompatibleNodes(const bigNumSet& sourceNodes, const bigNumSet& targetNodes);
		bool areCrossing(const linkAddressPair& first, const linkAddressPair& second);
		bool areIncompatible(const linkAddressPair& first, const linkAddressPair& second);
		
	public:
		void collectTags();
	private:
		void calculateInternalLinksStatistics(bool initial) const ;
	public:
		void calculateStatistics() const ;
		
#ifdef LATTICE
	private:
		void initLatticeData();
#endif //LATTICE
	};
}

#endif //__MULTITREEPAIR