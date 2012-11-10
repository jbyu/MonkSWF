/*
 *  mkTag.cpp
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/20/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#include "mkTag.h"

using namespace std;

namespace MonkSWF {
	
	bool TagHeader::read( Reader* reader ) {
		uint16_t tagcode_and_length = reader->get<uint16_t>();
		
		_code = (tagcode_and_length >> 6 );
		_length = (tagcode_and_length & 0x3F );
		if( _length == 0x3f )	// if long tag read an additional 32 bit length value
			_length = reader->get<uint32_t>();
				
		return true;
	}
	
	void TagHeader::print() {
		MK_TRACE("code:%2d, length:%d\n",  _code , _length);
	}
}