/*
 *  NonTerminal.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 17.11.06.
 *  Copyright 2006 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifndef __NONTERMINAL
#define __NONTERMINAL
#include "Node.h"
#include "Index.h"

namespace bg_zhechev_ventsislav {
	class NonTerminal : public Node {
		
		class const_iterator_base : public std::iterator<forward_iterator_tag, const addressType> {
		protected:
			const NonTerminal* node;
			int curPos;
			size_t splitPos;
			addressType curAddress;
			
		public:
			virtual ~const_iterator_base() {}
			
			inline bool operator==(const const_iterator_base& iter) const { return node == iter.node && curPos == iter.curPos && splitPos == iter.splitPos; }
			inline bool operator!=(const const_iterator_base& iter) const { return !(*this == iter); }
			
			const_iterator_base() : node(NULL), splitPos(0) {}
		protected:
			const_iterator_base(const const_iterator_base& iter) : node(iter.node), curPos(iter.curPos), splitPos(iter.splitPos) {}
			const_iterator_base(const NonTerminal* nd, const int& cP, const size_t& sP) : node(nd), curPos(cP), splitPos(sP) { curAddress = make_pair(curPos, splitPos != node->splits.size() ? node->splits.at(splitPos + 1) : 0); }
			
		public:
			inline const addressType& operator*() { return curAddress; }
			inline const addressType* operator->() { return &curAddress; }
		};
		
	public:
		class const_child_iterator : public const_iterator_base {
		public:
			const_child_iterator(const NonTerminal* node, int curPos, size_t splitPos = 0) : const_iterator_base(node, curPos, splitPos) {}
			
			const_child_iterator& operator++();
			const_child_iterator operator++(int);
		};
		
		inline NonTerminal::const_child_iterator begin() const { return splits.size() > 2 ? NonTerminal::const_child_iterator(this, position, 0) : end(); }
		inline NonTerminal::const_child_iterator end() const { return NonTerminal::const_child_iterator(this, 0, splits.size()); }
		
	public:
		static Index idBase;							//index generator for getting unique indices for NonTerminals
		static Index extraIDBase;
		static bool useExtraIDBase;
	private:
		static Index binBase;							//index generator for getting unique binarisation indeces (for the nodes added upon binarisarion)
		static binContextMap binContexts;	//a map holding all binarisation contexts that were encountered
		
		bigNumber binID;									//the binarisation index of the NonTerminal; 0 if the node wasn't added by binarisarion
		bigNumber subtrees;								//the number of subtrees the NonTerminal has
		bigNumber id;											//the index of the NonTerminal
		intVector splits;									//a vector encoding the positions of the NonTerminal's daughter nodes
		int span;													//the span of the NonTerminal in the sentence
//	protected:
		NonTerminal* mother;							//the mother node of the NonTerminal or NULL for the root
	public:
		NonTerminal();
		NonTerminal(wstring label, int position, NonTerminal* mother = NULL, int span = 0, bigNumber binIndex = 0, const intVector& splits = intVector(), bigNumber subtrees = 0);
		NonTerminal(bigNumber id, wstring label, int position, NonTerminal* mother = NULL, int span = 0, bigNumber binIndex = 0);
		NonTerminal(const NonTerminal& source);
		
		static const bigNumber binIndex(wstring context);	//a method that gives a proper binarisation index depending on the context
		inline const bigNumber binIndex() const { return binID; };	//returns the binarisation index of the NonTerminal
#ifdef __debug2__
		static void printBinMap();					//prints the list of binarisation indeces (for debugging purpouses)
#endif
		
		inline int Span() const { return span; };
		inline void setSpan(const int span = 1) { this->span = span; };
		inline void incSpan(const int increment = 1) { this->span += increment; };
		inline bigNumber ID() const { return id; };
		inline const addressType Address() const { return make_pair(position, span); };
		inline void setMother(NonTerminal* mother) { this->mother = mother; };
		inline NonTerminal* getMother() const { return mother; };
		
		void addSplit(const int span);
		inline const intVector& Splits() const { return splits; };
		void resetSplits();
		
		inline void setSubtrees(bigNumber subtrees) { this->subtrees = subtrees; };
		inline bigNumber Subtrees() const { return subtrees; };
	};
}

#endif