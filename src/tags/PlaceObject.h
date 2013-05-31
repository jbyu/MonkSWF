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

struct ButtonRecord;
class MovieClip;
class MovieObject;

class PlaceObjectTag : public ITag
{
	enum Mode {
		INVALID,
		PLACE,
		MOVE,
		REPLACE
	};

public:
	PlaceObjectTag( TagHeader& h ) 
		:ITag( h )
		,_character_id( 0xffff )
		,_depth( 0 )
        ,_clip_depth( 0 )
		,_has_matrix( 0 )
		,_has_clip_depth( 0 )
		,_has_color_transform( 0 )
		,_placeMode( INVALID )
		,_name( "none" )
	{
	}

	PlaceObjectTag( const ButtonRecord& h );

	virtual ~PlaceObjectTag()
	{
	}

	virtual bool read( Reader& reader, SWF&, MovieFrames& );

	virtual void print();

	virtual void setup(MovieClip&, bool skipAction);
	
	uint16_t depth()        const { return _depth; }
    uint16_t clipDepth()    const { return _clip_depth; }
	uint16_t characterID()  const { return _character_id; }

	void copyAttributes(MovieObject&);

	static ITag* create( TagHeader& header ) {
		return new PlaceObjectTag( header );
	}

private:
	uint16_t	_character_id;
	uint16_t	_depth;
	uint16_t	_clip_depth;

	uint8_t		_has_matrix;
	uint8_t		_has_clip_depth;
	uint32_t	_has_color_transform;

	Mode			_placeMode;

    MATRIX          _transform;
    CXFORM          _cxform;
	std::string     _name;

};

}//namespace
#endif // __PlaceObject_h__