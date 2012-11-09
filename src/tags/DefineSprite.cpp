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
        {
            FrameList::iterator it = _frames.begin();
            while(_frames.end() != it)
            {
                delete *it;
                ++it;
            }
        }
        {
            FrameTagList::iterator it = _tag_list.begin();
            while(_tag_list.end() != it)
            {
                delete *it;
                ++it;
            }
        }
    }

	bool DefineSpriteTag::read( Reader* reader, SWF* _swf ) {
		_sprite_id = reader->get<uint16_t>();
		_frame_count = reader->get<uint16_t>();
		
		// get the first tag
        TagHeader oHeader;
		TagHeader *tag_header = &oHeader;//new TagHeader();
		tag_header->read( reader );
		
		FrameTagList* frame_tags = new FrameTagList;

		while( tag_header->code() != ENDTAG ) { // while not the end tag 
			
			SWF::TagFactoryFunc tag_factory = _swf->getTagFactory( tag_header->code() );
			ITag* tag = 0;
			
			if( tag_factory ) {
				tag = tag_factory( tag_header );
                _tag_list.push_back(tag);

				int32_t end_pos = reader->getCurrentPos() + tag->length();
				
				if ( tag_header->code() == DEFINESPRITE ) {
					IDefineSpriteTag* sprite = (IDefineSpriteTag*)tag;
					//sprite->setSWF( _swf );
					_swf->addSprite( sprite, sprite->code());
				}
				
				tag->read( reader, _swf );
				reader->align();
				int32_t dif = end_pos - reader->getCurrentPos();
				if( dif != 0 )
					reader->skip( dif );
				
				if( tag_header->code() == SHOWFRAME ) {	// ShowFrame
					_frames.push_back( frame_tags );
					frame_tags = new FrameTagList;
					cout << "### SHOWFRAME ###" << endl;
				} else {
					tag->print();
					frame_tags->push_back( tag );
				}
				
			} else {	// no registered factory so skip this tag
				cout << "*** SKIPPING UNKOWN TAG ***" << endl;
				tag_header->print();
				reader->skip( tag_header->length() );
			}
			
			//delete tag_header;
			//tag_header = new TagHeader();
			tag_header->read( reader );
		}
        delete frame_tags;
		_currFrame = 0;
		return true;
	}

	void DefineSpriteTag::print() {
		cout << "DEFINESPRITE: id = " << _sprite_id << " frame count = " << _frame_count << endl;
		_header.print();
		
	}
	
	void DefineSpriteTag::draw( SWF* _swf ) {
        //if ( frame < 0 || frame >= _frame_count )	return;
        int frame = _currFrame++;
        if (_currFrame >= _frame_count) _currFrame = 0;

        // build up the display list
		FrameTagList* frame_tags = _frames[ frame ];
        FrameTagList::iterator it = frame_tags->begin();
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
					_display_list.erase( remove_object->depth() );
				} break;
	
				default:
					break;
			}
            ++it;
		}
		// draw the display list
		for (DisplayListIter iter = _display_list.begin(); iter != _display_list.end(); iter++ ) {
			IPlaceObjectTag *place_obj = iter->second;
			if (NULL == place_obj) continue;
			place_obj->setOffsetTranslate( _translate );
			place_obj->setOffsetScale( _scale );
			place_obj->draw( _swf ); 
		}
	}
	

	ITag* DefineSpriteTag::create( TagHeader* header ) {
		return (ITag*)(new DefineSpriteTag( *header ));
	}
	
}