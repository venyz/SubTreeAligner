/*
 *  TreePair.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 20.01.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "TreePair.h"

namespace bg_zhechev_ventsislav {
	extern wstring readChunk(wistream& input, wchar_t delim
#ifdef TMMT
													 , bool includeLast = false
#endif
													 );
}

#ifdef TRAINING
const bool TreePair::NODE_LABELS = true;
const bool TreePair::LEXICAL = true;
const bool TreePair::PRETERM = true;
#endif //TRAINING
int TreePair::SENTENCE_COUNT = 0;

#ifndef TRAINING
extAlignmentMap TreePair::sourceWordAlignments = extAlignmentMap();
extAlignmentMap TreePair::targetWordAlignments = extAlignmentMap();
extAlignmentMap TreePair::sourcePhraseAlignments = extAlignmentMap();
extAlignmentMap TreePair::targetPhraseAlignments = extAlignmentMap();
vcbMap TreePair::sourceVocabulary = vcbMap();
vcbMap TreePair::targetVocabulary = vcbMap();
#endif //TRAINING

bool TreePair::error = false;
bool TreePair::linked = true;
#ifdef TRAINING
linkContextMap TreePair::contexts = linkContextMap();
#endif //TRAINING

TreePair::TreePair() : source(Tree()), target(Tree()), sourceStrings(NULL), targetStrings(NULL), newSourceLexProbs(NULL), newTargetLexProbs(NULL) {}

void TreePair::reset() {
	source.reset();
	target.reset();
	
	links.clear();
#ifdef CHECK_MAN_LINKS
	manLinks.clear();
#endif
#ifdef TRAINING
	alignmentChart.clear();
#endif //TRAINING
	error = false;
#ifndef TRAINING
	if (sourceStrings != NULL) {
		sourceStrings->clear();
		delete(sourceStrings);
		sourceStrings = NULL;
	}
	if (targetStrings != NULL) {
		targetStrings->clear();
		delete(targetStrings);
		targetStrings = NULL;
	}
//	sourceLexProbs.clear();
//	targetLexProbs.clear();
	
	if (newSourceLexProbs != NULL)
		for (size_t i = 0; i <= size_t(source.sentLength()); delete [] newSourceLexProbs[i++]);
	if (newTargetLexProbs != NULL)
		for (size_t i = 0; i <= size_t(target.sentLength()); delete [] newTargetLexProbs[i++]);
	newSourceLexProbs = NULL;
	newTargetLexProbs = NULL;
	
	crossingLinksMap.clear();
	
	for (hypothesesMap::iterator it = inducedLinks.begin(); it != inducedLinks.end(); delete (it++)->second);
	inducedLinks.clear();
	linksProbabilityIndex.clear();
	one2oneLinksProbabilityIndex.clear();
	linksRowIndex.clear();
	linksColumnIndex.clear();
#ifdef LATTICE
	/*	for (vectorOfLinkSets::iterator it = maximalLinkSets.begin(); it != maximalLinkSets.end(); delete((it++)->second));
	 maximalLinkSets.clear();
	 for (linkPowerSet::iterator it = powerSet.begin(); it != powerSet.end(); delete(*(it++)));
	 powerSet.clear();*/
	linkStorage.clear();
	crossingLinks.clear();
	maximalLinkSets.clear();
	bestMaxLinkSet.clear();
#endif //#ifdef LATTICE
#endif //#ifndef TRAINING
}

TreePair::~TreePair() {
#ifndef TRAINING
	reset();
	//	for (hypothesesMap::iterator it = inducedLinks.begin(); it != inducedLinks.end(); delete (it++)->second);
#endif //TRAINING
}

void TreePair::readInLinks(const wstring& input) {
	if (input.empty())
		return;
	wstringstream in(input);
	bigNumber source, target;
	while (!in.eof()) {
		in >> source >> target;
#ifndef OUTPUT_SCORES
		links[source] = target;
#else
		links[source] = make_pair(target, 1);
#endif
	}
}

namespace bg_zhechev_ventsislav {
	wostream& operator<<(wostream& out, const TreePair& treePair) {
		treePair.printStandard(out);
		return out;
	}

	wistream& operator>>(wistream& in, TreePair& treePair) {
		treePair.reset();
		Tree::setLinks(&treePair.links);
#ifndef DOT_EXTRACTION
		if (DATA_SET != "HomeCentre")
			NonTerminal::idBase.reset(1);
#endif
		in >> treePair.source;
//		wcout << "÷˛÷˛÷˛÷˛ source length: " << treePair.source.sentLength() << endl;
		if (treePair.source.sentLength() < 0) {
			TreePair::error = true;
			return in;
		}
#ifndef DOT_EXTRACTION
		if (DATA_SET != "HomeCentre")
			NonTerminal::idBase.reset(1);
#endif
		in >> treePair.target;
		if (treePair.target.sentLength() < 0) {
			TreePair::error = true;
			return in;
		}
//		wchar_t input[1000];
		wstring input;
//		in.getline(input, 1000);
		getline(in, input);
		if (TreePair::linked)
			treePair.readInLinks(input);
//			treePair.readInLinks(wstring(input));
#ifdef CHECK_MAN_LINKS
		treePair.manLinks = treePair.links;
		treePair.links.clear();
#endif
//		in.getline(input, 5);
		getline(in, input);
		return in;
	}
}

#ifdef TRAINING
pair<wstring, const bigNumVector> TreePair::nodeContext(bigNumber nodeID, bool first, int depth = 0) const {
	wstring context = L"";
	bigNumVector frontier;
	const NonTerminal* node = source.findNode(nodeID);
	if (NODE_LABELS) {
		context += L"(" + node->Label();
	} else {
		context += L"(";
	}
	if (!LEXICAL)
		if (!first) {
			linksMap::const_iterator it = links.find(nodeID);
			if (it != links.end()) {
				context += L"__L)";
				frontier.push_back(it->second);
				return make_pair(context, frontier);
			}
		}
	if (node->Splits().size() < 3) {
		context += L" " + source.chart.find(make_pair(node->Position(), 0))->second->Label() + L")";
		return make_pair(context, frontier);
	}
#ifdef CONTEXT_DEPTH
	if (depth < CONTEXT_DEPTH) {
#endif
		int curPos = node->Position();
		for (intVector::size_type i = 1; i < node->Splits().size(); ++i) {
			curPos += node->Splits()[i - 1];
			if (const NonTerminal* dtrNode = dynamic_cast<NonTerminal*>(source.chart.find(make_pair(curPos, node->Splits()[i]))->second)) {
				pair<wstring, bigNumVector> dtr = nodeContext(dtrNode->ID(), false, depth + 1);
				context += dtr.first;
				for (bigNumVector::size_type i = 0; i < dtr.second.size(); frontier.push_back(dtr.second[i++]));
			}
		}
#ifdef CONTEXT_DEPTH
	}
#endif
	return make_pair(context + L")", frontier);
}

wstring TreePair::nodeContext(bigNumber nodeID, const bigNumVector& frontier, bool first, int depth = 0) const {
	wstring context = L"";
	const NonTerminal* node = target.findNode(nodeID);
	if (NODE_LABELS) {
		context += L"(" + node->Label();
	} else {
		context += L"(";
	}
	if (!LEXICAL)
		if (!first) {
			for (bigNumVector::size_type i = 0; i < frontier.size(); ++i) {
				if (nodeID == frontier[i]) {
					context += L"__L)";
					return context;
				}
			}
		}
	if (node->Splits().size() < 3) {
		context += L" " + target.chart.find(make_pair(node->Position(), 0))->second->Label() + L")";
		return context;
	}
#ifdef CONTEXT_DEPTH
	if (depth < CONTEXT_DEPTH) {
#endif
		int curPos = node->Position();
		for (intVector::size_type i = 1; i < node->Splits().size(); ++i) {
			curPos += node->Splits()[i - 1];
			if (const NonTerminal* dtrNode = dynamic_cast<NonTerminal*>(target.chart.find(make_pair(curPos, node->Splits()[i]))->second)) {
				context += nodeContext(dtrNode->ID(), frontier, false, depth + 1);
			}
		}
#ifdef CONTEXT_DEPTH
	}
#endif
	return context + L")";
}

pair<wstring, wstring> TreePair::nodeContext(linksMap::const_iterator link) const {
	pair<wstring, bigNumVector> sCon = nodeContext(link->first, true);
	return make_pair(sCon.first, nodeContext(link->second, sCon.second, true));
}

void TreePair::saveContexts() const {
	for (linksMap::const_iterator it = links.begin(); it != links.end(); ++it) {
		contextPair cont = nodeContext(it);
		//				wcout << "“" << cont << "”" << endl;
		contexts[cont]++;
	}
}

void TreePair::storeUnlinkedCounts() const {
	for (mainChartType::key_type::second_type j = 1; j <= source.sentLength(); ++j)
		for (mainChartType::key_type::first_type i = 1; i <= source.sentLength() - j + 1; ++i) {
			mainChartType::const_iterator it = source.chart.find(make_pair(i, j));
			if (it != source.chart.end()) {
				bigNumber sourceID = dynamic_cast<const NonTerminal*>(it->second)->ID();
				linksMap::const_iterator lit = links.find(sourceID);
				if (lit == links.end()) {
#ifdef __debug1__
					//					wcout << "Working on source nodeID " << sourceID << endl;
#endif
					pair<wstring, const bigNumVector> sourceContext = nodeContext(sourceID, true);
					for (linkContextMap::const_iterator it = contexts.begin(); it != contexts.end(); ++it)
						if (it->first.first == sourceContext.first) {
#ifdef __debug1__
							//							wcout << "sourceContext found: “" << it->first.first << "||”" << endl;
#endif
							contexts[make_pair(it->first.first, L"")]++;
							break;
						}
				}
			}
		}
}

map<bigNumber, int> TreePair::contextFound(pair<map<wstring, int>, pair<bigNumVector, bigNumVector> > context) const {
	map<bigNumber, int> nodes;
	map<wstring, int>::const_iterator it = context.first.find(L"");
	if (it != context.first.end())
		nodes[0] = it->second;
	for (mainChartType::key_type::second_type j = 1; j <= target.sentLength(); ++j)
		for (mainChartType::key_type::first_type i = 1; i <= target.sentLength() - j + 1; ++i) {
			mainChartType::const_iterator it = target.chart.find(make_pair(i, j));
			if (it != target.chart.end()) {
				bigNumber nodeID = dynamic_cast<const NonTerminal*>(it->second)->ID();
				bool linked = false;
				for (bigNumVector::size_type k = 0; k < context.second.second.size(); ++k) {
					if (context.second.second[k] == nodeID) {
#ifdef __debug1__
						wcout << ">>>>>Found an already linked node (" << nodeID << ")!!!<<<<<" << endl;
#endif
						linked = true;
						break;
					}
				}
				if (linked) continue;
				wstring cont = nodeContext(nodeID, context.second.first, true);
				for (map<wstring, int>::const_iterator it = context.first.begin(); it != context.first.end(); ++it) {
					if (it->first == cont) {
						nodes[nodeID] = it->second;
						break;
					}
				}
			}
		}
	return nodes;
}

void TreePair::linkTrees() {
	initAlignChart();
	for (mainChartType::key_type::second_type j = 2; j <= source.sentLength(); ++j)
		for (mainChartType::key_type::first_type i = 1; i <= source.sentLength() - j + 1; ++i) {
			linkNode(make_pair(i, j));
		};
	//	printAlignChart();
	writeLinks();
}

void TreePair::initAlignChart() {
	for (mainChartType::key_type::first_type i = 1; i <= source.sentLength(); ++i) {
		addressType address = make_pair(i, 1);
		
		const NonTerminal* node = dynamic_cast<const NonTerminal*>(source.chart.find(address)->second);
#ifdef __debug1__
		wcout << "Working with node " << node->ID() << ":" << endl;
#endif
		wstring sourceContext;
		if (NODE_LABELS) {
			sourceContext = L"(" + node->Label() + L" " + source.chart.find(make_pair(i, 0))->second->Label() + L")";
		} else {
			sourceContext = L"( " + source.chart.find(make_pair(i, 0))->second->Label() + L")";
		}
		bigNumVector empty;
		
		map<wstring, int> targetContexts;
		for (linkContextMap::const_iterator it = contexts.begin(); it != contexts.end(); ++it)
			if (it->first.first == sourceContext) {
#ifdef __debug1__
				//				wcout << "sourceContext found: “" << it->first.first << "||" << it->first.second << "” " << it->second << endl;
#endif
				targetContexts[it->first.second] = it->second;
			};
		
		map<bigNumber, doubleType> newTargetIDs;
		if (targetContexts.size() > 0) {
			map<bigNumber, int> targetIDs = contextFound(make_pair(targetContexts, make_pair(empty, empty)));
			if (targetIDs.size() > 0) {
				doubleType targetIDsTotalWeight = 0;
				for (map<bigNumber, int>::const_iterator it = targetIDs.begin(); it != targetIDs.end(); ++it) {
					targetIDsTotalWeight += it->second;
				}
				for (map<bigNumber, int>::const_iterator it = targetIDs.begin(); it != targetIDs.end(); ++it) {
					newTargetIDs[it->first] = (doubleType)it->second / targetIDsTotalWeight;
#ifdef __debug1__
					wcout << "  probability to link to node " << it->first << " in context “" << sourceContext << "”: " << newTargetIDs[it->first] * 100 << "%" << endl;
#endif
				}
			}
		}
		if (newTargetIDs.size() == 0) {
			newTargetIDs[0] = 1;
		}
		
		cellMap* cell;
		bool newCell = false;
		dynAlignmentMap::iterator it = alignmentChart.find(address);
		if (it != alignmentChart.end()) {
			cell = &(it->second);
		} else {
			cell = new cellMap();
			newCell = true;
		}
		for (map<bigNumber, doubleType>::const_iterator it = newTargetIDs.begin(); it != newTargetIDs.end(); ++it) {
			cellKey key;
			if (it->first == 0) {
				key = make_pair(sourceContext, 0);
			} else {
				key = make_pair(sourceContext, it->first);
				if (!LEXICAL)
					if (NODE_LABELS) {
						key = make_pair(L"(" + node->Label() + L"__L)", it->first);
					} else {
						key = make_pair(L"(__L)", it->first);
					}
			}
			(*cell)[key] = make_pair(it->second, vector<cellKey>());
		}
		if (newCell) {
			alignmentChart[address] = *cell;
			delete(cell);
		}
	}
}

void TreePair::printAlignChart() const {
	wcout << "Printing Alignment Chart:" << endl;
	for (addressType::second_type j = 1; j <= source.sentLength(); ++j)
		for (addressType::first_type i = 1; i <= source.sentLength() - j + 1; ++i) {
			addressType address = make_pair(i, j);
			dynAlignmentMap::const_iterator info = alignmentChart.find(address);
			if (info != alignmentChart.end()) {
				wcout << "position " << i << ", " << j << ":" << endl;
				for (cellMap::const_iterator it = info->second.begin(); it != info->second.end(); ++it) {
					wcout << "  link to " << it->first.second << " in context “" << it->first.first << "” with probability " << it->second.first * 100 << "%" << endl;
				}
			}
		}
	wcout << endl;
}
#endif //TRAINING

const linksMap& TreePair::Links() const {
	return links;
}

const Tree& TreePair::Target() const {
	return target;
}

const Tree& TreePair::Source() const {
	return source;
}

#ifdef TRAINING
bigNumVector TreePair::frontierHelper(const addressType& nodeAddress, const cellKey& dtrChoice, int depth = 0) const {
	bigNumVector frontier;
	if (dtrChoice.second == 0) {
		cellMap::const_iterator it = alignmentChart.find(nodeAddress)->second.find(dtrChoice);
		const NonTerminal* node = dynamic_cast<const NonTerminal*>(source.chart.find(nodeAddress)->second);
		if (node->Splits().size() > 2) {
			int curPos = node->Position();
			for(intVector::size_type i = 1; i < node->Splits().size(); ++i) {
				curPos += node->Splits()[i - 1];
				bigNumVector dtrFrontier = frontierHelper(make_pair(curPos, node->Splits()[i]), it->second.second[i - 1], depth + 1);
				frontier.insert(frontier.end(), dtrFrontier.begin(), dtrFrontier.end());
			}
		}
	} else {
		frontier.push_back(dtrChoice.second);
	}
	return frontier;
}

daughterContextMap TreePair::getContexts(const addressType& nodeAddress) const {
#ifdef __debug1__
	wcout << "Getting contexts for " << nodeAddress.first << ", " << nodeAddress.second << endl;
#endif
	daughterContextMap output;
	dynAlignmentMap::const_iterator cell = alignmentChart.find(nodeAddress);
	for (cellMap::const_iterator it = cell->second.begin(); it != cell->second.end(); ++it) {
#ifdef __debug1__
		wcout << "== cell element " << it->first.first << "; " << it->first.second << endl;
#endif
		bigNumVector frontier;
		if (!LEXICAL)
			if (it->first.second == 0) {
				const NonTerminal* node = dynamic_cast<const NonTerminal*>(source.chart.find(nodeAddress)->second);
				if (node->Splits().size() > 2) {		//if not a preTerminal
					int curPos = node->Position();
					for(intVector::size_type i = 1; i < node->Splits().size(); ++i) {
						curPos += node->Splits()[i - 1];
						bigNumVector dtrFrontier = frontierHelper(make_pair(curPos, node->Splits()[i]), it->second.second[i - 1]);
						frontier.insert(frontier.end(), dtrFrontier.begin(), dtrFrontier.end());
					}
				}
			} else {
				frontier.push_back(it->first.second);
			}
		output[it->first] = make_pair(it->second.first, frontier);
	}
	return output;
}

void TreePair::linkNode(const addressType& nodeAddress) {
	mainChartType::const_iterator it = source.chart.find(nodeAddress);
	if (it == source.chart.end())
		return;
	const NonTerminal* node = dynamic_cast<const NonTerminal*>(it->second);
#ifdef __debug1__
	wcout << "Working with node " << node->ID() << ":" << endl;
#endif
	vector<daughterContextMap> dtrContexts;
	int curPos = node->Position();
	for (intVector::size_type i = 1; i < node->Splits().size(); ++i) {
		curPos += node->Splits()[i - 1];
		dtrContexts.push_back(getContexts(make_pair(curPos, node->Splits()[i])));
	}
#ifdef __debug1__
	for (vector<daughterContextMap>::size_type i = 0; i < dtrContexts.size(); ++i) {
		wcout << " Daughter №" << i + 1 << " has " << dtrContexts[i].size() << " possible contexts." << endl;
	}
#endif
	processCombinations(nodeAddress, dtrContexts);
}

void TreePair::processCombinations(const addressType& nodeAddress, const vector<daughterContextMap>& dtrContexts, vector<daughterContextMap>::size_type index, vector<cellKey>& combination) {
#ifdef __debug1__
	int count = 1;
#endif
	for (daughterContextMap::const_iterator it = dtrContexts[index].begin(); it != dtrContexts[index].end(); ++it) {
#ifdef __debug1__
		wcout << "-->Dealing with combination " << index + 1 << ", " << count << endl;
		++count;
#endif
		combination.push_back(it->first);
		if (index == dtrContexts.size() - 1) {
			processACombination(nodeAddress, dtrContexts, combination);
		} else {
			processCombinations(nodeAddress, dtrContexts, index + 1, combination);
		}
		combination.pop_back();
	}
}

void TreePair::processCombinations(const addressType& nodeAddress, const vector<daughterContextMap>& dtrContexts) {
	vector<cellKey> combination;
	processCombinations(nodeAddress, dtrContexts, 0, combination);
}

void TreePair::processACombination(const addressType& nodeAddress, const vector<daughterContextMap>& dtrContexts, const vector<cellKey>& combination) {
	const NonTerminal* node = dynamic_cast<const NonTerminal*>(source.chart.find(nodeAddress)->second);
	
	wstring sourceContext;
	if (NODE_LABELS) {
		sourceContext = L"(" + source.chart.find(nodeAddress)->second->Label();
	} else {
		sourceContext = L"(";
	}
	bigNumVector targetFrontier;
	bigNumVector linkedNodes;
	doubleType combinationProbability = 1;
	int curPos = node->Position();
	for (bigNumVector::size_type i = 1; i < node->Splits().size(); ++i) {
		curPos += node->Splits()[i - 1];
		pair<wstring, pair<const bigNumVector, const bigNumVector> > dtrContext = nodeContext(make_pair(curPos, node->Splits()[i]), combination[i - 1]);
#ifdef __debug1__
		wcout << "Got frontier with " << dtrContext.second.first.size() << " elements: (";
		if (dtrContext.second.first.size() > 0) {
			for (bigNumVector::size_type j = 0; j < dtrContext.second.first.size() - 1; wcout << dtrContext.second.first[j++] << ",");
			wcout << dtrContext.second.first[dtrContext.second.first.size() - 1];
		}
		wcout << ")" << endl;
		wcout << "Got linkedNodes with " << dtrContext.second.second.size() << " elements: (";
		if (dtrContext.second.second.size() > 0) {
			for (bigNumVector::size_type j = 0; j < dtrContext.second.second.size() - 1; wcout << dtrContext.second.second[j++] << ",");
			wcout << dtrContext.second.second[dtrContext.second.second.size() - 1];
		}
		wcout << ")" << endl;
#endif
		for (int k = 0; k < dtrContext.second.second.size(); ++k)
			for (int l = 0; l < linkedNodes.size(); ++l)
				if (linkedNodes[l] == dtrContext.second.second[k]) {
#ifdef __debug1__
					wcout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
#endif
					return;
				};
		sourceContext += dtrContext.first;
		if (!LEXICAL)
			targetFrontier.insert(targetFrontier.end(), dtrContext.second.first.begin(), dtrContext.second.first.end());
		linkedNodes.insert(linkedNodes.end(), dtrContext.second.second.begin(), dtrContext.second.second.end());
		combinationProbability *= dtrContexts[i - 1].find(combination[i - 1])->second.first;
	}
	sourceContext += L")";
	
	map<wstring, int> targetContexts;
	for (linkContextMap::const_iterator it = contexts.begin(); it != contexts.end(); ++it)
		if (it->first.first == sourceContext) {
#ifdef __debug1__
			//			wcout << "sourceContext found: “" << it->first.first << "||" << it->first.second << "” " << it->second << endl;
#endif
			targetContexts[it->first.second] = it->second;
		};
#ifdef __debug1__
	wcout << targetContexts.size() << " targetContexts found initially" << endl;
#endif
	
	map<bigNumber, doubleType> newTargetIDs;
	if (targetContexts.size() > 0) {
		map<bigNumber, int> targetIDs = contextFound(make_pair(targetContexts, make_pair(targetFrontier, linkedNodes)));
		if (targetIDs.size() > 0) {
			doubleType targetIDsTotalWeight = 0;
			for (map<bigNumber, int>::const_iterator it = targetIDs.begin(); it != targetIDs.end(); ++it) {
				targetIDsTotalWeight += it->second;
			}
			for (map<bigNumber, int>::const_iterator it = targetIDs.begin(); it != targetIDs.end(); ++it) {
				newTargetIDs[it->first] = (doubleType)it->second / targetIDsTotalWeight;
			}
		}
	}
	if (newTargetIDs.size() == 0)
		newTargetIDs[0] = 1;
#ifdef __debug1__
	wcout << newTargetIDs.size() << " newTargetIDs left" << endl;
#endif
	
	cellMap* cell;
	bool newCell = false;
	dynAlignmentMap::iterator it = alignmentChart.find(nodeAddress);
	if (it != alignmentChart.end()) {
		cell = &(it->second);
	} else {
		cell = new cellMap();
		newCell = true;
	}
	for (map<bigNumber, doubleType>::const_iterator it = newTargetIDs.begin(); it != newTargetIDs.end(); ++it) {
		cellKey key;
		wstringstream srcCont;
		if (LEXICAL) {
			srcCont << sourceContext;
		} else {
			if (it->first == 0) {
				srcCont << sourceContext;
			} else {
				if (NODE_LABELS) {
					srcCont << "(" << source.chart.find(nodeAddress)->second->Label() << "__L)";
				} else {
					srcCont << L"(__L)";
				}
			}
		}
		for (bigNumVector::size_type i = 0; i < linkedNodes.size(); ++i) {
			srcCont << "," << linkedNodes[i];
		}
		key = make_pair(srcCont.str(), it->first);
		
#ifdef __debug1__
		wcout << "-->The cell key has context “" << key.first << "”" << endl;
		cellMap::const_iterator bla = cell->find(key);
		if (bla != cell->end())
			wcout << "FUCKING SHIT" << endl;
#endif
		
		(*cell)[key] = make_pair(it->second * combinationProbability, combination);
#ifdef __debug1__
		wcout << "  probability to link to node " << it->first << " in context “" << sourceContext << "”: " << it->second * combinationProbability * 100 << "%" << endl;
#endif
	}
	limitCombinations(cell);
	if (newCell) {
		alignmentChart[nodeAddress] = *cell;
		delete(cell);
	}
}

void TreePair::limitCombinations(cellMap* cell) {
	multimap<doubleType, cellKey> temp;
	for (cellMap::const_iterator it = cell->begin(); it != cell->end(); ++it) {
		temp.insert(make_pair(it->second.first, it->first));
	}
	int count = 1;
	cellMap newCell;
	//	doubleType best = temp.rbegin()->first;
	for (multimap<doubleType, cellKey>::reverse_iterator it = temp.rbegin(); it != temp.rend(); ++it) {
		//		if (it->first/best < 0.3) break;
#ifdef __debug1__
		wcout << "____stored probability " << it->first << endl;
#endif
		newCell[it->second] = cell->find(it->second)->second;
		++count;
		if (count > 100) break;
	}
#ifdef __debug1__
	wcout << endl;
#endif
	*cell = newCell;
}

void TreePair::writeLinks() {
#ifdef __debug1__
	wcout << endl << "Writing links…" << endl;
#endif
	addressType nodeAddress = make_pair(1, source.sentLength());
	doubleType bestProbability = 0;
	cellKey bestKey;
	dynAlignmentMap::const_iterator results = alignmentChart.find(nodeAddress);
	if (results == alignmentChart.end())
		exit(97);
	for (cellMap::const_iterator it = results->second.begin(); it != results->second.end(); ++it) {
		if (it->second.first > bestProbability) {
			bestProbability = it->second.first;
			bestKey = it->first;
		}
	}
	writeLinks(nodeAddress, bestKey);
}

void TreePair::writeLinks(addressType nodeAddress, cellKey& bestKey) {
	const NonTerminal* node = dynamic_cast<const NonTerminal*>(source.chart.find(nodeAddress)->second);
	//#ifdef __debug1__
	wcout << "Probability for node " << setw(2) << node->ID() << ":" << setw(10) << alignmentChart[nodeAddress][bestKey].first * 100 << "% (link to " << bestKey.second << ")" << endl;
	//	wcout << "  bestKey is " << bestKey.first << endl;
	//#endif
	if (bestKey.second != 0)
		links[node->ID()] = bestKey.second;
	if (node->Splits().size() > 2) {
		int curPos = node->Position();
		for (bigNumVector::size_type i = 1; i < node->Splits().size(); ++i) {
			curPos += node->Splits()[i - 1];
			addressType address = make_pair(curPos, node->Splits()[i]);
#ifdef __debug1__
			wcout << " advancing to address " << address.first << ", " << address.second << endl;
#endif
			writeLinks(address, alignmentChart[nodeAddress][bestKey].second[i - 1]);
		}
	}
}

const bigNumVector TreePair::nodeContext(const addressType& nodeAddress, const cellKey& key, bool) const {
	bigNumVector linkedNodes;
	const NonTerminal* node = dynamic_cast<const NonTerminal*>(source.chart.find(nodeAddress)->second);
	if (key.second != 0) {
		linkedNodes.push_back(key.second);
	}
	if (node->Splits().size() > 2) {
		int curPos = node->Position();
		for (bigNumVector::size_type i = 1; i < node->Splits().size(); ++i) {
			curPos += node->Splits()[i - 1];
			const bigNumVector dtrContext = nodeContext(make_pair(curPos, node->Splits()[i]), alignmentChart.find(nodeAddress)->second.find(key)->second.second[i - 1], true);
			linkedNodes.insert(linkedNodes.end(), dtrContext.begin(), dtrContext.end());
		}
	}
	return linkedNodes;
}	

pair<wstring, pair<const bigNumVector, const bigNumVector> > TreePair::nodeContext(const addressType& nodeAddress, const cellKey& key, int depth) const {
	if (error)
		return pair<wstring, pair<const bigNumVector, const bigNumVector> >();
	
	bigNumVector frontier;
	bigNumVector linkedNodes;
	const NonTerminal* node = dynamic_cast<const NonTerminal*>(source.chart.find(nodeAddress)->second);
	wstring context;
	if (NODE_LABELS) {
		context = L"(" + node->Label();
	} else {
		context = L"(";
	}
	if (key.second != 0) {
		if (!LEXICAL)
			frontier.push_back(key.second);
		linkedNodes.push_back(key.second);
		
		if (node->Splits().size() > 2) {
			int curPos = node->Position();
			for (bigNumVector::size_type i = 1; i < node->Splits().size(); ++i) {
				curPos += node->Splits()[i - 1];
				addressType address = make_pair(curPos, node->Splits()[i]);
				const bigNumVector dtrContext = nodeContext(address, alignmentChart.find(nodeAddress)->second.find(key)->second.second[i - 1], true);
				linkedNodes.insert(linkedNodes.end(), dtrContext.begin(), dtrContext.end());
			}
		}
		
		return make_pair(context + L"__L)", make_pair(frontier, linkedNodes));
	}
	if (node->Splits().size() > 2) {
		int curPos = node->Position();
		for (bigNumVector::size_type i = 1; i < node->Splits().size(); ++i) {
			curPos += node->Splits()[i - 1];
			addressType address = make_pair(curPos, node->Splits()[i]);
#ifdef CONTEXT_DEPTH
			if (depth < CONTEXT_DEPTH) {
#endif
				pair<wstring, pair<const bigNumVector, const bigNumVector> > dtrContext = nodeContext(address, alignmentChart.find(nodeAddress)->second.find(key)->second.second[i - 1], depth + 1);
				context += dtrContext.first;
				if (!LEXICAL)
					frontier.insert(frontier.end(), dtrContext.second.first.begin(), dtrContext.second.first.end());
				linkedNodes.insert(linkedNodes.end(), dtrContext.second.second.begin(), dtrContext.second.second.end());
#ifdef CONTEXT_DEPTH
			} else {
				const bigNumVector dtrContext = nodeContext(address, alignmentChart.find(nodeAddress)->second.find(key)->second.second[i - 1], true);
				linkedNodes.insert(linkedNodes.end(), dtrContext.begin(), dtrContext.end());
			}
#endif
		}
	} else {
		context += L" " + source.chart.find(make_pair(node->Position(), 0))->second->Label();
	}
	return make_pair(context + L")", make_pair(frontier, linkedNodes));
}
#endif //TRAINING

#ifndef TRAINING
void TreePair::readSourceWordAlignments(const string& fileName) {
	readWordAlignments(fileName, sourceWordAlignments);
}

void TreePair::readTargetWordAlignments(const string& fileName) {
	readWordAlignments(fileName, targetWordAlignments);
}

void TreePair::readWordAlignments(const string& fileName, extAlignmentMap& alignments) {
	//	wifstream input(fileName);
	wifstream input;
	input.open(fileName.c_str());
	if (input.fail()) {
		wcerr << "!!!Cannot open the file " << fileName.c_str() << "!!!" << endl;
		exit(1);
	}
	
	while (!input.eof()) {
		wstring target = readChunk(input, L' ');
		if (target.length() < 1) continue;
//		if (target.length() > 150) {
//			wcerr << " A WORD IS TOO LONG!!! " << target << endl;
//			exit(96);
//		}
#ifdef __debug2__
		wcerr << ">>target: " << target << endl;
#endif
		if (target == L"NULL") target = L"";

		wstring source = readChunk(input, L' ');
//		if (source.length() > 150) {
//			wcerr << " A WORD IS TOO LONG!!! " << source << endl;
//			exit(97);
//		}
#ifdef __debug2__
		wcerr << ">>source: " << source << endl;
#endif
		if (source == L"NULL") source = L"";

		wstring ln = readChunk(input, L'\n').c_str();
		
		wchar_t* offset;
		//		alignments[source][target] = doubleType(0.);
		alignments[source][target] = wcstod(ln.c_str(), &offset);
#ifdef __debug2__
		//		wcout << "stored: “" << source << "” “" << target << "” " << wcstod(ln, &offset) << endl;
		wcout << "stored: “" << source << "” “" << target << "” " << ln << endl;
		wcout << "  actually: " << alignments[source][target] << "/" << log(alignments[source][target]) << endl;
#endif
		
		/*		wstringstream line(ln);
		 line.getline(ln, 150, ' ');
		 wstring source(ln);
		 if (source.length() < 1) continue;
		 if (source == L"NULL") source = L"";
		 line.getline(ln, 150, ' ');
		 wstring target(ln);
		 if (target == L"NULL") target = L"";
		 line.getline(ln, 20, ' ');
		 wchar_t* offset;
		 alignments[source][target] = wcstod(ln, &offset);
		 #ifdef __debug2__
		 wcout << "stored: “" << source << "” “" << target << "” " << wcstod(ln, &offset) << endl;*/
		
		/*		wstring line(ln);
		 if (line.length() < 1) continue;
		 wstring source = line.substr(0, line.find(L" ||| ", 0));
		 line = line.substr(line.find(L" ||| ", 0) + 5);
		 #ifdef __debug2__
		 //		wcout << "  source: " << source << " ||| line: " << line << endl;
		 #endif
		 wstring target = line.substr(0, line.find(L" ||| ", 0));
		 line = line.substr(line.find(L" ||| ", 0) + 5);
		 #ifdef __debug2__
		 //		wcout << "  target: " << target << " ||| line: “" << line << "”" << endl;
		 #endif
		 wchar_t* offset;
		 alignments[source][target] = wcstod(line.c_str(), &offset);
		 #ifdef __debug2__
		 wcout << "stored: “" << source << "” “" << target << "” " << wcstod(line.c_str(), &offset) << endl;
		 #endif*/
	}
	/*	alignments[L"#eos#"][L"#eos#"] = 1;
	 #ifdef __debug2__
	 wcout << "stored: #eos# #eos# 1" << endl;
	 #endif*/
	
	input.close();
}

void TreePair::readPhraseAlignments(const string& fileName) {
	wifstream input;
	input.open(fileName.c_str());
	if (input.fail()) {
		wcerr << "!!!Cannot open the file " << fileName.c_str() << "!!!" << endl;
		exit(1);
	}
	
	while (!input.eof()) {
		wchar_t ln[5000];
		input.getline(ln, 5000);
		wstring line(ln);
		if (line.length() < 1) continue;
#ifdef __debug2__
		//		wcout << "line: “" << line << "”" << endl;
#endif
		wstring source = line.substr(0, line.find(L" ||| ", 0));
		line = line.substr(line.find(L" ||| ", 0) + 5);
#ifdef __debug2__
		//		wcout << "  source: " << source << " ||| line: " << line << endl;
#endif
		wstring target = line.substr(0, line.find(L" ||| ", 0));
		if (target.find(' ') == wstring::npos && source.find(' ') == wstring::npos) continue;
		line = line.substr(line.find(L" ||| ", 0) + 5);
#ifdef __debug2__
		//		wcout << "  target: " << target << " ||| line: “" << line << "”" << endl;
#endif
		wchar_t* offset;
		sourcePhraseAlignments[source][target] = wcstod(line.substr(0, line.find_first_of(' ', 0)).c_str(), &offset);
#ifdef __debug2__
		wcout << "stored: " << target << " ||| " << source << " ||| " << wcstod(line.substr(0, line.find_first_of(' ', 0)).c_str(), &offset) << endl;
#endif
		line = line.substr(line.find_first_of(' ', line.find_first_of(' ', 0) + 1) + 1);
		targetPhraseAlignments[target][source] = wcstod(line.substr(0, line.find_first_of(' ', 0)).c_str(), &offset);
#ifdef __debug2__
		wcout << "stored: " << source << " ||| " << target << " ||| " << wcstod(line.substr(0, line.find_first_of(' ', 0)).c_str(), &offset) << endl << endl;
#endif
	}
	
	input.close();
}

void TreePair::readSourceVocabulary(const char* fileName) {
	readVocabulary(fileName, sourceVocabulary);
}

void TreePair::readTargetVocabulary(const char* fileName) {
	readVocabulary(fileName, targetVocabulary);
}

void TreePair::readVocabulary(const char* fileName, vcbMap& vocabulary) {
	wifstream input(fileName);
	if (input.fail()) {
		wcerr << "!!!Cannot open the file " << fileName << "!!!" << endl;
		exit(1);
	}
	
	long wordCount = 0;
	while (!input.eof()) {
		wchar_t ln[200];
		input.getline(ln, 200);
		wstring line(ln);
		if (line.length() < 1) continue;
		//		wstring word = line.substr(0, line.find_first_of(SPACE));
		//		wstring count = line.substr(line.find_last_of(SPACE));
		wchar_t* offset;
		//		int cnt = wcstod(count.c_str(), &offset);
		vocabulary[line.substr(0, line.find_first_of(SPACE))] = wcstod(line.substr(line.find_last_of(SPACE)).c_str(), &offset);
		wordCount += wcstol(line.substr(line.find_last_of(SPACE)).c_str(), &offset, 10);
#ifdef __debug2__
		wcout << "stored: " << line.substr(0, line.find_first_of(SPACE)) << " >>> " << wcstod(line.substr(line.find_last_of(SPACE)).c_str(), &offset) << endl;
#endif
	}
	vocabulary[L""] = SENTENCE_COUNT;
	vocabulary[L"#eos#"] = SENTENCE_COUNT;
	wordCount += SENTENCE_COUNT*2;
	for (vcbMap::iterator it = vocabulary.begin(); it != vocabulary.end(); ++it) {
		it->second /= wordCount;
#ifdef __debug2__
		wcout << "changed: " << it->first << " >>> " << it->second << endl;
#endif
	}
}

void TreePair::createSubStringLists() {
	createSubStringLists(source, target);
}

void TreePair::createSubStringLists(Tree& source, Tree& target) {
	sourceStrings = source.getSubStrings();
#ifdef __debug2a__
	wcout << endl;
#endif
	targetStrings = target.getSubStrings();
#ifdef __debug2a__
	wcout << endl;
#endif
}

void TreePair::initLexicalInfo(const Tree& source, const Tree& target, const extAlignmentMap& alignments,
//															 lexicalProbabilities& probs,
															 doubleType**& newProbs) {
	doubleType* newCurMap;
	newProbs = new doubleType*[(source.sentLength() + 1)];
	for (size_t i = 0; i <= size_t(source.sentLength()); ++i) {
		newCurMap = new doubleType[target.sentLength() + 1];
		for (size_t j = 0; j <= size_t(target.sentLength()); newCurMap[j++] = 0.);
		newProbs[i] = newCurMap;
//		for (size_t j = 0; j <= size_t(target.sentLength()); ++j)
//			wcerr << i << "/" << j << ":" << newProbs[i][j] << " ";
//		wcerr << endl;
	}
	
	newCurMap = newProbs[0];
	
	
//	lexicalProbabilities::value_type::second_type(curMap);
	extAlignmentMap::mapped_type targets;
	//get the probabilities for NULL
	extAlignmentMap::const_iterator al_it = alignments.find(L"");
	if (al_it == alignments.end())
		goto LEXICAL;
	targets = al_it->second;
#ifdef __debug3a__
	wcout << "source word is “”" << endl << "  might link to:";
	for (extAlignmentMap::mapped_type::const_iterator it = targets.begin(); it != targets.end(); wcout << " “" << (it++)->first << "”");
	wcout << endl;
#endif
	for (Tree::const_word_iterator node = target.wbegin(); node != target.wend(); ++node) {
		extAlignmentMap::mapped_type::const_iterator it;
#ifdef LOWERCASE
		{
			wstring nodeLabel = node->Label();
			toLower(nodeLabel);
#ifdef __debug3__
			wcout << " target word is “" << nodeLabel << "”" << endl;
#endif
			it = targets.find(nodeLabel);
		}
#else //LOWERCASE
#ifdef __debug3__
		wcout << " target word is “" << node->Label() << "”" << endl;
#endif
		it = targets.find(node->Label());
#endif //#ifdef LOWERCASE
		if (it != targets.end()) {
//			curMap[dynamic_cast<const NonTerminal*>(target.findNode(make_pair(node->Position(), 1)))->ID()] = it->second;
			newCurMap[node->Position()] = it->second;
#ifdef __debug3__
			wcout << "  =position " << node->Position() << " with probability " << it->second << endl;
#endif
		}
	}
LEXICAL:
//	probs.insert(make_pair(0, curMap));
	
	//get the lexical probabilities
#ifdef NEW_PROBS
//	bigNumIndex(seenTargets);
	set<size_t>(newSeenTargets);
#endif
	for (Tree::const_word_iterator node = source.wbegin(); node != source.wend();
			 ++node
//			 probs.insert(make_pair(dynamic_cast<const NonTerminal*>(source.findNode(make_pair((node++)->Position(), 1)))->ID(), curMap))
			 ) {
//		curMap.clear();
		newCurMap = newProbs[node->Position()];
		
		extAlignmentMap::const_iterator al_it;
#ifdef LOWERCASE
		{
			wstring nodeLabel = node->Label();
			toLower(nodeLabel);
#ifdef __debug3a__
			wcout << "source word is “" << nodeLabel << "”" << endl;
#endif
			al_it = alignments.find(nodeLabel);
		}
#else //LOWERCASE
#ifdef __debug3a__
		wcout << "source word is “" << node->Label() << "”" << endl;
#endif
		al_it = alignments.find(node->Label());
#endif //#ifdef LOWERCASE
		//if a word cannot be linked to any of the words in the target string, then it must be linked to NULL with probability 1. and we continue with the next word
		if (al_it == alignments.end()) {
#ifdef __debug3a__
			wcout << "  No linking possibilities for word " << node->Position() << endl;
#endif
			//			exit(EXIT_FAILURE);
//			curMap[0] = 1.;
			newCurMap[0] = 1.;
			//			probs.insert(make_pair(node->Position(), curMap));
			continue;
		}
		extAlignmentMap::mapped_type targets = al_it->second;
#ifdef __debug3a__
		wcout << "  might link to:";
		for (extAlignmentMap::mapped_type::const_iterator it = targets.begin(); it != targets.end(); ++it)
			wcout << " “" << it->first << "”";
		wcout << endl;
#endif
		bool hasTargets = false;
		//we look through all possible target alignments and record the probabilities in the proper slots
		for (Tree::const_word_iterator targetNode = target.wbegin(); targetNode != target.wend(); ++targetNode) {
			extAlignmentMap::mapped_type::const_iterator it;
#ifdef LOWERCASE
			{
				wstring nodeLabel = targetNode->Label();
				toLower(nodeLabel);
#ifdef __debug3__
				wcout << " target word is “" << nodeLabel << "”" << endl;
#endif
				it = targets.find(nodeLabel);
			}
#else //LOWERCASE
#ifdef __debug3__
			wcout << " target word is “" << targetNode->Label() << "”" << endl;
#endif
			it = targets.find(targetNode->Label());
#endif //#ifdef LOWERCASE
			//we only record probabilities whenever they exist
			if (it != targets.end()) {
//				curMap[dynamic_cast<const NonTerminal*>(target.findNode(make_pair(targetNode->Position(), 1)))->ID()] = it->second;
				newCurMap[targetNode->Position()] = it->second;
				hasTargets = true;
#ifdef NEW_PROBS
				//keep a record of the nodes that appear as targets
//				seenTargets.insert(dynamic_cast<const NonTerminal*>(target.findNode(make_pair(targetNode->Position(), 1)))->ID());
				newSeenTargets.insert(targetNode->Position());
#endif
#ifdef __debug3__
				wcout << "  =position " << targetNode->Position() << " with probability " << it->second << endl;
#endif
			}
		}
		//check whether NULL is an available target and record the probability if it is
		extAlignmentMap::mapped_type::const_iterator it = targets.find(L"");
		if (it != targets.end()) {
//			curMap[0] = it->second;
			newCurMap[0] = it->second;
#ifdef __debug3__
			wcout << " target word is “”" << endl;
			wcout << "  =position 0 with probability " << it->second << endl;
#endif
			//or if we did not record any probabilities, link to NULL with probability 1.
//		} else if (curMap.empty()) {
		} else if (!hasTargets) {
//			curMap[0] = 1.;
			newCurMap[0] = 1.;
		}
		//		probs.insert(make_pair(node->Position(), curMap));
	}
#ifdef NEW_PROBS
	//for all nodes that have not been seen as targets, we set the probability of NULL to link to them to 1.
	for (Tree::const_preterm_iterator node = target.ptbegin(); node != target.ptend(); ++node) {
//		if (seenTargets.find(node->ID()) == seenTargets.end())
//			probs[0][node->ID()] = 1.;
		if (newSeenTargets.find(node->Position()) == newSeenTargets.end())
			newProbs[0][node->Position()] = 1.;
	}
#endif
#ifdef __debug3a__
//	wcout << "number of positions stored: " << probs.size() << " for length " << source.sentLength() << endl;
#endif
//	for (lexicalProbabilities::const_iterator i = probs.begin(); i != probs.end(); ++i)
//		for (lexicalProbabilities::value_type::second_type::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
//			if (newProbs[i->first ? source.findNode(i->first)->Position() : 0][j->first ? target.findNode(j->first)->Position() : 0] != j->second)
//				wcerr << "ııııı BAD  probability stored in newProbs for pair " << i->first << "/" << j->first << "!!!" << endl;
//			else
//				wcerr << "∞∞∞∞∞ Good probability stored in newProbs for pair " << i->first << "/" << j->first << "!!!" << endl;
}

void TreePair::initLexicalInfo() {
#ifdef _OPENMP
#pragma omp parallel sections num_threads(2)
	{
#pragma omp section
#endif
		initLexicalInfo(source, target, sourceWordAlignments,
//										sourceLexProbs,
										newSourceLexProbs);
#ifdef _OPENMP
#pragma omp section
#endif
		initLexicalInfo(target, source, targetWordAlignments,
//										targetLexProbs,
										newTargetLexProbs);
#ifdef _OPENMP
	} //parallel sections
#endif
}

#ifdef __debug7__
void TreePair::printLexMap(const Tree& source,
//													 const lexicalProbabilities& probs,
													 doubleType** const newProbs, const size_t& srcSentLen, const size_t& trgSentLen) const {
	for (size_t i = 0; i <= srcSentLen; ++i) {
		wcout << " ‡ position " << setw(2) << i << " ID" << setw(4) << (i ? dynamic_cast<const NonTerminal*>(source.findNode(make_pair(i, 1)))->ID() : 0) << endl;
		for (size_t j = 0; j <= trgSentLen; ++j) {
			wcout << " ‡   to position " << setw(2) << j << " with prob " << setw(10) << newProbs[i][j] << endl;
		}
	}
	
//	for (lexicalProbabilities::const_iterator i = probs.begin(); i != probs.end(); ++i) {
//		wcout << "  position " << setw(2) << (i->first ? dynamic_cast<const NonTerminal*>(source.findNode(i->first))->Position() : 0) << " ID" << setw(4) << i->first << " “" << (i->first ? dynamic_cast<const Terminal*>(source.findNode(make_pair(dynamic_cast<const NonTerminal*>(source.findNode(i->first))->Position(), 0)))->Label() : L"") << "”:" << endl;
//		lexicalProbabilities::value_type::second_type curMap = probs.find(i->first)->second;
//		for (lexicalProbabilities::value_type::second_type::const_iterator it = curMap.begin(); it != curMap.end(); ++it) {
//			wcout << "    to nodeID" << setw(4) << it->first << " with prob" << setw(10) << it->second << endl;
//		}
//	}
}

void TreePair::printLexMaps() const {
	wcout << "source probs:" << endl;
	printLexMap(source,
//							sourceLexProbs,
							newSourceLexProbs, source.sentLength(), target.sentLength());
	wcout << "target probs:" << endl;
	printLexMap(target,
//							targetLexProbs,
							newTargetLexProbs, target.sentLength(), source.sentLength());
}
#endif

void TreePair::prepareLexicalLinks() {
	prepareLexicalLinks(source, target);
}

void TreePair::prepareLexicalLinks(Tree& source, Tree& target) {
#ifndef TRAINING
	allPhrasesCount = 0;
#endif
	
#ifdef _OPENMP
	typedef vector<pair<const NonTerminal*, const NonTerminal*> > nodeStorage;
	nodeStorage nodes;
	for (Tree::const_nterm_iterator sourceNode = source.ntbegin(); sourceNode != source.ntend(); ++sourceNode)
		for (Tree::const_nterm_iterator targetNode = target.ntbegin(); targetNode != target.ntend(); ++targetNode)
			nodes.push_back(make_pair(&*sourceNode, &*targetNode));
	
	int node_i;
	const int size = nodes.size();
	
#pragma omp parallel for if(size > 500) default(shared) schedule(dynamic, min(max(size/50, 20), 100)) num_threads(omp_get_num_procs() + 1)
	/*	double runTime = omp_get_wtime();
	 #pragma omp parallel if(size > 100) default(shared) num_threads(omp_get_num_procs())
	 {
	 //#pragma omp single
	 //	wclog << endl << "<<<" << omp_get_num_threads() << ">>> |||" << min(max(size/20, 5), 25) << "|||" << size << "|||" << endl;
	 #pragma omp for schedule(dynamic, min(max(size/20, 1), 25))*/
#endif
	
	for
#ifdef _OPENMP
		(node_i = 0; node_i < size; ++node_i)
#else
		(Tree::const_nterm_iterator sourceNode = source.ntbegin(); sourceNode != source.ntend(); ++sourceNode)
#ifdef __debug4__
	{
		wcout << "Currently at node " << sourceNode->Label() << "-" << sourceNode->ID() << endl;
#endif
		
		for (Tree::const_nterm_iterator targetNode = target.ntbegin(); targetNode != target.ntend(); ++targetNode)
#endif //_OPENMP
		{
#ifdef _OPENMP
			//#pragma omp critical
			//			wclog << omp_get_thread_num() << "(" << omp_get_num_threads() << ")/" << node_i << " ";
			
			const NonTerminal *sourceNode, *targetNode;
			{
				const pair<const NonTerminal*, const NonTerminal*>& link = nodes[node_i];
				sourceNode = link.first;
				targetNode = link.second;
			}
#endif
			scoreLink(source, target, &*sourceNode, &*targetNode);
		}
#ifndef _OPENMP
#ifdef __debug4__
	}
#endif
	/*#else
	 }
	 wclog << "For " << size << " -> " << omp_get_wtime() - runTime << "sec" << endl;*/
#endif
}

void TreePair::scoreLink(Tree& source, Tree& target, const NonTerminal* sourceNode, const NonTerminal* targetNode) {
#ifdef __debug4__
	wcout << "  Considering the link between nodes " << sourceNode->Label() << "-" << sourceNode->ID();
	if (sourceNode->binIndex())
		wcout << BIN_CONNECT << sourceNode->binIndex();
	wcout << " and " << targetNode->Label() << "-" << targetNode->ID();
	if (targetNode->binIndex())
		wcout << BIN_CONNECT << targetNode->binIndex();
	wcout << endl;
#endif
	if (Tree::binariseMode && (sourceNode->binIndex() || targetNode->binIndex()))
		return;
	
	LinkHypothesis* hypothesis = new LinkHypothesis();
	//#ifndef STATISTICS
	*hypothesis *= insideProbability(source, target,
//																	sourceLexProbs,
																	newSourceLexProbs, *sourceNode, *targetNode
#ifdef __debugX__
																	, 0
#endif
																	);
#ifdef __debug4__
	wcout << "    after source inside prob: " << hypothesis->probability() << endl;
#endif
	if (*hypothesis == ProbabilityType::Zero) goto PHRASAL;
	
#ifndef SIMPLE_PROBS
	*hypothesis *= outsideProbability(source, target,
//																	 sourceLexProbs,
																	 newSourceLexProbs, *sourceNode, *targetNode
#ifdef __debugX__
																	 , 0
#endif
																	 );
#ifdef __debug4__
	wcout << "    after source outside prob: " << hypothesis->probability() << endl;
#endif
	if (*hypothesis == ProbabilityType::Zero) goto PHRASAL;
#endif //#ifndef SIMPLE_PROBS
	
	*hypothesis *= insideProbability(target, source,
//																	targetLexProbs,
																	newTargetLexProbs, *targetNode, *sourceNode
#ifdef __debugX__
																	, 1
#endif
																	);
#ifdef __debug4__
	wcout << "    after target inside prob: " << hypothesis->probability() << endl;
#endif
	if (*hypothesis == ProbabilityType::Zero) goto PHRASAL;
	
#ifndef SIMPLE_PROBS
	*hypothesis *= outsideProbability(target, source,
//																	 targetLexProbs,
																	 newTargetLexProbs, *targetNode, *sourceNode
#ifdef __debugX__
																	 , 1
#endif
																	 );
#ifdef __debug4__
	wcout << "    after target outside prob: " << hypothesis->probability() << endl;
#endif
#endif //#ifndef SIMPLE_PROBS
	//#endif //#ifndef STATISTICS
PHRASAL:
	bool hasPhrasal = false;
	if (USE_PHRASES) {
		if (sourceNode->Span() == 1 && targetNode->Span() == 1)
			goto DECISION;
		
		extAlignmentMap::const_iterator srcit = sourcePhraseAlignments.find((*sourceStrings)[sourceNode->ID()]);
		if (srcit != sourcePhraseAlignments.end()) {
#ifdef __debug4__
			wcout << " ?? What strings do we have ??" << endl;
			for (extAlignmentMap::value_type::second_type::const_iterator it = srcit->second.begin(); it != srcit->second.end(); ++it)
				wcout << "    ?>> " << it->first << " > " << it->second << endl;
			wcout << endl;
#endif
			extAlignmentMap::value_type::second_type::const_iterator trgit = srcit->second.find((*targetStrings)[targetNode->ID()]);
			if (trgit != srcit->second.end()) {
//				hypothesis *= trgit->second;
				hasPhrasal = true;
			}
		}
#ifdef __debug4__
//		wcout << "    source phrasal prob: " << hypothesis->sourcePhrasal << endl;
#endif
		srcit = targetPhraseAlignments.find((*targetStrings)[targetNode->ID()]);
		if (srcit != targetPhraseAlignments.end()) {
			extAlignmentMap::value_type::second_type::const_iterator trgit = srcit->second.find((*sourceStrings)[sourceNode->ID()]);
			if (trgit != srcit->second.end()) {
//				hypothesis *= trgit->second;
				hasPhrasal = true;
			}
		}
#ifdef __debug4__
//		wcout << "    target phrasal prob: " << hypothesis->targetPhrasal << endl;
#endif
	}
DECISION:
#ifdef __debug6__
	wcout << "  >>>overall hypothesis prob: " << hypothesis->probability() << endl;
#endif
	
#ifndef TRAINING
	if (USE_PHRASES &&
			hasPhrasal
//			(hypothesis->sourcePhrasal || hypothesis->targetPhrasal)
			)
		++allPhrasesCount;
#endif
	
	if (!*hypothesis)
		delete hypothesis;
	else
#ifdef _OPENMP
#pragma omp critical (saveHypothesis)
#endif
		inducedLinks.insert(make_pair(linkAddressPair(sourceNode->ID(), targetNode->ID()), hypothesis));
}

#ifdef RESCORE
bool TreePair::cleanLexProbabilites(const linkAddressPair& link) {
	return cleanLexProbabilites(source, target, link);
}

bool TreePair::cleanLexProbabilites(Tree& source, Tree& target, const linkAddressPair& link) {
#ifdef __debug7__
	wcerr << " ??? Trying to clean the lexical probabilities due to the link " << link.first << " --> " << link.second << " ???" << endl;
#endif
	changed = false;
	const NonTerminal* const sourceNode = source.findNode(link.first);
	const NonTerminal* const targetNode = target.findNode(link.second);
#ifdef _OPENMP
#pragma omp parallel sections num_threads(4)
	{
#pragma omp section
#endif
		changed = cleanLexProbabilites(sourceLexProbs, source.preTermSpan(*sourceNode), target.outsidePreTermSpan(*targetNode)) || changed;
#ifdef _OPENMP
#pragma omp section
#endif
		changed = cleanLexProbabilites(sourceLexProbs, source.outsidePreTermSpan(*sourceNode), target.preTermSpan(*targetNode)) || changed;
#ifdef _OPENMP
#pragma omp section
#endif
		changed = cleanLexProbabilites(targetLexProbs, target.preTermSpan(*targetNode), source.outsidePreTermSpan(*sourceNode)) || changed;
#ifdef _OPENMP
#pragma omp section
#endif
		changed = cleanLexProbabilites(targetLexProbs, target.outsidePreTermSpan(*targetNode), source.preTermSpan(*sourceNode)) || changed;
#ifdef _OPENMP
	}
#endif
#ifdef __debug7__
	printLexMaps();
#endif
	return changed;
}

bool TreePair::cleanLexProbabilites(lexicalProbabilities& probs, const bigNumVector& srcPreTermSpan, const bigNumVector& trgPreTermSpan) {
	bool changed = false;
	for (bigNumVector::size_type i = 0; i < srcPreTermSpan.size() - 1; ++i)
		for (bigNumVector::size_type j = 0; j != trgPreTermSpan.size() - 1; ++j) {
			lexicalProbabilities::value_type::second_type& target = probs.find(srcPreTermSpan[i])->second;
#ifdef _OPENMP
#pragma omp critical(cleanLexProb)
#endif //_OPENMP
			if (target.find(trgPreTermSpan[j]) != target.end()) {
#ifdef __debug7a__
				wcout << "The lexical alignment between " << srcPreTermSpan[i] << " and " << trgPreTermSpan[j] << " is no more…" << endl;
#endif
				target.erase(trgPreTermSpan[j]);
				changed = true;
			}
		}
	return changed;
}

void TreePair::rescoreLinks(const linkAddressPair& link) {
	if (cleanLexProbabilites(link))
		rescoreLinks();
#ifdef __debug7a__
	else
		wcout << "÷1÷ Not need to rescore because of the link " << link.first << " --> " << link.second << endl;
#endif
}

void TreePair::rescoreLinks(const linksVector& links) {
	bool changed = false;
	for (linksVector::const_iterator link = links.begin(); link != links.end(); ++link)
		changed = cleanLexProbabilites(*link) || changed;
	if (changed)
		rescoreLinks();
#ifdef __debug7a__
	else
		wcout << "÷2÷ Not need to rescore at all" << endl;
#endif
}

void TreePair::rescoreLinks() {
	rescoreLinks(source, target);
}

void TreePair::rescoreLinks(Tree& source, Tree& target) {
	vector<hypothesesMap::iterator> toDelete;
	for (hypothesesMap::iterator it = inducedLinks.begin(); it != inducedLinks.end(); toDelete.push_back(it++));
	for (vector<hypothesesMap::iterator>::iterator it = toDelete.begin(); it != toDelete.end(); ++it)
		if (!(*it)->second->isDecided()) {
			linkAddressPair link = (*it)->first;
			delete((*it)->second);
			inducedLinks.erase(*it);
			scoreLink(source, target, source.findNode(link.first), target.findNode(link.second));
		}
//	cleanDecided();
}
#endif //RESCORE

void TreePair::buildIndices() {
	buildIndices(source, target);
}

void TreePair::buildIndices(const Tree& source, const Tree& target) {
	linksProbabilityIndex.clear();
	if (VERSION != "0.7")
		one2oneLinksProbabilityIndex.clear();
	linksRowIndex.clear();
	linksColumnIndex.clear();
#ifdef __debug8__
	wcout << "≠≠≠hypotheses before bulding indices: " << inducedLinks.size() << endl;
#endif
	for (hypothesesMap::iterator it = inducedLinks.begin(); it != inducedLinks.end(); ++it) {
		if (VERSION == "0.7") {
			linksProbabilityIndex.insert(make_pair(it->second->probability(), it));
		} else if (VERSION == "0.8") {
			if (source.findNode(it->first.first)->Span() == 1 && target.findNode(it->first.second)->Span() == 1)
				one2oneLinksProbabilityIndex.insert(make_pair(it->second->probability(), it));
			else
				linksProbabilityIndex.insert(make_pair(it->second->probability(), it));
		} else if (VERSION == "0.8a") {
			if (source.findNode(it->first.first)->Span() == 1 || target.findNode(it->first.second)->Span() == 1)
				one2oneLinksProbabilityIndex.insert(make_pair(it->second->probability(), it));
			else
				linksProbabilityIndex.insert(make_pair(it->second->probability(), it));
		} else {
			exit(98);
		}
		
		linksRowIndex.insert(make_pair(it->first.first, it->second));
		linksColumnIndex.insert(make_pair(it->first.second, it->second));
	}
#ifdef __debug8__
	wcout << "≠≠≠hypotheses after bulding indices: " << inducedLinks.size() << endl;
#endif
}

void TreePair::printInducedLinks(bool printStatus, bool trees) const {
	printInducedLinks(printStatus, trees, source, target);
}

void TreePair::printInducedLinks(bool printStatus, bool trees, const Tree& source, const Tree& target) const {
	if (trees) {
		wcout << "->Printing according to trees<-" << endl;
		for (Tree::const_nterm_iterator sourceNode = source.ntbegin(); sourceNode != source.ntend(); ++sourceNode)
			for (Tree::const_nterm_iterator targetNode = target.ntbegin(); targetNode != target.ntend(); ++targetNode) {
				hypothesesMap::const_iterator hypit = inducedLinks.find(make_pair(sourceNode->ID(), targetNode->ID()));
				if (hypit != inducedLinks.end())
					wcout << " The " << hypit->second->status() << " link between " << sourceNode->Label() << "-" << sourceNode->ID() << " and " << targetNode->Label() << "-" << targetNode->ID() << " has probability " << hypit->second->probability() << endl;
				else
					wcout << " The decided link between " << sourceNode->Label() << "-" << sourceNode->ID() << " and " << targetNode->Label() << "-" << targetNode->ID() << " has probability 0" << endl;
			}
		
		wcout << "->Printing according to linkMap<-" << endl;
		for (hypothesesMap::const_iterator it = inducedLinks.begin(); it != inducedLinks.end(); ++it)
			wcout << " The " << it->second->status() << " link between " << source.findNode(it->first.first)->Label() << "-" << source.findNode(it->first.first)->ID() << " and " << target.findNode(it->first.second)->Label() << "-" << target.findNode(it->first.second)->ID() << " has probability " << it->second->probability() << endl;
	} else {
		
		int count = 0;
		wcout << "->Printing according to ProbabilityIndex<-" << endl;
		for (hypothesesMapProbabilityIndex::const_reverse_iterator it = linksProbabilityIndex.rbegin(); it != linksProbabilityIndex.rend(); ++it)
			wcout << " " << ++count << " The link between " << setw(10) << source.findNode(it->second->first.first)->Label() << "-" << setw(2) << source.findNode(it->second->first.first)->ID() << " and " << setw(10) << target.findNode(it->second->first.second)->Label() << "-" << setw(2) << target.findNode(it->second->first.second)->ID() << " has probability " << it->first << (printStatus ? L" (" + it->second->second->status() + L")" : L"") << endl;
		
		if (VERSION != "0.7") {
			count = 0;
			wcout << "-> ->Printing one2one according to ProbabilityIndex<-" << endl;
			for (hypothesesMapProbabilityIndex::const_reverse_iterator it = one2oneLinksProbabilityIndex.rbegin(); it != one2oneLinksProbabilityIndex.rend(); ++it)
				wcout << " " << ++count << " The link between " << setw(10) << source.findNode(it->second->first.first)->Label() << "-" << setw(2) << source.findNode(it->second->first.first)->ID() << " and " << setw(10) << target.findNode(it->second->first.second)->Label() << "-" << setw(2) << target.findNode(it->second->first.second)->ID() << " has probability " << it->first << (printStatus ? L" (" + it->second->second->status() + L")" : L"") << endl;
		}
	}
	
	/*	wcout << "->Printing according to RowIndex<-" << endl;
	 for (hypothesesMapElementIndex::const_iterator it = linksRowIndex.begin(); it != linksRowIndex.end(); ++it) {
	 wcout << " A " << it->second->status() << " link from " << source.findNode(it->first)->Label() << "-" << source.findNode(it->first)->ID() << " has probability " << it->second->probability() << endl;
	 }
	 
	 wcout << "->Printing according to ColumnIndex<-" << endl;
	 for (hypothesesMapElementIndex::const_iterator it = linksColumnIndex.begin(); it != linksColumnIndex.end(); ++it) {
	 wcout << " A " << it->second->status() << " link from " << target.findNode(it->first)->Label() << "-" << target.findNode(it->first)->ID() << " has probability " << it->second->probability() << endl;
	 }*/
}

ProbabilityType TreePair::insideProbability(Tree& source, Tree& target,
//																						const lexicalProbabilities& probs,
																						doubleType** const newProbs, const NonTerminal& sourceNode, const NonTerminal& targetNode
#ifdef __debugX__
																						, const unsigned short direction
#endif
) {
#ifdef __debugX__
	if (direction)
		wcout << "-->Target to Source Inside…" << endl;
	else
		wcout << "-->Source to Target Inside…" << endl;
#endif
	return translationProbability(
//																probs,
																newProbs,
//																source.preTermSpan(sourceNode), target.preTermSpan(targetNode),
																sourceNode, targetNode, source.sentLength(), target.sentLength(), false
#ifdef __debugX__
																, source, target
#endif
																);
}

ProbabilityType TreePair::outsideProbability(Tree& source, Tree& target,
//																						 const lexicalProbabilities& probs,
																						 doubleType** const newProbs, const NonTerminal& sourceNode, const NonTerminal& targetNode
#ifdef __debugX__
																						 , const unsigned short direction
#endif
) {
#ifdef __debugX__
	if (direction)
		wcout << "-->Target to Source Outside…" << endl;
	else
		wcout << "-->Source to Target Outside…" << endl;
#endif
	return translationProbability(
//																probs,
																newProbs,
//																source.outsidePreTermSpan(sourceNode), target.outsidePreTermSpan(targetNode),
																sourceNode, targetNode, source.sentLength(), target.sentLength(), true
#ifdef __debugX__
																, source, target
#endif
																);
}

ProbabilityType TreePair::translationProbability(
//																								 const lexicalProbabilities& probs,
																								 doubleType** const newProbs,
//																								 const bigNumVector& srcPreTermSpan, const bigNumVector& trgPreTermSpan,
																								 const NonTerminal& sourceNode, const NonTerminal& targetNode, const size_t& srcSentLen, const size_t& trgSentLen, bool outside
#ifdef __debugX__
																								 , const Tree& source, const Tree& target
#endif
) {
//	ProbabilityType result(1.);

	ProbabilityType newResult(1.);
	const size_t& srcBeg = sourceNode.Position();
	const size_t& srcEnd = srcBeg + sourceNode.Span();
	const size_t& srcLoopEnd = outside ? srcSentLen : (srcEnd - 1);
	const size_t& trgBeg = targetNode.Position();
	const size_t& trgEnd = trgBeg + targetNode.Span();
	const size_t& trgLoopEnd = outside ? trgSentLen : (trgEnd - 1);

#ifdef NEW_PROBS
//	ProbabilityType inverseDenominator(1./srcPreTermSpan.size());
//	wcerr << "ˆˆˆDividing by " << (outside ? (srcSentLen - sourceNode.Span() + 1)  : (sourceNode.Span() + 1)) << endl;
	ProbabilityType inverseDenominator(1./(outside ? (srcSentLen - sourceNode.Span() + 1)  : (sourceNode.Span() + 1)));
	
	//multiplication loop
	for (size_t i = 0; i <= trgLoopEnd; ++i) {
		if (outside) {
			if (i == trgBeg) {
				if (trgEnd <= trgSentLen) i = trgEnd; else break;
			}
		} else
			if (i && i < trgBeg) i = trgBeg;
		doubleType tempResult(0.);
		//addition loop
		for (size_t j = 0; j <= srcLoopEnd; ++j) {
			if (outside) {
				if (j == srcBeg) {
					if (srcEnd <= srcSentLen) j = srcEnd; else break;
				}
			} else
				if (j && j < srcBeg) j = srcBeg;
//			wcerr << "‚‚‚ Operating on positions " << j << "/" << i << endl;
//			if (newProbs[j][i])
				tempResult += newProbs[j][i];
		}
		
//		wcerr << "˛˛tempResult is " << tempResult << " for target position " << i << endl;
		if (tempResult)
			newResult *= inverseDenominator
#ifdef SEP_PROB_TYPE
			* ProbabilityType(tempResult, true)
#else
			* tempResult
#endif
			;
		else if (!i)
			newResult *= inverseDenominator;
		else
			return ProbabilityType::Zero;
	}
	
/*#ifdef __debugX__
	wcout << " 2nd display trgPreTermSpan:";
	for (bigNumVector::size_type i = 0; i < trgPreTermSpan.size() - 1; ++i)
		wcout << " " << i << "/" << target.findNode(trgPreTermSpan[i])->Label() << "-" << trgPreTermSpan[i];
	wcout << " " << (trgPreTermSpan.size() - 1) << "/NULL" << endl;
	wcout << "    2nd display srcPreTermSpan:";
	for (bigNumVector::size_type ii = 0; ii < srcPreTermSpan.size() - 1; ++ii)
		wcout << " " << ii << "/" << source.findNode(srcPreTermSpan[ii])->Label() << "-" << srcPreTermSpan[ii];
	wcout << " " << (srcPreTermSpan.size() - 1) << "/NULL" << endl;
#endif
	//multiplication loop
	for (bigNumVector::const_iterator i = trgPreTermSpan.begin();
#ifdef __debug5__
			 result &&
#endif
			 (i != trgPreTermSpan.end()); ++i) {
#ifdef __debug5__
		wcout << "     *** multiplicaton loop for target ID " << *i << endl;
#endif
		doubleType tempResult(0.);
		//addition loop
		for (bigNumVector::const_iterator j = srcPreTermSpan.begin(); j != srcPreTermSpan.end(); ++j) {
			const lexicalProbabilities::value_type::second_type& itj = probs.find(*j)->second;
			const lexicalProbabilities::value_type::second_type::const_iterator iti = itj.find(*i);
			if (iti != itj.end()) {
				tempResult += iti->second;
#ifdef __debug5a__
				wcout << "      +++ adding123 " << iti->second << " for source ID " << *j << endl;
			} else {
				wcout << "      +++ skipping123 for source ID " << *j << endl;
#endif
			}
		}
		//deal with NULL
		if (tempResult)
			//If we got some probability out of the addition loop, NULL has been taken care of. Perform multiplication and normalise.
			result *= inverseDenominator * ProbabilityType(tempResult, true);
		//If this is the multiplication loop for NULL and we came out with 0 out of the addition loop,
		else if (!*i)
			//assume that source NULL links to target NULL with probability 1 and normalise as usual.
			result *= inverseDenominator;
		//If this is the multiplication loop for a content word and we came out with 0 out of the addition loop,
		else
#ifndef __debug5__
			//then the source and target strings cannot be aligned, so produce an alignment score of 0.
			return 0.;
#else
		{
			result = 0.;
		}
		wcout << "     *** multiplying " << ((!tempResult && !*i ? 1. : tempResult) / srcPreTermSpan.size()) << " for target ID " << *i << " (" << result << ")" << endl;
#endif
	}
#ifdef __debug5__
	if (newResult != result) {
		wcerr << "  ™™™  Wrong result achieved for pair " << sourceNode.ID() << "/" << targetNode.ID() << "!!! ";
		wcerr << " Should be " << result << ", but is " << newResult << " instead." << endl;
	}
#endif*/
#else //#ifdef NEW_PROBS
	//multiplication loop
	for (size_t j = 0; j <= srcLoopEnd; ++j) {
		if (outside && j == srcBeg)
			if (srcEnd <= srcSentLen) j = srcEnd; else break;
		if (!outside && j && j < srcBeg) j = srcBeg;
		doubleType tempResult(0.);
		//addition loop
		for (size_t i = 0; i <= trgLoopEnd; ++i) {
			if (outside && i == trgBeg)
				if (trgEnd <= trgSentLen) i = trgEnd; else break;
			if (!outside && i && i < trgBeg) i = trgBeg;
			//			wcerr << "‚‚‚ Operating on positions " << j << "/" << i << endl;
//			if (newProbs[j][i])
				tempResult += newProbs[j][i];
		}

		if (tempResult)
			newResult *= tempResult;
		else if (j)
			return ProbabilityType::Zero;
	}
		
	
/*#ifdef __debugX__
	wcout << " 2nd display srcPreTermSpan:";
	for (bigNumVector::size_type i = 0; i < srcPreTermSpan.size() - 1; ++i)
		wcout << " " << source.findNode(srcPreTermSpan[i])->Label() << "-" << srcPreTermSpan[i];
	wcout << " NULL" << endl;
	wcout << "    2nd display trgPreTermSpan:";
	for (bigNumVector::size_type i = 0; i < trgPreTermSpan.size() - 1; ++i)
		wcout << " " << target.findNode(trgPreTermSpan[i])->Label() << "-" << trgPreTermSpan[i];
	wcout << " NULL" << endl;
#endif
	//multiplication loop
	for (bigNumVector::const_iterator j = srcPreTermSpan.begin(); result && (j != srcPreTermSpan.end()); ++j) {
#ifdef __debug5__
		wcout << "     *** multiplicaton loop for source ID " << *j << endl;
#endif
		doubleType tempResult(0.);
		//addition loop
		for (bigNumVector::const_iterator i = trgPreTermSpan.begin(); i != trgPreTermSpan.end(); ++i) {
			const lexicalProbabilities::value_type::second_type& itj = probs.find(*j)->second;
			lexicalProbabilities::value_type::second_type::const_iterator iti = itj.find(*i);
			if (iti != itj.end()) {
				tempResult += iti->second;
#ifdef __debug5__
				wcout << "      +++ adding123 " << iti->second << " for target ID " << *i << endl;
			} else {
				wcout << "      +++ skipping123 for target ID " << *i << endl;
#endif
			}
		}
		if (tempResult)
			result *= tempResult;
		else if (*j)
#ifndef __debug5__
			return 0.;
#else
			result = 0.;
		wcout << "     *** multiplying " << tempResult << " for source ID " << *j << endl;
#endif
	}
#ifdef __debug5__
	if (newResult != result) {
		wcerr << "  ™™™  Wrong result achieved for pair " << sourceNode.ID() << "/" << targetNode.ID() << "!!! ";
		wcerr << " Should be " << result << ", but is " << newResult << " instead." << endl;
	}
#endif*/
#endif //#ifndef NEW_PROBS
//	return result;
	return newResult;
}

void TreePair::processInducedLinks() {
#ifndef CHECK_MAN_LINKS
	buildIndices();
	if (collectExpensiveStatistics == "search" || collectExpensiveStatistics == "all")
		saveSearchSpace();
#ifdef __debug8__
	//	printInducedLinks();
#endif
#endif //#infndef CHECK_MAN_LINKS
FIRST_STAGE:
	reconsiderFirstStage = false;
	changed = true;
	while (!linksProbabilityIndex.empty() && changed) {
		changed = false;
		linkBest(linksProbabilityIndex);
	}
	if (VERSION != "0.7") {
		reconsiderFirstStage = !linksProbabilityIndex.empty();
#ifdef __debug8__
		wcout << endl << "!!!STAGE 2!!!" << endl;
#endif
		changed = true;
		while (!one2oneLinksProbabilityIndex.empty() && changed) {
			changed = false;
			linkBest(one2oneLinksProbabilityIndex);
			if (reconsiderFirstStage && changed) goto FIRST_STAGE;
		}
	}
}

#ifdef DELAY_CONSTITUENT
void TreePair::linkBest(hypothesesMapProbabilityIndex& linksProbabilityIndex, const skipStructure& skip) {
	if (linksProbabilityIndex.empty()) return;
	
#ifdef __debug8a__
	//	if (skip > 0) wcout << "!!Skipping " << skip << " levels!" << endl;
	printInducedLinks();
#endif
	
	ProbabilityType bestProbability = linksProbabilityIndex.rbegin()->first;
	hypothesesMapProbabilityIndex::reverse_iterator bestProbIterator;
	bool found = false;
	for (bestProbIterator = linksProbabilityIndex.rbegin(); !found && bestProbIterator != linksProbabilityIndex.rend(); ++bestProbIterator) {
		if (bestProbIterator->first < bestProbability) {
			bestProbability = bestProbIterator->first;
		}
		if (skip.first.find(bestProbIterator->second->first.first) == skip.first.end() &&
				skip.second.find(bestProbIterator->second->first.second) == skip.second.end())
			found = true;
	}
	if (!found) return;
	
	hypothesesMapProbabilityIndex::size_type count = linksProbabilityIndex.count(bestProbability);
	if (count > 1) {
		pair<hypothesesMapProbabilityIndex::const_iterator, hypothesesMapProbabilityIndex::const_iterator> range = linksProbabilityIndex.equal_range(bestProbability);
		linksIndex notSkipped;
		for (hypothesesMapProbabilityIndex::const_iterator it = range.first; it != range.second; ++it) {
			if (skip.first.find(it->second->first.first) == skip.first.end() &&
					skip.second.find(it->second->first.second) == skip.second.end()) {
				notSkipped[it->second->first] = it->second;
#ifdef __debug8a__
				wcout << "  >>not skipping " << it->second->first.first << "<=>" << it->second->first.second << endl;
#endif
			}
		}
		if (notSkipped.size() > 1) {
			if (!decideBest(notSkipped)) {
				skipStructure newSkip = skip;
				for (linksIndex::const_iterator it = notSkipped.begin(); it != notSkipped.end(); ++it) {
					newSkip.first.insert(it->second->first.first);
					newSkip.second.insert(it->second->first.second);
				}
				linkBest(linksProbabilityIndex, newSkip);
				return;
			}
		} else {
#ifndef RESCORE
			link(notSkipped.begin()->second);
#else
			hypothesesMap::iterator it = notSkipped.begin()->second;
			link(it);
			rescoreLinks(it->first);
#endif
		}
	} else {
		if (linksProbabilityIndex.find(bestProbability)->second->second->isDecided()) {
			wcout << "!!!ERROR!!! Linking decided link!!!" << endl;
			exit(99);
		}
#ifndef RESCORE
		link(linksProbabilityIndex.find(bestProbability)->second);
#else
		hypothesesMap::iterator it = linksProbabilityIndex.find(bestProbability)->second;
		link(it);
		rescoreLinks(it->first);
#endif
	}
	
	cleanDecided();
	if (reconsiderFirstStage)
		return;
	else
		linkBest(linksProbabilityIndex);
}

bool TreePair::decideBest(linksIndex& notSkipped) {
	linksVector links;
	links.reserve(notSkipped.size());
	for (linksIndex::const_iterator it = notSkipped.begin(); it != notSkipped.end(); ++it)
		links.push_back(it->second->first);
	
	if (!areCrossing(links)) {
		for (linksIndex::iterator cit = notSkipped.begin(); cit != notSkipped.end(); ++cit)
			link(cit->second);
#ifdef RESCORE
		rescoreLinks(links);
#endif
		
#ifdef __debug8__
		wcout << "!!!DECIDED!!!" << endl;
#endif
		return true;
	} else {
		return false;
	}
}
#else //#ifdef DELAY_CONSTITUENT
void TreePair::linkBest(hypothesesMapProbabilityIndex& linksProbabilityIndex, const int skip) {
	if (linksProbabilityIndex.empty()) return;
	
#ifdef __debug8a__
	if (skip > 0) wcout << "!!Skipping " << skip << " levels!" << endl;
	printInducedLinks(true);
#endif
	
	int tempSkip = skip;
	ProbabilityType bestProbability = linksProbabilityIndex.rbegin()->first;
	hypothesesMapProbabilityIndex::reverse_iterator bestProbIterator;
	for (bestProbIterator = linksProbabilityIndex.rbegin(); tempSkip > 0 && bestProbIterator != linksProbabilityIndex.rend(); ++bestProbIterator) {
		if (bestProbIterator->first < bestProbability) {
			bestProbability = bestProbIterator->first;
			tempSkip--;
		}
	}
	if (tempSkip > 0) return;
	
	hypothesesMapProbabilityIndex::size_type count = linksProbabilityIndex.count(bestProbability);
	if (count > 1) {
		if (decideBest(linksProbabilityIndex, skip)) {
			if (reconsiderFirstStage)
				return;
			else
				linkBest(linksProbabilityIndex);
		} else {
			linkBest(linksProbabilityIndex, skip + 1);
		}
	} else if (!linksProbabilityIndex.find(bestProbability)->second->second->isDecided()) {
#ifndef RESCORE
		link(linksProbabilityIndex.find(bestProbability)->second);
		cleanDecided();
#else
		hypothesesMap::iterator it = linksProbabilityIndex.find(bestProbability)->second;
		link(it);
		rescoreLinks(it->first);
		cleanDecided();
#endif
		if (reconsiderFirstStage)
			return;
		else
			linkBest(linksProbabilityIndex);
	}
}

bool TreePair::decideBest(hypothesesMapProbabilityIndex& linksProbabilityIndex, int skip) {
	ProbabilityType bestProbability = linksProbabilityIndex.rbegin()->first;
#ifdef __debug8__
	wcout << "      initial probability: " << bestProbability << endl;
#endif
	hypothesesMapProbabilityIndex::reverse_iterator bestProbIterator;
	for (bestProbIterator = linksProbabilityIndex.rbegin(); skip > 0 && bestProbIterator != linksProbabilityIndex.rend(); ++bestProbIterator) {
		if (bestProbIterator->first < bestProbability) {
#ifdef __debug8__
#ifdef SEP_PROB_TYPE
			wcout << "      skipping at probability: " << bestProbIterator->first << endl;
#else
			wcout << "      skipping at probability: " << bestProbIterator->first << " (diffrence: " << bestProbability - bestProbIterator->first << ")" << endl;
#endif
#endif
			bestProbability = bestProbIterator->first;
			skip--;
		}
	}
#ifdef __debug8__
	wcout << "      final probability: " << bestProbability << endl;
#endif
	
	pair<hypothesesMapProbabilityIndex::iterator, hypothesesMapProbabilityIndex::iterator> range = linksProbabilityIndex.equal_range(bestProbability);
	linksVector links;
	for (hypothesesMapProbabilityIndex::iterator it = range.first; it != range.second; ++it)
		links.push_back(it->second->first);
	
	if (!areCrossing(links)) {
//		range = linksProbabilityIndex.equal_range(bestProbability);
		for (hypothesesMapProbabilityIndex::iterator cit = range.first; cit != range.second; ++cit)
			link(cit->second);
#ifdef RESCORE
		cleanDecided();
		rescoreLinks(links);
#endif
		
#ifdef __debug8__
		wcout << "!!!DECIDED!!!" << endl;
#endif
		cleanDecided();
		return true;
	} else {
		return false;
	}
}
#endif //DELAY_CONSTITUENT

void TreePair::link(hypothesesMap::iterator it) {
	changed = true;
	
#ifdef __debug10__
	wcout << ">>Linked " << source.findNode(it->first.first)->Label() << "-" << it->first.first << " to " << target.findNode(it->first.second)->Label() << "-" << it->first.second << endl;
#endif
	LinkHypothesis* best = it->second;
	best->link();
#ifndef OUTPUT_SCORES
	links[it->first.first] = it->first.second;
#else
	links[it->first.first] = make_pair(it->first.second, it->second->probability());
#endif
	
#ifdef __debug8__
	wcout << " Blocking for the link between " << it->first.first << " and " << it->first.second << endl;
	wcout << "  block4src ";
#endif
	eliminateLinksForIndex(it->first.first, linksRowIndex);
	
#ifdef __debug8__
	wcout << "  block4trg ";
#endif
	eliminateLinksForIndex(it->first.second, linksColumnIndex);
	
	eliminateCrossingForLink(it->first);
}

void TreePair::eliminateLinksForIndex(bigNumber nodeID, const hypothesesMapElementIndex& index) {
	pair<hypothesesMapElementIndex::const_iterator, hypothesesMapElementIndex::const_iterator> range = index.equal_range(nodeID);
	for (hypothesesMapElementIndex::const_iterator elit = range.first; elit != range.second; ++elit)
		if (!elit->second->isDecided()) {
			elit->second->block();
#ifdef __debug8__
			wcout << ".";
#endif
		}
#ifdef __debug8__
	wcout << endl;
#endif
}

void TreePair::eliminateCrossingForLink(const linkAddressPair& link) {
	eliminateCrossingForLink(link, source, target);
}

void TreePair::eliminateCrossingForLink(const linkAddressPair& link, Tree& source, Tree& target) {
#ifdef __debug8__
	wcout << "   1. srcDesc vs trgNonDesc" << endl;
#endif
	blockLinks(source.descendants(link.first), target.nonDescendants(link.second));
	
#ifdef __debug8__
	wcout << "   2. srcNonDesc vs trgDesc" << endl;
#endif
	blockLinks(source.nonDescendants(link.first), target.descendants(link.second));
	
#ifdef __debug8__
	wcout << "   3. srcAnces vs trgNonAnces" << endl;
#endif
	blockLinks(source.ancestors(link.first), target.nonAncestors(link.second));
	
#ifdef __debug8__
	wcout << "   4. srcNonAnces vs trgAnces" << endl;
#endif
	blockLinks(source.nonAncestors(link.first), target.ancestors(link.second));
}

void TreePair::blockLinks(const bigNumVector& sourceNodes, const bigNumVector& targetNodes) {
#ifdef __debug8__
	wcout << "                            ";
	for (bigNumVector::const_iterator it = sourceNodes.begin(); it != sourceNodes.end(); wcout << *(it++) << " ");
	wcout << "/ ";
	for (bigNumVector::const_iterator it = targetNodes.begin(); it != targetNodes.end(); wcout << *(it++) << " ");
	wcout << endl;
#endif
	for (bigNumVector::const_iterator src = sourceNodes.begin(); src != sourceNodes.end(); ++src)
		for (bigNumVector::const_iterator trg = targetNodes.begin(); trg != targetNodes.end(); ++trg) {
			hypothesesMap::iterator it = inducedLinks.find(make_pair(*src, *trg));
			if (it != inducedLinks.end() && !it->second->isDecided()) {
				it->second->block();
#ifdef __debug8__
				//				wcout << "   Blocked the link between " << source.findNode(*src)->Label() << "-" << *src << " and " << target.findNode(*trg)->Label() << "-" << *trg << endl;
				wcout << "   Blocked the link between " << *src << " and " << *trg << endl;
#endif
			}
		}
}

void TreePair::cleanDecided() {
#ifdef __debug8__
	wcout << "≠≠≠hypotheses before cleaning: " << inducedLinks.size() << endl;
#endif
	vector<linkAddressPair> toDelete;
	for (hypothesesMap::iterator it = inducedLinks.begin(); it != inducedLinks.end(); ++it) {
		if (it->second->isDecided()) {
			toDelete.push_back(it->first);
			delete(it->second);
		}
	}
	for (vector<linkAddressPair>::size_type i = 0; i < toDelete.size(); ++i)
		inducedLinks.erase(toDelete[i]);
#ifdef __debug8__
	wcout << "≠≠≠hypotheses after cleaning: " << inducedLinks.size() << endl;
#endif
	
	buildIndices();
	if (collectExpensiveStatistics == "search" || collectExpensiveStatistics == "all")
		saveSearchSpace();
}

void TreePair::printProposedLinks() const {
	for (hypothesesMapProbabilityIndex::const_reverse_iterator it = linksProbabilityIndex.rbegin(); it != linksProbabilityIndex.rend(); ++it)
		if (it->second->second->isLinked()) {
			wcout << " +++Link between " << source.findNode(it->second->first.first)->Label() << "-" << source.findNode(it->second->first.first)->ID() << " and " << target.findNode(it->second->first.second)->Label() << "-" << target.findNode(it->second->first.second)->ID() << " with probability " << it->first << endl;
		}
}

bool TreePair::areCrossing(const linksVector& links) {
	for (linksVector::size_type i = 0; i < links.size() - 1; ++i)
		for (linksVector::size_type j = i + 1; j < links.size(); ++j)
			if (areCrossing(links[i], links[j])) return true;
	return false;
}

bool TreePair::areCrossing(const linkAddressPair& first, const linkAddressPair& second, Tree& source, Tree& target) {
	pair<linkAddressPair, linkAddressPair> linkPair = make_pair(first, second);
	map<pair<linkAddressPair, linkAddressPair>, bool>::const_iterator it = crossingLinksMap.find(linkPair);
	if (it != crossingLinksMap.end()) return it->second;
	
	bool result = (first.first == second.first ||
								 first.second == second.second ||
								 (source.dominates(first.first, second.first) && !target.dominates(first.second, second.second)) ||
								 (source.dominates(second.first, first.first) && !target.dominates(second.second, first.second)) ||
								 (target.dominates(first.second, second.second) && !source.dominates(first.first, second.first)) ||
								 (target.dominates(second.second, first.second) && !source.dominates(second.first, first.first)));
	
	crossingLinksMap[linkPair] = result;
	return result;
}

#ifdef CHECK_MAN_LINKS
void TreePair::fixManualLinks() {
	buildIndices();
#ifdef __debug8__
	printInducedLinks();
#endif
	for (linksMap::const_iterator it = manLinks.begin(); it != manLinks.end(); ++it)
		if (source.findNode(it->first)->Span() == 1 || target.findNode(it->second)->Span() == 1)
			link(inducedLinks.find(*it));
	blockZeroes();
	cleanDecided();
	
	for (int i = 1; i < source.Length(); ++i) {
		pair<hypothesesMapElementIndex::const_iterator, hypothesesMapElementIndex::const_iterator> range = linksRowIndex.equal_range(dynamic_cast<NonTerminal*>(source.findNode(make_pair(i, 1)))->ID());
		if (range.first != range.second) {
			for (hypothesesMapElementIndex::const_iterator it = range.first; it != range.second; ++it)
				it->second->block();
		}
	}
	for (int i = 1; i < target.Length(); ++i) {
		pair<hypothesesMapElementIndex::const_iterator, hypothesesMapElementIndex::const_iterator> range = linksColumnIndex.equal_range(dynamic_cast<NonTerminal*>(target.findNode(make_pair(i, 1)))->ID());
		if (range.first != range.second) {
			for (hypothesesMapElementIndex::const_iterator it = range.first; it != range.second; ++it)
				it->second->block();
		}
	}
	cleanDecided();
}

void TreePair::freezeLinks() {
	bigNumVector manToDelete, autToDelete;
	linksVector autLinks;
	for (linksMap::const_iterator it = links.begin(); it != links.end(); ++it) {
		if (source.findNode(it->first)->Span() != 1 && target.findNode(it->second)->Span() != 1)
			//		if (source.findNode(it->first)->Span() == 1 || target.findNode(it->second)->Span() == 1) {
			if (manLinks.find(it->first) == manLinks.end()) {
				autToDelete.push_back(it->first);
				//				wcout << "1.Deleting the link " << it->first << " " << it->second << endl;
			} else {
				manToDelete.push_back(it->first);
				autLinks.push_back(*it);
				//				wcout << "2.Deleting the link " << it->first << " " << it->second << endl;
			}
			else
				autLinks.push_back(*it);
	}
	for (bigNumVector::size_type i = 0; i < autToDelete.size(); links.erase(autToDelete[i++]));
	for (bigNumVector::size_type i = 0; i < manToDelete.size(); manLinks.erase(manToDelete[i++]));
	
	autLinks.push_back(linkAddressPair());
	for (linksMap::const_iterator it = manLinks.begin(); it != manLinks.end(); ++it) {
		if (source.findNode(it->first)->Span() != 1 && target.findNode(it->second)->Span() != 1) {
			//		if (source.findNode(it->first)->Span() == 1 || target.findNode(it->second)->Span() == 1) {
			autLinks[autLinks.size() - 1] = *it;
			if (!areCrossing(autLinks)) {
				links.insert(*it);
				autLinks.push_back(linkAddressPair());
				//				wcout << "3.Adding the link " << it->first << " " << it->second << endl;
			} else {
				//				wcout << "4.The link " << it->first << " " << it->second << " crosses the old ones" << endl;
			}
		}
	}
	/*	if (source.depth() > 1 && target.depth() > 1) {
	 bigNumVector toDelete;
	 for (linksMap::const_iterator it = links.begin(); it != links.end(); ++it) {
	 wstring srcLabel = source.findNode(it->first)->Label();
	 wstring trgLabel = target.findNode(it->second)->Label();
	 if ((srcLabel.substr(srcLabel.length() - 1) == L"x" && srcLabel != L"VPaux") || trgLabel.substr(trgLabel.length() - 1) == L"x")
	 //			if (source.findNode(it->first)->Span() == 1 && target.findNode(it->second)->Span() == 1)
	 toDelete.push_back(it->first);
	 }
	 for (bigNumVector::size_type i = 0; i < toDelete.size(); links.erase(toDelete[i++]));
	 }*/
}
#endif //CHECK_MAN_LINKS1

void TreePair::link() {
	if (USE_PHRASES)
		createSubStringLists();
	initLexicalInfo();
#ifdef __debug7__
	printLexMaps();
#endif
	prepareLexicalLinks();
#ifndef STATISTICS
	if (collectExpensiveStatistics == "search" || collectExpensiveStatistics == "all") {
		searchSpace.push_back(bigNumVector());
		saveSearchSpace();
	}
#ifdef __debug8b__
	printInducedLinks(true, true);
#endif
#endif //#ifndef STATISTICS
	calculateInternalLinksStatistics(true);
#ifndef STATISTICS
#ifdef CHECK_MAN_LINKS
	fixManualLinks();
#ifdef __debug8__
	printInducedLinks(true);
#endif
#endif //#ifdef CHECK_MAN_LINKS
#ifdef LATTICE
	/*	buildLinkPowerSet();
	 //	printPowerSet(wcout);
	 findMaximalLinkSets();
	 printMaximalLinkSets(wcout);*/
	initLatticeData();
#ifdef __debug1_lat__
	printLinkIDs(wcout);
	printCrossings(wcout);
#endif
	findMaximalLinkSets();
#ifdef __debug2_lat__
	printMaximalLinkSets(wcout);
	selectAndPrintMaxLinkSet(wcout);
#endif
#endif //#ifdef LATTICE
	processInducedLinks();
#endif //#ifndef STATISTICS
	calculateInternalLinksStatistics(false);
#ifndef STATISTICS
#ifdef __debug8__
	printInducedLinks(true);
#endif
#ifdef __debug8__
	buildIndices();
	printInducedLinks();
	printProposedLinks();
#endif
#endif //#ifndef STATISTICS
#ifdef CHECK_MAN_LINKS1
	freezeLinks();
#endif //CHECK_MAN_LINKS1
}
#endif //#ifndef TRAINING

void TreePair::printStandard(wostream& out) const {
	printStandard(out, source, target);
}

void TreePair::printStandard(wostream& out, const Tree& source, const Tree& target) const {
	bool pairedMode = Tree::pairedMode;
	Tree::pairedMode = true;
	out << "Source sentence (length:" << setw(3) << source.sentLength() << "; depth:" << setw(3) << source.depth() << "): " << source.sentence() << endl << "Target sentence (length: " << setw(2) << target.sentLength() << "; depth:" << setw(3) << target.depth() << "): " << target.sentence() << endl;
	out << "Source tree:" << endl << source;
	Tree::setLinks(NULL);
	out << "Target tree:" << endl << target << endl;
	Tree::setLinks(&links);
	Tree::pairedMode = pairedMode;
}

void TreePair::printXML(wostream& out) const {
	printBracketed(out, source, target, true);
}

void TreePair::printBracketed(wostream& out, bool is_XML) const {
	printBracketed(out, source, target, is_XML);
}

void TreePair::printBracketed(wostream& out, const Tree& source, const Tree& target, bool is_XML) const {
	source.printBracketed(out, is_XML);
	target.printBracketed(out, is_XML);
	if (!links.empty())
		for (linksMap::const_iterator it = links.begin(); it != links.end(); ++it)
#ifndef OUTPUT_SCORES
			out << it->first << " " << it->second << " ";
#else
	out << it->first << " " << it->second.first << " " << it->second.second << " ";
#endif
	out << endl;
}

void TreePair::printSentences(wostream& out) const {
	printSentences(out, source, target);
}

void TreePair::printSentences(wostream& out, const Tree& source, const Tree& target) const {
	out << source.sentence() << endl << target.sentence() << endl;
}

tagMap TreePair::sourceTags = tagMap();
tagMap TreePair::targetTags = tagMap();
tagMap TreePair::linkTags = tagMap();

void TreePair::collectTagsHelper(const Tree& tree, tagMap& tags) {
	for (Tree::const_nterm_iterator node = tree.ntbegin(); node != tree.ntend(); tags[(node++)->Label()]++);
}

void TreePair::collectTags() {
	collectTags(source, target);
}

void TreePair::collectTags(const Tree& source, const Tree& target) {
	if (collectExpensiveStatistics != "POS" && collectExpensiveStatistics != "all")
		return;
	collectTagsHelper(source, sourceTags);
	collectTagsHelper(target, targetTags);
	for (linksMap::const_iterator it = links.begin(); it != links.end(); ++it)
#ifndef OUTPUT_SCORES
		linkTags[source.findNode(it->first)->Label() + L" <=> " + target.findNode(it->second)->Label()]++;
#else
	linkTags[source.findNode(it->first)->Label() + L" <=> " + target.findNode(it->second.first)->Label()]++;
#endif
}

void TreePair::printTagsHelper(wostream& out, const tagMap& tags) {
	for (tagMap::const_iterator it = tags.begin(); it != tags.end(); ++it)
		out << setw(50) << it->first << " " << it->second << endl;
	//		out << setw(15) << it->first << setw(6) << it->second << endl;
}

void TreePair::printTags(wostream& out) {
	if (collectExpensiveStatistics != "POS" && collectExpensiveStatistics != "all")
		return;
	/*	sourceTags.erase(L"#ROOT#");
	 sourceTags.erase(L"#EOS#");
	 targetTags.erase(L"#ROOT#");
	 targetTags.erase(L"#EOS#");*/
	out << sourceTags.size() << " source tags:" << endl;
	printTagsHelper(out, sourceTags);
	out << endl << targetTags.size() << " target tags:" << endl;
	printTagsHelper(out, targetTags);
	out << endl << linkTags.size() << " link tags:" << endl;
	printTagsHelper(out, linkTags);
}

void TreePair::printTaggedSentences(wostream& out) const {
	source.printTaggedSentence(out);
	target.printTaggedSentence(out);
}

string TreePair::collectExpensiveStatistics = "all";
double TreePair::sourceAvgLinks = 0;
double TreePair::targetAvgLinks = 0;
double TreePair::totalAvgLinks = 0;
double TreePair::existingPhrasesAvg = 0;
double TreePair::existingPhrasesAvg2 = 0;
double TreePair::phrasesAvg = 0;
double TreePair::internalLinksAvg = 0;
double TreePair::strictInternalLinksAvg = 0;
double TreePair::initialNonzeroLinksAvg = 0;
double TreePair::initialInternalLinksAvg = 0;
double TreePair::initialStrictInternalLinksAvg = 0;
double TreePair::realisedInternalLinksAvg = 0;
double TreePair::realisedStrictInternalLinksAvg = 0;
double TreePair::linksAvg = 0;
double TreePair::sourceNodesAvg = 0;
double TreePair::targetNodesAvg = 0;
double TreePair::sourceLengthAvg = 0;
double TreePair::targetLengthAvg = 0;
double TreePair::undecidedAvg = 0;
unsigned TreePair::allPhrasesCount = 0;
unsigned TreePair::existingPhrasesCount = 0;
unsigned TreePair::sentencesWithPhrases = 0;
unsigned TreePair::treePairs = 0;
unsigned TreePair::totalUndecided = 0;
unsigned TreePair::initialInternalLinksCount = 0;
unsigned TreePair::initialStrictInternalLinksCount = 0;
unsigned TreePair::sentencesWithInternalLinks = 0;
unsigned TreePair::sentencesWithStrictInternalLinks = 0;
vector<bigNumVector> TreePair::searchSpace = vector<bigNumVector>();
#ifdef LATTICE
unsigned TreePair::totalSentencesWithFullSolutions = 0;
unsigned TreePair::totalSentencesWithAmbiguousFullSolutions = 0;
unsigned TreePair::totalSolutionsFound = 0;
unsigned TreePair::totalUndecidedWithFullSolutions = 0;
unsigned TreePair::totalUndecidedSolutionsFound = 0;
unsigned TreePair::totalUndecidedAmbiguousSolutionsFound = 0;
unsigned TreePair::totalAmbiguousSolutionsFound = 0;
#endif

void TreePair::saveSearchSpace() const {
	searchSpace[searchSpace.size() - 1].push_back(inducedLinks.size());
}

void TreePair::calculateInternalLinksStatistics(bool initial) const {
	calculateInternalLinksStatistics(initial, source, target);
}

void TreePair::calculateInternalLinksStatistics(bool initial, const Tree& source, const Tree& target) const {
	if (initial) {
		initialInternalLinksCount = 0;
		initialStrictInternalLinksCount = 0;
	}
	int realisedInternalLinksCount = 0, realisedStrictInternalLinksCount = 0;
	if (initial) {
		for (hypothesesMap::const_iterator it = inducedLinks.begin(); it != inducedLinks.end(); ++it) {
			//			if (!it->second->probability())
			//				continue;
			if (source.findNode(it->first.first)->Span() == 1 || target.findNode(it->first.second)->Span() == 1)
				++initialInternalLinksCount;
			if (source.findNode(it->first.first)->Span() > 1 && target.findNode(it->first.second)->Span() > 1)
				++initialStrictInternalLinksCount;
		}
	} else {
		for (linksMap::const_iterator it = links.begin(); it != links.end(); ++it) {
#ifndef OUTPUT_SCORES
			bigNumber second = it->second;
#else
			bigNumber second = it->second.first;
#endif
			if (source.findNode(it->first)->Span() == 1 || target.findNode(second)->Span() == 1)
				++realisedInternalLinksCount;
			if (source.findNode(it->first)->Span() > 1 && target.findNode(second)->Span() > 1)
				++realisedStrictInternalLinksCount;
		}
	}
	
	if (initial) {
		initialNonzeroLinksAvg += inducedLinks.size();
		initialInternalLinksAvg += initialInternalLinksCount;
		initialStrictInternalLinksAvg += initialStrictInternalLinksCount;
		if (initialInternalLinksCount)
			++sentencesWithInternalLinks;
#ifdef __debug8__
		else
			wcout << "!!!AAAAGRH!!! No Internal Links!" << endl;
#endif
		if (initialStrictInternalLinksCount)
			++sentencesWithStrictInternalLinks;
#ifdef __debug8__
		else
			wcout << "!!!AAAAGRH!!! No Strictly Internal Links!" << endl;
#endif
	} else {
		if (initialInternalLinksCount)
			realisedInternalLinksAvg += (realisedInternalLinksCount*1.)/initialInternalLinksCount;
		if (initialStrictInternalLinksCount)
			realisedStrictInternalLinksAvg += (realisedStrictInternalLinksCount*1.)/initialStrictInternalLinksCount;
	}
}

void TreePair::calculateStatistics() const {
	calculateStatistics(source, target);
}

void TreePair::calculateStatistics(const Tree& source, const Tree& target) const {
	++treePairs;
	sourceAvgLinks += (links.size()*1.) / (source.size() - source.sentLength());
	targetAvgLinks += (links.size()*1.) / (target.size() - target.sentLength());
	totalAvgLinks += (links.size()*1.) / ((source.size() - source.sentLength())*(target.size() - target.sentLength()));
	
	int internalLinksCount = 0, strictInternalLinksCount = 0, undecidedCount = 0;
	
#ifndef TRAINING
	//Count undecided links
	for (hypothesesMap::const_iterator it = inducedLinks.begin(); it != inducedLinks.end(); ++it)
		if (!it->second->isDecided())
			++undecidedCount;
	
	if (USE_PHRASES) {
		//Count the number of linked phrase-table phrases
		existingPhrasesCount = 0;
		for (linksMap::const_iterator it = links.begin(); it != links.end(); ++it) {
			extAlignmentMap::const_iterator srcit = sourcePhraseAlignments.find(sourceStrings->find(it->first)->second);
			if (srcit != sourcePhraseAlignments.end()) {
#ifndef OUTPUT_SCORES
				extAlignmentMap::value_type::second_type::const_iterator trgit = srcit->second.find(targetStrings->find(it->second)->second);
#else
				extAlignmentMap::value_type::second_type::const_iterator trgit = srcit->second.find(targetStrings->find(it->second.first)->second);
#endif
				if (trgit != srcit->second.end()) {
					++existingPhrasesCount;
					continue;
				}
			}
#ifndef OUTPUT_SCORES
			srcit = targetPhraseAlignments.find(targetStrings->find(it->second)->second);
#else
			srcit = targetPhraseAlignments.find(targetStrings->find(it->second.first)->second);
#endif
			if (srcit != targetPhraseAlignments.end()) {
				extAlignmentMap::value_type::second_type::const_iterator trgit = srcit->second.find(sourceStrings->find(it->first)->second);
				if (trgit != srcit->second.end()) {
					++existingPhrasesCount;
				}
			}
		}
	}
#endif //TRAINING
	
	//Count the number of (strictly) internal links
	for (linksMap::const_iterator it = links.begin(); it != links.end(); ++it) {
#ifndef OUTPUT_SCORES
		bigNumber second = it->second;
#else
		bigNumber second = it->second.first;
#endif
		if (source.findNode(it->first)->Span() == 1 || target.findNode(second)->Span() == 1)
			++internalLinksCount;
		if (source.findNode(it->first)->Span() > 1 && target.findNode(second)->Span() > 1)
			++strictInternalLinksCount;
	}
	
#ifndef TRAINING
	if (USE_PHRASES) {
		//Record the statistics on phrase-table phrases. Irrelevant when doing training.
		if (!links.empty())
			existingPhrasesAvg += existingPhrasesCount / (links.size()*1.);
		
		if (allPhrasesCount)
			existingPhrasesAvg2 += existingPhrasesCount / (allPhrasesCount*1.);
		
		phrasesAvg += allPhrasesCount;
		
		if (allPhrasesCount)
			++sentencesWithPhrases;
	}
#endif //TRAINING
	
	if (!links.empty()) {
		internalLinksAvg += internalLinksCount / (links.size()*1.);
		strictInternalLinksAvg += strictInternalLinksCount / (links.size()*1.);
	}
	linksAvg += links.size();
	
	sourceNodesAvg += source.size() - source.sentLength();
	targetNodesAvg += target.size() - target.sentLength();
	
	sourceLengthAvg += source.sentLength();
	targetLengthAvg += target.sentLength();
	
#ifndef STATISTICS
	undecidedAvg += undecidedCount;
	if (undecidedCount) {
		++totalUndecided;
		//		wcout << "Undecided crap!" << endl;
	}
#endif //STATISTICS
	
#ifdef LATTICE
	if (maximalLinkSets.size() != 0) {
		++totalSentencesWithFullSolutions;
		if (maximalLinkSets.size() > 1)
			++totalSentencesWithAmbiguousFullSolutions;
		if (undecidedCount)
			++totalUndecidedWithFullSolutions;
	}
	pair<bool, bool> latticeChecks = latticeChecksWithStandard();
	if (latticeChecks.first) {
		++totalSolutionsFound;
		if (undecidedCount)
			++totalUndecidedSolutionsFound;
	}
	if (latticeChecks.second) {
		++totalAmbiguousSolutionsFound;
		if (undecidedCount)
			++totalUndecidedAmbiguousSolutionsFound;
	}
#endif
	
}

void TreePair::printStatistics(wostream& out) {
	out << "Total number of tree pairs: " << treePairs << endl;
#ifdef LATTICE
	out << "==== Lattice Statistics ====" << endl;
	out << "Total number of tree pairs with full solutions: " << totalSentencesWithFullSolutions << endl;
	out << "Total number of tree pairs with ambiguous full solutions: " << totalSentencesWithAmbiguousFullSolutions << endl;
	out << "The greedy method found " << totalSolutionsFound << " solutions (" << (100. * totalSolutionsFound) / totalSentencesWithFullSolutions << "%)." << endl;
	out << "The greedy method found " << totalAmbiguousSolutionsFound << " solutions incl. ambiguous (" << (100. * totalAmbiguousSolutionsFound) / totalSentencesWithFullSolutions << "%)." << endl;
	out << "Total number of tree pairs with undecided links and full solutions: " << totalUndecidedWithFullSolutions << endl;
	out << "The greedy method found " << totalUndecidedSolutionsFound << " solutions for tree pairs with undecided links (" << (100. * totalUndecidedSolutionsFound) / totalUndecidedWithFullSolutions << "%)." << endl;
	out << "The greedy method found " << totalUndecidedAmbiguousSolutionsFound << " solutions incl. ambiguous for tree pairs with undecided links (" << (100. * totalUndecidedAmbiguousSolutionsFound) / totalUndecidedWithFullSolutions << "%)." << endl;
	out << "Total number of tree pairs without undecided links and full solutions: " << totalSentencesWithFullSolutions - totalUndecidedWithFullSolutions << endl;
	out << "The greedy method found " << totalSolutionsFound - totalUndecidedSolutionsFound << " solutions for tree pairs without undecided links (" << (100. * (totalSolutionsFound - totalUndecidedSolutionsFound)) / (totalSentencesWithFullSolutions - totalUndecidedWithFullSolutions) << "%)." << endl;
	out << "The greedy method found " << totalAmbiguousSolutionsFound - totalUndecidedAmbigousSolutionsFound << " solutions incl. ambigous for tree pairs without undecided links (" << (100. * (totalAmbiguousSolutionsFound - totalUndecidedAmbigousSolutionsFound)) / (totalSentencesWithFullSolutions - totalUndecidedWithFullSolutions) << "%)." << endl;
	out << "==== ======= ========== ====" << endl;
#endif //LATTICE
	out << "Average percentage of linked nodes in source trees:          " << (sourceAvgLinks / treePairs)*100 << "%" << endl;
	out << "Average percentage of linked nodes in target trees:          " << (targetAvgLinks / treePairs)*100 << "%" << endl;
	out << "Average percentage of links made vs. possible links:         " << (totalAvgLinks / treePairs)*100 << "%" << endl;
#ifndef TRAINING
	if (USE_PHRASES) {
		out << "Average % of links occurring in phrase table (rel to links): " << (existingPhrasesAvg / sentencesWithPhrases)*100 << "%" << endl;
		out << "Average % of links occurring in phrase table (rel to phr_t): " << (existingPhrasesAvg2 / sentencesWithPhrases)*100 << "%" << endl;
		out << "Average number of phrase table alignments per sentence pair: " << (phrasesAvg / sentencesWithPhrases) << endl;
		out << "Total number of sentence pairs with phrase table occurrings: " << sentencesWithPhrases << endl;
	}
#ifndef STATISTICS
	out << "Average number of undecided links per sentence pair:         " << undecidedAvg / totalUndecided << endl;
	out << "Total number of sentence pairs with undecided links:         " << totalUndecided << endl;
#endif //STATISTICS
#endif //TRAINING
	out << "Average percentage of internal links (rel to links):         " << (internalLinksAvg / treePairs)*100 << "%" << endl;
	out << "Average percentage of strictly internal links (rel 2 links): " << (strictInternalLinksAvg / treePairs)*100 << "%" << endl;
	out << "Total number of sentence pairs with lexical links:           " << sentencesWithInternalLinks << endl;
	out << "Total number of sentence pairs with strictly internal links: " << sentencesWithStrictInternalLinks << endl;
#ifndef STATISTICS
	out << "Average number of initial nonzero links:                     " << (initialNonzeroLinksAvg / treePairs) << endl;
	out << "Average number of initial lexical links:                     " << (initialInternalLinksAvg / sentencesWithInternalLinks) << endl;
	out << "Average number of initial strictly internal links:           " << (initialStrictInternalLinksAvg / sentencesWithStrictInternalLinks) << endl;
	out << "Average percentage of realised internal links:               " << (realisedInternalLinksAvg / sentencesWithInternalLinks)*100 << "%" << endl;
	out << "Average percentage of realised strictly internal links:      " << (realisedStrictInternalLinksAvg / sentencesWithStrictInternalLinks)*100 << "%" << endl;
#endif //STATISTICS
	out << "Average number of links per sentence pair:                   " << (linksAvg / treePairs) << endl;
	if (collectExpensiveStatistics == "POS" || collectExpensiveStatistics == "all")
		out << "Total number of tag-pairs:                                   " << linkTags.size() << endl;
	out << "Average number of nodes per source sentence:                 " << (sourceNodesAvg / treePairs) << endl;
	out << "Average number of nodes per target sentence:                 " << (targetNodesAvg / treePairs) << endl;
	out << "Average source sentence length:                              " << (sourceLengthAvg / treePairs) << endl;
	out << "Average target sentence length:                              " << (targetLengthAvg / treePairs) << endl;
	if (collectExpensiveStatistics == "search" || collectExpensiveStatistics == "all") {
		out << endl << "Printing search-space-reduction statistics:" << endl;
		long reductionStagesAvg = 0;
		for (vector<bigNumVector>::const_iterator i = searchSpace.begin(); i != searchSpace.end(); ++i) {
			reductionStagesAvg += i->size();
			//		out << "TreePair№" << (i+1) << ":";
			//		for (bigNumVector::size_type j = 0; j < searchSpace[i].size(); ++j) {
			//			out << " " << ((searchSpace[i][j]/(searchSpace[i][0]*1.))*100) << "%";
			//		}
			//		out << endl;
			//		if (i > 2 && ((i + 1) % 255 == 0))
			//			out << endl;
		}
		out << "Average number of search space reduction stages per pair:    " << ((reductionStagesAvg*1.) / treePairs) << endl;
	}
}

#ifdef EVALUATE
vector<pair<doubleType, doubleType> > TreePair::evaluate(const TreePair& automatic, const TreePair& manual) {
	/*bigNumVector toDelete;
	for (linksMap::const_iterator it = automatic.links.begin(); it != automatic.links.end(); ++it)
	 if (automatic.source.findNode(it->first)->Label() == L"Nx" || automatic.target.findNode(it->second)->Label() == L"Nx")
	 toDelete.push_back(it->first);
	 for (bigNumVector::size_type i = 0; i < toDelete.size(); automatic.links.erase(toDelete[i++]));
	 toDelete.clear();
	 for (linksMap::const_iterator it = manual.links.begin(); it != manual.links.end(); ++it)
	 if (manual.source.findNode(it->first)->Label() == L"Nx" || manual.target.findNode(it->second)->Label() == L"Nx")
	 toDelete.push_back(it->first);
	 for (bigNumVector::size_type i = 0; i < toDelete.size(); manual.links.erase(toDelete[i++]));*/
	return evaluate(automatic, automatic.links, manual, manual.links);
}
#endif //EVALUATE
	
vector<pair<doubleType, doubleType> > TreePair::evaluate(const TreePair& automatic, const linksMap& automaticLinks, const TreePair& manual, const linksMap& manualLinks) {
	int internalLinksCountAutomaitc = 0, strictInternalLinksCountAutomatic = 0, internalLinksCountManual = 0, strictInternalLinksCountManual = 0;
	
	for (linksMap::const_iterator it = automaticLinks.begin(); it != automaticLinks.end(); ++it) {
		if (automatic.source.findNode(it->first)->Span() > 1 && automatic.target.findNode(it->second)->Span() > 1)
			++strictInternalLinksCountAutomatic;
		if (automatic.source.findNode(it->first)->Span() == 1 || automatic.target.findNode(it->second)->Span() == 1)
			++internalLinksCountAutomaitc;
	}
	for (linksMap::const_iterator it = manualLinks.begin(); it != manualLinks.end(); ++it) {
		if (manual.source.findNode(it->first)->Span() > 1 && manual.target.findNode(it->second)->Span() > 1)
			++strictInternalLinksCountManual;
		if (manual.source.findNode(it->first)->Span() == 1 || manual.target.findNode(it->second)->Span() == 1)
			++internalLinksCountManual;
	}
	
	int precision = 0, recall = 0, internalPrecision = 0, internalRecall = 0, strictInternalPrecision = 0, strictInternalRecall = 0;
	for (linksMap::const_iterator ait = automaticLinks.begin(); ait != automaticLinks.end(); ++ait) {
		linksMap::const_iterator mit = manualLinks.find(ait->first);
		if (mit != manualLinks.end() && ait->second == mit->second) {
			++precision;
			if (automatic.source.findNode(ait->first)->Span() > 1 && automatic.target.findNode(ait->second)->Span() > 1)
				++strictInternalPrecision;
			if (automatic.source.findNode(ait->first)->Span() == 1 || automatic.target.findNode(ait->second)->Span() == 1)
				++internalPrecision;
		}
	}
	for (linksMap::const_iterator mit = manualLinks.begin(); mit != manualLinks.end(); ++mit) {
		linksMap::const_iterator ait = automaticLinks.find(mit->first);
		if (ait != automaticLinks.end() && ait->second == mit->second) {
			++recall;
			if (automatic.source.findNode(ait->first)->Span() > 1 && automatic.target.findNode(ait->second)->Span() > 1)
				++strictInternalRecall;
			if (automatic.source.findNode(ait->first)->Span() == 1 || automatic.target.findNode(ait->second)->Span() == 1)
				++internalRecall;
		}
	}
	vector<pair<doubleType, doubleType> > result;

	if (!manualLinks.size()) {
		result.push_back(make_pair(-1, -1));
		result.push_back(make_pair(-1, -1));
		result.push_back(make_pair(-1, -1));
		return result;
	}
	
	if (!automaticLinks.size())
		result.push_back(make_pair(0, 0));
	else
		result.push_back(make_pair((precision*1.)/automaticLinks.size(), (recall*1.)/manualLinks.size()));
	
	if (!internalLinksCountAutomaitc && !internalLinksCountManual)
		result.push_back(make_pair(-1, -1));
	else if (!internalLinksCountAutomaitc)
		if (!internalLinksCountManual)
			result.push_back(make_pair(0, (internalRecall*1.)/internalLinksCountManual));
		else
			result.push_back(make_pair(1, (internalRecall*1.)/internalLinksCountManual));
		else if (!internalLinksCountManual)
			if (!internalLinksCountAutomaitc)
				result.push_back(make_pair((internalPrecision*1.)/internalLinksCountAutomaitc, 0));
			else
				result.push_back(make_pair((internalPrecision*1.)/internalLinksCountAutomaitc, 1));
			else
				result.push_back(make_pair((internalPrecision*1.)/internalLinksCountAutomaitc, (internalRecall*1.)/internalLinksCountManual));
	
	if (!strictInternalLinksCountAutomatic && !strictInternalLinksCountManual)
		result.push_back(make_pair(-1, -1));
	else if (!strictInternalLinksCountAutomatic)
		if (!strictInternalLinksCountManual)
			result.push_back(make_pair(0, (strictInternalRecall*1.)/strictInternalLinksCountManual));
		else
			result.push_back(make_pair(1, (strictInternalRecall*1.)/strictInternalLinksCountManual));
		else if (!strictInternalLinksCountManual)
			if (!strictInternalLinksCountAutomatic)
				result.push_back(make_pair((strictInternalPrecision*1.)/strictInternalLinksCountAutomatic, 0));
			else
				result.push_back(make_pair((strictInternalPrecision*1.)/strictInternalLinksCountAutomatic, 1));
			else
				result.push_back(make_pair((strictInternalPrecision*1.)/strictInternalLinksCountAutomatic, (strictInternalRecall*1.)/strictInternalLinksCountManual));
	return result;
}

#ifdef EXTRACT_CHUNKS
binContextMap TreePair::chunks = binContextMap();

void TreePair::collectChunks() {
	createSubStringLists();
	for (linksMap::const_iterator it = links.begin(); it != links.end(); ++it) {
		wstring chunk = sourceStrings->find(it->first)->second + L" ||| " + targetStrings->find(it->second)->second + L" ||| ";
		if (chunks.find(chunk) != chunks.end())
			chunks[chunk]++;
		else
			chunks[chunk] = 1;
	}
}

void TreePair::printChunks(wostream& out) {
	for (binContextMap::const_iterator it = chunks.begin(); it != chunks.end(); ++it)
		out << it->first << it->second << endl;
}
#endif //EXTRACT_CHUNKS

#if defined(EXTRACT_CHUNKS) && defined(SAMT_RULES)
void TreePair::printSAMTRules(wostream& out) {
	
}
#endif

#ifdef LATTICE
/*void TreePair::initLinkPowerSet() {
 powerSet = linkPowerSet();
 powerSet.push_back(new LinkPowerSetRow());
 LinkPowerSetRow& currentRow = *powerSet[0];
 for (hypothesesMap::const_iterator link = inducedLinks.begin(); link != inducedLinks.end(); ++link)
 currentRow.push_back(new LinkSet(new linkAddressPair(link->first)));
 }
 
 void TreePair::buildNextLinkPowerSetRowHelper(linkPowerSet::size_type newRow, LinkPowerSetRow::size_type firstID, LinkPowerSetRow::size_type lastID) {
 LinkPowerSetRow& currentRow = *powerSet[newRow - 1];
 for (LinkPowerSetRow::size_type id = firstID; id < lastID; ++id) {
 bool used = false;
 for (LinkPowerSetRow::size_type last = id + 1; last <= lastID; ++last)
 if (!areCrossing(*(currentRow[id]->Last()), *(currentRow[last]->Last()))) {
 powerSet[newRow]->push_back(new LinkSet(currentRow[id], id, currentRow[last]->Last()));
 currentRow[last]->setSubSet();
 used = true;
 }
 if (used)
 currentRow[id]->setSubSet();
 }
 }
 
 void TreePair::buildNextLinkPowerSetRow() {
 linkPowerSet::size_type newRow = powerSet.size();
 
 if (newRow == 2) {
 bigNumber numberOfCrossingLinks = 0;
 for (map<pair<linkAddressPair, linkAddressPair>, bool>::const_iterator it = crossingLinksMap.begin(); it != crossingLinksMap.end(); ++it)
 if (it->second) numberOfCrossingLinks++;
 wcout << "There are " << numberOfCrossingLinks << " crossing links out of " << crossingLinksMap.size() << " possible combinations." << endl;
 }
 
 wcout << "Building row " << newRow << "… (previous row has " << powerSet[newRow - 1]->size() << " elements)" << endl;
 powerSet.push_back(new LinkPowerSetRow());
 
 int base = powerSet[newRow - 1]->getFirstBase();
 LinkPowerSetRow::size_type firstID = 0;
 LinkPowerSetRow::size_type lastID = powerSet[newRow - 1]->lastIDforBase(base, firstID);
 buildNextLinkPowerSetRowHelper(newRow, firstID, lastID);
 
 while (powerSet[newRow - 1]->getNextBase(base, firstID, lastID)) {
 lastID = powerSet[newRow - 1]->lastIDforBase(base, firstID);
 buildNextLinkPowerSetRowHelper(newRow, firstID, lastID);
 }
 }
 
 void TreePair::buildLinkPowerSet() {
 initLinkPowerSet();
 while (powerSet[powerSet.size() - 1]->size() > 0)
 buildNextLinkPowerSetRow();
 }
 
 void TreePair::printPowerSet(wostream& out, int lastRow) const {
 out << "PowerSet:" << endl;
 if (lastRow < 0)
 lastRow = powerSet.size() - 1;
 int i = 0;
 for (linkPowerSet::const_iterator row = powerSet.begin(); i <= lastRow; ++row) {
 out << " Row " << i++ << ": ";
 int j = 0;
 for (LinkPowerSetRow::iterator set = (*row)->begin(); set != (*row)->end(); ++set) {
 out << j++ << ") ";
 const setOfLinks* links = (*set)->Links();
 for (setOfLinks::const_iterator link = links->begin(); link != links->end(); ++link)
 out << (*link)->first << "/" << (*link)->second << ", ";
 delete links;
 out << "[" << (*set)->Base() << "; ";
 out << (*set)->Last()->first << "/" << (*set)->Last()->second << "] ";
 if ((*set)->isSubSet())
 out << "U ";
 out << "| ";
 }
 out << endl;
 }
 }
 
 bool TreePair::checkForSubSet(const setOfLinks& source, const LinkSet& targetSet) {
 const setOfLinks& target = *(targetSet.Links());
 setOfLinks::size_type targetID = 0;
 linkAddressPair* targetLink = target[targetID];
 for (setOfLinks::size_type sourceID = 0; sourceID < source.size() - 1; ++sourceID) {
 linkAddressPair* sourceLink = source[sourceID];
 while (*targetLink < *sourceLink && sourceID + 1 > targetID) {
 ++targetID;
 if (targetID >= target.size() - 1) {
 delete &target;
 return false;
 }
 targetLink = target[targetID];
 }
 if (*targetLink != *sourceLink) {
 delete &target;
 return false;
 }
 }
 delete &target;
 return true;
 }
 
 bool TreePair::checkForSubSet(const LinkSet& set, int setID, linkPowerSet::size_type row) const {
 LinkPowerSetRow* currentRow = powerSet[row];
 const setOfLinks& setLinks = *(set.Links());
 for (LinkPowerSetRow::iterator currSet = currentRow->begin(); currSet != currentRow->end() && setID > (*currSet)->Base(); ++currSet)
 if (*set.Last() == *((*currSet)->Last()) && checkForSubSet(setLinks, **currSet)) {
 delete &setLinks;
 return true;
 }
 delete &setLinks;
 return false;
 }
 
 void TreePair::findMaximalLinkSets() {
 bool change = true;
 for (linkPowerSet::size_type row = powerSet.size() - 1; change && row > 0; --row) {
 //#ifdef __debug1_lat__
 wcout << "Currently checking for finals at row " << row - 1 << "…" << endl;
 //#endif
 change = false;
 int setID = 0;
 LinkPowerSetRow* currentRow = powerSet[row - 1];
 for (LinkPowerSetRow::iterator set = currentRow->begin(); set != currentRow->end(); ++set, ++setID)
 if (!(*set)->isSubSet() && !checkForSubSet(**set, setID, row)) {
 change = true;
 maximalLinkSets.insert(make_pair((*set)->probabilityMass(inducedLinks), (*set)->Links()));
 
 if (maximalLinkSets.size() >= 100)
 return;
 }
 }
 }
 
 void TreePair::printMaximalLinkSets(wostream& out) const {
 out << "MaximalLinkSets: ";
 int j = 0;
 for (vectorOfLinkSets::const_reverse_iterator links = maximalLinkSets.rbegin(); links != maximalLinkSets.rend(); ++links) {
 out << j++ << ") ";
 for (setOfLinks::const_iterator link = (*links).second->begin(); link != (*links).second->end(); ++link)
 out << (*link)->first << "/" << (*link)->second << ", ";
 out << "<" << (*links).first << "> ";
 out << "| ";
 }
 out << endl;
 }*/

void TreePair::initLatticeData() {
	//Generate a vector of all possible links and sort it by the number of crossing links to optimise performance
	linkStorage = linkStorageVector();
	for (hypothesesMap::const_iterator link = inducedLinks.begin(); link != inducedLinks.end(); ++link)
		linkStorage.push_back(link->first);
	
	crossingLinks = crossingLinksStorageMap();
	for (linkIDType currentID = 0; currentID < linkStorage.size(); ++currentID) {
		crossingLinksStorageMap::iterator currentIDIterator = crossingLinks.insert(make_pair(currentID, setOfLinks())).first;
		for (linkIDType comparisonID = 0; comparisonID < linkStorage.size(); ++comparisonID)
			if (currentID == comparisonID || areCrossing(linkStorage[currentID], linkStorage[comparisonID]))
				currentIDIterator->second.insert(comparisonID);
		currentIDIterator->second.erase(currentID);
	}
	
	multimap<bigNumber, crossingLinksStorageMap::const_iterator> sortMap;
	for (crossingLinksStorageMap::const_iterator it = crossingLinks.begin(); it != crossingLinks.end(); ++it)
		sortMap.insert(make_pair(it->second.size(), it));
	
	linkStorageVector newStorage;
	map<linkIDType, linkIDType> reorderMap;
	for (multimap<bigNumber, crossingLinksStorageMap::const_iterator>::const_iterator it = sortMap.begin(); it != sortMap.end(); ++it) {
		newStorage.push_back(linkStorage[it->second->first]);
		reorderMap.insert(make_pair(it->second->first, newStorage.size() - 1));
	}
	
	crossingLinksStorageMap newCrossing;
	for (crossingLinksStorageMap::const_iterator link = crossingLinks.begin(); link != crossingLinks.end(); ++link) {
		crossingLinksStorageMap::iterator newCross = newCrossing.insert(make_pair(reorderMap[link->first], setOfLinks())).first;
		for (setOfLinks::const_iterator cross = link->second.begin(); cross != link->second.end(); ++cross)
			newCross->second.insert(reorderMap[*cross]);
	}
	
	linkStorage.clear();
	linkStorage.assign(newStorage.begin(), newStorage.end());
	crossingLinks.clear();
	crossingLinks.insert(newCrossing.begin(), newCrossing.end());
}

#ifdef __debug1_lat__
void TreePair::printLinkIDs(wostream& out) const {
	out << "Links Index:" << endl;
	for (linkIDType ID = 0; ID < linkStorage.size(); ++ID) {
		out << "  " << ID << ") ";
		const NonTerminal* sourceNode = source.findNode(linkStorage[ID].first);
		const NonTerminal* targetNode = target.findNode(linkStorage[ID].second);
		out << sourceNode->Label() << "-" << sourceNode->ID() << " <-> " << targetNode->Label() << "-" << targetNode->ID() << endl;
	}
	out << endl;
}

void TreePair::printCrossings(wostream& out) const {
	out << "Available Crossings:" << endl;
	for (crossingLinksStorageMap::const_iterator it = crossingLinks.begin(); it != crossingLinks.end(); ++it) {
		out << "  " << it->first << ": ";
		for (setOfLinks::const_iterator cross = it->second.begin(); cross != it->second.end(); out << *(cross++) << " ");
		out << endl;
	}
	out << endl;
}
#endif

doubleType TreePair::linkSetProbabilityMass(const setOfLinks& links) const {
	assert(links.size() > 0);
	doubleType probability = 0.;
	for (setOfLinks::const_iterator link = links.begin(); link != links.end(); ++link)
		probability += inducedLinks.find(linkStorage[*link])->second->probability()
#ifdef SEP_PROB_TYPE
		.getDouble()
#endif
		;	
	return probability;
}

void TreePair::saveMaximalLinkSet(const setOfLinks& linkSet) {
#ifdef __debug2_lat__
#ifndef __debug2a_lat__
	++totalMaximalLinkSetsFound;
#endif
#endif
	doubleType probabilityMass = linkSetProbabilityMass(linkSet);
#ifndef __debug2a_lat__
	if (!maximalLinkSets.empty())
		if (probabilityMass < maximalLinkSets.rbegin()->first)
			return;
		else if (probabilityMass > maximalLinkSets.rbegin()->first)
			maximalLinkSets.clear();
#endif
	mapOfLinkSets::iterator store = maximalLinkSets.insert(make_pair(probabilityMass, linkStorageVector()));
	for (setOfLinks::const_iterator link = linkSet.begin(); link != linkSet.end(); ++link)
		store->second.push_back(linkStorage[*link]);
	sort(store->second.begin(), store->second.end());
}

void TreePair::findMaximalLinkSetsHelper(const vectorOfLinks& workingVector, setOfLinks& resultSet, const linkIDType& currentID) {
#ifdef __debug1_lat__
	wcout << ">>Result:";
	for (setOfLinks::const_iterator it = resultSet.begin(); it != resultSet.end(); wcout << " " << *(it++));
	wcout << endl << ">>Current: " << currentID << endl << ">>Working:";
	for (vectorOfLinks::const_iterator it = workingVector.begin(); it != workingVector.end(); wcout << " " << *(it++));
	wcout << endl;
#endif
	
	if (workingVector.size() == 0) {
#ifdef __debug1_lat__
		wcout << "!! Output!" << endl << "!! Backtracking…" << endl << endl;
#endif
		saveMaximalLinkSet(resultSet);
		return;
	}
	
	bool lessThanCurrent = false;
#ifdef __debug1_lat__
	bool foundBigger = false;
	bool processed = false;
#endif
	for (vectorOfLinks::size_type newID = 0; newID < workingVector.size(); ++newID) {
		if (workingVector[newID] < currentID) {
			lessThanCurrent = true;
			continue;
		}
#ifdef __debug1_lat__
		processed = false;
#endif
		if (lessThanCurrent) {
#ifdef __debug1_lat__
			foundBigger = true;
#endif
			bool checked = true;
			for (vectorOfLinks::size_type less = 0; checked && workingVector[less] < currentID; ++less) {
				checked = false;
				const linkIDType& lessID = workingVector[less];
				for (vectorOfLinks::size_type checkerID = newID; !checked && checkerID < workingVector.size(); ++checkerID) {
					const setOfLinks& checker = crossingLinks[workingVector[checkerID]];
					if (checker.find(lessID) != checker.end())
						checked = true;
				}
			}
			if (!checked)
				break;
#ifdef __debug1_lat__
			else
				wcout << "!! Working less than Current!" << endl << "!!>> Excluded!" << endl;
#endif
		}
		const linkIDType& newLink = workingVector[newID];
		setOfLinks::iterator lastResult = resultSet.insert(newLink).first;
		vectorOfLinks transferVector;
		for (vectorOfLinks::const_iterator link = workingVector.begin(); link != workingVector.end(); ++link)
			if (*link != newLink) {
				const setOfLinks& newLinkCrossings = crossingLinks[newLink];
				if (newLinkCrossings.find(*link) == newLinkCrossings.end())
					transferVector.push_back(*link);
			}
#ifdef __debug1_lat__
		processed = true;
#endif
		findMaximalLinkSetsHelper(transferVector, resultSet, newLink);
		resultSet.erase(lastResult);
	}
#ifdef __debug1_lat__
	if (lessThanCurrent && !processed) {
		wcout << "!! Working less than Current!" << endl;
		if (foundBigger)
			wcout << "!!>> Not excluded!" << endl;
		else
			wcout << "!!>> All are less!" << endl;
	} else
		wcout << "!! No more elements in working!" << endl;
	
	wcout << "!! Backtracking…" << endl << endl;
#endif
}

void TreePair::findMaximalLinkSets() {
#ifdef __debug2_lat__
#ifndef __debug2a_lat__
	totalMaximalLinkSetsFound = 0;
#endif
#endif
	if (linkStorage.size() > 100)
		return;
	vectorOfLinks workingVector;
	for (crossingLinksStorageMap::iterator link = crossingLinks.begin(); link != crossingLinks.end(); workingVector.push_back((link++)->first));
	setOfLinks resultSet;
	linkIDType currentID = 0;
	findMaximalLinkSetsHelper(workingVector, resultSet, currentID);
}

#ifdef __debug2_lat__
void TreePair::printMaximalLinkSets(wostream& out) const {
	if (maximalLinkSets.size() == 0) {
		out << "Problem too big (" << linkStorage.size() << " available links)!" << endl;
		return;
	}
	out << "MaximalLinkSets (" << maximalLinkSets.count(maximalLinkSets.rbegin()->first) << " candidates out of " <<
#ifndef __debug2a_lat__
	totalMaximalLinkSetsFound
#else
	maximalLinkSets.size()
#endif
	<< " / " << linkStorage.size() << " available links): " << endl;
	int j = 0;
	for (mapOfLinkSets::const_iterator links = maximalLinkSets.begin(); links != maximalLinkSets.end(); ++links) {
		out << j++ << ") ";
		for (linkStorageVector::const_iterator link = links->second.begin(); link != links->second.end(); ++link)
			out << link->first << "/" << link->second << ", ";
		out << "<" << (*links).first << "> " << "| ";
	}
	out << endl;
}

void TreePair::selectAndPrintMaxLinkSet(wostream& out) {
	if (maximalLinkSets.size() == 0)
		return;
	mapOfLinkSets::const_iterator it = maximalLinkSets.begin();
	linkStorageVector firstSet = it->second;
	++it;
	for (linkStorageVector::const_iterator link = firstSet.begin(); link != firstSet.end(); ++link) {
		bool common = true;
		for (mapOfLinkSets::const_iterator links = it; common && links != maximalLinkSets.end(); ++links) {
			bool found = false;
			for (linkStorageVector::const_iterator ln = links->second.begin(); !found && ln != links->second.end(); ++ln)
				if (*ln == *link)
					found = true;
			common = found;
		}
		if (common) {
			bestMaxLinkSet.push_back(*link);
//			wcout << "  >>> inserted link " << link->first << "/" << link->second << endl;
		}
	}
	out << "Largest Common LinkSubSet (" << bestMaxLinkSet.size() << " links): ";
	for (linkStorageVector::const_iterator link = bestMaxLinkSet.begin(); link != bestMaxLinkSet.end(); ++link)
		out << link->first << "/" << link->second << ", ";
	out << endl;
}
#endif

pair<bool, bool> TreePair::latticeChecksWithStandard() const {
	bool checks = false;
	for (mapOfLinkSets::const_reverse_iterator latticeLinks = maximalLinkSets.rbegin(); !checks && latticeLinks != maximalLinkSets.rend(); ++latticeLinks) {
		checks = true;
		for (linkStorageVector::const_iterator link = latticeLinks->second.begin(); checks && link != latticeLinks->second.end(); ++link)
			if (links.find(link->first) == links.end() || links.find(link->first)->second != link->second)
				checks = false;
	}

	bool checks2 = bestMaxLinkSet.size() > 0;
	for (linkStorageVector::const_iterator link = bestMaxLinkSet.begin(); checks2 && link != bestMaxLinkSet.end(); ++link)
		if (links.find(link->first) == links.end() || links.find(link->first)->second != link->second)
			checks2 = false;

#ifdef __debug2_lat__
	if (checks)
		wcout << "The correct answer HAS been found!" << endl;
	else
		wcout << "The correct answer has NOT been found!" << endl;
	
	if (checks2)
		wcout << "  The correct answer HAS been found in the biggest common subset!" << endl;
	else
		wcout << "  The correct answer has NOT been found in the biggest common subset!" << endl;
#endif
	return make_pair(checks, checks2);
}

vector<pair<doubleType, doubleType> > TreePair::evaluateLattice() const {
	linksMap latticeLinks = linksMap();
	for (linkStorageVector::const_iterator link = bestMaxLinkSet.begin(); link != bestMaxLinkSet.end(); ++link)
		latticeLinks[link->first] = link->second;
	return evaluate(*this, links, *this, latticeLinks);
}
#endif //LATTICE