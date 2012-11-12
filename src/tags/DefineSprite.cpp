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
		MK_TRACE("=== DEFINESPRITE BEGIN ===\n");
		_sprite_id = reader->get<uint16_t>();
		_frame_count = reader->get<uint16_t>();
		
		// get the first tag
        TagHeader oHeader;
		TagHeader *tag_header = &oHeader;//new TagHeader();
		tag_header->read( reader );
		
		TagList* frame_tags = new TagList;
		while( tag_header->code() != TAG_END ) { // while not the end tag 
			ITag* tag = NULL;
			SWF::TagFactoryFunc tag_factory = _swf->getTagFactory( tag_header->code() );
			if( tag_factory ) {
				tag = tag_factory( tag_header );

				int32_t end_pos = reader->getCurrentPos() + tag->length();
				tag->read( reader, _swf );
				tag->print();
				reader->align();

				int32_t dif = end_pos - reader->getCurrentPos();
				if( dif != 0 ) {
					MK_TRACE("WARNING: tag not read correctly. trying to skip.\n");
					reader->skip( dif );
				}

#if 0				
				if ( tag_header->code() == DEFINESPRITE ) {
					IDefineSpriteTag* sprite = (IDefineSpriteTag*)tag;
					//sprite->setSWF( _swf );
					_swf->addSprite( sprite, sprite->code());
				}
#endif				
				
				if( tag_header->code() == TAG_SHOW_FRAME ) {	// ShowFrame
					_frame_list.push_back( frame_tags );
					frame_tags = new TagList;
					delete tag;
				} else {
					frame_tags->push_back( tag );
				}
				
			} else {	// no registered factory so skip this tag
				MK_TRACE("*** SKIPPING UNKOWN TAG *** ");
				tag_header->print();
				reader->skip( tag_header->length() );
			}
			
			//delete tag_header;
			//tag_header = new TagHeader();
			tag_header->read( reader );
		}
		MK_TRACE("=== DEFINESPRITE  END  ===\n");
        delete frame_tags;
		MK_ASSERT(_frame_list.size() == _frame_count);
		return true;
	}

	void DefineSpriteTag::print() {
		_header.print();
		MK_TRACE("\tDEFINESPRITE: id=%d, frame_count=%d\n", _sprite_id, _frame_count);
	}

#if 0	
	void DefineSpriteTag::draw( SWF* _swf, int frame)
	{
        MK_ASSERT ( 0 <= frame && frame < getFrameCount() );
        // build up the display list
		TagList* frame_tags = _frame_list[ frame ];
		setup_frame(_display_list, *frame_tags );
		/*
        TagList::iterator it = frame_tags->begin();
		while( it != frame_tags->end() ) {
            ITag* tag = *it;
			switch ( tag->code() ) {
				case PLACEOBJECT2: {
					IPlaceObjectTag* place_obj = (IPlaceObjectTag*)tag;
					
					if( place_obj->doMove() ) {	// just move the object at the current depth
						IPlaceObjectTag* move_obj = _display_list[ place_obj->depth() ];
						if( move_obj )
                            place_obj->copyNoTransform( move_obj );
						// this should work but doesnt: move_obj->copyTransform( place_obj );
					}
					_display_list[ place_obj->depth() ] = place_obj;
				} break;

				case REMOVEOBJECT:
				case REMOVEOBJECT2:	{
					IRemoveObjectTag* remove_object = (IRemoveObjectTag*)tag;
					_display_list[ remove_object->depth() ] = NULL;
					//_display_list.erase( remove_object->depth() );
				} break;
	
				default:
					break;
			}
            ++it;
		}
		*/
		// draw the display list
		DisplayList::iterator iter = _display_list.begin();
		while ( _display_list.end() != iter )
		{
			IPlaceObjectTag *place_obj = iter->second;
			if (place_obj) 
				place_obj->draw( _swf ); 
			++iter;
		}
	}
#endif	

	ITag* DefineSpriteTag::create( TagHeader* header ) {
		return (ITag*)(new DefineSpriteTag( *header ));
	}

	
}