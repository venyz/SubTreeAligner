/*
 *  Index.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 20.11.06.
 *  Copyright 2006 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 *	Used to create unique indices.
 *	By default the first index value is 1.
 *
 */

#ifndef __INDEX
#define __INDEX

namespace bg_zhechev_ventsislav {
	class Index {
		bigNumber index;
	public:
		Index(bigNumber init = 1) : index(init) {}	//optionally the starting index can be specified as a parameter to the constructor
		inline bigNumber current() const { return index; }			//gets the current index, without increasing
		inline bigNumber operator++() { return ++index; }				//increases the index with 1
		inline bigNumber operator++(int) { return index++; }		//increases the index with 1
		inline void reset(bigNumber init = 1) { index = init; }	//restarts the indexing from 1
	};
}

#endif