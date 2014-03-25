/*
 *  Node.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 05.12.06.
 *  Copyright 2006 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 *	A general node class that holds the common methods and fields for both Terminal and NonTerminal nodes.
 *
 */

#ifndef __NODE
#define __NODE

namespace bg_zhechev_ventsislav {
	class Node {
	protected:
		wstring label;		//the label of the node
		int position;			//the position of the node in the string, i.e. the position of the first terminal it covers
	public:
		Node() : label(L""), position(0) {}
		virtual ~Node() {}
		Node(wstring lb, int pos = 0) : label(lb), position(pos) {}
		//having a virtual function allows for the use of dynamic_cast
		inline virtual const bool operator==(const Node& node) const { return (this->position == node.position && this->label == node.label); }
		inline virtual const addressType Address() const { return make_pair(position, 0); }
		
		inline const wstring& Label() const { return label; }
		inline void setLabel(const wstring& label) { this->label = label; }
		inline const int& Position() const { return position; }
	};
}

#endif