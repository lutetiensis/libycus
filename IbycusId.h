/* 
 * IbycusId.h: IbycusId class -- 
 *    Class for represent libycus ID's
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
 * $Revision: 1.3 $
 * $Date: 2001/10/11 19:02:39 $
 * $Author: seanredmond $
 */

#ifndef	_IBYCUSID_H_
#define _IBYCUSID_H_

#include <cassert>
#include <algorithm>
#include "IbycusCit.h"
#include "IbycusFile.h"
#include "IbycusIdNumber.h"
#include "IbycusException.h"
#include "IbycusParseException.h"

__IBYCUS_BEGIN_NAMESPACE
typedef map<ibychar_t, ibyIdNumber, less<ibychar_t> > IDMAP;

class IbycusId : private ibyCitation
{

public:
	IbycusId() : Flag_(NOFLAG) {};
	IbycusId(const IbycusId & rhs) : Flag_(rhs.Flag_), AuthDesc_(rhs.AuthDesc_),
		WorkDesc_(rhs.WorkDesc_),Ids_(rhs.Ids_) {};
	IbycusId(ibyFile & src) : Flag_(NOFLAG)
		{ Read(src, IbycusId()); };
	IbycusId(ibyFile & src, const IbycusId & prev) : Flag_(NOFLAG)
		{ Read(src, prev); };
	IbycusId(const IbycusId & rhs, const ibyIdNumber & cit1, 
		const ibyIdNumber & cit2 = ibyIdNumber(),
		const ibyIdNumber & cit3 = ibyIdNumber(),
		const ibyIdNumber & cit4 = ibyIdNumber(),
		const ibyIdNumber & cit5 = ibyIdNumber());

	virtual ~IbycusId() {};

	
	bool IsTitle() const;
	enum comp_level { cl_auth, cl_work,	cl_sect	};
	enum format_flags { 
		fmt_titles  = 0x00, 
		fmt_numbers = 0x01, 
		fmt_diff    = fmt_numbers << 1, 
		fmt_auth    = fmt_diff    << 1, 
		fmt_work    = fmt_auth    << 1, 
		fmt_section = fmt_work    << 1, 
		fmt_same    = fmt_section << 1 
	};

	bool SameAs(const IbycusId & rhs, const comp_level & cl = cl_sect) const;
	void Clear();
	void Read(istream & src, const IbycusId & prev);
	IbycusId & operator=(const IbycusId & rhs);
	bool operator==(const IbycusId & rhs) const
		{ return Flag_ == rhs.Flag_ && Ids_ == rhs.Ids_; };
	bool operator!=(const IbycusId & rhs) const
		{ return Flag_ != rhs.Flag_ || Ids_ != rhs.Ids_; };
    bool operator<(const IbycusId & rhs) const
		{ return Ids_ < rhs.Ids_; };
    bool operator>(const IbycusId & rhs) const
		{ return Ids_ > rhs.Ids_; };
    bool operator<=(const IbycusId & rhs) const
		{ return ((Ids_ < rhs.Ids_) || (Ids_ == rhs.Ids_)); };
    bool operator>=(const IbycusId & rhs) const
		{ return ((Ids_ > rhs.Ids_) || (Ids_ == rhs.Ids_)); };
	int Flag() const;
	ibystring_t Id(ibychar_t level) const;
	bool IsNull(const comp_level & cl = cl_sect) const;
	ibystring_t ToString(const int fmt = fmt_same,
		const IbycusId * prev = NULL) const;

	enum id_level {
		LV_A    = 0x00,
		LV_B    = 0x01,
		LV_C    = 0x02,
		LV_D	= 0x03,
		LN_Z	= 0x08,
		LN_Y	= 0x09,
		LN_X	= 0x0A,
		LN_W	= 0x0B,
		LN_V	= 0x0C,
		LN_N	= 0x0D,
		LN_ESC	= 0x0E,
		LN_SPEC	= 0x0F
	};

	enum id_flag {
		NOFLAG,
		ENDOFBLOCK,
		ENDOFFILE,
		EXCEPTION_START,
		EXCEPTION_END
	};

private:
	void insert(ibychar_t level, const ibyIdNumber & p_id);
	static format_flags Format_;

	int Flag_;
	ibyIdNumber AuthDesc_, WorkDesc_;
	IDMAP Ids_, Descrip_;

	enum rn_special {
		rn_eob		= 0xE,
		rn_eof		= 0x0,
		rn_ex_start	= 0x8,
		rn_ex_end	= 0x9
	};

friend ostream & operator<<(ostream & s, const IbycusId & rhs);
};

IbycusId::IbycusId(const IbycusId & rhs,  const ibyIdNumber & cit1, 
	const ibyIdNumber & cit2, const ibyIdNumber & cit3,
	const ibyIdNumber & cit4, const ibyIdNumber & cit5) : Flag_(rhs.Flag_), 
		AuthDesc_(rhs.AuthDesc_), WorkDesc_(rhs.WorkDesc_), Ids_(rhs.Ids_)
{

	if(!cit5.IsNull()) {
		insert('z', cit5);
		insert('y', cit4);
		insert('x', cit3);
		insert('w', cit2);
		insert('v', cit1);
	} else if (!cit4.IsNull()) {
		insert('z', cit4);
		insert('y', cit3);
		insert('x', cit2);
		insert('w', cit1);
	} else if (!cit3.IsNull()) {
		insert('z', cit3);
		insert('y', cit2);
		insert('x', cit1);
	} else if (!cit2.IsNull()) {
		insert('z', cit2);
		insert('y', cit1);
	} else if (!cit1.IsNull()) {
		insert('z', cit1);
	}


}

void IbycusId::Read(istream & src, const IbycusId & prev) {

	Ids_ = prev.Ids_;

	bool keepon = true;
	unsigned char ch;
	unsigned char level_byte;
	ibychar_t level;
	ibyIdNumber blanknum;

	bool is_descrip = false;
	ch = src.get();
	while ((ch & HIGHBIT) && keepon) {
		is_descrip = false;
		switch (left_nibble(ch)) {
		case IDX_LN_ESC:
			switch(level_byte = src.get() & LOWBITS) {
			case LV_A:
				level = 'a';
				break;
			case LV_B:
				level = 'b';
				break;
			case LV_C:
				level = 'c';
				break;
			case LV_D:
				level = 'd';
				break;
			default:
				if ('a' <= level_byte && 'z' >= level_byte) {
					is_descrip = true;
				} else {
					char tmp[50];
					sprintf(tmp, "Unhandled code (%X) in id", ch);
					throw IbycusParseException(tmp, src.tellg());
				}
			}

			if (is_descrip)
				Descrip_[level_byte] = ibyIdNumber(src, ch, blanknum);
			else
				insert(level, ibyIdNumber(src, ch, Ids_[level]));
			break;
		case IDX_LN_SPEC:
			switch(right_nibble(ch)) {
			case rn_eof:
				Flag_ = ENDOFFILE;
				break;
			case rn_eob:
				Flag_ = ENDOFBLOCK;
				break;
			case rn_ex_start:
				Flag_ = EXCEPTION_START;
				break;
			case rn_ex_end:
				Flag_ = EXCEPTION_END;
				break;
			default:
				char tmp[50];
				sprintf(tmp, "Unhandled code (%X) in id", ch);
				throw IbycusParseException(tmp, src.tellg());
			}
			keepon = false;
			break;
		default:
			switch (left_nibble(ch)) { // & 0x7
			case LN_Z:
				level = 'z';
				break;
			case LN_Y:
				level = 'y';
				break;
			case LN_X:
				level = 'x';
				break;
			case LN_W:
				level = 'w';
				break;
			case LN_V:
				level = 'v';
				break;
			case LN_N:
				level = 'n';
				break;
			default:
				char tmp[50];
				sprintf(tmp, "Unhandled code (%X) in id", ch);
				throw IbycusParseException(tmp, src.tellg());
			}

			// TODO: pass LN_Z, etc. to ibyIdNumber instead of setting level
			//       get rid of switch statement

			try {
				insert(level, ibyIdNumber(src, ch, Ids_[level]));
			} catch (...) {
				throw IbycusParseException("Error inserting ID", src.tellg());
			}
		}

		// basic_ifstream bug?
		// src >> ch causes the stream to get out of sync
		ch = src.get();
	}
	src.putback(ch);
}

inline int IbycusId::Flag() const
{ 
	return Flag_;
}

ibystring_t IbycusId::ToString(const int fmt, const IbycusId * prev) const
{
	ibystring_t retval = "";
	IDMAP::key_type k = 'a';
	if (fmt & fmt_auth)
		k = 'a';
	else if (fmt & fmt_work)
		k = 'b';
	else if (fmt & fmt_section)
		k = 'e';

	IDMAP::const_iterator i = Ids_.lower_bound(k);

	if ((fmt & fmt_diff) && prev != NULL)
		i = mismatch(Ids_.lower_bound(k), Ids_.end(), 
			prev->Ids_.lower_bound(k)).first;

	while (i != Ids_.end()) {
		switch((*i).first) {
		case 'a':
			if (fmt & fmt_auth) {
				retval += (fmt & fmt_numbers)
					? (*i).second.ToString() : AuthDesc_.ToString();
			}
			break;

		case 'b':
			if (fmt & (fmt_work|fmt_auth)) {
				retval += retval.empty() ? "" : ".";
				retval += (fmt & fmt_numbers)
					? (*i).second.ToString() : WorkDesc_.ToString();
			}
			break;

		default:
			retval += retval.empty() ? "" : ".";
			retval += (*i).second.ToString();
		}
		i++;
	}

	return retval;
}

void IbycusId::insert(ibychar_t level, const ibyIdNumber & p_id)
{
	assert(level <= 'd' || level == 'n' || level >= 'v');


	// insert the id

	if (level == 'c') {
		WorkDesc_ = p_id;
		Ids_.erase(level);
	} else if (level == 'd') {
		AuthDesc_ = p_id;
		Ids_.erase(level);
	} else {
		Ids_[level] = p_id;
		if (level <= 'b') {
			Ids_.erase(++(Ids_.find(level)), Ids_.end());
		} else if (level == 'n') {
			Ids_.erase(++(Ids_.find(level)), Ids_.end());
		} else if (level >= 'v') {
			// set lower levels to 1
			ibychar_t j;
			for (j = ++level; j <= 'z'; j++)
				Ids_[j] = 1; //ibyIdNumber(1);
		}
	}
}

ibystring_t IbycusId::Id(ibychar_t level) const
{
	IDMAP::const_iterator i;
	if ((i = Ids_.find(level)) != Ids_.end())
		return (*i).second.ToString();
	else {
		char tmp[50];
		sprintf(tmp, "No level %c in ID", level);
		throw IbycusException(tmp);
	}

	return "";
}

IbycusId & IbycusId::operator=(const IbycusId & rhs)
{
	Flag_ = rhs.Flag_;
	AuthDesc_ = rhs.AuthDesc_;
	WorkDesc_ = rhs.WorkDesc_;
	Ids_ = rhs.Ids_;
	return *this;
}

void IbycusId::Clear()
{
	Flag_ = NOFLAG;
	AuthDesc_.Clear();
	WorkDesc_.Clear();
	Ids_.clear();
	Descrip_.clear();
}

bool IbycusId::SameAs(const IbycusId & rhs, const comp_level & cl) const
{
	cout << "SameAs: " << ToString() << " : " << rhs << "\n";
	IDMAP::key_type k;
	if (cl == cl_auth)
		k ='a';
	else if (cl == cl_work)
		k = 'b';
	else 
		k = (*Ids_.upper_bound('b')).first;

	return equal(Ids_.begin(), Ids_.upper_bound(k), rhs.Ids_.begin());
}

bool IbycusId::IsNull(const comp_level & cl) const
{
	IDMAP::key_type k;
	if (cl == cl_auth)
		k ='a';
	else if (cl == cl_work)
		k = 'b';
	else 
		k = (*Ids_.upper_bound('b')).first;

	return Ids_.empty(); 
};


bool IbycusId::IsTitle() const
{
	IDMAP::const_iterator i;
	if((i = Ids_.find('b')) != Ids_.end()) {
		while (++i != Ids_.end()) {
			if ((*i).second.IsTitle())
				return true;
		}
	}

	return false;
}

ostream & operator<<(ostream & s, const IbycusId & rhs) {
	IDMAP::const_iterator i;
	for (i = rhs.Ids_.begin(); i != rhs.Ids_.end(); ++i) {
		if (!(*i).second.IsNull()) {
			if (i != rhs.Ids_.begin())
				s << '.';
			s << (*i).second;
		}
	}
	return s;
}
__IBYCUS_END_NAMESPACE

#endif	/* Not _IBYCUSID_H_ */

