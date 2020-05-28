/* 
 * IbycusIdtSection.h: ibyIdtSection class -- 
 *    Section in an ibycusIdtWork
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
 * $Revision: 1.5 $
 * $Date: 2001/10/11 21:52:16 $
 * $Author: seanredmond $
 */

#ifndef	_IBYCUSIDTSECTION_H_
#define _IBYCUSIDTSECTION_H_

#include "IbycusFile.h"
#include "IbycusId.h"
#include "IbycusIdFile.h"
#include "IbycusIdException.h"
#include "IbycusParseException.h"
#include "IbycusNoId.h"

__IBYCUS_BEGIN_NAMESPACE
class ibyIdtSection
{

public:
	ibyIdtSection() : Block_(0) {};
	ibyIdtSection(ibyIdFile & idt);
	virtual ~ibyIdtSection() {};
	inline bool operator<(const ibyIdtSection & rhs) const 
		{ return StartId_ < rhs.StartId_; };
	inline bool operator==(const ibyIdtSection & rhs) const
		{ return StartId_ == rhs.StartId_ && EndId_ < rhs.EndId_; };
	const IbycusId & Start() const;
	const IbycusId & End() const;
	ibylen_t Block(const IbycusId & id) const throw(IbycusNoId);
	int Span() const
		{ return LastIds_.size(); };

private:
	ibylen_t Block_;
	IbycusId StartId_, EndId_;
	vector<IbycusId> LastIds_;
	vector<ibyIdException> Exceptions_;
};


ibyIdtSection::ibyIdtSection(ibyIdFile & idt)
{
	ibychar_t c;
	idt >> c;
	if (c != idt.idt_new_sect) {
		char tmp[100];
		sprintf(tmp, "No new section in IDT file at %X (%X)", (unsigned int)idt.tellg(), c);
		throw IbycusParseException(tmp);
	}

	idt >> Block_;
	bool keepon = true;
	while (keepon) {
		c = idt.get();
		switch(c) {
		case idt.idt_new_work :
			idt.putback(c);
			keepon = false;
			break;
		case idt.idt_new_sect :
			idt.putback(c);
			keepon = false;
			break;
		case idt.idt_sect_start:
			idt >> StartId_;
			break;
		case idt.idt_sect_end:
			idt >> EndId_;
			break;
		case idt.idt_last_id:
			LastIds_.push_back(IbycusId(idt, idt.last_id));
			break;
		case idt.idt_exc_start:
			Exceptions_.push_back(ibyIdException(idt));
			break;
		case idt.idt_exc_end:
			(Exceptions_.back()).SetEnd(idt);
			break;
		case idt.idt_exc_sing:
			Exceptions_.push_back(ibyIdException(idt));
			break;
		case idt.idt_eof:			// Handles error in format
			idt.putback(c);
			keepon = false;
			break;
		default:
			throw IbycusParseException("Unexpected byte", idt.tellg());
		}
	}
}

ibylen_t ibyIdtSection::Block(const IbycusId & id) const throw(IbycusNoId)
{
	if (id.IsNull(IbycusId::cl_sect)) {
		return Block_;
	} else {
		if (Span() > 0) {
			IbycusId last_start = StartId_;
			unsigned int i;
			for (i = 0; i < LastIds_.size(); i++) {
				if (id >= last_start && id <= LastIds_[i]) {
					return Block_ + i;
				}
			}

			throw IbycusNoId("Id " + id.ToString() + " not found");
		} else {
			return Block_;
		}
	}
	return -1;
}

inline const IbycusId & ibyIdtSection::Start() const
{
	cout << "Section Start: " << StartId_ << "\n";
	return StartId_; 
}


inline const IbycusId & ibyIdtSection::End() const
{
	return EndId_; 
}
__IBYCUS_END_NAMESPACE
#endif	/* Not _IBYCUSIDTSECTION_H_ */
