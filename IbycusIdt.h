/* 
 * IbycusIdt.h: IbycusIdtFile class -- 
 *    Interface to a TLG/PHI .IDT file
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
 * $Date: 2001/10/11 22:12:35 $
 * $Author: seanredmond $
 */

#ifndef	_IBYCUSIDT_H_
#define _IBYCUSIDT_H_

#include "IbycusIdFile.h"
#include "IbycusIdtAuth.h"
#include "IbycusFileException.h"

__IBYCUS_BEGIN_NAMESPACE
class IbycusIdtFile
{
public:
	IbycusIdtFile() : CurrentAuthIndex_(-1)  {};
	IbycusIdtFile(const ibystring_t & id, const ibystring_t & vol);
	virtual ~IbycusIdtFile();
	void Open(const ibystring_t & id, const ibystring_t & vol);
	int Count(const int auth = 0, const int work = -1);
	const ibystring_t & Name(const int auth = 0, const int work = -1);
	const IbycusId & Id(const int auth = 0, const int work = -1, 
		const int sect = -1);
	const IbycusId & Start(const int auth = 0, const int work = 0, 
		const int sect = 0);
	const IbycusId & End(const int auth = 0, const int work = -1, 
		const int sect = -1);
	ibylen_t Block(const int auth = 0, const int work = 0, 
		const int sect = 0, const IbycusId & id = IbycusId())
		throw(IbycusNoId);
	int Span(const int auth = 0, const int work = 0, const int sect = 0);
	int tellg();
	int CiteLevels(int auth=0, int work=0);

private:
	//ibylen_t Block_;
	//ibylen_t Length_;
	ibyIdFile Idt_;
	ibyIdtAuth CurrentAuth_;
	int CurrentAuthIndex_;

	void _get(const int auth = 0, const int work = -1, const int sect = -1);
};


IbycusIdtFile::IbycusIdtFile(const ibystring_t & id, const ibystring_t & vol)
 : CurrentAuthIndex_(-1)
{
	Open(id, vol);
}

// TODO: Throw an error if the file cannot be opened

IbycusIdtFile::~IbycusIdtFile()
{
	Idt_.close();
}

void IbycusIdtFile::Open(const ibystring_t & id, const ibystring_t & vol)
{
	ibystring_t fname = vol + "/" + id + ".IDT";
	Idt_.open(fname.c_str());
	if (!Idt_.is_open())
		throw IbycusFileException("Couldn't open file: " + fname);
}

int IbycusIdtFile::Count(const int auth, const int work)
{
	if (auth > -1) { // count the works in an author
		_get(auth);
		return CurrentAuth_.Count(Idt_, work);
	} else {		 // count the authors in the .idt file
		ibyIdtAuth temp;
		Idt_.clear();
		Idt_.seekg(0); 
		int count = 0;
		while (!Idt_.eof()) {
			Idt_ >> temp;
			count++;
		}
		return --count;
	}

	return -1; // failure
}

ibylen_t IbycusIdtFile::Block(const int auth, const int work, 
					   const int sect, const IbycusId & id) 
					   throw(IbycusNoId)
{
	_get(auth,work); 
	return CurrentAuth_.Block(Idt_, work, sect, id); 
}

int IbycusIdtFile::CiteLevels(int auth, int work)
{ 
	_get(auth, work);
	return CurrentAuth_.CiteLevels(Idt_, work);
}

const ibystring_t & IbycusIdtFile::Name(const int auth, const int work)
{
	_get(auth,work); 
	return CurrentAuth_.Name(work); 
}

const IbycusId & IbycusIdtFile::Id(const int auth, const int work, const int sect)
{
	_get(auth,work); 
	return CurrentAuth_.Id(work,sect);
}

const IbycusId & IbycusIdtFile::Start(const int auth, const int work, const int sect)
{ 
	_get(auth,work,sect); 
	return CurrentAuth_.Start(Idt_, work, sect); 
}

const IbycusId & IbycusIdtFile::End(const int auth, const int work, const int sect)
{
	_get(auth,work); 
	return CurrentAuth_.End(Idt_, work, sect); 
}

int IbycusIdtFile::Span(const int auth, const int work, const int sect)
{
	_get(auth,work); 
	return CurrentAuth_.Span(Idt_, work,sect); 
}

int IbycusIdtFile::tellg()
{ 
	return Idt_.tellg(); 
}

void IbycusIdtFile::_get(const int auth, const int work, const int sect)
{
	if (auth != CurrentAuthIndex_) {
		int i;
		Idt_.clear();
		Idt_.seekg(0);
		for (i = 0; i <= auth; i++)
			Idt_ >> CurrentAuth_;

		CurrentAuth_.Read(Idt_);
		CurrentAuthIndex_ = auth;
	}

	if (work > -1)
		CurrentAuth_.Get(Idt_, work, sect);
}

__IBYCUS_END_NAMESPACE

#endif	/* Not _IBYCUSIDT_H_ */
