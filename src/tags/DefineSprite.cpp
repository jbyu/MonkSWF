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
		// foreach frames in sprite
		FrameList::iterator it = _frame_list.begin();
		while (_frame_list.end() != it)
        {
			// foreach tags in frame
			TagList *_tag_list = *it;
            TagList::iterator tag_it = _tag_list->begin();
            while(_tag_list->end() != tag_it)
            {
                delete *tag_it;
                ++tag_it;
            }

            delete *it;
            ++it;
        }
    }

	bool DefineSpriteTag::read( Reader* reader, SWF* _swf ) {
		MK_TRACE("=== DEFINE_SPRITE BEGIN ===\n");
		_sprite_id = reader->get<uint16_t>();
		_frame_count = reader->get<uint16_t>();
		
		// get the first tag
        TagHeader oHeader;
		TagHeader *tag_header = &oHeader;//new TagHeader();
		tag_header->read( reader );
		
		TagList* frame_tags = new TagList;
		while( tag_header->code() != TAG_END ) { // while not the end tag 
			ITag* tag = NULL;
            const uint32_t code = tag_header->code();
			SWF::TagFactoryFunc factory = _swf->getTagFactory( code );
			if( factory ) {
				tag = factory( tag_header );

				int32_t end_pos = reader->getCurrentPos() + tag->length();
				tag->read( reader, _swf );
				tag->print();
				reader->align();

				int32_t dif = end_pos - reader->getCurrentPos();
				if( dif != 0 ) {
					MK_TRACE("WARNING: tag not read correctly. trying to skip.\n");
					reader->skip( dif );
				}
                // process tag
                bool keepTag = tag->process(_swf);
                if (keepTag) {
                    frame_tags->push_back( tag );
                } else {
                    delete tag;
                }
                // create a new frame
                if ( TAG_SHOW_FRAME == code )
                {
					_frame_list.push_back( frame_tags );
					frame_tags = new TagList;
                }
			} else {	// no registered factory so skip this tag
				MK_TRACE("*** SKIP *** ");
				tag_header->print();
				reader->skip( tag_header->length() );
			}
			
			//delete tag_header;
			//tag_header = new TagHeader();
			tag_header->read( reader );
		}
		MK_TRACE("=== DEFINE_SPRITE  END  ===\n");
        delete frame_tags;
		MK_ASSERT(_frame_list.size() == _frame_count);
		return true;
	}

	void DefineSpriteTag::print() {
		_header.print();
		MK_TRACE("id=%d, frame_count=%d\n", _sprite_id, _frame_count);
	}

    bool DefineSpriteTag::process(SWF* swf )
    {
        swf->addCharacter( this, _sprite_id );
        return true; // keep tag
    }

	ITag* DefineSpriteTag::create( TagHeader* header ) {
		return (ITag*)(new DefineSpriteTag( *header ));
	}

	
}