#include "globals.h"

using namespace std;

int main(int argc, char *argv[]){
	report_warning = [](std::string_view w){
		cerr << w << flush;
	};
	init_epoch();


}
