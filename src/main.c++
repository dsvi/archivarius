#include "globals.h"

using namespace std;

int main(int argc, char *argv[]){
	auto report_warning = [](std::string &&w){
		cerr << w << flush;
	};
	init_epoch();


}
