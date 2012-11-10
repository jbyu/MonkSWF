/*
 *  RemoveObject.h
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 5/7/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#ifndef __RemoveObject_h__
#define __RemoveObject_h__

#include "mkTag.h"

using namespace std;

namespace MonkSWF {
	class RemoveObjectTag : public IRemoveObjectTag {
	public:
		RemoveObjectTag( TagHeader& h ) 
		: IRemoveObjectTag( h )
		{}
		
		virtual ~RemoveObjectTag()
        {}
		
		virtual bool read( Reader* reader, SWF* ) {
			if( code() == REMOVEOBJECT )
				_character_id = reader->get<uint16_t>();
				
			_depth = reader->get<uint16_t>();
			
			return true;
		}
		
		virtual void print() {
			if( code() == REMOVEOBJECT )
				MK_TRACE("REMOVEOBJECT id=%d, depth=%d\n", _character_id, _depth);
			else
				MK_TRACE("REMOVEOBJECT2 depth=%d\n", _depth);
		}
		
		static ITag* create( TagHeader* header ) {
			return (ITag*)(new RemoveObjectTag( *header ));
		}				
	};
	
}	
#endif	// __RemoveObject_h__