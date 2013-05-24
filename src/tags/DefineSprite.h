/*
 *  DefineSprite.h
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 8/28/10.
 *  Copyright 2010 MP Engineering. All rights reserved.
 *
 */

#ifndef __DefineSprite_h__
#define __DefineSprite_h__

#include "mkTag.h"

namespace MonkSWF {

	class DefineSpriteTag : public ITag {
	public:
		DefineSpriteTag( TagHeader& h )
		:	ITag( h )
		{
		}
	
		virtual ~DefineSpriteTag();

		virtual bool read( Reader& reader, SWF&, MovieFrames& );
		virtual void print();
	
		const MovieFrames& getMovieFrames(void) const { return _data; }

		static ITag* create( TagHeader& header ) {
		    return new DefineSpriteTag( header );
	    }

	private:
		uint16_t	_sprite_id;
		uint16_t	_frame_count;
		MovieFrames	_data;
	};
}
#endif // __DefineSprite_h__
