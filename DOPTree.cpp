/*
 *  DOPTree.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 08.12.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "DOPTree.h"

const linkAddressPair DOPTree::addExtraNodes(const wstring& topCatString, const wstring& eosCatString, const wstring& eosWordString, bool linked) {
	++length;
	//	NonTerminal::useExtraIDBase = true;
	chart[make_pair(length, 0)] = new Terminal(eosWordString, length);
	NonTerminal* preTerminal = new NonTerminal(linked ? eosCatString + LNK_CONNECT + eosCatString : eosCatString, length, NULL, 1);
	preTerminal->addSplit(1);
	chart[make_pair(length, 1)] = preTerminal;
	binarise(preTerminal);
	NonTerminal* root = new NonTerminal(linked ? topCatString + LNK_CONNECT + topCatString : topCatString, 1, NULL, length);
	root->addSplit(length - 1);
	root->addSplit(1);
	chart[make_pair(1, length)] = root;
	binarise(root);
	preTerminal->setMother(root);
	//	NonTerminal::useExtraIDBase = false;
	indexNodes();
#ifdef DOT_EXTRACTION
	externalNodes.insert(root->ID());
	externalNodes.insert(preTerminal->ID());
#endif
	
	return make_pair(root->ID(), preTerminal->ID());
}

DOPTree::DOPTree() : Tree() {}

DOPTree::DOPTree(wchar_t input[]) : Tree(input) {
//	addExtraNodes();
}

DOPTree::DOPTree(wstring input) : Tree(input) {
//	addExtraNodes();
}

DOPTree::DOPTree(const Tree& source) : Tree(source) {
//	addExtraNodes();
}

void DOPTree::reset() {
	Tree::reset();
	externalNodes.clear();
	externalPCFGRules.clear();
}

namespace bg_zhechev_ventsislav {
	wistream& operator>>(wistream& in, DOPTree& tree) {
		tree.reset();
		Tree temp;
		in >> temp;
		if (temp.sentLength() < 0) {
			tree.length = -1;
		} else {
			for (Tree::const_word_iterator word = temp.wbegin(); word != temp.wend(); ++word)
				tree.chart[make_pair(word->Position(), 0)] = new Terminal(*word);
			for (Tree::const_nterm_iterator node = temp.ntbegin(); node != temp.ntend(); ++node)
				tree.chart[node->Address()] = new NonTerminal(*node);
			tree.length = temp.sentLength();
			tree.indexNodes();
//			tree.addExtraNodes();
		}
		return in;
	}
}

DOPTree& DOPTree::operator=(const DOPTree& source) {
	reset();
	for (mainChartType::const_iterator it = source.chart.begin(); it != source.chart.end(); ++it)
		if (const NonTerminal* node = dynamic_cast<const NonTerminal*>(it->second))
			chart[it->first] = new NonTerminal(*node);
		else
			chart[it->first] = new Terminal(*dynamic_cast<const Terminal*>(it->second));
	length = source.length;
	indexNodes();
	externalNodes.insert(source.externalNodes.begin(), source.externalNodes.end());
	
	return *this;
}

void DOPTree::convert(const Tree& src, const Tree& trg, linksMap& links, bool reverse) {
	reset();
	for (Tree::const_word_iterator word = src.wbegin(); word != src.wend(); ++word)
		chart[make_pair(word->Position(), 0)] = new Terminal(*word);
	linksMap newLinks;
	for (Tree::const_nterm_iterator node = src.ntbegin(); node != src.ntend(); ++node) {
		linksMap::iterator link = links.find(node->ID());
		if (link != links.end()) {
			chart[node->Address()] = new NonTerminal((reverse ? trg.findNode(link->second)->Label() + LNK_CONNECT + node->Label() : node->Label() + LNK_CONNECT + trg.findNode(link->second)->Label()), node->Position(), node->getMother(), node->Span(), node->binIndex(), node->Splits(), node->Subtrees());
#ifdef __debug1_ex_dot__
			wcout << "…converting ID " << link->first << " to ID ";
#endif
			newLinks[link->second] = dynamic_cast<const NonTerminal*>(chart[node->Address()])->ID();
			externalNodes.insert(dynamic_cast<const NonTerminal*>(chart[node->Address()])->ID());
#ifdef __debug1_ex_dot__
			wcout << dynamic_cast<const NonTerminal*>(chart[node->Address()])->ID() << " (linking to " << link->second << ")" << endl;
#endif
		} else
			chart[node->Address()] = new NonTerminal(node->Label(), node->Position(), node->getMother(), node->Span(), node->binIndex(), node->Splits(), node->Subtrees());
	}
	length = src.sentLength();
	indexNodes();
	links.clear();
	links.insert(newLinks.begin(), newLinks.end());
}

bool DOPTree::isExternal(const bigNumber nodeID) const {
	return (externalNodes.find(nodeID) != externalNodes.end());
}

void DOPTree::resetPCFGRulesStorage() {
	PCFGRules.clear();
	internalOnlyLabels.clear();
}

PCFGRulesMap DOPTree::PCFGRules = PCFGRulesMap();
wstringSet DOPTree::internalOnlyLabels = wstringSet();
#ifdef DOT_EXTRACTION
PCFGRulesMap DOPTree::PCFGRules2 = PCFGRulesMap();
wstringSet DOPTree::internalOnlyLabels2 = wstringSet();
PCFGRulesMap DOPTree::extRulesDepot = PCFGRulesMap();
#endif

void DOPTree::collectPCFGRules(wostream& output, const_nterm_iterator root, bool internal
#ifdef DOT_EXTRACTION
															 , bool target
#endif
)
#ifndef DOT_EXTRACTION
const
#endif
{
#ifdef DOT_EXTRACTION
	PCFGRulesMap& PCFGRules = target ? PCFGRules2 : DOPTree::PCFGRules;
#endif
	wstring rootLabel = root->Label();
	wstringstream ss;
	if (root->binIndex())
		ss << BIN_CONNECT << root->binIndex();
	if (internal)
		ss << GDM_CONNECT << root->ID();
	rootLabel += ss.str();
	PCFGRulesMap::iterator rootRules = PCFGRules.find(rootLabel);
	if (rootRules == PCFGRules.end())
		rootRules = PCFGRules.insert(make_pair(rootLabel, make_pair(root->Subtrees(), PCFGRulesMap::value_type::second_type::second_type()))).first;
	else
		rootRules->second.first += root->Subtrees();
	
	if (root->Span() == 1) {
		wstring rhsLabel = chart.find(make_pair(root->Position(), 0))->second->Label() + L"\t";
#ifdef __debug1_ex__
		wcout << "0≠looking at rule " << rootLabel << " -> " << rhsLabel << endl;
#endif
		if (internal) {
			output << rootLabel << "\t" << rhsLabel << "\t1" << endl;
		} else {
			PCFGRulesMap::value_type::second_type::second_type::iterator rhsit = rootRules->second.second.find(rhsLabel);
			if (rhsit == rootRules->second.second.end())
				rootRules->second.second[rhsLabel] = 1;
			else
				++(rhsit->second);
#ifdef DOT_EXTRACTION
			extRulesMap::iterator ritExt;
			if (!target) {
				wstring lhs = rootLabel + L"\t" + wordAtPosition(root->Position())->Label() + L"\t";
				ritExt = externalPCFGRules.find(lhs);
				if (ritExt == externalPCFGRules.end())
					ritExt = externalPCFGRules.insert(make_pair(lhs, make_pair(1, extRulesMap::value_type::second_type::second_type()))).first;
				else
					++(ritExt->second.first);
				
				bigNumVector rhs;
				rhs.push_back(links->find(root->ID())->second);
				ritExt->second.second.push_back(make_pair(1, rhs));
			}
#endif
#ifdef __debug1_ex__
			wcout << "  0>For " << rootLabel << " we now have " << rootRules->second.first << endl;
			wcout << "  0>  and for " << rootLabel << " -> " << rhsLabel << " we now have " << rootRules->second.second[rhsLabel] << endl;
#endif
		}
	} else {
		const NonTerminal* leftChild = dynamic_cast<const NonTerminal*>(chart.find(make_pair(root->Position(), root->Splits()[1]))->second);
		const NonTerminal* rightChild = dynamic_cast<const NonTerminal*>(chart.find(make_pair(leftChild->Position() + leftChild->Span(), root->Splits()[2]))->second);
		wstring rhsLabel;
		
		if (!leftChild->binIndex() && internalOnlyLabels.find(leftChild->Label()) == internalOnlyLabels.end() &&
#ifdef DOT_EXTRACTION
				externalNodes.find(leftChild->ID()) != externalNodes.end() &&
#endif
				!rightChild->binIndex() && internalOnlyLabels.find(rightChild->Label()) == internalOnlyLabels.end()
#ifdef DOT_EXTRACTION
				&& externalNodes.find(rightChild->ID()) != externalNodes.end()
#endif
				) {
			rhsLabel = leftChild->Label() + L"\t" + rightChild->Label();
#ifdef __debug2_ex__
			wcout << "1≠looking at rule " << rootLabel << " -> " << rhsLabel << endl;
#endif
			if (internal) {
				output << rootLabel << "\t" << rhsLabel << "\t" << (1.0/root->Subtrees()) << endl;
			} else {
				PCFGRulesMap::value_type::second_type::second_type::iterator rhsit = rootRules->second.second.find(rhsLabel);
				if (rhsit == rootRules->second.second.end())
					rootRules->second.second[rhsLabel] = 1;
				else
					++(rhsit->second);
#ifdef DOT_EXTRACTION
				extRulesMap::iterator currRulePair;
				if (!target) {
					wstring srcRule = rootLabel + L"\t" + leftChild->Label() + L"\t" + rightChild->Label();
					currRulePair = externalPCFGRules.find(srcRule);
					if (currRulePair == externalPCFGRules.end())
						currRulePair = externalPCFGRules.insert(make_pair(srcRule, make_pair(1, extRulesMap::value_type::second_type::second_type()))).first;
					else
						currRulePair->second.first += 1;
					
					bigNumVector trgRule;
					trgRule.push_back(links->find(root->ID())->second);
					trgRule.push_back(links->find(leftChild->ID())->second);
					trgRule.push_back(links->find(rightChild->ID())->second);
					trgRule.push_back(leftChild->ID() < rightChild->ID() ? 0 : 1);
					currRulePair->second.second.push_back(make_pair(1, trgRule));
				}
#endif
#ifdef __debug2_ex__
				wcout << "  1>For " << rootLabel << " we now have " << rootRules->second.first << endl;
				wcout << "  1>  and for " << rootLabel << " -> " << rhsLabel << " we now have " << rootRules->second.second[rhsLabel] << endl;
#endif
			}
		}
		
		if (!rightChild->binIndex() && internalOnlyLabels.find(rightChild->Label()) == internalOnlyLabels.end()
#ifdef DOT_EXTRACTION
				&& externalNodes.find(rightChild->ID()) != externalNodes.end()
#endif
				) {
			wstringstream ss;
			ss << leftChild->Label();
			if (leftChild->binIndex())
				ss << BIN_CONNECT << leftChild->binIndex();
			ss << GDM_CONNECT << leftChild->ID() << "\t" << rightChild->Label();
			rhsLabel = ss.str();
#ifdef __debug2_ex__
			wcout << "2≠looking at rule " << rootLabel << " -> " << rhsLabel << endl;
#endif
			if (internal) {
				output << rootLabel << "\t" << rhsLabel << "\t" << ((1.0*leftChild->Subtrees())/root->Subtrees()) << endl;
			} else {
				PCFGRulesMap::value_type::second_type::second_type::iterator rhsit = rootRules->second.second.find(rhsLabel);
				if (rhsit == rootRules->second.second.end())
					rootRules->second.second[rhsLabel] = leftChild->Subtrees();
				else
					rhsit->second += leftChild->Subtrees();
#ifdef __debug2_ex__
				wcout << "  2>For " << rootLabel << " we now have " << rootRules->second.first << endl;
				wcout << "  2>  and for " << rootLabel << " -> " << rhsLabel << " we now have " << rootRules->second.second[rhsLabel] << endl;
#endif
			}
		}
		
		if (!leftChild->binIndex() && internalOnlyLabels.find(leftChild->Label()) == internalOnlyLabels.end()
#ifdef DOT_EXTRACTION
				&& externalNodes.find(leftChild->ID()) != externalNodes.end()
#endif
				) {
			wstringstream ss;
			ss << leftChild->Label() << "\t" << rightChild->Label();
			if (rightChild->binIndex())
				ss << BIN_CONNECT << rightChild->binIndex();
			ss << GDM_CONNECT << rightChild->ID();
			rhsLabel = ss.str();
#ifdef __debug2_ex__
			wcout << "3≠looking at rule " << rootLabel << " -> " << rhsLabel << endl;
#endif
			if (internal) {
				output << rootLabel << "\t" << rhsLabel << "\t" << ((1.0*rightChild->Subtrees())/root->Subtrees()) << endl;
			} else {
				PCFGRulesMap::value_type::second_type::second_type::iterator rhsit = rootRules->second.second.find(rhsLabel);
				if (rhsit == rootRules->second.second.end())
					rootRules->second.second[rhsLabel] = rightChild->Subtrees();
				else
					rhsit->second += rightChild->Subtrees();
#ifdef __debug2_ex__
				wcout << "  3>For " << rootLabel << " we now have " << rootRules->second.first << endl;
				wcout << "  3>  and for " << rootLabel << " -> " << rhsLabel << " we now have " << rootRules->second.second[rhsLabel] << endl;
#endif
			}
		}
		
		if (1) {
			wstringstream ss;
			ss << leftChild->Label();
			if (leftChild->binIndex())
				ss << BIN_CONNECT << leftChild->binIndex();
			ss << GDM_CONNECT << leftChild->ID() << "\t" << rightChild->Label();
			if (rightChild->binIndex())
				ss << BIN_CONNECT << rightChild->binIndex();
			ss << GDM_CONNECT << rightChild->ID();
			rhsLabel = ss.str();
#ifdef __debug2_ex__
			wcout << "4≠looking at rule " << rootLabel << " -> " << rhsLabel << endl;
#endif
			if (internal) {
				output << rootLabel << "\t" << rhsLabel << "\t" << ((1.0*leftChild->Subtrees()*rightChild->Subtrees())/root->Subtrees()) << endl;
			} else {
				PCFGRulesMap::value_type::second_type::second_type::iterator rhsit = rootRules->second.second.find(rhsLabel);
				if (rhsit == rootRules->second.second.end())
					rootRules->second.second[rhsLabel] = leftChild->Subtrees() * rightChild->Subtrees();
				else
					rhsit->second += leftChild->Subtrees() * rightChild->Subtrees();
#ifdef __debug2_ex__
				wcout << "  4>For " << rootLabel << " we now have " << rootRules->second.first << endl;
				wcout << "  4>  and for " << rootLabel << " -> " << rhsLabel << " we now have " << rootRules->second.second[rhsLabel] << endl;
#endif
			}
		}
	}
}

#ifdef DOT_EXTRACTION
const extRulesMap&
#else
void
#endif
DOPTree::collectPCFGRules(wostream& output
#ifdef DOT_EXTRACTION
													, bool target
#endif
)
#ifndef DOT_EXTRACTION
const
#endif
{
	for (const_nterm_iterator it = ntbegin(); it != ntend(); ++it) {
		if (!it->binIndex() && internalOnlyLabels.find(it->Label()) == internalOnlyLabels.end()
#ifdef DOT_EXTRACTION
				&& externalNodes.find(it->ID()) != externalNodes.end()
#endif
				)
			collectPCFGRules(output, it, 0
#ifdef DOT_EXTRACTION
											 , target
#endif
											 );
		if (
#ifdef DOT_EXTRACTION
				1
#else
				it->Span() != length
#endif
				)
			collectPCFGRules(output, it, 1
#ifdef DOT_EXTRACTION
											 , target
#endif
											 );
	}
	
#ifdef DOT_EXTRACTION
	return externalPCFGRules;
#endif
}

void DOPTree::outputPCFGRules(wostream& output
#ifdef DOT_EXTRACTION
															, bool target
#endif
) {
#ifdef DOT_EXTRACTION
	const PCFGRulesMap& PCFGRules = target ? PCFGRules2 : DOPTree::PCFGRules;
#endif
	for (PCFGRulesMap::const_iterator rule = PCFGRules.begin(); rule != PCFGRules.end(); ++rule)
		for (PCFGRulesMap::value_type::second_type::second_type::const_iterator rhs = rule->second.second.begin(); rhs != rule->second.second.end(); ++rhs) {
			output << rule->first << "\t" << rhs->first << "\t" << ((1.0*rhs->second)/rule->second.first) << endl;
		}
	
}

#ifdef DOT_EXTRACTION
void DOPTree::convertAndStoreExtRules(const extRulesMap& rules) const {
	for (extRulesMap::const_iterator rulePair = rules.begin(); rulePair != rules.end(); ++rulePair) {
		PCFGRulesMap::iterator extRulePair = extRulesDepot.find(rulePair->first);
		if (extRulePair == extRulesDepot.end())
			extRulePair = extRulesDepot.insert(make_pair(rulePair->first, make_pair(rulePair->second.first, PCFGRulesMap::value_type::second_type::second_type()))).first;
		else
			extRulePair->second.first += rulePair->second.first;
		for (extRulesMap::value_type::second_type::second_type::const_iterator trgRule = rulePair->second.second.begin(); trgRule != rulePair->second.second.end(); ++trgRule) {
			wstring trgRuleStr = findNode(trgRule->second[0])->Label() + L"\t";
			if (findNode(trgRule->second[0])->Span() == 1) {
#ifdef __debug1_dot__
				wcout << "unary>> " << rulePair->first << " >> " << trgRule->second[0] << endl;
#endif
				trgRuleStr += wordAtPosition(findNode(trgRule->second[0])->Position())->Label() + L"\t\tnx";
			} else {
				const NonTerminal *leftNode, *rightNode;
#ifdef __debug1_dot__
				wcout << "n-ary>> " << rulePair->first << " >> " << trgRule->second[0] << endl;
#endif
				const NonTerminal* rootNode = findNode(trgRule->second[0]);
				const intVector& splits = rootNode->Splits();
				leftNode = dynamic_cast<const NonTerminal*>(findNode(make_pair(rootNode->Position(), splits[1])));
				rightNode = dynamic_cast<const NonTerminal*>(findNode(make_pair(rootNode->Position() + splits[1], splits[2])));
				wstringstream ss;
				ss << leftNode->Label();
				if (leftNode->binIndex())
					ss << BIN_CONNECT << leftNode->binIndex();
				if (trgRule->second.size() == 1 || externalNodes.find(leftNode->ID()) == externalNodes.end())
					ss << GDM_CONNECT << leftNode->ID();
				ss << "\t" << rightNode->Label();
				if (rightNode->binIndex())
					ss << BIN_CONNECT << rightNode->binIndex();
				if (trgRule->second.size() == 1 || externalNodes.find(rightNode->ID()) == externalNodes.end())
					ss << GDM_CONNECT << rightNode->ID();
				ss << "\t";
				if (trgRule->second.size() == 1 || trgRule->second[3] == 0)
					ss << "n";
				ss << "x";
				trgRuleStr += ss.str();
			}
#ifdef __debug1_dot__
			wcout << " <<<storing: " << trgRule->first << endl;
#endif
			PCFGRulesMap::value_type::second_type::second_type::iterator extTrgRule = extRulePair->second.second.find(trgRuleStr);
			if (extTrgRule == extRulePair->second.second.end())
				extRulePair->second.second[trgRuleStr] = trgRule->first;
			else
				extTrgRule->second += trgRule->first;
		}
	}
}
#endif