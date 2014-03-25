/*
 *  Tree.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 17.11.06.
 *  Copyright 2006 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "Tree.h"

Tree::const_nterm_iterator_base::~const_nterm_iterator_base() {
//	if (node != NULL && node->Position() > tree->length)
//		delete dynamic_cast<const NonTerminal*>(node);
}

Tree::const_word_iterator::~const_word_iterator() {
//	if (node != NULL && node->Position() > tree->length)
//		delete dynamic_cast<const Terminal*>(node);
}

Tree::const_word_iterator& Tree::const_word_iterator::operator++() {              // Pre-increment
	//out-of-bounds condition
	if (node != NULL) {
		if (node->Position() >= tree->length)
			node = NULL;
		else
			node = dynamic_cast<Terminal*>(tree->chart.find(make_pair(node->Position() + 1, 0))->second);
	}
	return *this;
}

Tree::const_word_iterator Tree::const_word_iterator::operator++(int) {           // Post-increment
	const_word_iterator tmp = *this;
	operator++();
	return tmp;
}

Tree::const_preterm_iterator& Tree::const_preterm_iterator::operator++() {              // Pre-increment
	//out-of-bounds condition
	if (node != NULL) {
		if (node->Position() >= tree->length)
			node = NULL;
		else
			node = dynamic_cast<NonTerminal*>(tree->chart.find(make_pair(node->Position() + 1, 1))->second);
	}
	return *this;
}

Tree::const_preterm_iterator Tree::const_preterm_iterator::operator++(int) {           // Post-increment
	const_preterm_iterator tmp = *this;
	operator++();
	return tmp;
}

Tree::const_nterm_iterator::const_nterm_iterator(const Tree* tr, nodeIndexMap::const_iterator iter) : const_nterm_iterator_base(tr, iter->second), it(iter) {
	if (it == tree->nodeIndex.end())
		node = NULL;
}

Tree::const_nterm_iterator& Tree::const_nterm_iterator::operator++() {              // Pre-increment
	++it;
	if (it != tree->nodeIndex.end())
		node = it->second;
	else
		node = NULL;
	return *this;
}

Tree::const_nterm_iterator Tree::const_nterm_iterator::operator++(int) {           // Post-increment
	const_nterm_iterator tmp = *this;
	operator++();
	return tmp;
}

bool Tree::pairedMode = false;
bool Tree::binariseMode = true;
const linksMap* Tree::links = NULL;

bool Tree::checkInputIntegrity(const wstring& input) {
	return !(input.length() < 5 || input.find('(') == wstring::npos || input.find(' ') == wstring::npos ||
					 (input.find_first_not_of('(', input.find_first_not_of(SPACE)) < input.find('(') && input.find_first_not_of('(') < input.find_first_of(SPACE, input.find_first_not_of(SPACE))) ||
					 (input.find_last_not_of(')', input.find_last_not_of(SPACE)) > input.find_last_of(')') && input.find_last_not_of(')') > input.find_last_of(SPACE, input.find_last_not_of(SPACE))));
}

void Tree::init() {
	length = 0;
	chart = mainChartType();
	preTermSpanMap = map<bigNumber, bigNumVector*>();
	outsidePreTermSpanMap = map<bigNumber, bigNumVector*>();
	descendantMap = map<bigNumber, bigNumVector>();
	nonDescendantMap = map<bigNumber, bigNumVector>();
	ancestorMap = map<bigNumber, bigNumVector>();
	nonAncestorMap = map<bigNumber, bigNumVector>();
}

Tree::Tree() {
	init();
}

Tree::Tree(wchar_t input[]) {
	init();
	if (checkInputIntegrity(input)) {
		length = parse(wstring(input), NULL);
		indexNodes();
	} else {
		length = -1;
	}
//	if (length == -1) wcerr << endl << "Bad sentence!" << endl;
}

Tree::Tree(wstring input) {
	init();
	if (checkInputIntegrity(input)) {
		length = parse(input, NULL);
		indexNodes();
	} else {
		length = -1;
	}
//	if (length == -1) wcerr << endl << "Bad sentence!" << endl;
}

Tree::~Tree() {
#ifdef __debug__
	wcout << "!!!Destructing tree!!!" << endl;
#endif
	reset();
}

#ifdef __debug__
namespace bg_zhechev_ventsislav {
	int indent;
	void Indent() { for (int i = 0; i < indent; ++i) wcout << " "; }
}
#endif

int Tree::parse(wstring input, NonTerminal* mother) {
#ifdef __debug__
	if (mother != NULL) {
		indent += 2;
	} else {
		indent = 0;
	}
	wcout << endl;
	Indent();
	wcout << "parsing string " << input;
	if (mother != NULL)
		wcout << " for mother " << mother->Label() << endl;
	else
		wcout << " for no mother" << endl;
#endif
	str_pos_type start = input.find_first_not_of(SPACE);
	input = input.substr(start, input.find_last_not_of(SPACE) + 1 - start);
	start = input.find_first_not_of(SPACE, 1);
	if (input.find_first_of(SPACE, start) == wstring::npos)
		return -1;
	wstring nt_label = input.substr(start, input.find_first_of(SPACE, start) - start);
	start = input.find_first_of(SPACE, start);
	input = input.substr(input.find_first_not_of(SPACE, start),
											 input.find_last_not_of(SPACE) - input.find_first_not_of(SPACE, start));
	input = input.substr(0, input.find_last_not_of(SPACE) + 1);
#ifdef __debug__
	Indent();
	wcout << nt_label << endl;
	Indent();
	wcout << input << endl;
	Indent();
	wcout << index.current() << endl << endl;
#endif
	NonTerminal* current;
	if (!pairedMode)
		current = new NonTerminal(nt_label, (int)index.current(), mother);
	else {
		wchar_t* offset;
		bigNumber ind = wcstol(nt_label.substr(nt_label.find_last_of('-') + 1).c_str(), &offset, 0);
		nt_label = nt_label.substr(0, nt_label.find_last_of('-'));
		current = new NonTerminal(ind, nt_label, (int)index.current(), mother);
	}

	if (input.find('(') != wstring::npos && input.find(')') != wstring::npos &&
			input.find_first_of('(') < input.find_first_not_of('(') &&
			input.find_last_of(')') > input.find_last_not_of(')')) {
#ifdef __debug__
		Indent();
		wcout << "NT daughters\n";
#endif
		wstring next;
		while (getDaughter(input, next)) {
			if (length < 0) {
				delete (current); 
				return length;
			}
#ifdef __debug__
			Indent();
			wcout << input << endl;
			Indent();
			wcout << next << endl << endl;
#endif
			if (parse(next, current) < 0) {
				input = next;
				goto TERMINAL;
			}
#ifdef __debug__
			Indent();
			if (mother != NULL)
				wcout << "spans: " << mother->Span() << " and " << current->Span() << " for " << mother->Label() << " and " << current->Label() << endl;
			else
				wcout << "span (no mother): " << current->Span() << " for " << current->Label() << endl;
#endif
		}
		if (mother != NULL) mother->incSpan(current->Span());
	} else {
TERMINAL:
#ifdef __debug__
		Indent();
		wcout << "T daughter\n";
#endif
		chart[make_pair(index.current(), 0)] = new Terminal(input.substr(0, input.find_last_not_of(SPACE) + 1), (int)index.current());
		++index;
		current->addSplit(1);
		current->setSpan();
		if (mother != NULL) mother->incSpan();
	}

#ifdef __debug__
	Indent();
	wcout << "Adding " << current->Label() << " to chart at address " << current->Position() << ", " << current->Span() << endl;
#endif
	addressType address = make_pair(current->Position(), current->Span());
	mainChartType::iterator it = chart.find(address);
	if (it != chart.end()) {
		it->second->setLabel(current->Label() + L"::" + it->second->Label());
		delete(current);
		current = dynamic_cast<NonTerminal*>(it->second);
		current->setMother(mother);
	} else {
		if (binariseMode) binarise(current);
		chart[address] = current;
	}
	if (mother != NULL) mother->addSplit(current->Span());
	
#ifdef __debug__
	Indent();
	wcout << "exiting parse";
	if (mother != NULL)
		wcout << " for mother " << mother->Label() << " with current span " << mother->Span() << endl;
	else
		wcout << endl;
#endif
	
	if (mother != NULL) {
#ifdef __debug__
		indent -= 2;
#endif
		return 0;
	} else {
		return current->Span();
	}
}

//In addition to binarisation, this method calculates the number of possible subtrees the node could root
void Tree::binarise(NonTerminal* mother) {
	if (mother->Splits().size() < 3) {
		mother->setSubtrees(1);
		return;
	}
	if (mother->Splits().size() < 4) {
		mother->setSubtrees((dynamic_cast<NonTerminal*>(chart[make_pair(mother->Position(), mother->Splits()[1])])->Subtrees() + 1) *
												(dynamic_cast<NonTerminal*>(chart[make_pair(mother->Position() + mother->Splits()[1], mother->Splits()[2])])->Subtrees() + 1));
		return;
	}
#ifdef __debug__
	Indent();
	wcout << "Binarising node " << mother->Label() << " at position " << mother->Position() << ", with span " << mother->Span() << " and splits ";
	for (intVector::size_type i = 0; i < mother->Splits().size() - 1; wcout << mother->Splits()[i++] << ", ");
	wcout << mother->Splits()[mother->Splits().size() - 1] << endl;
#endif
	int firstSplit = mother->Splits()[1];
	//Generate the required intermediate node that will parent all daughters except the leftmost one
	if (Tree::pairedMode)
		NonTerminal::useExtraIDBase = true;
	NonTerminal* newNode = new NonTerminal(
//																				 chart[make_pair(mother->Position(), firstSplit)]->Label(),
																				 mother->Label(),
																				 mother->Position() + firstSplit,
																				 mother,
																				 mother->Span() - firstSplit,
																				 NonTerminal::binIndex(binContext(mother))
																				 );
	if (Tree::pairedMode)
		NonTerminal::useExtraIDBase = false;
	//Link all daughters except the leftmost one to the newly created node
	int curPos = mother->Position(); // + mother->Splits()[0] which is always zero
	for (intVector::size_type i = 2; i < mother->Splits().size(); ++i) {
		curPos += mother->Splits()[i - 1];
		dynamic_cast<NonTerminal*>(chart[make_pair(curPos, mother->Splits()[i])])->setMother(newNode);
		newNode->addSplit(mother->Splits()[i]);
	}
#ifdef __debug__
	Indent();
	wcout << "newNode splits: ";
	for (intVector::size_type i = 0; i < newNode->Splits().size() - 1; wcout << newNode->Splits()[i++] << ", ");
	wcout << newNode->Splits()[newNode->Splits().size() - 1] << endl;
#endif
	//Reset the mother node to only point to its original leftmost child and the newly created node
	mother->resetSplits();
	mother->addSplit(firstSplit);
	mother->addSplit(newNode->Span());
	//Binarise the newly created node in case the mother had more than three children
	binarise(newNode);
	//Recalculate the number of subtrees for the mother node
	mother->setSubtrees(
											(dynamic_cast<NonTerminal*>(chart[make_pair(mother->Position(), firstSplit)])->Subtrees() + 1) *
											(newNode->Subtrees())
											);
	//Save the new node in the tree chart
	chart[make_pair(newNode->Position(), newNode->Span())] = newNode;
}

wstring Tree::binContext(NonTerminal* mother) {
	wstringstream context;
	context << mother->Label();
	if (mother->binIndex())
		context << BIN_CONNECT << mother->binIndex();
	context << " ";
	int curPos = mother->Position();
	for (intVector::size_type i = 1; i < mother->Splits().size(); ++i) {
		curPos += mother->Splits()[i - 1];
		const NonTerminal* child = dynamic_cast<const NonTerminal*>(chart[make_pair(curPos, mother->Splits()[i])]);
		context << child->Label();
		if (child->binIndex())
			context << BIN_CONNECT << child->binIndex();
		context << " ";
	}
#ifdef __debug__
	Indent();
	wcout << " constructed context: " << context.str() << endl;
#endif
	return context.str();
}

const bool Tree::getDaughter(wstring& input, wstring& output) {
#ifdef __debug__
	Indent();
	wcout << "getting a daughter from string " << input << endl;
#endif
	if (input.length() == 0) return false;
	str_pos_type start = input.find('(');
	if (start == wstring::npos) {
		length = -1;
		return false;
	}
	unsigned int count = 1;
	str_pos_type i;
	const wchar_t* const newSPACE(L"() \f\n\r\t\v");
	for (i = start + 1; count > 0 && i < input.length(); ++i) {
#ifdef __debug__
		Indent();
		wcout << "⇝⇝ at position " << i << " for character " << input[i] << " with count " << count << endl;
#endif
		if (input[i] == '(') {
			//if there is any material before the next ), than this is a real opening (
			if (input.find_first_of(')', i + 1) >=
					input.find_first_not_of(SPACE, input.find_first_of(SPACE, input.find_first_not_of(newSPACE, i + 1)))) {
				++count;
#ifdef __debug__
				Indent();
				wcout << "⇈ increasing count" << endl;
#endif
			}
		}
		if (input[i] == ')') {
			wstring temp = input.substr(0, i);
#ifdef __debug__
			Indent();
			wcout << " ↳temp string is «" << temp << "»" << endl;
			Indent();
			wcout << "  ↳input string is «" << input << "»" << endl;
#endif
			//Let’s make sure that there is both a non-terminal label and a terminal label
			//locate the end of word-charachters
			str_pos_type j = temp.find_last_of(SPACE);
			//if there’s no SPACE, we must be within a label, so skip
			if (j == wstring::npos)
				continue;
			//if there’s material between the first SPACE and the current charachter, grab the SPACE just in case
			//the thing is, this particular SPACE might be our delimiter (when the label consists only of parentheses)
			if (j + 1 < i)
				++j;
			//crop temp to the important part of it
			//(there should definitely be a ( before the ) we are operating on here; otherwise we would’ve discarded the sentence)
			temp = temp.substr(0, j);
			temp = temp.substr(temp.find_last_of('('));
#ifdef __debug__
			Indent();
			wcout << " ↳That is what the new temp looks like: «" << temp << "»" << endl;
#endif
			//if there’re no SPACEs left in temp, then we are still within a label
			if (temp.find_first_of(SPACE) != wstring::npos &&
					(input.find('(', i + 1) == wstring::npos ||
					 (input.find_first_of('(', i + 1) < input.find_first_not_of(newSPACE, i + 1) &&
						input.find_first_of(')', input.find_first_of('(', i + 1)) > input.find_first_not_of(newSPACE, i + 1)))) {
				--count;
#ifdef __debug__
				Indent();
				wcout << "⇊ decreasing count to " << count << endl;
#endif
			}
		}
	}
	str_pos_type j = input.find_first_of('(', i);
	if (j != wstring::npos && input.find_last_of(')') > j) {
		output = input.substr(start, j - start);
		input = input.substr(j);
	} else {
		output = input;
		input = L"";
	}
	return true;
}

Tree::Tree(const Tree& source) {
	*this = source;
}

Tree& Tree::operator=(const Tree& source) {
	reset();
#ifdef __debug__
	wcout << endl << "Starting Tree assignment" << endl;
#endif
	for (mainChartType::const_iterator it = source.chart.begin(); it != source.chart.end(); ++it)
		if (const NonTerminal* node = dynamic_cast<const NonTerminal*>(it->second))
			chart[it->first] = new NonTerminal(*node);
		else
			chart[it->first] = new Terminal(*dynamic_cast<const Terminal*>(it->second));
	length = source.length;
	indexNodes();
	
	return *this;
}

const wstring Tree::sentence() const {
	if (length < 0) return L"";
	mainChartType::const_iterator it = chart.find(make_pair(1, 0));
	if (it != chart.end()) {
		wstring sentence = L"";
		for (int i = 1; i < length; sentence += chart.find(make_pair(i++, 0))->second->Label() + L" ");
		sentence += chart.find(make_pair(length, 0))->second->Label();
		return sentence;
	}
	return L"";
}

namespace bg_zhechev_ventsislav {
	
	wostream& operator<<(wostream& out, const Tree& tree) {
		mainChartType::const_iterator it;
		for (int j = tree.length; j > 0; j--)
			for (int i = 1; i <= tree.length - j + 1; ++i) {
				it = tree.chart.find(make_pair(i, j));
				if (it != tree.chart.end()) {
					NonTerminal* node = dynamic_cast<NonTerminal*>(it->second);
					out << "Node ID" << node->ID() << " " << node->Label();
					if (node->binIndex()) {
						out << BIN_CONNECT << node->binIndex();
					}
					if (!Tree::pairedMode) {
						out << " (" << node->Subtrees() << " subtree";
						if (node->Subtrees() > 1) out << "s";
						out << ") at position " << node->Position() << " with span " << node->Span();
					} else if (Tree::links != NULL) {
						linksMap::const_iterator it;
						it = Tree::links->find(node->ID());
						if (it != Tree::links->end()) {
#ifndef OUTPUT_SCORES
							out << " linked to target node " << it->second;
#else
							out << " linked to target node " << it->second.first << " with score " << it->second.second;
#endif
						} else {
							out << " not linked to target tree";
						}
					}
					out << " has daughter";
					if (node->Splits().size() > 2) out << "s";
					out << " ";
					intVector splits = node->Splits();
					int curPos = node->Position();
#ifdef __debug__
					wcout << "/" << splits.size() << "/ ";
#endif
					for (intVector::size_type k = 1; k < splits.size() - 1; ++k) {
						curPos += splits[k - 1];
						it = tree.chart.find(make_pair(curPos, splits[k]));
						if (it != tree.chart.end()) {
#ifdef __debug__
							wcout << "(" << curPos << ", " << splits[k] << ")";
#endif
							out << it->second->Label();
							if (NonTerminal* node = dynamic_cast<NonTerminal*>(it->second)) {
								if (node->binIndex() != 0)
									wcout << "%" << node->binIndex();
								wcout << "_" << node->ID();
							}
							out << " ";
						} else {
							exit(166);
						}
					}
					it = tree.chart.find(make_pair(curPos + splits[splits.size() - 2], splits[splits.size() > 2 ? splits.size() - 1 : 0]));
					if (it != tree.chart.end()) {
#ifdef __debug__
						wcout << "(" << (curPos + splits[splits.size() - 2]) << ", " << splits[splits.size() > 2 ? splits.size() - 1 : 0] << ")";
#endif
						out << it->second->Label();
						if (NonTerminal* node = dynamic_cast<NonTerminal*>(it->second)) {
							if (node->binIndex() != 0)
								wcout << "%" << node->binIndex();
							wcout << "_" << node->ID();
						}
						out << endl;
					} else {
						exit(167);
					}
				}
			};
		return out;
	}
	
	wistream& operator>>(wistream& in, Tree& tree) {
		tree.reset();
//		wchar_t input[10000];
//		in.getline(input, 10000);
		wstring input;
		getline(in, input);
		if (Tree::checkInputIntegrity(input)) {
//		if (Tree::checkInputIntegrity(wstring(input))) {
#ifdef __debug__
			wcerr << "Building tree from «" << input << "»" << endl;
#endif
			tree.length = tree.parse(input, NULL);
			tree.indexNodes();
		} else {
			tree.length = -1;
		}
//	if (tree.length == -1) wcerr << endl << "Bad sentence! " << input << endl;
		return in;
	}
	
} //namespace
	
int Tree::depthHelper(int pos, int span) const {
	intVector depth;
	const NonTerminal* node = dynamic_cast<const NonTerminal*>(chart.find(make_pair(pos, span))->second);
	const intVector& splits = node->Splits();
	if (splits.size() < 2) return 0;
	if (splits.size() < 3) return 1;
	for (intVector::size_type i = 1; i < splits.size(); ++i) {
		pos += splits[i - 1];
		depth.push_back(depthHelper(pos, splits[i]));
	}
	return 1 + *max_element(depth.begin(), depth.end());
}

void Tree::reset() {
	index.reset();
	for (mainChartType::const_iterator it = chart.begin(); it != chart.end(); ++it) {
		if (NonTerminal* node = dynamic_cast<NonTerminal*>(it->second))
			delete(node);
		else
			delete(dynamic_cast<Terminal*>(it->second));
	}
	chart.clear();
	length = 0;
	nodeIndex.clear();
	
	for (map<bigNumber, bigNumVector*>::const_iterator it = preTermSpanMap.begin(); it != preTermSpanMap.end(); delete((it++)->second));
	preTermSpanMap.clear();
	for (map<bigNumber, bigNumVector*>::const_iterator it = outsidePreTermSpanMap.begin(); it != outsidePreTermSpanMap.end(); delete((it++)->second));
	outsidePreTermSpanMap.clear();
	descendantMap.clear();
	nonDescendantMap.clear();
	ancestorMap.clear();
	nonAncestorMap.clear();
}

bigNumber Tree::numberOfFragments() const {
	mainChartType::const_iterator it;
	it = chart.find(make_pair(1, length));
	if (it != chart.end())
		return dynamic_cast<NonTerminal*>(it->second)->Subtrees();
	else
		return 0;
}

const NonTerminal* Tree::findNode(bigNumber ID) const {
	nodeIndexMap::const_iterator it = nodeIndex.find(ID);
	if (it != nodeIndex.end())
		return it->second;
	else
		return NULL;
}

const Node* Tree::findNode(const addressType& address) const {
	mainChartType::const_iterator it = chart.find(address);
	if (it != chart.end())
		return it->second;
	else
		return NULL;
}

void Tree::indexNodes() {
	nodeIndex.clear();
	for (mainChartType::iterator it = chart.begin(); it != chart.end(); ++it)
		if (NonTerminal* node = dynamic_cast<NonTerminal*>(it->second))
			nodeIndex[node->ID()] = node;
}

/* Deprecated due to the introduction of a faster score storage strategy
 const bigNumVector& Tree::preTermSpan(const NonTerminal& node) {
#ifdef __debug11__
	wcout << "<|||>address: " << node.Position() << ", " << node.Span() << " for ID " << node.ID() << endl;
	for (map<bigNumber, bigNumVector*>::const_iterator it = preTermSpanMap.begin(); it != preTermSpanMap.end(); ++it) {
		wcout << ">>|<<ID: " << it->first << ":";
		for (bigNumVector::size_type i = 0; i < it->second->size() - 1; ++i)
			wcout << " " << findNode(it->second->at(i))->Label() << "-" << it->second->at(i);
		wcout << endl;
	}
#endif
	map<bigNumber, bigNumVector*>::iterator it = preTermSpanMap.find(node.ID());
	if (it != preTermSpanMap.end()) {
#ifdef __debug11__
		wcout << " 0th display preTermSpan " << it->second->size() << ":";
		for (bigNumVector::size_type i = 0; i < it->second->size() - 1; ++i)
			wcout << " " << findNode(it->second->at(i))->Label() << "-" << it->second->at(i);
		wcout << endl;
#endif
		return *(it->second);
	}
	bigNumVector* span = new bigNumVector();
	for (int i = node.Position(); i < node.Position() + node.Span(); ++i)
		span->push_back(dynamic_cast<const NonTerminal*>(chart.find(make_pair(i, 1))->second)->ID());
	span->push_back(0);
#ifdef _OPENMP
#pragma omp critical (preTermSpan)
	{
		it = preTermSpanMap.find(node.ID());
		if (it == preTermSpanMap.end())
#endif
			preTermSpanMap.insert(it, make_pair(node.ID(), span));
#ifdef _OPENMP
		else {
			delete span;
			span = it->second;
		}
	} //omp critical
#endif
	return *span;
}

const bigNumVector& Tree::outsidePreTermSpan(const NonTerminal& node) {
#ifdef __debug11__
	wcout << "<|#|>We're outside for ID " << node.ID() << endl;
	for (map<bigNumber, bigNumVector*>::const_iterator it = outsidePreTermSpanMap.begin(); it != outsidePreTermSpanMap.end(); ++it) {
		wcout << ">>#<<ID: " << it->first << ":";
		for (bigNumVector::size_type i = 0; i < it->second->size() - 1; ++i)
			wcout << " " << findNode(it->second->at(i))->Label() << "-" << it->second->at(i);
		wcout << endl;
	}
#endif
	map<bigNumber, bigNumVector*>::iterator it = outsidePreTermSpanMap.find(node.ID());
	if (it != outsidePreTermSpanMap.end()) {
#ifdef __debug11__
		wcout << " 0th display outsidePreTermSpan " << it->second->size() << ":";
		for (bigNumVector::size_type i = 0; i < it->second->size() - 1; ++i)
			wcout << " " << findNode(it->second->at(i))->Label() << "-" << it->second->at(i);
		wcout << endl;
#endif
		return *(it->second);
		}
	const bigNumVector& span = preTermSpan(node);
	bigNumVector* outsideSpan = new bigNumVector();
	for (Tree::const_preterm_iterator inode = ptbegin(); inode != ptend(); ++inode) {
		bigNumber nodeID = inode->ID();
		if (nodeID < span[0] || nodeID > span[span.size() - 2])
			outsideSpan->push_back(nodeID);
	}
	outsideSpan->push_back(0);
#ifdef _OPENMP
#pragma omp critical (outsidePreTermSpan)
	{
		it = outsidePreTermSpanMap.find(node.ID());
		if (it == outsidePreTermSpanMap.end())
#endif
			outsidePreTermSpanMap.insert(it, make_pair(node.ID(), outsideSpan));
#ifdef _OPENMP
		else {
			delete outsideSpan;
			outsideSpan = it->second;
		}
	} //omp critical
#endif
	return *outsideSpan;
}*/

void Tree::descendantsHelper(const addressType& address, bigNumVector& vector) const {
	const NonTerminal* node = dynamic_cast<const NonTerminal*>(chart.find(address)->second);
	vector.push_back(node->ID());
	if (node->Splits().size() < 3) return;
	int curPos = node->Position();
	for (intVector::size_type i = 1; i < node->Splits().size(); ++i) {
		curPos += node->Splits()[i - 1];
		descendantsHelper(make_pair(curPos, node->Splits()[i]), vector);
	}
}

const bigNumVector& Tree::descendants(bigNumber ID) {
	map<bigNumber, bigNumVector>::const_iterator it = descendantMap.find(ID);
	if (it != descendantMap.end()) return it->second;
	bigNumVector& vector = descendantMap[ID];
	const NonTerminal* node = findNode(ID);
	if (node->Splits().size() < 3) return vector;
	int curPos = node->Position();
	for (intVector::size_type i = 1; i < node->Splits().size(); ++i) {
		curPos += node->Splits()[i - 1];
		descendantsHelper(make_pair(curPos, node->Splits()[i]), vector);
	}
	return vector;
}

const bigNumVector& Tree::nonDescendants(bigNumber ID, Tree& tree) {
	map<bigNumber, bigNumVector>::const_iterator it = nonDescendantMap.find(ID);
	if (it != nonDescendantMap.end()) return it->second;
	otherNodes(tree.descendants(ID), nonDescendantMap[ID]);
	return nonDescendantMap[ID];
}

const bigNumVector& Tree::ancestors(bigNumber ID) {
	map<bigNumber, bigNumVector>::const_iterator it = ancestorMap.find(ID);
	if (it != ancestorMap.end()) return it->second;
	bigNumVector& vector = ancestorMap[ID];
	const NonTerminal* mother = findNode(ID)->getMother();
	while (mother != NULL) {
		vector.push_back(mother->ID());
		mother = mother->getMother();
	}
	return vector;
}

const bigNumVector& Tree::nonAncestors(bigNumber ID, Tree& tree) {
	map<bigNumber, bigNumVector>::const_iterator it = nonAncestorMap.find(ID);
	if (it != nonAncestorMap.end()) return it->second;
	otherNodes(tree.ancestors(ID), nonAncestorMap[ID]);
	return nonAncestorMap[ID];
}

void Tree::otherNodes(const bigNumVector& source, bigNumVector& result) {
	bigNumIndex tempMap;
	for (bigNumVector::const_iterator i = source.begin(); i != source.end(); tempMap.insert(*(i++)));
	for (nodeIndexMap::const_iterator nodeIt = nodeIndex.begin(); nodeIt != nodeIndex.end(); ++nodeIt)
		if (tempMap.find(nodeIt->first) == tempMap.end()) result.push_back(nodeIt->first);
}

bool Tree::dominates(bigNumber source, bigNumber target, Tree& tree) {
//	if (source == target) return false;
	const bigNumVector& srcDescendants = tree.descendants(source);
	for (bigNumVector::const_iterator i = srcDescendants.begin(); i != srcDescendants.end(); ++i)
		if (*i == target) return true;
	return false;
}

/*void Tree::removeExtraNodes() {
	delete chart[make_pair(length, 0)];
	chart.erase(make_pair(length, 0));
	delete chart[make_pair(length, 1)];
	chart.erase(make_pair(length, 1));
	delete chart[make_pair(1, length)];
	chart.erase(make_pair(1, length));
	length--;
}*/

bool Tree::printBracketedHelper(wostream& out, addressType address, bool is_XML) const {
	const NonTerminal* node = dynamic_cast<const NonTerminal*>(chart.find(address)->second);
//ONLY used for dealing with HomeCentre multi-word units
/*	if (node->Label().substr(node->Label().length() - 1) == L"x" && node->Label() != L"VPaux") {
		wcout << chart.find(make_pair(node->Position(), 0))->second->Label();
		return true;
	}*/
	out << (is_XML ? "<" : "(") << node->Label();
	if (node->binIndex()) {
		out << "%" << node->binIndex();
	}
	out << "-" << node->ID() << (is_XML ? ">" : " ");
	if (node->Splits().size() < 3) {
		out << chart.find(make_pair(node->Position(), 0))->second->Label();
	} else {
		int curPos = node->Position();
		for (intVector::size_type i = 1; i < node->Splits().size(); ++i) {
			curPos += node->Splits()[i - 1];
			if (printBracketedHelper(out, make_pair(curPos, node->Splits()[i]), is_XML) && i != node->Splits().size() - 1)
				out << "+";
		}
	}
	if (is_XML)
		out << "</" << node->Label() << "-" << node->ID() << ">";
	else
		out << ")";
	return false;
}

void Tree::printTaggedSentence(wostream& out) const {
	for (int i = 1; i <= length; ++i)
		out << chart.find(make_pair(i, 1))->second->Label() << "->" << chart.find(make_pair(i, 0))->second->Label() << " ";
//		out << chart.find(make_pair(i, 1))->second->Label() << "|||" << chart.find(make_pair(i, 0))->second->Label() << " ";
	out << endl;
}

const wstring Tree::subStringsHelper(subStringsVector* strings, const addressType& address) const {
	const NonTerminal* node = dynamic_cast<const NonTerminal*> (chart.find(address)->second);
	wstring result = L"";
	if (node->Splits().size() < 3) {
		result = chart.find(make_pair(node->Position(), 0))->second->Label();
#ifdef LOWERCASE
		toLower(result);
#endif
	} else {
		int curPos = node->Position();
		for (bigNumVector::size_type i = 1; i < node->Splits().size() - 1; ++i) {
			curPos += node->Splits()[i - 1];
			result += subStringsHelper(strings, make_pair(curPos, node->Splits()[i])) + L" ";
		}
		result += subStringsHelper(strings, make_pair(curPos + node->Splits()[node->Splits().size() - 2], node->Splits()[node->Splits().size() - 1]));
	}
	(*strings)[node->ID()] = result;
#ifdef __debug2a__
	wcout << "stored: “" << result << "” -> " << node->ID() << endl;
#endif
	return result;
}

subStringsVector* Tree::getSubStrings() const {
	subStringsVector* strings = new subStringsVector();
	subStringsHelper(strings, addressType(1, length));
	return strings;
}