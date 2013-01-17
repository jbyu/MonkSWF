#ifndef __DO_ACTION_H__
#define __DO_ACTION_H__

#include "mkTag.h"

namespace MonkSWF
{
//=========================================================================
	class DoActionTag : public ITag
    {
        struct ACTION 
        {
            uint8_t code;
            uint8_t padding;
            uint16_t data;
        };
        typedef std::vector<ACTION> ActionList;
        ActionList moActions;

	public:
		DoActionTag( TagHeader& h ) 
		: ITag( h )
		{}
		
		virtual ~DoActionTag()
        {}

		virtual bool read( Reader& reader, SWF& swf, MovieFrames& data);

		virtual void print()
        {
    		_header.print();
            ActionList::iterator it = moActions.begin();
            while(moActions.end()!=it)
            {
                MK_TRACE("0x%x,", (*it).code);
                ++it;
            }
            MK_TRACE("\n");
        }

        virtual void setup(MovieClip&);

		static ITag* create( TagHeader& header ) {
			return (ITag*)(new DoActionTag( header ));
		}				
    };
}	
#endif	// __DO_ACTION_H__