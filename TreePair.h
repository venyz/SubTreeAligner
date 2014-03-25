/*
 *  TreePair.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 20.01.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 *	A class describing a pair of aligned trees and the internal alignments that may exist between them
 *
 */

#ifndef __TREEPAIR
#define __TREEPAIR

#include "Tree.h"
#include "LinkHypothesis.h"
#ifdef SEP_PROB_TYPE
#include "ProbabilityType.h"
#endif
/*#ifdef LATTICE
#include "LinkSet.h"
#include "LinkPowerSetRow.h"
#endif*/

namespace bg_zhechev_ventsislav {
	class TreePair {
#ifdef TRAINING
		static const bool NODE_LABELS, LEXICAL, PRETERM;
#endif //TRAINING
	public:
		static int SENTENCE_COUNT;
		
	protected:
#ifndef TRAINING
		subStringsVector *sourceStrings, *targetStrings;
//		lexicalProbabilities sourceLexProbs, targetLexProbs;
		doubleType **newSourceLexProbs, **newTargetLexProbs;
		hypothesesMap inducedLinks;
		hypothesesMapProbabilityIndex linksProbabilityIndex, one2oneLinksProbabilityIndex;
		hypothesesMapElementIndex linksRowIndex, linksColumnIndex;
#endif //TRAINING
		
		Tree source, target;	//the source and target language trees
		//	bigNumMap links;			//the alignments between nodes of the two trees
		linksMap links;
		
#ifdef CHECK_MAN_LINKS
		linksMap manLinks;
		void fixManualLinks();
#endif
		
		void readInLinks(const wstring& input);	//used to read in the links from a string in the form of pairs of indeces
		
#ifdef TRAINING
		static linkContextMap contexts;
		
		dynAlignmentMap alignmentChart;
		cellMap getTargetIDs(mainChartType::key_type nodeIndex) const ;
		void initAlignChart();
		
	public:
		void printAlignChart() const ;
#endif //TRAINING
		
	public:
		TreePair();
		
		static bool error;	//a flag to indicate the status of some of the operations on the TreePair
		static bool linked;	//a flag that indicates whether the trees already have internal alignments or not
		
		friend wostream& operator<<(wostream& out, const TreePair& treePair);
		friend wistream& operator>>(wistream& in, TreePair& treePair);
		
		void reset();
		virtual ~TreePair();
		
#ifdef TRAINING
	private:
		//a pair of helper methods to the following one; the first of them is used on source trees and the second on target trees using the results from the execution of the first method
		pair<wstring, const bigNumVector> nodeContext(bigNumber nodeID, bool first, int depth) const ;
		wstring nodeContext(bigNumber nodeID, const bigNumVector& frontier, bool first, int depth) const ;
		pair<wstring, wstring> nodeContext(linksMap::const_iterator link) const ; //returns the pair of contexts in which an alignment occurs; used during training
	public:
		void saveContexts() const ;
		void storeUnlinkedCounts() const ;
		
	private:
		map<bigNumber, int> contextFound(pair<map<wstring, int>, pair<bigNumVector, bigNumVector> > context) const ;
		
		bigNumVector frontierHelper(const addressType& nodeAddress, const cellKey& dtrChoice, int depth) const ;
		daughterContextMap getContexts(const addressType& nodeAddress) const ;
		
		void linkNode(const addressType& nodeAddress);
		
		void processCombinations(const addressType& nodeAddress, const vector<daughterContextMap>& dtrContexts);
		void processCombinations(const addressType& nodeAddress, const vector<daughterContextMap>& dtrContexts, vector<daughterContextMap>::size_type index, vector<cellKey>& combination);
		void processACombination(const addressType& nodeAddress, const vector<daughterContextMap>& dtrContexts, const vector<cellKey>& combination);
		static void limitCombinations(cellMap* cell);
		
		const bigNumVector nodeContext(const addressType& nodeAddress, const cellKey& key, bool) const ;
		pair<wstring, pair<const bigNumVector, const bigNumVector> > nodeContext(const addressType& nodeAddress, const cellKey& key, int depth = 1) const ;
		
		void writeLinks();
		void writeLinks(addressType nodeAddress, cellKey& bestKey);
	public:
		void linkTrees();	//a method that creates alignments between the source and target trees
#endif //TRAINING
		
		const linksMap& Links() const ;
		const Tree& Source() const ;
		const Tree& Target() const ;
		
#ifndef TRAINING
	protected:
		static extAlignmentMap sourceWordAlignments, targetWordAlignments, sourcePhraseAlignments, targetPhraseAlignments;
		static vcbMap sourceVocabulary, targetVocabulary;
		
		static void readWordAlignments(const string& fileName, extAlignmentMap& alignments);
		static void readVocabulary(const char* fileName, vcbMap& vocabulary);
	public:
		static void readSourceWordAlignments(const string& fileName);
		static void readTargetWordAlignments(const string& fileName);
		static void readPhraseAlignments(const string& fileName);
		static void readSourceVocabulary(const char* fileName);
		static void readTargetVocabulary(const char* fileName);
		
	protected:
		virtual void createSubStringLists();
		void createSubStringLists(Tree& source, Tree& target);
		
		void initLexicalInfo(const Tree& source, const Tree& target, const extAlignmentMap& alignments,
//												 lexicalProbabilities& probs,
												 doubleType**& newProbs);
		virtual void initLexicalInfo();
#ifdef __debug7__
		void printLexMap(const Tree& source,
//										 const lexicalProbabilities& probs,
										 doubleType** const newProbs, const size_t& srcSentLen, const size_t& trgSentLen) const;
		virtual void printLexMaps() const;
#endif
		
		virtual void prepareLexicalLinks();
		void prepareLexicalLinks(Tree& source, Tree& target);
		void scoreLink(Tree& source, Tree& target, const NonTerminal* sourceNode, const NonTerminal* targetNode);
		virtual void printInducedLinks(bool printStatus = false, bool trees = false) const ;
		void printInducedLinks(bool printStatus, bool trees, const Tree& source, const Tree& target) const ;
		
		ProbabilityType insideProbability(Tree& source, Tree& target,
//																			const lexicalProbabilities& probs,
																			doubleType** const newProbs, const NonTerminal& sourceNode, const NonTerminal& targetNode
#ifdef __debugX__
																			, const unsigned short direction
#endif
																			);
		ProbabilityType outsideProbability(Tree& source, Tree& target,
//																			 const lexicalProbabilities& probs,
																			 doubleType** const newProbs, const NonTerminal& sourceNode, const NonTerminal& targetNode
#ifdef __debugX__
																			 , const unsigned short direction
#endif
																			 );
		ProbabilityType translationProbability(
//																					 const lexicalProbabilities& probs,
																					 doubleType** const newProbs,
//																					 const bigNumVector& srcPreTermSpan, const bigNumVector& trgPreTermSpan,
																					 const NonTerminal& sourceNode, const NonTerminal& targetNode, const size_t& srcSentLen, const size_t& trgSentLen, bool outside
#ifdef __debugX__
																					 , const Tree& source, const Tree& target
#endif
																					 );
		
	protected:
		bool reconsiderFirstStage;
		void processInducedLinks();
		void printProposedLinks() const ;
	public:
#ifdef CHECK_MAN_LINKS1
		void freezeLinks();
#endif //CHECK_MAN_LINKS1
	protected:
		
#ifdef RESCORE
//		bigNumVector frontierHelper(const bigNumber ID) const ;
		virtual bool cleanLexProbabilites(const linkAddressPair& link);
		bool cleanLexProbabilites(Tree& source, Tree& target, const linkAddressPair& link);
		bool cleanLexProbabilites(lexicalProbabilities& probs, const bigNumVector& srcPreTermSpan, const bigNumVector& trgPreTermSpan);
		void rescoreLinks(const linkAddressPair& link);
		void rescoreLinks(const linksVector& links);
		virtual void rescoreLinks();
		void rescoreLinks(Tree& source, Tree& target);
#endif //RESCORE
		
		virtual void buildIndices();
		void buildIndices(const Tree& source, const Tree& target);
		bool changed;
#ifdef DELAY_CONSTITUENT
		void linkBest(hypothesesMapProbabilityIndex& linksProbabilityIndex, const skipStructure& skip = skipStructure());
		bool decideBest(linksIndex& notSkipped);
#else //DELAY_CONSTITUENT
		void linkBest(hypothesesMapProbabilityIndex& linksProbabilityIndex, const int skip = 0);
		bool decideBest(hypothesesMapProbabilityIndex& linksProbabilityIndex, int skip = 0);
#endif //DELAY_CONSTITUENT
		void link(hypothesesMap::iterator it);
		void eliminateLinksForIndex(bigNumber nodeID, const hypothesesMapElementIndex& index);
		virtual void eliminateCrossingForLink(const linkAddressPair& link);
		void eliminateCrossingForLink(const linkAddressPair& link, Tree& source, Tree& target);
		void blockLinks(const bigNumVector& source, const bigNumVector& target);
		void cleanDecided();
		map<pair<linkAddressPair, linkAddressPair>, bool> crossingLinksMap;
		bool areCrossing(const linksVector& links);
		inline virtual bool areCrossing(const linkAddressPair& first, const linkAddressPair& second) { return areCrossing(first, second, source, target); };
		bool areCrossing(const linkAddressPair& first, const linkAddressPair& second, Tree& source, Tree& target);
	public:
		void link();
#endif //TRAINING
		
	public:
		virtual void printStandard(wostream& out) const ;
	protected:
		void printStandard(wostream& out, const Tree& source, const Tree& target) const ;
	public:
		virtual void printXML(wostream& out) const ;
		virtual void printBracketed(wostream& out, bool is_XML = false) const ;
	protected:
		void printBracketed(wostream& out, const Tree& source, const Tree& target, bool is_XML = false) const ;
	public:
		virtual void printSentences(wostream& out) const ;
	protected:
		void printSentences(wostream& out, const Tree& source, const Tree& target) const ;
		
		//The following are used to gather and display statistics about the POS tags used
	protected:
		static tagMap sourceTags, targetTags, linkTags;
		void collectTagsHelper(const Tree& tree, tagMap& tags);
		static void printTagsHelper(wostream& out, const tagMap& tags);
	public:
		virtual void collectTags();
	protected:
		void collectTags(const Tree& source, const Tree& target);
	public:
		static void printTags(wostream& out);
		void printTaggedSentences(wostream& out) const ;
		//end block
		
		//The following are used to gather general statistics about the performance of the aligner
	public:
		static string collectExpensiveStatistics;
	protected:
		static double sourceAvgLinks, targetAvgLinks, totalAvgLinks, existingPhrasesAvg, existingPhrasesAvg2, phrasesAvg, internalLinksAvg, strictInternalLinksAvg, initialNonzeroLinksAvg, initialInternalLinksAvg, initialStrictInternalLinksAvg, realisedInternalLinksAvg, realisedStrictInternalLinksAvg, linksAvg, sourceNodesAvg, targetNodesAvg, sourceLengthAvg, targetLengthAvg, undecidedAvg;
		static unsigned existingPhrasesCount, allPhrasesCount, sentencesWithPhrases, treePairs, totalUndecided, initialInternalLinksCount, initialStrictInternalLinksCount, sentencesWithInternalLinks, sentencesWithStrictInternalLinks;
#ifdef LATTICE
		static unsigned totalSentencesWithFullSolutions, totalSentencesWithAmbiguousFullSolutions, totalSolutionsFound, totalUndecidedWithFullSolutions, totalUndecidedSolutionsFound, totalUndecidedAmbiguousSolutionsFound, totalAmbiguousSolutionsFound;
#endif
		static vector<bigNumVector> searchSpace;
		void saveSearchSpace() const ;
		virtual void calculateInternalLinksStatistics(bool initial) const ;
		void calculateInternalLinksStatistics(bool initial, const Tree& source, const Tree& target) const ;
	public:
		virtual void calculateStatistics() const ;
	protected:
		void calculateStatistics(const Tree& source, const Tree& target) const ;
	public:
		static void printStatistics(wostream& out);
		//end block
		
#ifdef EVALUATE
		static vector<pair<doubleType, doubleType> > evaluate(const TreePair& automatic, const TreePair& manual);
#endif //EVALUATE
	protected:
		static vector<pair<doubleType, doubleType> > evaluate(const TreePair& automatic, const linksMap& automaticLinks, const TreePair& manual, const linksMap& manualLinks);
		
#ifdef EXTRACT_CHUNKS
	public:
		static binContextMap chunks;
		void collectChunks();
		static void printChunks(wostream& out);
#endif //EXTRACT_CHUNKS

#if defined(EXTRACT_CHUNKS) && defined(SAMT_RULES)
	public:
		void printSAMTRules(wostream& out);
#endif

#ifdef LATTICE
	protected:
		/*	linkPowerSet powerSet;
		 vectorOfLinkSets maximalLinkSets;
		 
		 void initLinkPowerSet();
		 void buildNextLinkPowerSetRowHelper(linkPowerSet::size_type newRow, LinkPowerSetRow::size_type firstID, LinkPowerSetRow::size_type lastID);
		 void buildNextLinkPowerSetRow();
		 void buildLinkPowerSet();
		 void printPowerSet(wostream& out, int lastRow = -1) const ;
		 
		 static bool checkForSubSet(const setOfLinks& source, const LinkSet& targetSet);
		 bool checkForSubSet(const LinkSet& set, int setID, linkPowerSet::size_type row) const ;
		 void findMaximalLinkSets();
		 void printMaximalLinkSets(wostream& out) const ;*/
		
		linkStorageVector linkStorage;
		crossingLinksStorageMap crossingLinks;
		mapOfLinkSets maximalLinkSets;
		linkStorageVector bestMaxLinkSet;
#ifdef __debug2_lat__
#ifndef __debug2a_lat__
		bigNumber totalMaximalLinkSetsFound;
#endif
#endif
		
		virtual void initLatticeData();
#ifdef __debug1_lat__
		void printLinkIDs(wostream& out) const ;
		void printCrossings(wostream& out) const ;
#endif
		doubleType linkSetProbabilityMass(const setOfLinks& links) const ;
		void saveMaximalLinkSet(const setOfLinks& linkSet);
		void findMaximalLinkSetsHelper(const vectorOfLinks& workingVector, setOfLinks& resultSet, const linkIDType& currentID);
		void findMaximalLinkSets();
#ifdef __debug2_lat__
		void printMaximalLinkSets(wostream& out) const ;
		void selectAndPrintMaxLinkSet(wostream& out);
#endif
		
	public:
		pair<bool, bool> latticeChecksWithStandard() const ;
		vector<pair<doubleType, doubleType> > evaluateLattice() const ;
#endif //LATTICE
	};
}

#endif //__TREEPAIR