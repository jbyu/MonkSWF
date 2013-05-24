#include "DoAction.h"
#include "mkSWF.h"

namespace MonkSWF
{
    enum ACTION_CODE
    {
        //SWF 3 action model
        ACTION_NEXT_FRAME       = 0x04,
        ACTION_PREV_FRAME       = 0x05,
        ACTION_PLAY             = 0x06,
        ACTION_STOP             = 0x07,
        ACTION_TOGGLE_QUALITY   = 0x08,
        ACTION_STOP_SOUNDS      = 0x09,
        ACTION_GOTO_FRAME       = 0x81,
        ACTION_GET_URL          = 0x83,
        ACTION_WAIT_FOR_FRAME   = 0x8A,
        ACTION_SET_TARGET       = 0x8B,
        ACTION_GOTO_LABEL       = 0x8C,
    };

	TagHeader DoActionTag::scButtonHeader;

	DoActionTag::~DoActionTag() {
		ActionArray::iterator it = moActions.begin();
		while(moActions.end() != it) {
			delete [] it->buffer;
			it->buffer = NULL;
			++it;
		}
	}

    bool DoActionTag::read( Reader& reader, SWF& swf, MovieFrames&)
	{
        uint8_t code;
        do {
			code = reader.get<uint8_t>();
            ACTION action = {code, 0, 0, NULL};
    		if (code & 0x80) {
				// Action contains extra data.
				uint16_t length = reader.get<uint16_t>();
                uint16_t read = 0;
				switch(code) {
				case ACTION_GOTO_FRAME:
                    action.data = reader.get<uint16_t>();
                    read = 2;
					break;
				case ACTION_GOTO_LABEL:
					{
					int pos = reader.getCurrentPos();
					const char *label = reader.getString();
					action.data = strlen(label) + 1;
					action.buffer = new char[action.data];
					memcpy(action.buffer, label, action.data);
                    read =  reader.getCurrentPos() - pos;
					}
					break;
				case ACTION_GET_URL: 
					{
                    // extract parameters
					int pos = reader.getCurrentPos();
					const char* url = reader.getString();
					const char *target = reader.getString();
					if (strncmp("FSCommand:", url, 10) == 0) {
						action.padding = 1;
						url += 10;
					}
					action.data = strlen(url) + 1;
					const int paramSize = strlen(target) + 1;
					action.buffer = new char[action.data + paramSize];
					memcpy(action.buffer, url, action.data);
					memcpy(action.buffer + action.data, target, paramSize);
					read = reader.getCurrentPos() - pos;
					}
					break;
				default:
					break;
                }
                reader.skip(length - read);
			}
            moActions.push_back(action);
        } while(0 != code);
		return true; // keep tag
	}

    void DoActionTag::setup(MovieClip& movie)
    {
        ActionArray::iterator it = moActions.begin();
        while(moActions.end() != it) {
            const ACTION& action = (*it);
            switch(action.code) {
            case ACTION_PLAY:
                movie.play(true);
                break;
            case ACTION_STOP:
                movie.play(false);
                break;
            case ACTION_GOTO_FRAME:
                //movie.gotoAndPlay(action.data);
				movie.gotoFrame(action.data);
                break;
            case ACTION_GOTO_LABEL:
				movie.gotoLabel(action.buffer, false);
                break;
            case ACTION_GET_URL:
                {
                    SWF* swf = movie.getSWF();
                    if (! swf) break;
                    SWF::GetURLCallback callback = swf->getGetURL();
                    if (! callback) break;
					const char *url = action.buffer;
					const char *target = action.buffer + action.data;
					callback( movie, 0<action.padding, url, target );
                }
                break;
            case ACTION_NEXT_FRAME:
                movie.step();
                break;
            case ACTION_PREV_FRAME:
                movie.step(-1);
                break;

            default:
                break;
            }
            ++it;
        }
    }

}//namespace
