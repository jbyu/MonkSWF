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
#include "tags/SetBackgroundColor.h"

namespace MonkSWF {
	
	SWF::SWF()
	:_elapsedAccumulator(0.0f)
    ,mpRenderer(NULL)
	,mpMovieClip(NULL)
	{
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
			// delete container
            delete *it;
            ++it;
        }
		// foreach movieclip in sprite
		MovieList::iterator mit = _movie_list.begin();
		while (_movie_list.end() != mit)
        {
            delete *mit;
            ++mit;
        }
		// swf movie
		delete mpMovieClip;
		mpMovieClip = NULL;
    }

	MovieClip *SWF::createMovieClip(const IDefineSpriteTag &tag)
	{
		const DefineSpriteTag& spriteImpl = (const DefineSpriteTag&) tag;
		MovieClip *movie = new MovieClip( spriteImpl.getFrameList() );
		_movie_list.push_back(movie);
		return movie;
	}


	bool SWF::initialize(Renderer *renderer) {
        mpRenderer = renderer;
		
		// setup the factories
		addFactory( TAG_END,			EndTag::create );
		
		addFactory( TAG_DEFINE_SHAPE,	DefineShapeTag::create );		// DefineShape
		addFactory( TAG_DEFINE_SHAPE2,	DefineShapeTag::create );		// DefineShape2
		addFactory( TAG_DEFINE_SHAPE3,	DefineShapeTag::create );		// DefineShape3
		addFactory( TAG_DEFINE_SHAPE4,	DefineShapeTag::create );		// DefineShape4

		addFactory( TAG_DEFINE_SPRITE,	DefineSpriteTag::create );
		addFactory( TAG_PLACE_OBJECT2,	PlaceObject2Tag::create );
		addFactory( TAG_REMOVE_OBJECT2,	RemoveObjectTag::create );
		addFactory( TAG_SHOW_FRAME,		ShowFrameTag::create );
		
		addFactory( TAG_SET_BACKGROUND_COLOR, SetBackgroundColorTag::create );

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
            const uint32_t code = tag_header->code();
			TagFactoryFunc factory = getTagFactory( code );
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

                switch(code)
                {
                case TAG_SET_BACKGROUND_COLOR:
                    {
					SetBackgroundColorTag* bg = (SetBackgroundColorTag*)tag;
                    _bgColor.r = bg->r / 255.f;
                    _bgColor.g = bg->g / 255.f;
                    _bgColor.b = bg->b / 255.f;
                    _bgColor.a = 1.f;
                    } break;
                case TAG_DEFINE_SHAPE:
                case TAG_DEFINE_SHAPE2:
                case TAG_DEFINE_SHAPE3:
                case TAG_DEFINE_SHAPE4:
                    {
					IDefineShapeTag* shape = (IDefineShapeTag*)tag;
					addShape( shape, shape->shapeId() );
				    } break;
                case TAG_DEFINE_SPRITE:
                    {
					IDefineSpriteTag* sprite = (IDefineSpriteTag*)tag;
					addSprite( sprite, sprite->spriteId() );
				    } break;
                }
                // ShowFrame
				if ( TAG_SHOW_FRAME == code ) {
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
		MK_ASSERT(_frame_list.size() == getFrameCount());
		mpMovieClip = new MovieClip( _frame_list );
		mpMovieClip->setFrame( 0 );
		return true;
	}
	
	void SWF::print()
	{
		_header.print();
	}
	
	void SWF::play( float delta )
	{
        _elapsedAccumulator += delta;
        const float secondPerFrame = getFrameRate();
        while (secondPerFrame <= _elapsedAccumulator)
        {
            _elapsedAccumulator -= secondPerFrame;
			mpMovieClip->setFrame( mpMovieClip->getFrame() + 1 );
		}
    }

	void SWF::step( void )
	{
		mpMovieClip->setFrame( mpMovieClip->getFrame() + 1 );
    }

	int SWF::getFrame( void ) const
	{
        return mpMovieClip->getFrame();
    }


	void SWF::draw(void) {
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

		mpMovieClip->draw( this );

#ifdef USE_OPENVG
		// restore old matrix
		vgLoadMatrix( oldMatrix );
#endif
	}
}