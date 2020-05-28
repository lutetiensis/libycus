// GetOpts.cpp: implementation of the GetOpts class.
//
//////////////////////////////////////////////////////////////////////

#include "GetOpts.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GetOpts::GetOpts(int argc, char* argv[])
{
	int opt_count = 1;
	TCSTRING temp, tag, val;

	if(argc < opt_count)
		return;

	while(opt_count < argc) {
		temp = argv[opt_count++];
		if (temp[0] == '-') {
			tag = temp.substr(1);

			if (opt_count == argc) {
				printf("No value for command line argument %s\n", temp);
				exit(1);
			}

			val = argv[opt_count++];
			if (val[0] == '-') {
				printf("No value for command line argument %s\n", temp);
				exit(1);
			}

			opts[tag] = val;

		} else {
			printf("Cannot parse command line argument %s\n", temp);
			exit(1);
		}

	}

}

GetOpts::~GetOpts()
{

}

TCSTRING GetOpts::GetString(TCSTRING tag)
{
	return opts[tag];
}
