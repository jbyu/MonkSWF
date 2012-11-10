/*
 *  PlaceObject.h
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/26/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#ifndef __PlaceObject_h__
#define __PlaceObject_h__

#include "mkTag.h"
#include <string>

namespace MonkSWF {

	class PlaceObject2Tag : public IPlaceObjectTag {
	public:
		PlaceObject2Tag( TagHeader& h ) 
			:IPlaceObjectTag( h )
            ,_texture(0)
			,_frame(0)
			,_name( "__none__" )
		{
		}
		
		virtual ~PlaceObject2Tag()
		{
		}
		
		virtual bool read( Reader* reader, SWF* );
		virtual void print();
		virtual void draw( SWF* swf );
		virtual void copyTransform( IPlaceObjectTag* other );
		
		static ITag* create( TagHeader* header ) {
			return (ITag*)(new PlaceObject2Tag( *header ));
		}

	private:
        unsigned int    _texture;
		int				_frame;
		std::string		_name;
        MATRIX          _transfrom;
	};
}
#endif // __PlaceObject_h__