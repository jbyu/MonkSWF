/*
 *  mkSWF.cpp
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/20/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#include "mkSWF.h"
#include "tags/DefineShape.h"
#include "tags/PlaceObject.h"
#include "tags/ShowFrame.h"
#include "tags/RemoveObject.h"
#include "tags/DefineSprite.h"


namespace MonkSWF {
	
	SWF::SWF()
	:_offsetScale( 1 )
    ,_frame(0)
    ,_accumulator(0)
    ,mpRenderer(NULL)
	{
		_offsetTranslate[0] = 0;
		_offsetTranslate[1] = 0;
		
		for ( int i = 0; i < 9; i++ ) {
			_rootTransform[i] = 0;
		}
		// set root tranform to identity
		_rootTransform[0] = 1;
		_rootTransform[4] = 1;
		_rootTransform[8] = 1;
		
	}

	SWF::~SWF()
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


	bool SWF::initialize(Renderer *renderer) {
        mpRenderer = renderer;
		
		// setup the factories
		addFactory( ENDTAG,			EndTag::create );
		
		addFactory( DEFINESHAPE,	DefineShapeTag::create );		// DefineShape
		addFactory( DEFINESHAPE2,	DefineShapeTag::create );		// DefineShape2
		addFactory( DEFINESHAPE3,	DefineShapeTag::create );		// DefineShape3
		addFactory( DEFINESHAPE4,	DefineShapeTag::create );		// DefineShape4

		addFactory( DEFINESPRITE,	DefineSpriteTag::create );
		addFactory( PLACEOBJECT2,	PlaceObject2Tag::create );
		addFactory( REMOVEOBJECT2,	RemoveObjectTag::create );
		addFactory( SHOWFRAME,		ShowFrameTag::create );
		
		return true;
	}
	
	bool SWF::read( Reader* reader ) {
		_reader = reader;
		_header.read( reader );
		
		// get the first tag
        TagHeader oHeader;
		TagHeader *tag_header = &oHeader;//new TagHeader();
		tag_header->read( reader );
				
		// get all tags and build frame lists
		TagList* frame_tags = new TagList;
		while( tag_header->code() != 0 ) { // while not the end tag 
			ITag* tag = NULL;
			TagFactoryFunc factory = getTagFactory( tag_header->code() );
			if ( factory ) {
				tag = factory( tag_header );

				int32_t end_pos = reader->getCurrentPos() + tag->length();
				tag->read( reader, this );
				tag->print();
				reader->align();

				int32_t dif = end_pos - reader->getCurrentPos();
				if( dif != 0 ) {
					MK_TRACE("WARNING: tag not read correctly. trying to skip.\n");
					reader->skip( dif );
				}

				if( tag_header->code() == DEFINESHAPE || tag_header->code() == DEFINESHAPE2 
				   || tag_header->code() == DEFINESHAPE3 || tag_header->code() == DEFINESHAPE4 ) {
					IDefineShapeTag* shape = (IDefineShapeTag*)tag;
					addShape( shape, shape->shapeId() );
				}

				if ( tag_header->code() == DEFINESPRITE ) {
					IDefineSpriteTag* sprite = (IDefineSpriteTag*)tag;
					addSprite( sprite, sprite->spriteId() );
				}

				if ( tag_header->code() == SHOWFRAME ) {	// ShowFrame
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
				//delete tag_header;
			}
			
			//tag_header = new TagHeader();
			tag_header->read( reader );
		}

		delete frame_tags;
		return true;
	}
	
	void SWF::print() {
		_header.print();
	}
	
	void SWF::play( float delta ) {
        _accumulator += delta;
        const float secondPerFrame = getFrameRate();
        while (secondPerFrame <= _accumulator)
        {
            ++_frame;
            if (getFrameCount() <= _frame)
                _frame = 0;
            _accumulator -= secondPerFrame;
        }
    }

	void SWF::drawFrame( int32_t frame ) {
#ifdef USE_OPENVG
		// make sure we use even odd fill rule
		vgSeti( VG_FILL_RULE, VG_EVEN_ODD );
		// clear the screen
		VGfloat clearColor[] = {1,1,1,1};
		vgSetfv(VG_CLEAR_COLOR, 4, clearColor);
		vgClear(0,0,_header.getFrameWidth(), _header.getFrameHeight() );
		
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		VGfloat oldMatrix[9];
		vgGetMatrix( oldMatrix );
		vgMultMatrix( _rootTransform );
#endif
        MK_ASSERT ( 0 <= frame && frame < getFrameCount() );

        // build up the display list
		TagList* frame_tags = _frame_list[ frame ];
		setup_frame(_display_list, *frame_tags);

		DisplayList::iterator iter = _display_list.begin();
		while ( _display_list.end() != iter )
		{
			IPlaceObjectTag *place_obj = iter->second;
			if (place_obj) 
				place_obj->draw( this ); 
			++iter;
		}

#ifdef USE_OPENVG
		// restore old matrix
		vgLoadMatrix( oldMatrix );
#endif
	}
}