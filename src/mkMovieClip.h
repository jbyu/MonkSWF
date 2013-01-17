
#ifndef __MOVIE_CLIP_H__
#define __MOVIE_CLIP_H__

#include "mkTag.h"

namespace MonkSWF {

    class Reader;
    class SWF;

	class MovieClip {
        friend class PlaceObjectTag;
        friend class SWF;

    protected:
        void setupFrame(const TagList& tags);
        void clearDisplayList(void);
		void update(void);

        void createTransform(void)
        {
            _transform = new MATRIX;
            *_transform = kMatrixIdentity;
        }

	public:
		MovieClip( const MovieFrames& data )
            :_data(data)
            ,_owner(NULL)
            ,_transform(NULL)
            ,_play(true)
		    ,_frame(0xffffffff)
        {}

		MovieClip( SWF* swf,  const MovieFrames& data )
            :_data(data)
            ,_owner(swf)
            ,_transform(NULL)
            ,_play(true)
		    ,_frame(0xffffffff)
        {}

        virtual ~MovieClip() { delete _transform; }

        MATRIX* getTransform(void) { return _transform; }

        void play( bool enable ) { _play = enable; }

		void gotoFrame( uint32_t frame );

        void gotoLabel( const char* label, bool jump = false )
        {
            LabelList::const_iterator it = _data._labels.find(label);
            if (_data._labels.end() == it)
                return;
            if (jump)
            {
                gotoFrame(it->second);
                return;
            }
            while (it->second != _frame)
            {
                step();
            }
        }

		virtual void draw(void);

		uint32_t getCurrentFrame( void ) const { return _frame; }
        uint32_t getFrameCount( void ) const { return _data._frames.size(); }

        void step( void ) { gotoFrame( this->getCurrentFrame() + 1 );  }

        bool isPlay(void) const { return _play; }

        DisplayList& getDisplayList(void) { return _display_list; }

        const RECT& getRectangle(void) const { return _data._rectangle; }

        SWF* getSWF(void) { return _owner; }

        static bool createFrames( Reader& reader, SWF& swf, MovieFrames& );
        static void destroyFrames( MovieFrames& );

	private:
		const MovieFrames   &_data;
        SWF                 *_owner;
        MATRIX              *_transform;
        bool        _play;
		uint32_t	_frame;
		DisplayList	_display_list;
	};

}

#endif //__MOVIE_CLIP_H__
