/*
 *  NonTerminal.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 17.11.06.
 *  Copyright 2006 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "NonTerminal.h"

#ifdef __debug__
namespace bg_zhechev_ventsislav {
	extern void Indent();
}
#endif

NonTerminal::const_child_iterator& NonTerminal::const_child_iterator::operator++() {				//Pre-increment
	if (splitPos < node->splits.size() - 2) {
		++splitPos;
		curPos += node->splits.at(splitPos);
		curAddress = make_pair(curPos, node->splits.at(splitPos + 1));
	} else if (splitPos >= node->splits.size() - 2) {
		splitPos = node->splits.size();
		curPos = 0;
	}
	
	return *this;
}

NonTerminal::const_child_iterator NonTerminal::const_child_iterator::operator++(int) {				//Post-increment
	const_child_iterator tmp = *this;
	operator++();
	return tmp;
}

Index NonTerminal::idBase = Index();
Index NonTerminal::extraIDBase = Index(1000000);
bool NonTerminal::useExtraIDBase = false;
Index NonTerminal::binBase = Index();
binContextMap NonTerminal::binContexts;

NonTerminal::NonTerminal() : Node() {
	binID = 0;
	subtrees = 0;
	id = useExtraIDBase ? extraIDBase++ : idBase++;
	splits.push_back(0);
	span = 0;
	mother = NULL;
}

NonTerminal::NonTerminal(wstring label, int position, NonTerminal* mother, int span, bigNumber binIndex, const intVector& splits, bigNumber subtrees) : Node(label, position) {
#ifdef __debug__
	Indent();
	wcout << "creating non-terminal with label " << label << endl;
#endif
	binID = binIndex;
	this->subtrees = subtrees;
	id = useExtraIDBase ? extraIDBase++ : idBase++;
	if (splits.empty())
		this->splits.push_back(0);
	else
		this->splits.assign(splits.begin(), splits.end());
	this->span = span;
	this->mother = mother;
}

NonTerminal::NonTerminal(bigNumber id, wstring label, int position, NonTerminal* mother, int span, bigNumber binIndex) : Node(label, position) {
#ifdef __debug__
	Indent();
	wcout << "creating non-terminal with label " << label << endl;
#endif
	binID = binIndex;
	subtrees = 0;
	this->id = id;
	splits.push_back(0);
	this->span = span;
	this->mother = mother;
}

NonTerminal::NonTerminal(const NonTerminal& source) : Node(source.label, source.position) {
#ifdef __debug__
	wcout << "copying pointer " << &source << " to pointer " << this << " with label " << this->label << endl;
#endif
	binID = source.binID;
	subtrees = source.subtrees;
	id = source.id;
	splits.assign(source.splits.begin(), source.splits.end());
	span = source.span;
	mother = source.mother;
}

const bigNumber NonTerminal::binIndex(wstring context) {
	binContextMap::iterator it = binContexts.find(context);
	if (it != binContexts.end())
		return it->second;
	else {
		binContexts[context] = binBase++;
		return binContexts[context];
	}
}

#ifdef __debug2__
void NonTerminal::printBinMap() {
	wcout << binContexts.size() << " binarisation contexts." << endl;
	for (binContextMap::const_iterator it = binContexts.begin(); it != binContexts.end(); ++it)
		wcout << it->first << " -> " << it->second << endl;
}
#endif

void NonTerminal::addSplit(const int span) {
#ifdef __debug__
	Indent();
	wcout << "---> Adding split " << span << " to Node " << label << "…" << endl;
#endif
	splits.push_back(span);
}

void NonTerminal::resetSplits() {
	splits.clear();
	splits.push_back(0);
}