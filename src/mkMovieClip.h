
#ifndef __MOVIE_CLIP_H__
#define __MOVIE_CLIP_H__

#include "mkTag.h"

namespace MonkSWF {

    class Reader;
    class SWF;
	class Button;
	class DefineSpriteTag;
	class DefineButton2Tag;

	class MovieClip : public ICharacter {
        friend class SWF;

    protected:
		void setupFrame(const TagList& tags);

        void clearDisplayList(void);

        void createTransform(void) {
            _transform = new MATRIX;
            *_transform = kMatrixIdentity;
        }

        Button* createButton(DefineButton2Tag &tag);
        MovieClip* createMovieClip(const DefineSpriteTag &tag);

	public:
		MovieClip( SWF* swf,  const MovieFrames& data );
        virtual ~MovieClip();

		ICharacter* getInstance(const PlaceObjectTag*);

        MATRIX* getTransform(void) { return _transform; }

        void gotoLabel( const char* label, bool jump = false );

		// override ICharacter function
        virtual const RECT& getRectangle(void) const { return _data._rectangle; }
		virtual void draw(void);
		virtual void update(void);
        virtual void play( bool enable ) { 
			_play = enable; 
		}
		virtual void gotoFrame( uint32_t frame );
		virtual ICharacter* getTopMost(float localX, float localY);

		uint32_t getCurrentFrame( void ) const { return _frame; }
        uint32_t getFrameCount( void ) const { return _data._frames.size(); }

        void step( void ) { gotoFrame( this->getCurrentFrame() + 1 );  }

        bool isPlay(void) const { return _play; }

        DisplayList& getDisplayList(void) { return _display_list; }

        SWF* getSWF(void) { return _owner; }

        static bool createFrames( Reader& reader, SWF& swf, MovieFrames& );
        static void destroyFrames( MovieFrames& );
		static bool sbCalculateRectangle;

	protected:
		typedef std::map<const ITag*, ICharacter*>	CharacterCache;
		typedef std::vector< MovieClip* >			MovieClipArray;
		typedef std::vector< Button* >				ButtonArray;

		CharacterCache		_characters;
		MovieClipArray		_movies;
		ButtonArray			_buttons;

		const MovieFrames   &_data;
        SWF                 *_owner;
        MATRIX              *_transform;
        bool        _play;
		uint32_t	_frame;
		DisplayList	_display_list;
	};

}

#endif //__MOVIE_CLIP_H__
