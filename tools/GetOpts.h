// GetOpts.h: interface for the GetOpts class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GETOPS_H
#define _GETOPS_H

#if _MSC_VER >= 1000
	#pragma once
	#pragma warning( disable : 4786 )
#endif // _MSC_VER >= 1000


#include <map>
#include <string>
using namespace std;

typedef basic_string<char> TCSTRING;
typedef map<TCSTRING, TCSTRING> OPTIONS;

class GetOpts  
{
public:
	TCSTRING GetString(TCSTRING tag);
	GetOpts(int argc, char* argv[]);
	virtual ~GetOpts();

private:
	OPTIONS opts;
};

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
				printf("No value for command line argument %s\n", temp.c_str());
				exit(1);
			}

			val = argv[opt_count++];
			if (val[0] == '-') {
				printf("No value for command line argument %s\n", temp.c_str());
				exit(1);
			}

			opts[tag] = val;

		} else {
			printf("Cannot parse command line argument %s\n", temp.c_str());
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

#endif	/* Not _GETOPS_H */
