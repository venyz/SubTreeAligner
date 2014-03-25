/*
 *  Tree.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 17.11.06.
 *  Copyright 2006 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 *	A class describing a parse tree.
 *
 */

#ifndef __TREE
#define __TREE
#include "NonTerminal.h"
#include "Terminal.h"
#include "Index.h"

namespace bg_zhechev_ventsislav {
	class Tree {
		
		//Iterator definitions
		class const_iterator_base : public std::iterator<forward_iterator_tag, const Node> {
		protected:
			const Tree* tree;
			const Node* node;
			
		public:
			virtual ~const_iterator_base() {}
			
			inline bool operator==(const const_iterator_base& iter) const { return tree == iter.tree && ((node == NULL && iter.node == NULL) || ((node != NULL && iter.node != NULL) && (node->Position() == iter.node->Position()))); }
			inline bool operator!=(const const_iterator_base& iter) const { return !(*this == iter); }
			
			const_iterator_base() : tree(NULL), node(NULL) {}
		protected:
			const_iterator_base(const const_iterator_base& iter) : tree(iter.tree), node(iter.node) {}
			const_iterator_base(const Tree* tr, const Node* nd) : tree(tr), node(nd) {}
		};
		
		class const_nterm_iterator_base : public const_iterator_base {
		public:
			~const_nterm_iterator_base();
			
			const_nterm_iterator_base(const const_nterm_iterator_base& iter) : const_iterator_base(iter) {}
			const_nterm_iterator_base(const Tree* tree, const Node* node) : const_iterator_base(tree, node) {}
			
			inline const NonTerminal& operator*() { return *(dynamic_cast<const NonTerminal*>(this->node)); }
			inline const NonTerminal* operator->() { return dynamic_cast<const NonTerminal*>(this->node); }
		};
		
	public:
		class const_word_iterator : public const_iterator_base {
		public:
			~const_word_iterator();
			
			const_word_iterator(const const_word_iterator& iter) : const_iterator_base(iter) {}
			const_word_iterator(const Tree* tree, const Terminal* node) : const_iterator_base(tree, node) {}
			
			const_word_iterator& operator++();
			const_word_iterator operator++(int);
			
			inline const Terminal& operator*() { return *(dynamic_cast<const Terminal*>(this->node)); }
			inline const Terminal* operator->() { return dynamic_cast<const Terminal*>(this->node); }
		};
		
		class const_preterm_iterator : public const_nterm_iterator_base {
		public:
			const_preterm_iterator(const const_preterm_iterator& iter) : const_nterm_iterator_base(iter) {}
			const_preterm_iterator(const Tree* tree, const NonTerminal* node) : const_nterm_iterator_base(tree, node) {}
			
			const_preterm_iterator& operator++();
			const_preterm_iterator operator++(int);
		};
		
		class const_nterm_iterator : public const_nterm_iterator_base {
		protected:
			nodeIndexMap::const_iterator it;
			
		public:
			const_nterm_iterator(const const_nterm_iterator& iter) : const_nterm_iterator_base(iter), it(iter.it) {}
			const_nterm_iterator(const Tree* tr, nodeIndexMap::const_iterator iter);
			const_nterm_iterator(const Tree* tree, const NonTerminal* node, nodeIndexMap::const_iterator iter) : const_nterm_iterator_base(tree, node), it(iter) {}
			
			const_nterm_iterator& operator++();
			const_nterm_iterator operator++(int);
		};
		
		inline Tree::const_word_iterator wbegin() const { return Tree::const_word_iterator(this, dynamic_cast<Terminal*>(chart.find(make_pair(1, 0))->second)); }
		inline Tree::const_word_iterator wend() const { return Tree::const_word_iterator(this, NULL); }
		inline Tree::const_preterm_iterator ptbegin() const { return Tree::const_preterm_iterator(this, dynamic_cast<NonTerminal*>(chart.find(make_pair(1, 1))->second)); }
		inline Tree::const_preterm_iterator ptend() const { return Tree::const_preterm_iterator(this, NULL); }
		inline Tree::const_nterm_iterator ntbegin() const { return Tree::const_nterm_iterator(this, nodeIndex.begin()); }
		inline Tree::const_nterm_iterator ntend() const { return Tree::const_nterm_iterator(this, NULL, nodeIndex.end()); }
		//End of the iterator definitions
		
		static bool pairedMode;
		static bool binariseMode;
	protected:
		static const linksMap* links;			//the tree alignments in case of paired mode operation
	public:
		inline static void setLinks(const linksMap* links) { Tree::links = links; }
	protected:
		
		Index index;						//an index generator used in the parsing of bracketed representations of trees
		int length;							//the length of the sentence (number of tokens)
		mainChartType chart;		//the main chart holding the parse tree
		nodeIndexMap nodeIndex;	//node index by ID
		
		void indexNodes();			//index the nodes by ID
		
		int parse(wstring input, NonTerminal* mother);	//method for parsing bracketed representations of trees; returns the length of the sentence
		void binarise(NonTerminal* mother);							//method used for binarising the nodes of the tree; should only be used during the initial parsing of the bracketed representation
		wstring binContext(NonTerminal* mother);				//generates the binarisation context of a node
		
		const bool getDaughter(wstring& input, wstring& output);	//cuts a part of the input string that looks like a daughter subtree and puts it in the output string. the input string is replaced by what is left after taking away the output string. this method is only used in the process of parsing bracketed representations of trees
		
		int depthHelper(int pos, int span) const;								//a recursive helper method used for determining the depth of the tree
		static bool checkInputIntegrity(const wstring& input);	//a very simple method that contains a basic check on whether a string could contain a bracketed representation of a tree
		
		void init();
		
		map<bigNumber, bigNumVector*> preTermSpanMap;
		map<bigNumber, bigNumVector*> outsidePreTermSpanMap;
		map<bigNumber, bigNumVector> descendantMap, nonDescendantMap, ancestorMap, nonAncestorMap;
		void descendantsHelper(const addressType& address, bigNumVector& vector) const ;
	public:
//		const bigNumVector& preTermSpan(const NonTerminal& node);
//		const bigNumVector& outsidePreTermSpan(const NonTerminal& node);
		virtual const bigNumVector& descendants(bigNumber ID);
		inline virtual const bigNumVector& nonDescendants(bigNumber ID) { return nonDescendants(ID, *this); }
	protected:
		const bigNumVector& nonDescendants(bigNumber ID, Tree& tree);
	public:
		virtual const bigNumVector& ancestors(bigNumber ID);
		inline virtual const bigNumVector& nonAncestors(bigNumber ID) { return nonAncestors(ID, *this); }
	protected:
		const bigNumVector& nonAncestors(bigNumber ID, Tree& tree);
	public:
		void otherNodes(const bigNumVector& source, bigNumVector& result);
		inline virtual bool dominates(bigNumber source, bigNumber target) { return dominates(source, target, *this); }
	protected:
		bool dominates(bigNumber source, bigNumber target, Tree& tree);
	public:
		
		//	void removeExtraNodes();

		Tree();
		Tree(wchar_t input[]);
		Tree(wstring input);
		Tree(const Tree& source);
		Tree& operator=(const Tree& source);
		friend wostream& operator<<(wostream& out, const Tree& tree);
		friend wistream& operator>>(wistream& in, Tree& tree);
		virtual ~Tree();
		
		void reset();																						//a method used for resetting the tree object
		
		const NonTerminal* findNode(bigNumber ID) const ;					//a method for finding nodes in a tree by their indeces
		const Node* findNode(const addressType& address) const ;	//a method for finding nodes in a tree by their addresses
		inline const Node* wordAtPosition(int position) const { return position > length ? NULL : chart.find(make_pair(position, 0))->second; }
		
		const wstring sentence() const ;			//returns a string representing the actual sentence
		inline int depth() const { return depthHelper(1, length); }										//returns the depths of the tree
		inline int sentLength() const { return length; }							//returns the length of the sentence
		inline mainChartType::size_type size() const { return chart.size(); }										//returns the size of the chart for the sentence
		bigNumber numberOfFragments() const ;	//returns the number of fragments that can be produced from the tree
		inline void printXML(wostream& out) const { printBracketed(out, true); }	//prints an XML representation of the tree
		inline void printBracketed(wostream& out, bool is_XML = false) const { printBracketedHelper(out, make_pair(1, length), is_XML); out << endl; }					//prints a bracketed representation of the tree
		void printTaggedSentence(wostream& out) const ;
	private:
		bool printBracketedHelper(wostream& out, addressType address, bool is_XML = false) const ;
		
		const wstring subStringsHelper(subStringsVector* strings, const addressType& address) const ;
	public:
		virtual subStringsVector* getSubStrings() const ;
		
		const NonTerminal* root() const { return dynamic_cast<const NonTerminal*>(chart.find(make_pair(1, length))->second); }
	};
} //namespace

#endif