#include "mkMovieClip.h"
#include "mkSWF.h"
#include "tags/PlaceObject.h"

namespace MonkSWF {

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
		ITag* tag = NULL;
		// get the first tag
        TagHeader header;
		header.read( reader );
		// get all tags and build frame lists
		TagList* frame_tags = new TagList;
		while( header.code() != TAG_END ) { // while not the end tag 
            const uint32_t code = header.code();
			SWF::TagFactoryFunc factory = SWF::getTagFactory( code );
			if ( factory ) {
				tag = factory( header );

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
                if ( TAG_SHOW_FRAME == code )
                {
					data._frames.push_back( frame_tags );
					frame_tags = new TagList;
                }
			} else { // no registered factory so skip this tag
				MK_TRACE("*** SKIP *** ");
				header.print();
				reader.skip( header.length() );
			}
			header.read( reader );
		}
		delete frame_tags;
        return true;
    }

    void MovieClip::clearDisplayList(void)
    {
		DisplayList::iterator iter = _display_list.begin();
		while ( _display_list.end() != iter )
		{
    		//PlaceObjectTag *place_obj = iter->second;
       		//if (place_obj) place_obj->play(false);
			iter->second = NULL;
			++iter;
		}
    }

	void MovieClip::gotoFrame( uint32_t frame )
	{
		if (getFrameCount() <= frame)
		{
			frame = 0;
            clearDisplayList();
		}
        // build up the display list
		_frame = frame;
		TagList* frame_tags = _data._frames[ frame ];
		setupFrame( *frame_tags );
	}

    void MovieClip::update(void)
    {
        if (_play)
            gotoFrame(_frame + 1);

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

	void MovieClip::setupFrame(const TagList& tags)
	{
        TagList::const_iterator it = tags.begin();
		while( it != tags.end() )
        {
            ITag* tag = *it;
            tag->setup(*this);
            ++it;
		}
	}

}//namespace
