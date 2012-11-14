#ifndef __DO_ACTION_H__
#define __DO_ACTION_H__

#include "mkTag.h"

namespace MonkSWF
{
//=========================================================================
	class DoActionTag : public ITag
    {
        std::vector<uint8_t> moActions;

	public:
		DoActionTag( TagHeader& h ) 
		: ITag( h )
		{}
		
		virtual ~DoActionTag()
        {}

		virtual bool read( Reader* reader, SWF* swf);

        virtual bool process(SWF* swf ) { return true; }

        virtual void setup(MovieClip&);

		virtual void print() {
    		_header.print();
			//MK_TRACE("id=%d\n", _soundId);
		}

		static ITag* create( TagHeader* header ) {
			return (ITag*)(new DoActionTag( *header ));
		}				
    };
}	
#endif	// __DO_ACTION_H__