/*
 *  LinkPowerSetRow.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 25.12.07.
 *  Copyright 2007 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "LinkPowerSetRow.h"

LinkPowerSetRow::~LinkPowerSetRow() {
	for (vector<LinkSet*>::iterator it = this->begin(); it != this->end(); delete(*(it++)));
}

bool LinkPowerSetRow::getNextBase(int& base, LinkPowerSetRow::size_type& firstID, LinkPowerSetRow::size_type start) const {
	for (LinkPowerSetRow::size_type id = start; id < this->size(); ++id)
		if ((*this)[id]->Base() != base) {
			base = (*this)[id]->Base();
			firstID = id;
			return true;
		}
	return false;
}

LinkPowerSetRow::size_type LinkPowerSetRow::lastIDforBase(int base, LinkPowerSetRow::size_type start) const {
	for (LinkPowerSetRow::size_type id = start; id < this->size(); ++id)
		if ((*this)[id]->Base() != base)
			return id - 1;
	return size() - 1;
}