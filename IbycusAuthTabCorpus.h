/* 
 * IbycusAuthTabCorpus.h: ibyAuthTabCorpus class -- 
 *    Corpus of ibyAuthTabAuthor's in a TLG/PHI AUTHTAB.DIR
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
 * $Revision: 1.8 $
 * $Date: 2001/11/12 18:40:17 $
 * $Author: seanredmond $
 */

#ifndef	_IBYCUSAUTHTABCORPUS_H_
#define _IBYCUSAUTHTABCORPUS_H_

#include "IbycusAuthTabAuthor.h"
#include "IbycusParseException.h"

#define AT_DELIM_ALIAS		0x80
#define AT_DELIM_REMARKS	0x81
#define AT_DELIM_UNKNOWN	0x82
#define AT_DELIM_ALPHABET	0x83
#define AT_DELIM_AUTH		0xFF

__IBYCUS_BEGIN_NAMESPACE
class ibyAuthTabCorpus
{
public:
	ibyAuthTabCorpus() {};
	ibyAuthTabCorpus(ibyFile & src, ibylen_t len, ibystring_t tag) throw(IbycusParseException);
	virtual ~ibyAuthTabCorpus() {};
	const ibystring_t & Tag() const { return tag_; };
	const ibystring_t & Name() const { return name_; };
	const ibystring_t & Name(int author) const throw(IbycusException);
	const ibystring_t & Id(int author) const throw(IbycusException);
	int Count() const
		{ return authlist_.size(); };
	int AliasCount(int author) const throw(IbycusException);
	const ibystring_t & Alias(int author, int alias) const throw(IbycusException);
	bool operator<(const ibyAuthTabCorpus & rhs) const
		{ return tag_ < rhs.tag_; };
	bool operator==(const ibyAuthTabCorpus & rhs) const
		{ return tag_ == rhs.tag_; };



private:
	vector<ibyAuthTabAuthor> authlist_;
	ibychar_t   alpha_;
	ibystring_t tag_, name_;
};



ibyAuthTabCorpus::ibyAuthTabCorpus(ibyFile & src, ibylen_t len, ibystring_t tag) throw(IbycusParseException) 
	: tag_(tag.substr(tag.find_first_not_of("*")))
{
	len--;
	ibyFile::pos_type start_pos = src.tellg();
	unsigned char delim;
	src >> name_;
	ibyAuthTabAuthor auth;
	ibystring_t id, name, tmp;
	bool first = true;

	while ((ibyFile::pos_type)len > ((ibyFile::pos_type)src.tellg() - start_pos)) {
		delim = src.get();
		switch (delim) {
		case AT_DELIM_AUTH:
			src >> id;

			if (id.length() > 0) {
				name = id.substr(8);
				id = id.substr(0,7);
				
				if (first) {
					first = false;
				} else {
					authlist_.push_back(auth);
				}

				auth = ibyAuthTabAuthor(id, name);

			}
			break;
		case AT_DELIM_UNKNOWN:
			// What is this field for?
			src >> tmp;
			break;
		case AT_DELIM_REMARKS:
			src >> tmp;
			auth.SetComment(tmp);
			break;
		case AT_DELIM_ALPHABET:
			auth.SetAlphabet(src.get());
			break;
		case AT_DELIM_ALIAS:
			src >> tmp;
			if (tmp.length() > 0)
				auth.AddAlias(tmp);
			break;
		default:
			char tmp2[500];
			unsigned int errpos = src.tellg();
			sprintf(tmp2, "Error reading AUTHTAB.DIR. Unexpected value (%X) at file offset %X", delim, errpos);
			throw IbycusParseException(tmp2, errpos);
		}
		
	}
	authlist_.push_back(auth);
	src.get(); // Eat final delimiter
}

const ibystring_t & ibyAuthTabCorpus::Name(int author) const throw(IbycusException)
{
	try {
		return authlist_[author].Name();
	} catch(out_of_range E) {
		throw IbycusException("No author with the index");
	}
}

const ibystring_t & ibyAuthTabCorpus::Id(int author) const throw(IbycusException)
{
	try {
		return authlist_[author].Id();
	} catch(out_of_range E) {
		throw IbycusException("No author with the index");
	}
}

int ibyAuthTabCorpus::AliasCount(int author) const throw(IbycusException)
{
	try {
		return authlist_[author].AliasCount();
	} catch(out_of_range E) {
		throw IbycusException("No author with the index");
	}
}

const ibystring_t & ibyAuthTabCorpus::Alias(int author, int alias) const throw(IbycusException)
{
	try {
		return authlist_[author].Alias(alias);
	} catch(out_of_range E) {
		throw IbycusException("No author with the index");
	}
}


__IBYCUS_END_NAMESPACE

#endif	/* Not _IBYCUSAUTHTABCORPUS_H_ */
