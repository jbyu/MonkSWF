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

namespace MonkSWF {
	class RemoveObjectTag : public ITag {
		uint16_t	_character_id;
		uint16_t	_depth;

	public:
		RemoveObjectTag( TagHeader& h ) 
		:ITag( h )
        ,_character_id( 0 )
		{}
		
		virtual ~RemoveObjectTag()
        {}
		
		uint16_t characterId() const {
			return _character_id;
		}
		uint16_t depth() const {
			return _depth;
		}

        virtual bool process(SWF* swf ) { return true; }

        virtual void setup(MovieClip &movie)
        {
            DisplayList& _display_list = movie.getDisplayList();
		    const uint16_t depth = this->depth();
            PlaceObjectTag* current_obj = _display_list[ depth ];
            if (current_obj)
                current_obj->gotoFrame(0xffffffff);
            _display_list[ depth ] = NULL;
        }

		virtual bool read( Reader* reader, SWF* ) {
			if ( code() == TAG_REMOVE_OBJECT )
            {
				_character_id = reader->get<uint16_t>();
            }	
			_depth = reader->get<uint16_t>();
		
			return true;
		}
		
		virtual void print() {
			if( code() == TAG_REMOVE_OBJECT )
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