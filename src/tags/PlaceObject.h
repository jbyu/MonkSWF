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
			,_name( "__none__" )
			,_scale(1.0f)
            ,_texture(0)
		{
		}
		
		virtual ~PlaceObject2Tag() {
		}
		
		virtual bool read( Reader* reader, SWF* );
		virtual void print();
		virtual void draw( SWF* swf );
		virtual void copyTransform( IPlaceObjectTag* other );
		
		virtual void setOffsetTranslate( float t[2] ) {
			_offset[0] = t[0];
			_offset[1] = t[1];
		}
		virtual void getOffsetTranslate( float t[2] ) const  {
			t[0] = _offset[0];
			t[1] = _offset[1];
		}
		virtual void setOffsetScale( float s ) {
			_scale = s;
		}
		virtual float offsetScale() const {
			return _scale;
		}
		
		static ITag* create( TagHeader* header ) {
			return (ITag*)(new PlaceObject2Tag( *header ));
		}
		

	private:
		// offset
		float			_offset[2];
		float			_scale;
        MATRIX          _transfrom;
        unsigned int    _texture;
		std::string		_name;
	};
}
#endif // __PlaceObject_h__