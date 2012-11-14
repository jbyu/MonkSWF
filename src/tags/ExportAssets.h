#ifndef __EXPORT_ASSETS_H__
#define __EXPORT_ASSETS_H__

#include "mkTag.h"

namespace MonkSWF
{
	class ExportAssetsTag : public ITag
    {
        uint16_t count;

	public:
		ExportAssetsTag( TagHeader& h ) 
		: ITag( h )
		{}
		
		virtual ~ExportAssetsTag()
        {}

		virtual bool read( Reader* reader, SWF* swf) {
			count = reader->get<uint16_t>();
            for (uint16_t i = 0; i < count; ++i)
            {
			    uint16_t tag = reader->get<uint16_t>();
                const char *name = reader->getString();
                swf->addAsset(tag, name );
                MK_TRACE("asset[%d] %s\n", tag, name);
            }
			return true;
		}

		virtual void print() {
    		_header.print();
			//MK_TRACE("EXPORT_ASSETS:%d\n", count);
		}

		static ITag* create( TagHeader* header ) {
			return (ITag*)(new ExportAssetsTag( *header ));
		}				
    };

}	
#endif	// __EXPORT_ASSETS_H__