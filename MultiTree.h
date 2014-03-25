/*
 *  MultiTree.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 07.06.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifndef __MULTITREE
#define __MULTITREE

#include "Tree.h"
#include "MultiNonTerminal.h"
//typedef map<addressType, set<Node*> > mainChartTypeMulti;

namespace bg_zhechev_ventsislav {
	wostream& operator<<(wostream& out, const multiNTVector& mothers);

	class MultiTree : public Tree {
	public:
		static string input_type;
		static bool error;
		
		void reset();

		friend wostream& operator<<(wostream& out, MultiTree& tree);
		friend wistream& operator>>(wistream& in, MultiTree& tree);
	private:
		void printBracketed(wostream& out, const linksMap& linkedNodes, const MultiNonTerminal* root, bool is_XML = false, bool preserveStructure = false) const ;
		void printBracketed(wostream& out, const linksMap& linkedNodes, const addressType& rootAddress, bool is_XML = false, bool preserveStructure = false) const ;
	public:
		void printBracketed(wostream& out, const linksMap& linkedNodes, bool is_XML = false, bool preserveStructure = false) const ;
		
	private:
		MultiTree& operator=(const wchar_t input[]);
		const mainChartType* getChart() const;
	public:
		const bigNumVector& descendants(bigNumber ID);
		const bigNumVector& nonDescendants(bigNumber ID);
		const bigNumVector& ancestors(bigNumber ID);
		const bigNumVector& nonAncestors(bigNumber ID);
		bool dominates(bigNumber source, bigNumber target);
	public:
		bigNumSet& getIncompatibleNodes(bigNumber nodeID);
		void eliminateIncompatibleNodes(const bigNumSet& nodes);
	private:
		void addWord(const wstring& word);
		void addWordWithPOS(const wstring& wordWithPOS);
	public:
		bool areIncompatible(bigNumber source, bigNumber target);
	private:
		void populateChart();
		void propagateMothers();
		void clearMothers();
	public:
		subStringsVector* getSubStrings() const ;
	};
}

#endif