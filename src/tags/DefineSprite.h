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
#include <map>

namespace MonkSWF {

	class DefineSpriteTag : public ITag {
	public:
		DefineSpriteTag( TagHeader& h )
		:	ITag( h )
		{
		}
		
		virtual ~DefineSpriteTag();
		
        virtual bool process(SWF* swf );
		virtual bool read( Reader* reader, SWF* );
		virtual void print();
		
		const FrameList& getFrameList(void) const { return _frame_list; }
		
		static ITag* create( TagHeader* header );
		
	private:
		uint16_t		_sprite_id;
		uint16_t		_frame_count;
		FrameList		_frame_list;
	};
}
#endif // __DefineSprite_h__
