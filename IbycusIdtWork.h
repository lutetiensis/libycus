/* 
 * IbycusIdtWork.h: ibyIdtWork class -- 
 *    Work in an ibyIdtAuth
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
 * $Date: 2001/10/11 22:00:04 $
 * $Author: seanredmond $
 */

#ifndef	_IBYCUSIDTWORK_H_
#define _IBYCUSIDTWORK_H_

#include "IbycusIdFile.h"
#include "IbycusIdtSection.h"
#include "IbycusParseException.h"

__IBYCUS_BEGIN_NAMESPACE
typedef map<unsigned char, ibystring_t, less<unsigned char> > level_map_t;

class ibyIdtWork
{
public:
	ibyIdtWork() 
		: _length(0), _block(0), _start_pos(0), _end_header(0), 
		_SectsReady(false)
		{};
	virtual ~ibyIdtWork() {};
	void Read(ibyIdFile & s);
	const ibystring_t & Name() const { return _name; };
	// TODO: implement Id() for IdtSections
	const IbycusId & Id(const int sect = -1) const { return _id; };
	int Count(ibyIdFile & s) throw(IbycusParseException)
		{ _Get(s); return _Sections.size(); };
	const IbycusId & Start(ibyIdFile & s, const int sect = 0) throw(IbycusParseException)
		{ _Get(s); return _Sections[sect].Start(); };
	const IbycusId & End(ibyIdFile & s, int sect = -1);
	ibylen_t Block(ibyIdFile & s, const int sect, 
		const IbycusId & id) throw(IbycusNoId, IbycusParseException);
	int Span(ibyIdFile & s, const int sect = 0) throw(IbycusParseException)
		{ _Get(s); return _Sections[sect].Span(); };
	int CiteLevels() const
		{ return _levelDesc.size(); }

private:
	ibylen_t _length;
	ibylen_t _block;
	IbycusId _id;
	ibystring_t _name;
	vector<ibyIdtSection> _Sections;
	ibyIdFile::pos_type _start_pos, _end_header;
	level_map_t _levelDesc;
	bool _SectsReady;

	void _Get(ibyIdFile & s) throw(IbycusParseException);

friend ibyIdFile & operator>>(ibyIdFile & s, ibyIdtWork & rhs);
};

ibyIdFile & operator>>(ibyIdFile & s, ibyIdtWork & rhs) {
	rhs._block = rhs._length = rhs._start_pos = rhs._end_header = 0;
	rhs._Sections.clear();
	rhs._SectsReady = false;
	unsigned char c;
	ibyFile::pos_type sp = s.tellg();

	s >> c;
	if (c != s.idt_new_work)
		if (c == s.idt_eof) {
			s.setstate(s.eofbit);
			return s;
		} else {
			throw IbycusParseException("No new work in IDT file", s.tellg());
		}

	s >> rhs._length >> rhs._block;
	rhs._start_pos = s.tellg();
	rhs._length -= (rhs._start_pos-sp);
	s.seekg(rhs._length + rhs._start_pos); // move past end of work

	return s;
}

void ibyIdtWork::Read(ibyIdFile & s)
{
	s.clear();
	s.seekg(_start_pos);
	//ibyIdFile::pos_type endp = _length + _start_pos;
	unsigned char c;
	s >> _id;

	s >> c;
	if (c != s.idt_desc_ab)
		throw IbycusParseException("Error Reading file: Expected level description", s.tellg());
	s >> c;
	if (c != 1)	// should be level 1
		throw IbycusParseException("Error Reading file: expected level 1 description", s.tellg());

	s.GetLenString(_name);

	ibystring_t ls;
	s >> c;
	while (c == s.idt_desc_nz) {
		s >> c;				// level
		s.GetLenString(_levelDesc[c]); // description
		s >> c;
	}

	_end_header = -1 + s.tellg(); // order solves operator+ ambiguity
}

void ibyIdtWork::_Get(ibyIdFile & s) throw(IbycusParseException)
{
	if (_SectsReady)
		return;

	s.clear();
	s.seekg(_end_header);
	ibyIdFile::pos_type endp = _length + _start_pos;
	while (s.tellg() < endp) {
		// CATCHES AN ERROR IN LAT1351 (Tacitus)
		try { 
			_Sections.push_back(ibyIdtSection(s));
		} catch (IbycusParseException E) {
			if (s.get() == s.idt_eof)
				endp = s.tellg();
			else
				throw E;
		}
		s.clear();
		_SectsReady = true;
	}
}

ibylen_t ibyIdtWork::Block(ibyIdFile & s, const int sect, 
							const IbycusId & id) throw(IbycusNoId, IbycusParseException)
{
	_Get(s);
	return _Sections[sect].Block(id);
}

const IbycusId & ibyIdtWork::End(ibyIdFile & s, int sect)
{
	_Get(s); 
	if (sect < 0) {
		sect = _Sections.size()-1;
	}

	return _Sections[sect].End(); 
}


__IBYCUS_END_NAMESPACE
#endif	/* Not _IBYCUSIDTWORK_H_ */
