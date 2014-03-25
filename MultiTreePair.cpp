/*
 *  MultiTreePair.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 19.11.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "MultiTreePair.h"

string MultiTreePair::operation_mode = "str2str";
string MultiTreePair::input_type = "plain";
string MultiTreePair::output_type = "standard";

namespace bg_zhechev_ventsislav {
	wostream& operator<<(wostream& out, MultiTreePair& treePair) {
		if (MultiTreePair::output_type == "XML") {
			treePair.printXML(out);
			return out;
		}
		if (MultiTreePair::output_type == "parse") {
			treePair.printBracketed(out);
			return out;
		}
		out << "#BOP" << endl << treePair.source;
		Tree::setLinks(NULL);
		out << treePair.target;
		Tree::setLinks(&treePair.links);
		
		out << "#LINKS\t";
		if (!treePair.links.empty())
			for (linksMap::const_iterator it = treePair.links.begin(); it != treePair.links.end(); ++it)
#ifndef OUTPUT_SCORES
				out << it->first << " " << it->second << " ";
#else
		out << it->first << " " << it->second.first << " " << it->second.second << " ";
#endif
		out << endl << "#EOP" << endl;
		
		return out;
	}
	
	wistream& operator>>(wistream& in, MultiTreePair& treePair) {
		treePair.reset();
		//	MultiTree::setLinks(&treePair.links);
		MultiTree::binariseMode = false;
		
		if (MultiTreePair::input_type == "parsed" && DATA_SET == "HomeCentre")
			Tree::pairedMode = true;
		
		if (MultiTreePair::operation_mode == "str2tree") {
			if (MultiTreePair::input_type == "parsed") {
				wstringstream tempStream;
				MultiTreePair::parsedToTagged(in, tempStream);
				if (MultiTreePair::error)
					return in;
			
				MultiTree::input_type = "tagged";
				tempStream >> treePair.source;
			} else {
				if (MultiTreePair::input_type == "tagged")
					MultiTree::input_type = "tagged";
				else
					MultiTree::input_type = "plain";
				in >> treePair.source;
			}
				
			MultiTree::input_type = "parsed";
			in >> treePair.target;
			if (MultiTree::error) {
				MultiTreePair::error = true;
				return in;
			}
		} else if (MultiTreePair::operation_mode == "tree2str") {
			bool pairedMode = Tree::pairedMode;
			if (DATA_SET == "HomeCentre")
				Tree::pairedMode = true;
			MultiTree::input_type = "parsed";
			in >> treePair.source;
			if (DATA_SET == "HomeCentre")
				Tree::pairedMode = pairedMode;
			if (MultiTree::error) {
				MultiTreePair::error = true;
				return in;
			}
			
			if (MultiTreePair::input_type == "parsed") {
				wstringstream tempStream;
				MultiTreePair::parsedToTagged(in, tempStream);
				if (MultiTreePair::error)
					return in;
				
				MultiTree::input_type = "tagged";
				tempStream >> treePair.target;
			} else {
				if (MultiTreePair::input_type == "tagged")
					MultiTree::input_type = "tagged";
				else
					MultiTree::input_type = "plain";
				in >> treePair.target;
			}
		} else if (MultiTreePair::input_type == "parsed") {
			wstringstream tempStream;
			MultiTreePair::parsedToTagged(in, tempStream);
			if (MultiTreePair::error)
				return in;
			
			MultiTreePair::parsedToTagged(in, tempStream);
			if (MultiTreePair::error)
				return in;
			
			MultiTree::input_type = "tagged";
			tempStream >> treePair.source >> treePair.target;
		} else {
			if (MultiTreePair::input_type == "tagged")
				MultiTree::input_type = "tagged";
			in >> treePair.source;
			if ((MultiTreePair::error = MultiTree::error))
				return in;
			in >> treePair.target;
			if ((MultiTreePair::error = MultiTree::error))
				return in;
		}
		
		wstring input;
		getline(in, input);
		getline(in, input);
		return in;
	}
} //namespace

void MultiTreePair::parsedToTagged(wistream& in, wostream& out) {
	using namespace boost;
	Tree temp;
	in >> temp;
	if (temp.sentLength() < 0) {
		MultiTreePair::error = true;
		return;
	}
	wstring sentence;
	wstringstream tempStream;
	temp.printTaggedSentence(tempStream);
	sentence = tempStream.str();
//	wcout << endl << "??? The sentence is: " << sentence << endl;
	for (token_iterator_generator<char_separator<wchar_t>, wstring::const_iterator, wstring>::type it = make_token_iterator<wstring>(sentence.begin(), sentence.end(), char_separator<wchar_t>(SPACE)), end; it != end; ++it) {
		out << "((" << it->substr(0, it->find(L"->")) << " " << it->substr(it->find(L"->") + 2) << "))";
//		wcout << " ------ added data from " << *it << " «((" << it->substr(0, it->find(L"->")) << " " << it->substr(it->find(L"->") + 2) << "))»";
	}
	out << endl;
}

void MultiTreePair::printBracketed(wostream& out, bool is_XML) const {
	bool preserveStructure;
	if (MultiTreePair::operation_mode == "tree2str")
		preserveStructure = true;
	else
		preserveStructure = false;
	//We pass the links as a parameter, so that only linked nodes could be printed out
	source.printBracketed(out, links, is_XML, preserveStructure);
	//For the printout to work with the same method for the target tree, we create a reversed map of links
	linksMap reverseLinks;
	for (linksMap::const_iterator it = links.begin(); it != links.end(); ++it)
		reverseLinks[it->second] = it->first;
	if (MultiTreePair::operation_mode == "str2tree")
		preserveStructure = true;
	else
		preserveStructure = false;
	target.printBracketed(out, reverseLinks, is_XML, preserveStructure);
	//Here we print out the links, if there are any
	if (!links.empty())
		for (linksMap::const_iterator it = links.begin(); it != links.end(); ++it)
#ifndef OUTPUT_SCORES
			wcout << it->first << " " << it->second << " ";
#else
	wcout << it->first << " " << it->second.first << " " << it->second.second << " ";
#endif
	wcout << endl;
}

void MultiTreePair::printXML(wostream& out) const {
	//We simply instruct the printBracketed method to use XML notation, rather than replicating the code here
	printBracketed(out, true);
}

void MultiTreePair::createSubStringLists() {
	//Call the superclass method with the MultiTrees from the current MultiTreePair object as parameters
	TreePair::createSubStringLists(source, target);
}

#ifdef __debug7__
void MultiTreePair::printLexMaps() const {
	wcout << "source probs:" << endl;
	TreePair::printLexMap(source,
//												sourceLexProbs,
												newSourceLexProbs, source.sentLength(), target.sentLength());
	wcout << "target probs:" << endl;
	TreePair::printLexMap(target,
//												targetLexProbs,
												newTargetLexProbs, target.sentLength(), source.sentLength());
}
#endif

void MultiTreePair::prepareLexicalLinks() {
	//Call the superclass method with the MultiTrees from the current MultiTreePair object as parameters
	TreePair::prepareLexicalLinks(source, target);
}

void MultiTreePair::initLexicalInfo() {
	//Call the superclass methods with the MultiTrees from the current MultiTreePair object as parameters
	TreePair::initLexicalInfo(source, target, sourceWordAlignments,
//														sourceLexProbs,
														newSourceLexProbs);
	TreePair::initLexicalInfo(target, source, targetWordAlignments,
//														targetLexProbs,
														newTargetLexProbs);
}

void MultiTreePair::printInducedLinks(bool printStatus, bool trees) const {
	//Call the superclass method with the MultiTrees from the current MultiTreePair object as parameters
	TreePair::printInducedLinks(printStatus, trees, source, target);
}

#ifdef RESCORE
bool MultiTreePair::cleanLexProbabilites(const linkAddressPair& link) {
	//Call the superclass method with the MultiTrees from the current MultiTreePair object as parameters
	return TreePair::cleanLexProbabilites(source, target, link);
}

void MultiTreePair::rescoreLinks() {
	//Call the superclass method with the MultiTrees from the current MultiTreePair object as parameters
	TreePair::rescoreLinks(source, target);
}
#endif //RESCORE

void MultiTreePair::buildIndices() {
	//Call the superclass method with the MultiTrees from the current MultiTreePair object as parameters
	TreePair::buildIndices(source, target);
}

void MultiTreePair::eliminateCrossingForLink(const linkAddressPair& link) {
	//First eliminate incompatible nodes in the source and target trees
	bigNumSet& sourceIncompatible = source.getIncompatibleNodes(link.first);
	bigNumSet& targetIncompatible = target.getIncompatibleNodes(link.second);
	eliminateLinksForIncompatibleNodes(sourceIncompatible, targetIncompatible);
	source.eliminateIncompatibleNodes(sourceIncompatible);
	target.eliminateIncompatibleNodes(targetIncompatible);
	delete &sourceIncompatible;
	delete &targetIncompatible;
//	buildIndices();
	//Then call the superclass method with the MultiTrees from the current MultiTreePair object as parameters to perform the standard crossing-links elimination, taking care that the proper methods are run for pre-parsed trees (for str2tree and tree2str)
//	if (operation_mode == "tree2str") {
//		TreePair::eliminateCrossingForLink(link, static_cast<Tree&>(source), target);
//	} else if (operation_mode == "str2tree") {
//		TreePair::eliminateCrossingForLink(link, source, static_cast<Tree&>(target));
//	} else {
//		TreePair::eliminateCrossingForLink(link, source, target);
//	}

	
#ifdef __debug8__
	wcout << "   1. srcDesc vs trgNonDesc" << endl;
#endif
	if (operation_mode == "tree2str") {
		blockLinks(source.Tree::descendants(link.first), target.nonDescendants(link.second));
	} else if (operation_mode == "str2tree") {
		blockLinks(source.descendants(link.first), target.Tree::nonDescendants(link.second));
	} else {
		blockLinks(source.descendants(link.first), target.nonDescendants(link.second));
	}
	
#ifdef __debug8__
	wcout << "   2. srcNonDesc vs trgDesc" << endl;
#endif
	if (operation_mode == "tree2str") {
		blockLinks(source.Tree::nonDescendants(link.first), target.descendants(link.second));
	} else if (operation_mode == "str2tree") {
		blockLinks(source.nonDescendants(link.first), target.Tree::descendants(link.second));
	} else {
		blockLinks(source.nonDescendants(link.first), target.descendants(link.second));
	}
	
#ifdef __debug8__
	wcout << "   3. srcAnces vs trgNonAnces" << endl;
#endif
	if (operation_mode == "tree2str") {
		blockLinks(source.Tree::ancestors(link.first), target.nonAncestors(link.second));
	} else if (operation_mode == "str2tree") {
		blockLinks(source.ancestors(link.first), target.Tree::nonAncestors(link.second));
	} else {
		blockLinks(source.ancestors(link.first), target.nonAncestors(link.second));
	}
	
#ifdef __debug8__
	wcout << "   4. srcNonAnces vs trgAnces" << endl;
#endif
	if (operation_mode == "tree2str") {
		blockLinks(source.Tree::nonAncestors(link.first), target.ancestors(link.second));
	} else if (operation_mode == "str2tree") {
		blockLinks(source.nonAncestors(link.first), target.Tree::ancestors(link.second));
	} else {
		blockLinks(source.nonAncestors(link.first), target.ancestors(link.second));
	}
	
}

void MultiTreePair::eliminateLinksForIncompatibleNodes(const bigNumSet& sourceNodes, const bigNumSet& targetNodes) {
#ifdef __debug8__
	wcout << "   Source incompatible nodes:" << endl << "    ";
	for (bigNumSet::const_iterator it = sourceNodes.begin(); it != sourceNodes.end(); ++it)
		wcout << *it << " ";
	wcout << endl << "   Target incompatible nodes:" << endl << "    ";
	for (bigNumSet::const_iterator it = targetNodes.begin(); it != targetNodes.end(); ++it)
		wcout << *it << " ";
	wcout << endl;
#endif
	pair<hypothesesMapElementIndex::iterator, hypothesesMapElementIndex::iterator> range;
	for (bigNumSet::const_iterator it = sourceNodes.begin(); it != sourceNodes.end(); ++it) {
#ifdef __debug8__
		wcout << "  block4srcNode " << *it << " ";
#endif
		eliminateLinksForIndex(*it, linksRowIndex);
	}
	for (bigNumSet::const_iterator it = targetNodes.begin(); it != targetNodes.end(); ++it) {
#ifdef __debug8__
		wcout << "  block4trgNode " << *it << " ";
#endif
		eliminateLinksForIndex(*it, linksColumnIndex);
	}
}

bool MultiTreePair::areCrossing(const linkAddressPair& first, const linkAddressPair& second) {
	//Add the extra MultiTree condition for incompatibility to the standard crossing links condition
	return (areIncompatible(first, second) || TreePair::areCrossing(first, second, source, target));
}

bool MultiTreePair::areIncompatible(const linkAddressPair& first, const linkAddressPair& second) {
	return (source.areIncompatible(first.first, second.first) ||
					target.areIncompatible(first.second, second.second));
}

void MultiTreePair::collectTags() {
	//Call the superclass method with the MultiTrees from the current MultiTreePair object as parameters
	TreePair::collectTags(source, target);
}

void MultiTreePair::calculateInternalLinksStatistics(bool initial) const {
	//Call the superclass method with the MultiTrees from the current MultiTreePair object as parameters
	TreePair::calculateInternalLinksStatistics(initial, source, target);
}

void MultiTreePair::calculateStatistics() const {
	//Call the superclass method with the MultiTrees from the current MultiTreePair object as parameters
	TreePair::calculateStatistics(source, target);
}

#ifdef LATTICE
void MultiTreePair::initLatticeData() {
	linkStorage = linkStorageVector();
	for (hypothesesMap::const_iterator link = inducedLinks.begin(); link != inducedLinks.end(); ++link)
		linkStorage.push_back(link->first);
	
	crossingLinks = crossingLinksStorageMap();
	for (linkIDType currentID = 0; currentID < linkStorage.size(); ++currentID) {
		crossingLinksStorageMap::iterator currentIDIterator = crossingLinks.insert(make_pair(currentID, setOfLinks())).first;
		for (linkIDType comparisonID = 0; comparisonID < linkStorage.size(); ++comparisonID)
			if (currentID == comparisonID || areCrossing(linkStorage[currentID], linkStorage[comparisonID]) || areIncompatible(linkStorage[currentID], linkStorage[comparisonID]))
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
#endif //LATTICE