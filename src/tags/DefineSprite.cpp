/*
 *  DefineSprite.cpp
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 8/28/10.
 *  Copyright 2010 MP Engineering. All rights reserved.
 *
 */

#include "DefineSprite.h"
#include "mkSWF.h"

namespace MonkSWF {

    DefineSpriteTag::~DefineSpriteTag()
    {
        MovieClip::destroyFrames(_data);
    }

	bool DefineSpriteTag::read( Reader& reader, SWF& swf, MovieFrames& ) {
		MK_TRACE("=== DEFINE_SPRITE BEGIN ===\n");
		_sprite_id = reader.get<uint16_t>();
		_frame_count = reader.get<uint16_t>();

        MovieClip::createFrames(reader, swf, _data);

        MK_ASSERT(_data._frames.size() == _frame_count);
		MK_TRACE("RECT:{%.2f,%.2f,%.2f,%.2f}\n",
			_data._rectangle.xmin, _data._rectangle.ymin,
			_data._rectangle.xmax, _data._rectangle.ymax);
		MK_TRACE("=== DEFINE_SPRITE  END  ===\n");
        swf.addCharacter( this, _sprite_id );
        return true; // keep tag
	}

	void DefineSpriteTag::print() {
		_header.print();
		MK_TRACE("id=%d, frame_count=%d\n", _sprite_id, _frame_count);
	}
	
}