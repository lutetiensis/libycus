/* 
 * IbycusException.h: IbycusException class -- 
 *    Base class for libycus exceptions
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
 * $Date: 2001/10/11 20:10:08 $
 * $Author: seanredmond $
 */

#ifndef	_IBYCUSEXCEPTION_H_
#define _IBYCUSEXCEPTION_H_

#include "IbycusDefs.h"

__IBYCUS_BEGIN_NAMESPACE
class IbycusException : public runtime_error
{
public:
	int offset;
	ibystring_t type;
	IbycusException(ibystring_t msg = "", int off=0)
		: runtime_error(msg), offset(off), type("general") {};
	virtual ~IbycusException() _NOEXCEPT {};

};

__IBYCUS_END_NAMESPACE
#endif	/* Not _IBYCUSEXCEPTION_H_ */
