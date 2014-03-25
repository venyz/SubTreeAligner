/*
 *  DOTTreePair.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 08.12.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "DOTTreePair.h"

PCFGRulesMap DOTTreePair::externalPCFGRules = PCFGRulesMap();
wstring DOTTreePair::topCatString = L"#TOP#";
wstring DOTTreePair::eosCatString = L"#EOS#";
wstring DOTTreePair::eosWordString = L"#eos#";

bool DOTTreePair::error = false;

namespace bg_zhechev_ventsislav {
	wistream& operator>>(wistream& in, DOTTreePair& treePair) {
		TreePair::linked = true;
		Tree::binariseMode = true;
		Tree::pairedMode = true;
		TreePair temp;
		in >> temp;
//		wcout << "™™™™™™™ TreePair error is " << TreePair::error << " ™™™™™™™" << endl;
		if (TreePair::error) {
//			wcout << "™™™™™™™ Bad DOTTreePair!!! ™™™™™™™ " << endl;
			DOTTreePair::error = true;
			return in;
		}
#ifdef __debug1_ex_dot__
		temp.printBracketed(wcerr);
#endif
		treePair = temp;
		
		return in;
	}
} //namespace

void DOTTreePair::reset() {
	TreePair::reset();
	error = false;
}

DOTTreePair& DOTTreePair::operator=(const TreePair& sourcePair) {
	reset();
	const linksMap& origLinks = sourcePair.Links();
	linksMap srcLinks = sourcePair.Links();
	source.convert(sourcePair.Source(), sourcePair.Target(), srcLinks);
	linksMap(trgLinks);
	for (linksMap::const_iterator link = origLinks.begin(); link != origLinks.end(); ++link)
		trgLinks[link->second] = link->first;
	target.convert(sourcePair.Target(), sourcePair.Source(), trgLinks, true);
	
	for (linksMap::const_iterator link = origLinks.begin(); link != origLinks.end(); ++link) {
#ifdef __debug1_ex_dot__
		wcout << "  ::saving link " << srcLinks[link->second] << "/" << trgLinks[link->first] << " from " << link->first << "/" << link->second << endl;
#endif
		links[srcLinks[link->second]] = trgLinks[link->first];
	}
	
	linkAddressPair srcLink = source.addExtraNodes(topCatString, eosCatString, eosWordString, true);
	linkAddressPair trgLink = target.addExtraNodes(topCatString, eosCatString, eosWordString, true);
	links[srcLink.first] = trgLink.first;
	links[srcLink.second] = trgLink.second;
	DOPTree::setLinks(&links);
	
	return *this;
}

void DOTTreePair::printXML(wostream& out) const {
	TreePair::printBracketed(out, source, target, true);
}

void DOTTreePair::printBracketed(wostream& out, bool is_XML) const {
	TreePair::printBracketed(out, source, target, is_XML);
}

void DOTTreePair::printSentences(wostream& out) const {
	TreePair::printSentences(out, source, target);
}

void DOTTreePair::collectPCFGRules(wostream& srcGrammar, wostream& trgGrammar, wostream& alignments) {
	target.convertAndStoreExtRules(source.collectPCFGRules(srcGrammar, false));
	target.collectPCFGRules(trgGrammar, true);
	
	for (linksMap::const_iterator link = links.begin(); link != links.end(); ++link)
		alignments << link->first << " " << link->second << endl;
}

void DOTTreePair::outputPCFGRules(wostream& srcGrammar, wostream& trgGrammar, wostream& externals) {
	DOPTree::outputPCFGRules(srcGrammar, false);
	DOPTree::outputPCFGRules(trgGrammar, true);
	
	for (PCFGRulesMap::const_iterator it = DOPTree::extRulesDepot.begin(); it != DOPTree::extRulesDepot.end(); ++it) {
#ifdef __debug1_dot__
		wcout << "___printing extern for " << it->first << " (" << it->second.second.size() << " elements)" << endl;
#endif
		externals << it->first;
		for (PCFGRulesMap::value_type::second_type::second_type::const_iterator rit = it->second.second.begin(); rit != it->second.second.end(); ++rit) {
			externals << "\t" << rit->first << "\t" << ((1.0*rit->second)/it->second.first);
#ifdef __debug1_dot__
			wcout << it->first << "\t" << rit->first << "\t(" << rit->second << "/" << it->second.first << ") " << ((1.0*rit->second)/it->second.first) << endl;
#endif
		}
		externals << endl;
	}
}