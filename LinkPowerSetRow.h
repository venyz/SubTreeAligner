/*
 *  LinkPowerSetRow.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 25.12.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifndef __LINKPOWERSETROW
#define __LINKPOWERSETROW

#include "LinkSet.h"

struct LinkPowerSetRow : public vector<LinkSet*> {
	~LinkPowerSetRow();
	
	inline const int& getFirstBase() const { return (*this)[0]->Base(); };
	bool getNextBase(int& base, size_type& firstID, size_type start) const ;
	size_type lastIDforBase(int base, size_type start) const ;
};

#endif //__LINKPOWERSETROW