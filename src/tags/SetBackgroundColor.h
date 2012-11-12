#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

#include "mkTag.h"

namespace MonkSWF
{
	class SetBackgroundColorTag : public ITag {
	public:
        uint8_t r,g,b;

		SetBackgroundColorTag( TagHeader& h ) 
		: ITag( h )
		{}
		
		virtual ~SetBackgroundColorTag()
        {}
		
		virtual bool read( Reader* reader, SWF* ) {
			r = reader->get<uint8_t>();
			g = reader->get<uint8_t>();
			b = reader->get<uint8_t>();
			return true;
		}
		
		virtual void print() {
			MK_TRACE("SetBackgroundColor %d,%d,%d\n", r,g,b);
		}
		
		static ITag* create( TagHeader* header ) {
			return (ITag*)(new SetBackgroundColorTag( *header ));
		}				
	};
	
}	
#endif	// __BACKGROUND_H__