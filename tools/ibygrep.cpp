/* 
 * ibygrep.cpp -- 
 *    Search tool for PHI/TLG  files
 *
 * Copyright (c) 2000  Sean Redmond
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Revision: 1.4 $
 * $Date: 2001/10/11 22:08:36 $
 * $Author: seanredmond $
 */

#include <time.h>
#include <iostream>
#include <string>
#include <map>
using namespace std;


#include "IbycusAuthTab.h"
#include "IbycusTxtFile.h"
#include "GetOpts.h"

typedef map<ibystring_t, ibystring_t, less<ibystring_t> > HITMAP;

void match(const ibystring_t & pat, IbycusTxtFile & txt, 
	bool & seen, int fmt, IbycusId & last_id);

void search_file(const ibystring_t & pat, const ibystring_t & fn, const ibystring_t & dir, IbycusId & last_id);

void show_help();

int main(int argc, char *argv[])
{

	// Print out copyright notice
	cout <<	"Ibygrep version 1.0, Copyright (C) 2000 Sean Redmond" << "\n" << 
	"Ibygrep is free software. It may be used and copied under" << "\n" << 
	"the terms of the GNU General Public License: " << "\n" << 
	"http://www.gnu.org/copyleft/gpl.html. Ibygrep comes with " << "\n" << 
	"ABSOLUTE NO WARRANTY. Source code is available from " << "\n" << 
	"http://libycus.sourceforge.net" << "\n\n";
	

	// Get command line options
	GetOpts opts(argc, argv);
	ibystring_t pat = opts.GetString("t");
	ibystring_t dir = opts.GetString("d");
	ibystring_t corpus = opts.GetString("C");
	ibystring_t auth = opts.GetString("a");
	ibystring_t authid = opts.GetString("A");


	// Open up AUTHTAB.DIR, catch exceptions
	IbycusAuthTab at;
	try {
		at = IbycusAuthTab(dir);
	} catch(IbycusException E) {
		cout << E.what() << "\n";
		exit(1);
	} catch (runtime_error E) {
		cout << E.what() << "\n";
	}

	HITMAP hitlist; // Stores hits from author name search
	IbycusId last_id; // Stores ID of last hit

	time_t start_time, stop_time;
	time(&start_time); // start the timer

	if (!auth.empty())  { // Looking for a name?

		// Convert to lower case
		transform(auth.begin(), auth.end(), auth.begin(), tolower);
		ibystring_t name;

		int i, j, k;
		for (i = 0; i < at.Count(); i++) { // iterate through corpora
			for(j = 0; j < at.Count(i); j++) { // iterate through authors
				name = at.Name(i, j);
				// Convert to lower case
				transform(name.begin(), name.end(), name.begin(), tolower);

				if (name.find(auth) != name.npos) { // Search name for auth
					hitlist[at.Name(i, j)] = at.Id(i, j); // add to hitlist
				} else { // Search the aliases if no match yet
					for (k = 0; k < at.AliasCount(i,j); k++) {
						name = at.Alias(i,j,k);
						transform(name.begin(), name.end(), name.begin(), tolower);
						if (name.find(auth) != name.npos) {
							hitlist[at.Name(i, j)] = at.Id(i, j);
						}
					}
				}
			}
		}

		// print out results of author search
		cout << hitlist.size() << " authors:\n";
		HITMAP::iterator it;
		for (it = hitlist.begin(); it != hitlist.end(); ++it) {
			cout << (*it).second << '\t' << (*it).first << '\n';

			// search matching authors if a search term has been supplied
			if (!pat.empty()) { 
				search_file(pat, (*it).second, dir, last_id);
			}
		}

	} else if (!authid.empty()) { // Search only one file
		// Have to have something to seach for
		if (pat.empty()) {
			cout << "You must give a string for which to search!\n";
		} else {
			search_file(pat, authid, dir, last_id);
		}
	} else { // Search all files
		int st_c, end_c;

		// Limit to corpus specified by -C option
		if (!corpus.empty()) {
			try {
				st_c = at.Index(corpus);
			} catch(IbycusException E) {
				cout << "No corpus " << corpus << "! Choices are:\n";
				int i;
				for(i = 0; i < at.Count(); i++)
					cout << at.Tag(i) << " ";
				cout << "\n";
				exit(1);
			}
			end_c = st_c + 1;
		} else {
			st_c = 0;
			end_c = at.Count();
		}

		int crp, aut;
		for (crp = st_c; crp < end_c; crp++) {         // iterate through the corpora
			for(aut = 0; aut < at.Count(crp); aut++) { // iterate through the authors
				search_file(pat, at.Id(crp, aut), dir, last_id);
			}
		}
	}

	// Stop the timer and print out search time if there was a search term
	time(&stop_time);
	if (!pat.empty())
		cout << "\n\nSearch time: " << stop_time - start_time << " seconds.\n";
	return 1;

}

// Open file fn and search it for string pat
void search_file(const ibystring_t & pat, const ibystring_t & fn, const ibystring_t & dir, IbycusId & last_id) {
	try {
		IbycusTxtFile txt(fn, dir);

		bool seen = false;

		txt.Top();
		while (!txt.eoa()) {
			match(pat, txt, seen, IbycusId::fmt_auth, last_id);
			txt.Next();
		}
	} catch (IbycusException E) {
		cout << E.what() << "\n";
		exit(1);
	} catch (exception E) {
		cout << "Unhandled standard exception\n";
		cout << E.what() << "\n";
		exit(1);
	} catch (...) {
		cout << "Totally unhandled exception\n";
		cout << "\n";
		exit(1);
	}

}

// Search for a match
// Recursively restores hyphenated words at the end of a line
void match(const ibystring_t & pat, IbycusTxtFile & txt, 
	bool & seen, int fmt, IbycusId & last_id)
{
	ibystring_t tmp = txt.StripCodes();
	unsigned int hyphen;
	if ((hyphen = tmp.rfind("-")) != tmp.npos && (tmp.length() - hyphen) <= 3) {
		IbycusId this_id = txt.Id();
		ibystring_t this_text = txt.Text();

		ibystring_t tmp2;
		if (txt.Next()) {
			tmp2 = txt.StripCodes();
			int space_pos = tmp2.find(" ");
			tmp.replace(hyphen, 1, tmp2.substr(0, space_pos));
			tmp2 = tmp2.substr(space_pos+1);
		}


		if (tmp.find(pat) != tmp.npos) {
			if (!seen) {
				cout << "\n" << txt.Name() << " (" << txt.Id() << ")\n";
				cout << "-----------------------" << "\n";
				seen = true;
			}

			cout << this_id.ToString(fmt, &last_id) << ": " << this_text << "\n";
			last_id = this_id;
		}
	
		if (!tmp2.empty()) {
			match(pat, txt, seen, fmt, last_id);
		}

	} else {
		if (tmp.find(pat) != tmp.npos) {
			if (!seen) {
				cout << "\n" << txt.Name() << "\n";
				cout << "-----------------------" << "\n";
				seen = true;
			}

			cout << txt.Id().ToString(fmt, &last_id) << ": " << txt.Text() << "\n";
			last_id = txt.Id();
		}
	}
}
