// Author: Tim Diels <timdiels.m@gmail.com>

#include "commands.h"
#include <ncurses.h> // best documentation is: http://www.tldp.org/LDP/lpg/node85.html

using namespace std;

namespace DEEP_BLUE_GENOME {
namespace DATABASE {

class NCurses {
public:
	NCurses() {
		initscr();;
	}

	~NCurses() {
		endwin();
		//refresh();
	}
};

}} // end namespace
