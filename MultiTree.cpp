/*
 *  MultiTree.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 07.06.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "MultiTree.h"

string MultiTree::input_type = "plain";
bool MultiTree::error = false;

void MultiTree::reset() {
	Tree::reset();
	error = false;
}

namespace bg_zhechev_ventsislav {
	wostream& operator<<(wostream& out, MultiTree& tree) {
		tree.propagateMothers();
		out << "#BOS" << endl;
		for (Tree::const_word_iterator it = tree.wbegin(); it != tree.wend(); ++it) {
			const NonTerminal* mother = dynamic_cast<const NonTerminal*>(tree.chart.find(make_pair(it->Position(), 1))->second);
			out << it->Label() << "\t" << mother->ID() << endl;
		}
		for (int j = 1; j <= tree.length; ++j)
			for (int i = 1; i <= tree.length - j + 1; ++i) {
				if (const MultiNonTerminal* node = dynamic_cast<const MultiNonTerminal*>(tree.chart.find(make_pair(i, j))->second)) {
					if (node->binIndex())
						continue;
					out << "#" << node->ID() << " " << node->Label();
//					if (node->binIndex())
//						out << BIN_CONNECT << node->binIndex();
					out << "\t";
					out << node->getMothers();
//					const multiNTVector& mothers = node->getMothers();
//					out << mothers;
//					if (mothers.empty())
//						out << "0 ";
//					else
//						for(multiNTVector::const_iterator mother = mothers.begin(); mother != mothers.end(); ++mother) {
//							out << (*mother)->ID() << " ";
//						}
					out << endl;
				}
			}
		out << "#EOS" << endl;
		tree.clearMothers();
		
		return out;
	}
	
	wostream& operator<<(wostream& out, const multiNTVector& mothers) {
		if (mothers.empty())
			out << "0 ";
		else
			for(multiNTVector::const_iterator mother = mothers.begin(); mother != mothers.end(); ++mother) {
				if ((*mother)->binIndex())
					out << (*mother)->getMothers();
				else
					out << (*mother)->ID() << " ";
			}
		
		return out;
	}
	
	wistream& operator>>(wistream& in, MultiTree& tree) {
		NonTerminal::idBase.reset(1);
#if defined(HAVE_BOOST_TOKENIZER_HPP) or defined(HAVE_LIBBOOST_REGEX)
		using namespace boost;
#endif
		tree.reset();
		wstring sent;
		getline(in, sent);
		if (sent.empty()) {
			MultiTree::error = true;
			return in;
		}
		if (MultiTree::input_type == "parsed") {
			tree = sent.c_str();
			if (tree.sentLength() < 0) {
				MultiTree::error = true;
				return in;
			}
		} else {
			if (sent.find(L"((") == wstring::npos) {
				if (MultiTree::input_type == "tagged") {
					wcerr << "!!! ERROR !!!\nWrong input format!" << endl;
					exit(31);
				}
#ifdef HAVE_BOOST_TOKENIZER_HPP
				str_pos_type firstChar = sent.find_first_not_of(SPACE);
//				wcerr << "First meaningful char is at pos " << firstChar << " and SPACE can be found at " << sent.find_first_of(SPACE, firstChar) << endl;
				if (sent.find_first_of(SPACE, firstChar) == wstring::npos) {
//					wcerr << "Adding single word «" << sent.substr(firstChar, sent.find_last_not_of(SPACE) - firstChar + 1) << "»" << endl;
					tree.addWord(sent.substr(firstChar, sent.find_last_not_of(SPACE) - firstChar + 1));
				} else {
					for (token_iterator_generator<char_separator<wchar_t>, wstring::const_iterator, wstring>::type it = make_token_iterator<wstring>(sent.begin(), sent.end(), char_separator<wchar_t>(SPACE)), end; it != end; ++it)
						tree.addWord(*it);
				}
#else
				wcerr << "!!! ERROR !!!\nboost/tokenizer.hpp is needed in order to read this format!" << endl;
				exit(22);
#endif //HAVE_BOOST_TOKENIZER_HPP
			} else {
#ifdef HAVE_LIBBOOST_REGEX
//			wcout << "Parsing " << MultiTree::input_type.c_str() << " sentence: " << endl << sent << endl;
				wchar_t* regex;
				if (MultiTree::input_type == "tagged")
					regex = (wchar_t*)(L"\\s*\\(\\(\\s*" L"(.+? .+?)" L"\\s*\\)\\)\\s*");
				else
					regex = (wchar_t*)(L"\\s*\\(\\(\\s*" L"(.+?)" L"\\s*\\)\\)\\s*");
				wregex re(regex);
				for (wsregex_iterator it(sent.begin(), sent.end(), re), end; it != end; ++it)
					if (MultiTree::input_type == "tagged")
						tree.addWordWithPOS((*it)[1].str());
					else
						tree.addWord((*it)[1].str());
#else
				wcerr << "!!! ERROR !!!\nlibboost_regex is needed in order to read this format!" << endl;
				exit(23);
#endif //HAVE_LIBBOOST_REGEX
			}
		}
		
		tree.populateChart();
		tree.indexNodes();
		return in;
	}
} //namespace

void MultiTree::printBracketed(wostream& out, const linksMap& linkedNodes, const MultiNonTerminal* root, bool is_XML, bool preserveStructure) const {
	if (is_XML)
		out << "<" << root->Label() << "-" << root->ID() << ">";
	else
		out << "(" << root->Label() << "-" << root->ID() << " ";
	if (root->Span() == 1)
		out << chart.find(make_pair(root->Position(), 0))->second->Label();
	else
		printBracketed(out, linkedNodes, root->Address(), is_XML, preserveStructure);
	if (is_XML)
		out << "</" << root->Label() << "-" << root->ID() << ">";
	else
		out << ")";
}

void MultiTree::printBracketed(wostream& out, const linksMap& linkedNodes, const addressType& rootAddress, bool is_XML, bool preserveStructure) const {
	if (rootAddress.second == 1) {
		out << chart.find(make_pair(rootAddress.first, 0))->second->Label();
		return;
	}
	int i = rootAddress.first;
	do {
		for (int j = rootAddress.second - max(1, (i - rootAddress.first)); j >= 1; --j) {
			if (const MultiNonTerminal* root = dynamic_cast<const MultiNonTerminal*>(chart.find(make_pair(i, j))->second))
				if (root->Span() == 1 || linkedNodes.find(root->ID()) != linkedNodes.end() ||
					(preserveStructure && !root->binIndex())) {
					printBracketed(out, linkedNodes, root, is_XML, preserveStructure);
					i += root->Span();
					break;
				}
		}
	} while (i < rootAddress.first + rootAddress.second);
}

void MultiTree::printBracketed(wostream& out, const linksMap& linkedNodes, bool is_XML, bool preserveStructure) const {
	const MultiNonTerminal* root = dynamic_cast<const MultiNonTerminal*>(chart.find(make_pair(1, length))->second);
	if (preserveStructure || linkedNodes.find(root->ID()) != linkedNodes.end())
		printBracketed(out, linkedNodes, root, is_XML, preserveStructure);
	else {
		if (is_XML)
			out << "<X-100000>";
		else
			out << "(X-100000 ";
		printBracketed(out, linkedNodes, root->Address(), is_XML, preserveStructure);
		if (is_XML)
			out << "</X-100000>";
		else
			out << ")";
	}
	out << endl;
}

MultiTree& MultiTree::operator=(const wchar_t input[]) {
	init();
	if (Tree::checkInputIntegrity(wstring(input)))
		length = parse(wstring(input), NULL);
	else
		length = -1;
	if (length < 0)
		error = true;
	
	return *this;
}

const mainChartType* MultiTree::getChart() const {
	return &chart;
}

const bigNumVector& MultiTree::descendants(bigNumber ID) {
	map<bigNumber, bigNumVector>::const_iterator it = descendantMap.find(ID);
	if (it != descendantMap.end()) return it->second;
	
	bigNumVector& vector = descendantMap[ID];
	const NonTerminal* node = findNode(ID);
	bigNumIndex tempMap;
	for (int j = node->Span() - 1; j > 0; --j)
		if (const NonTerminal* desc = dynamic_cast<const NonTerminal*>(chart.find(make_pair(node->Position(), j))->second)) {
			tempMap.insert(desc->ID());
			const bigNumVector& temp = descendants(desc->ID());
			for (bigNumVector::const_iterator it = temp.begin(); it != temp.end(); tempMap.insert(*(it++)));
			break;
		}
	for (int i = 1; i < node->Span(); ++i)
		if (const NonTerminal* desc = dynamic_cast<const NonTerminal*>(chart.find(make_pair(node->Position() + i, node->Span() - i))->second)) {
			tempMap.insert(desc->ID());
			const bigNumVector& temp = descendants(desc->ID());
			for (bigNumVector::const_iterator it = temp.begin(); it != temp.end(); tempMap.insert(*(it++)));
			break;
		}
	for (bigNumIndex::const_iterator it = tempMap.begin(); it != tempMap.end(); vector.push_back(*(it++)));
	
	return vector;
}

const bigNumVector& MultiTree::nonDescendants(bigNumber ID) {
	return Tree::nonDescendants(ID, *this);
}

const bigNumVector& MultiTree::ancestors(bigNumber ID) {
	map<bigNumber, bigNumVector>::const_iterator it = ancestorMap.find(ID);
	if (it != ancestorMap.end()) return it->second;
	
	bigNumVector& vector = ancestorMap[ID];
	const NonTerminal* node = findNode(ID);
	bigNumIndex tempMap;
	for (int j = node->Span() + 1; j <= length - node->Position() + 1; ++j)
		if (const NonTerminal* anc = dynamic_cast<const NonTerminal*>(chart.find(make_pair(node->Position(), j))->second)) {
			tempMap.insert(anc->ID());
			const bigNumVector& temp = ancestors(anc->ID());
			for (bigNumVector::const_iterator it = temp.begin(); it != temp.end(); tempMap.insert(*(it++)));
			break;
		}
	for (int i = node->Position() - 1; i > 0; --i)
		if (const NonTerminal* anc = dynamic_cast<const NonTerminal*>(chart.find(make_pair(i, node->Span() + node->Position() - i))->second)) {
			tempMap.insert(anc->ID());
			const bigNumVector& temp = ancestors(anc->ID());
			for (bigNumVector::const_iterator it = temp.begin(); it != temp.end(); tempMap.insert(*(it++)));
			break;
		}
	for (bigNumIndex::const_iterator it = tempMap.begin(); it != tempMap.end(); vector.push_back(*(it++)));
	
	return vector;
}

const bigNumVector& MultiTree::nonAncestors(bigNumber ID) {
	return Tree::nonAncestors(ID, *this);
}

bool MultiTree::dominates(bigNumber source, bigNumber target) {
	return Tree::dominates(source, target, *this);
}

bigNumSet& MultiTree::getIncompatibleNodes(bigNumber nodeID) {
	const NonTerminal* node = findNode(nodeID);
	bigNumSet* nodes = new bigNumSet();
	if (node->Span() == 1)
		return *nodes;
	for (int j = 2; j < length; ++j)
		for (int i = node->Position() - max(j - node->Span(), 0) - 1; i >= node->Position() - (min(j, node->Position()) - 1); --i)
			if (const NonTerminal* desc = dynamic_cast<const NonTerminal*>(chart.find(make_pair(i, j))->second))
				nodes->insert(desc->ID());
	for (int i = node->Position() + 1; i < node->Position() + node->Span(); ++i)
		for (int j = max(node->Span() - (i - node->Position()) + 1, 2); j <= length - i + 1; ++j)
			if (const NonTerminal* desc = dynamic_cast<const NonTerminal*>(chart.find(make_pair(i, j))->second))
				nodes->insert(desc->ID());
	return *nodes;
}

void MultiTree::eliminateIncompatibleNodes(const bigNumSet& nodes) {
	for (bigNumSet::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
		const MultiNonTerminal* node = dynamic_cast<const MultiNonTerminal*>(findNode(*it));
		chart.erase(node->Address());
		delete node;
	}
	indexNodes();
}

bool MultiTree::areIncompatible(bigNumber source, bigNumber target) {
	//	if (source == target) return false;
	const bigNumSet* srcIncompatible = &(getIncompatibleNodes(source));
	bool found = srcIncompatible->find(target) != srcIncompatible->end();
	//	for (bigNumSet::const_iterator i = srcIncompatible->begin(); !found && i != srcIncompatible->end(); ++i)
	//		if (*i == target) found = true;
	delete srcIncompatible;
	return found;
}

void MultiTree::addWord(const wstring& word) {
	++length;
	chart[make_pair(length, 0)] = new Terminal(word, length);
}

void MultiTree::addWordWithPOS(const wstring& wordWithPOS) {
	str_pos_type delimiter = wordWithPOS.find_first_of(SPACE, 0);
	wstring word = wordWithPOS.substr(delimiter + 1, wordWithPOS.length());
	wstring POS = wordWithPOS.substr(0, delimiter);
	addWord(word);
	chart[make_pair(length, 1)] = new MultiNonTerminal(POS, length, 1);

//	wcout << "-> For wordWithPOS “" << wordWithPOS << "” we added word “" << word << "” with POS “" << POS << "”." << endl;
}

void MultiTree::populateChart() {
	if (input_type == "parsed") {
		map<NonTerminal*, MultiNonTerminal*> nodeMap;
		for (mainChartType::iterator it = chart.begin(); it != chart.end(); ++it)
			if (NonTerminal* node = dynamic_cast<NonTerminal*>(it->second)) {
				MultiNonTerminal* multiNode = new MultiNonTerminal(*node);
				nodeMap.insert(make_pair(node, multiNode));
				it->second = multiNode;
				delete(node);
			}
		for (mainChartType::iterator it = chart.begin(); it != chart.end(); ++it)
			if (MultiNonTerminal* node = dynamic_cast<MultiNonTerminal*>(it->second)) {
#ifdef __debug8__
				const NonTerminal* mum = node->getMother();
				wcerr << "2. Setting mother ID as " << (mum == NULL ? 0 : nodeMap.find(node->getMother())->second->ID()) << "(" << node->getMother() << ") for MultiNonTerminal ID " << node->ID() << "(" << node << ")" << endl;
#endif
				NonTerminal* mother = node->getMother();
				if (mother != NULL) {
					node->setMother(nodeMap.find(mother)->second);
				} else {
					node->setMother(NULL);
				}
#ifdef __debug8__
				mum = node->getMother();
				wcerr << "  -> now the mother is " << (mum == NULL ? 0 : mum->ID()) << endl;
#endif
			}
	} else
		for (int j = (input_type == "tagged" ? 2 : 1); j <= length; ++j)
			for (int i = 1; i <= length - j + 1; ++i)
				chart[make_pair(i, j)] = new MultiNonTerminal(L"X", i, j);
}

void MultiTree::propagateMothers() {
	for (int j = length; j > 1; --j)
		for (int i = 1; i <= length - j + 1; ++i)
			if (const MultiNonTerminal* mother = dynamic_cast<const MultiNonTerminal*>(chart.find(make_pair(i, j))->second))
				for (int l = 1; l < j; ++l) {
					MultiNonTerminal *leftChild, *rightChild;
					if ((leftChild = dynamic_cast<MultiNonTerminal*>(chart.find(make_pair(mother->Position(), l))->second)) &&
							(rightChild = dynamic_cast<MultiNonTerminal*>(chart.find(make_pair(mother->Position() + l, j - l))->second))) {
						leftChild->addMother(mother);
						rightChild->addMother(mother);
					}
				}
}

void MultiTree::clearMothers() {
	for (mainChartType::iterator it = chart.begin(); it != chart.end(); ++it) {
		if (MultiNonTerminal* node = dynamic_cast<MultiNonTerminal*>(it->second)) {
			node->clearMothers();
		}
	}
}

subStringsVector* MultiTree::getSubStrings() const {
	subStringsVector* strings = new subStringsVector();
	
	for (const_nterm_iterator node = ntbegin(); node != ntend(); ++node) {
		wstring span = L"";
		for (int i = node->Position(); i < node->Position() + node->Span(); ++i) {
#ifdef LOWERCASE
			wstring result = dynamic_cast<const Terminal*>(chart.find(make_pair(i, 0))->second)->Label();
			toLower(result);
			span += result + L" ";
#else
			span += dynamic_cast<const Terminal*>(chart.find(make_pair(i, 0))->second)->Label() + L" ";
#endif
		}
		strings->insert(make_pair(node->ID(), span.substr(0, span.length() - 1)));
	}
	
	return strings;
}