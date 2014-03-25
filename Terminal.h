/*
 *  Terminal.h
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 17.11.06.
 *  Copyright 2006 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#ifndef __TERMINAL
#define __TERMINAL
#include "Node.h"

namespace bg_zhechev_ventsislav {
	class Terminal : public Node {
	public:
		Terminal(wstring label, int position);
		Terminal(const Terminal& source);
		Terminal(const Node& source);
	};
}

#endif