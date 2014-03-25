/*
 *  Terminal.cpp
 *  TreeTester
 *
 *  Created by Венцислав Жечев on 17.11.06.
 *  Copyright 2006 Венцислав Жечев.
 *  Released under the GPL. See COPYING for information.
 *
 */

#include "Terminal.h"

#ifdef __debug__
namespace bg_zhechev_ventsislav {
	extern void Indent();
}
#endif

Terminal::Terminal(wstring label, int position = 0) : Node(label, position) {
#ifdef __debug__
	Indent();
	wcout << "creating terminal with label " << label << " and position " << position << endl;
#endif
}

Terminal::Terminal(const Terminal& source) : Node(source.label, source.position) {
#ifdef __debug__
	Indent();
	wcout << "copying pointer " << &source << " to pointer " << this << " with label " << this->label << endl;
#endif
}

Terminal::Terminal(const Node& source) : Node(source.Label(), source.Position()) {
#ifdef __debug__
	Indent();
	wcout << "copying pointer " << &source << " to pointer " << this << " with label " << this->label << endl;
#endif
}
