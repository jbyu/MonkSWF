#include "mkMovieClip.h"
#include "mkSWF.h"
#include "tags/PlaceObject.h"
#include "tags/DefineSprite.h"
#include "tags/DefineShape.h"
#include "tags/DefineButton.h"

namespace MonkSWF {

bool MovieClip::sbCalculateRectangle = false;

void MovieClip::destroyFrames(MovieFrames &data)
{
	// foreach frames in sprite
	FrameList::iterator it = data._frames.begin();
	while (data._frames.end() != it)
    {
		// foreach tags in frame
		TagList *_tag_list = *it;
        TagList::iterator tag_it = _tag_list->begin();
        while(_tag_list->end() != tag_it)
        {
            delete *tag_it;
            ++tag_it;
        }
		// delete the tags container
        delete *it;
        ++it;
    }
}

bool MovieClip::createFrames( Reader& reader, SWF& swf, MovieFrames &data )
{
	RECT empty = {FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX};
	data._rectangle = empty;
	sbCalculateRectangle = true;

	// get all tags and build frame lists
    TagHeader header;
	ITag* tag = NULL;
	TagList* frame_tags = new TagList;
	do {
		header.read( reader );
        const uint32_t code = header.code();
		SWF::TagFactoryFunc factory = SWF::getTagFactory( code );
		if (! factory ) {
			// no registered factory so skip this tag
			MK_TRACE("*** SKIP *** ");
			header.print();
			reader.skip( header.length() );
			continue;
		}
		tag = factory( header );
		MK_ASSERT(tag);

		int32_t end_pos = reader.getCurrentPos() + tag->length();
		bool keepTag = tag->read( reader, swf, data );
		tag->print();
		reader.align();

		int32_t dif = end_pos - reader.getCurrentPos();
		if( dif != 0 ) {
			MK_TRACE("WARNING: tag not read correctly. trying to skip.\n");
			reader.skip( dif );
		}

        if (keepTag) {
            frame_tags->push_back( tag );
        } else {
            delete tag;
        }

        // create a new frame
        if ( TAG_SHOW_FRAME == code ) {
			data._frames.push_back( frame_tags );
			frame_tags = new TagList;
			sbCalculateRectangle = false;
        }
	} while( header.code() != TAG_END );
	delete frame_tags;
    return true;
}

//-----------------------------------------------------------------------------

MovieClip::MovieClip( SWF* swf,  const MovieFrames& data )
	:_data(data)
	,_owner(swf)
	,_transform(NULL)
	,_play(true)
	,_frame(ICharacter::kFRAME_MAXIMUM)
{
	MK_TRACE("create MovieClip\n");
}

MovieClip::~MovieClip()
{
	MK_TRACE("delete MovieClip\n");
	delete _transform;
	_transform = NULL;

	// clean up
	MovieClipArray::iterator mit = _movies.begin();
	while ( _movies.end() != mit ) {
		delete (*mit);
		++mit;
	}
	ButtonArray::iterator bit = _buttons.begin();
	while ( _buttons.end() != bit ) {
		delete (*bit);
		++bit;
	}

	_characters.clear();
}

Button *MovieClip::createButton(DefineButton2Tag &tag)
{
	Button *btn = new Button( *this, tag );
	_buttons.push_back(btn);
	return btn;
}

MovieClip *MovieClip::createMovieClip(const DefineSpriteTag &sprite)
{
	MovieClip *movie = new MovieClip( _owner, sprite.getMovieFrames() );
	_movies.push_back(movie);
	return movie;
}

ICharacter *MovieClip::getInstance(const PlaceObjectTag* placeTag) 
{
	// find cache
	CharacterCache::iterator it = _characters.find(placeTag);
	if (it != _characters.end())
		return it->second;

	uint16_t character_id = placeTag->characterID();
	ITag *tag = _owner->getCharacter( character_id );
	if (! tag)
		return NULL;

	ICharacter *character = NULL;
	switch ( tag->code() ) {
	case TAG_DEFINE_SPRITE:
		character = createMovieClip( *(DefineSpriteTag*) tag);
		_characters[placeTag] = character;
		break;
	case TAG_DEFINE_BUTTON2:
		character = createButton( *(DefineButton2Tag*) tag);
		_characters[placeTag] = character;
		break;
	default:
        character = (DefineShapeTag*)tag;
		break;
	}
	return character;
}

void MovieClip::clearDisplayList(void)
{
#if 0
	DisplayList::iterator iter = _display_list.begin();
	while ( _display_list.end() != iter )
	{
    	//PlaceObjectTag *place_obj = iter->second;
       	//if (place_obj) place_obj->play(false);
		iter->second = NULL;
		++iter;
	}
#endif
	_display_list.clear();
}

void MovieClip::gotoFrame( uint32_t frame, bool skipAction )
{
	if (getFrameCount() <= frame)
	{
		frame = 0;
        clearDisplayList();
	}
    // build up the display list
	_frame = frame;
	TagList* frame_tags = _data._frames[ frame ];
	setupFrame( *frame_tags, skipAction );
}

void MovieClip::gotoLabel( const char* label)
{
    LabelList::const_iterator it = _data._labels.find(label);
    if (_data._labels.end() == it)
        return;
	gotoAndPlay( it->second );
}

void MovieClip::gotoAndPlay( uint32_t frame )
{
	if (frame < _frame) {
		gotoFrame(ICharacter::kFRAME_MAXIMUM, true);
	}
    while (frame != _frame) {
        step(1, true);
    }
}

void MovieClip::update(void)
{
    if (_play)
        gotoFrame(_frame + 1, false);

    // update the display list
	DisplayList::iterator iter = _display_list.begin();
	while ( _display_list.end() != iter )
	{
		PlaceObjectTag *place_obj = iter->second;
		if (place_obj) 
		{
			place_obj->update();
		}
		++iter;
	}
}

void MovieClip::draw(void)
{
    PlaceObjectTag *mask = NULL;
	uint16_t highest_masked_layer = 0;
	// draw the display list
	DisplayList::iterator iter = _display_list.begin();
	while ( _display_list.end() != iter )
	{
		PlaceObjectTag *place_obj = iter->second;
		if (place_obj)
        {
            const uint16_t clip = place_obj->clipDepth();
            const uint16_t depth = place_obj->depth();
			if (mask && depth > highest_masked_layer)
            {
                // restore stencil
                Renderer::getRenderer()->maskOffBegin();
                mask->draw(); 
                Renderer::getRenderer()->maskOffEnd();
    			mask = NULL;
			}
			if (0 < clip) {
                // draw mask
    			Renderer::getRenderer()->maskBegin();
                place_obj->draw(); 
    			Renderer::getRenderer()->maskEnd();
	    		highest_masked_layer = clip;
				mask = place_obj;
			} else {
                place_obj->draw(); 
            }
        }
		++iter;
	}
	if (mask)
	{
		// If a mask masks the scene all the way up to the highest layer, 
        // it will not be disabled at the end of drawing the display list, so disable it manually.
        Renderer::getRenderer()->maskOffBegin();
        mask->draw(); 
        Renderer::getRenderer()->maskOffEnd();
    	mask = NULL;
	}
}

void MovieClip::setupFrame(const TagList& tags, bool skipAction)
{
    TagList::const_iterator it = tags.begin();
	while( it != tags.end() )
    {
        ITag* tag = *it;
		tag->setup(*this, skipAction);
        ++it;
	}
}

ICharacter* MovieClip::getTopMost(float x, float y, bool polygonTest) {
	ICharacter* pRet = NULL;
	DisplayList::reverse_iterator rit = _display_list.rbegin();
	while( rit != _display_list.rend() ) {
		PlaceObjectTag* pObj = rit->second;
		MK_ASSERT(pObj);
		ICharacter* pCharacter = pObj->getCharacter();
		if (pCharacter) {
			MATRIX m;
			POINTf local, world = {x,y};
			m.setInverse(pObj->getTransform());
			m.transform(local, world);
			pRet = pCharacter->getTopMost(local.x, local.y, polygonTest);
			if (pRet)
				break;
		}
        ++rit;
	}
	return pRet;
}

}//namespace
