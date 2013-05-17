// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "vld.h"
#endif//_DEBUG

#include <stdint.h>
#include <GL/freeglut.h>
#include <map>
#include <string>
#include <time.h>
#include "SOIL.h"
#include "mkSWF.h"

MonkSWF::SWF *gpSWF = NULL;
unsigned int suDefaultTexture = 0;
const GLuint kStencilMask = 0xffffffff;

typedef std::map<std::string, unsigned int> AssetCache;

#define USE_FMOD
#ifdef  USE_FMOD
#include "fmod.hpp"
#include "fmod_errors.h"

FMOD::System *gpFMOD = NULL;

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}

class fmodSpeaker : public MonkSWF::Speaker
{
    AssetCache moCache;

public:
    ~fmodSpeaker()
    {
        FMOD_RESULT result;
        AssetCache::iterator it = moCache.begin();
        while(moCache.end() != it)
        {
            FMOD::Sound *sound = (FMOD::Sound *)it->second;
            result = sound->release();
            ERRCHECK(result);
            ++it;
        }
    }

    virtual unsigned int getSound( const char *filename )
    {
        AssetCache::iterator it = moCache.find(filename);
        if (moCache.end()!=it)
            return it->second;

        FMOD_RESULT result;
        FMOD::Sound *sound;
        result = gpFMOD->createSound(filename, FMOD_HARDWARE, 0, &sound);
        ERRCHECK(result);
        unsigned int ret = (unsigned int)sound;
        moCache[filename] = ret;
        return ret;
    }
    virtual void playSound( unsigned int sound, bool loop, bool stop, bool noMultiple )
    {
        if (0 == sound)
            return;
        FMOD::Channel *channel = 0;
        FMOD_RESULT result;
        result = gpFMOD->playSound(FMOD_CHANNEL_FREE, (FMOD::Sound *)sound, false, &channel);
        ERRCHECK(result);
    }
};

#endif//USE_FMOD

//=========================================================================
uint32_t myLoadAssetCallback( const char *name, bool import )
{
    if (import)
        return 0;
    if (strstr(name,".png"))
    {
        return MonkSWF::Renderer::getRenderer()->getTexture(name);
    }
    else if (strstr(name,".wav"))
    {
        return MonkSWF::Speaker::getSpeaker()->getSound(name);
    }
    return 0;
}

//=========================================================================
class glRenderer : public MonkSWF::Renderer
{
    typedef std::map<std::string, GLuint> TextureCache;
    TextureCache moCache;
    int miMaskLevel;

public:
    glRenderer():miMaskLevel(0)
    {
    }
    ~glRenderer()
    {
        TextureCache::iterator it = moCache.begin();
        while(moCache.end() != it)
        {
            GLuint id = it->second;
            glDeleteTextures(1, &id);
            ++it;
        }
    }

	virtual void createTempTexAssets(void) {}
	virtual void destroyTempTexAssets(void) {}
    
	void maskBegin(void)
	{
		if (0 == miMaskLevel)
		    glEnable(GL_STENCIL_TEST);
		// we set the stencil buffer to 'm_mask_level+1' 
		// where we draw any polygon and stencil buffer is 'm_mask_level'
		glStencilFunc(GL_EQUAL, miMaskLevel++, kStencilMask);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); 
		glColorMask(0, 0, 0, 0);
	}

	void maskEnd(void)
	{	     
		// we draw only where the stencil is m_mask_level (where the current mask was drawn)
		glStencilFunc(GL_EQUAL, miMaskLevel, kStencilMask);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);	
		glColorMask(1, 1, 1, 1);
	}

	void maskOffBegin(void)
	{	     
		MK_ASSERT(0 < miMaskLevel);
		// we set the stencil buffer back to 'm_mask_level' 
		// where the stencil buffer m_mask_level + 1
		glStencilFunc(GL_EQUAL, miMaskLevel--, kStencilMask);
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR); 
		glColorMask(0, 0, 0, 0);
	}
	void maskOffEnd(void)
	{	     
		maskEnd();
		if (0 == miMaskLevel)
			glDisable(GL_STENCIL_TEST); 
	}

    void applyTransform( const MonkSWF::MATRIX3f& mtx )
    {
        // convert mtx33 to mtx44, also negate the y axis
        GLfloat m[16];
        m[0] = mtx.f[0];
        m[1] = mtx.f[1];
        m[2] = mtx.f[2];
        m[3] = 0;
        m[4] = mtx.f[3];
        m[5] = mtx.f[4];
        m[6] = mtx.f[5];
        m[7] = 0;
        m[8] = 0;
        m[9] = 0;
        m[10]= 1;
        m[11]= 0;
        m[12]= mtx.f[6];
        m[13]= mtx.f[7];
        m[14]= mtx.f[8];
        m[15]= 1;
        glLoadMatrixf(m);
    }

#if 1
    void applyPassMult( const MonkSWF::CXFORM& cxform  )
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &cxform.mult.r);
        glColor4f(cxform.add.r, cxform.add.g, cxform.add.b, cxform.mult.a);
    }
    void applyPassAdd( const MonkSWF::CXFORM& cxform  )
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor4f(cxform.add.r, cxform.add.g, cxform.add.b, 1);
    }
#else
    void applyPassMult( const MonkSWF::CXFORM& cxform )
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        float r = cxform.mult.r + cxform.add.r;
        float g = cxform.mult.g + cxform.add.g;
        float b = cxform.mult.b + cxform.add.b;
        glColor4f(r, g, b, cxform.mult.a);
    }
    void applyPassAdd( const MonkSWF::CXFORM& cxform )
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
        glColor4f(cxform.add.r, cxform.add.g, cxform.add.b, 1);
    }
#endif
    void drawQuad( const MonkSWF::RECT& rect, const MonkSWF::CXFORM& cxform, unsigned int texture )
    {
        if (0 != texture)
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture);
        }
        // C' = C * Mult + Add
        // in opengl, use blend mode and multi-pass to achieve that
        // 1st pass TexEnv(GL_BLEND) with glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        //      Cp * (1-Ct) + Cc *Ct 
        // 2nd pass TexEnv(GL_MODULATE) with glBlendFunc(GL_SRC_ALPHA, GL_ONE)
        //      dest + Cp * Ct
        // let Mult as Cc and Add as Cp, then we get the result
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &cxform.mult.r);
        glColor4f(cxform.add.r, cxform.add.g, cxform.add.b, cxform.mult.a);
        glBegin(GL_QUADS);
		    glTexCoord2f(0,0);
            glVertex2f(rect.xmin, rect.ymin);

		    glTexCoord2f(1,0);
            glVertex2f(rect.xmax, rect.ymin);

    	    glTexCoord2f(1,1);
            glVertex2f(rect.xmax, rect.ymax);

		    glTexCoord2f(0,1);
            glVertex2f(rect.xmin, rect.ymax);
	    glEnd();
#if 1
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor4f(cxform.add.r, cxform.add.g, cxform.add.b, 1);
        glBegin(GL_QUADS);
		    glTexCoord2f(0,0);
            glVertex2f(rect.xmin, rect.ymin);

		    glTexCoord2f(1,0);
            glVertex2f(rect.xmax, rect.ymin);

    	    glTexCoord2f(1,1);
            glVertex2f(rect.xmax, rect.ymax);

		    glTexCoord2f(0,1);
            glVertex2f(rect.xmin, rect.ymax);
	    glEnd();
#endif
    }

    void drawImportAsset( const MonkSWF::RECT& rect, const MonkSWF::CXFORM& cxform, unsigned int handle )
    {
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glColor4f(1, 0, 0, 1);
		    glTexCoord2f(0,0);
            glVertex2f(rect.xmin, rect.ymin);

        glColor4f(0, 1, 0, 1);
		    glTexCoord2f(1,0);
            glVertex2f(rect.xmax, rect.ymin);

        glColor4f(0, 0, 1, 1);
    	    glTexCoord2f(1,1);
            glVertex2f(rect.xmax, rect.ymax);

        glColor4f(1, 1, 0, 1);
		    glTexCoord2f(0,1);
            glVertex2f(rect.xmin, rect.ymax);
	    glEnd();
    }
    
    void drawBegin(void)
    {
	glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
	glLoadIdentity();
    }
    void drawEnd(void)
    {
	glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    }

    unsigned int getTexture( const char *filename )
    {
        unsigned int ret = 0;
        //char path[256];
        //sprintf_s(path, 256, "%s.png", filename);
        TextureCache::iterator it = moCache.find(filename);
        if (moCache.end()!=it)
            return it->second;

        ret = SOIL_load_OGL_texture(
					filename,
					SOIL_LOAD_AUTO,
					SOIL_CREATE_NEW_ID,
					SOIL_FLAG_POWER_OF_TWO
					| SOIL_FLAG_MIPMAPS
					//| SOIL_FLAG_MULTIPLY_ALPHA
					//| SOIL_FLAG_COMPRESS_TO_DXT
					//| SOIL_FLAG_DDS_LOAD_DIRECT
					//| SOIL_FLAG_NTSC_SAFE_RGB
					//| SOIL_FLAG_CoCg_Y
					//| SOIL_FLAG_TEXTURE_RECTANGLE
					);
        moCache[filename] = ret;
        MonkSWF::Log("SOIL: %s\n", SOIL_last_result());
        return ret;
    }

};

//=========================================================================
void _terminate_(void)
{
#ifdef  USE_FMOD
    delete MonkSWF::Speaker::getSpeaker();
    FMOD_RESULT result;
    result = gpFMOD->close();
    ERRCHECK(result);
    result = gpFMOD->release();
    ERRCHECK(result);
#endif
    glDeleteTextures(1, &suDefaultTexture);
    delete MonkSWF::Renderer::getRenderer();
	delete gpSWF;
    gpSWF = NULL;
}

//Called to update the display.
//You should call glutSwapBuffers after all of your rendering to display what you rendered.
//If you need continuous updates of the screen, call glutPostRedisplay() at the end of the function.
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    if (gpSWF) 
    {
        const MonkSWF::COLOR4f& color = gpSWF->getBackgroundColor();
        const float w = gpSWF->getFrameWidth();
        const float h = gpSWF->getFrameHeight();
        glDisable(GL_TEXTURE_2D);
   	    glColor4f(color.r, color.g, color.b, color.a);
	    glBegin(GL_QUADS);
	    glVertex2f(0, 0);
	    glVertex2f(w, 0);
	    glVertex2f(w, h);
	    glVertex2f(0, h);
	    glEnd();
        glBindTexture(GL_TEXTURE_2D, suDefaultTexture);
        glEnable(GL_TEXTURE_2D);
        gpSWF->draw();
        //gpSWF->drawDuplicate();
    }

	glutSwapBuffers();
}

//Called whenever the window is resized. The new window size is given, in pixels.
//This is an opportunity to call glViewport or glScissor to keep up with the change in size.
void reshape (int w, int h)
{
    /*
    if (gpSWF)
    {
        const SWF_RECT& rect = gpSWF->GetRect();
        w = rect.m_dwXMax - rect.m_dwXMin;
        h = rect.m_dwYMax - rect.m_dwYMin;
    	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
    	gluOrtho2D(rect.m_dwXMin, rect.m_dwXMax, rect.m_dwYMin, rect.m_dwYMax);
        return;
    }
    */
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, h, 0);
}

static bool gbUpdate = true;

const int kMilliSecondPerFrame = 16;
static clock_t siLastTick = 0;

void Timer(int)
{
	clock_t tick = clock();
	float delta = float(tick - siLastTick) *0.001f;
	siLastTick = tick;

    if (gbUpdate)
    {
        if (gpSWF)
        {
            gpSWF->update(delta);
            //gpSWF->updateDuplicate(delta);
        }
    }
#ifdef  USE_FMOD
    if (gpFMOD)
        gpFMOD->update();
#endif
	glutPostRedisplay();
	glutTimerFunc(kMilliSecondPerFrame, Timer, 0);
}

//Called whenever a key on the keyboard was pressed.
//The key is given by the ''key'' parameter, which is in ASCII.
//It's often a good idea to have the escape key (ASCII value 27) call glutLeaveMainLoop() to 
//exit the program.
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
    case 's':
        if (gpSWF) 
        {
            gpSWF->gotoFrame(0xffffffff);
            gpSWF->play(true);
        }
        break;
    case 'q':
        if (gpSWF) 
            gpSWF->gotoLabel("end");
        break;
    case 'z':
        if (gpSWF) {
            gpSWF->step();
            printf("frame[%d]\n",gpSWF->getCurrentFrame());
        }
        break;
    case 32:
        gbUpdate ^= 1;
        break;
	case 27:
		glutLeaveMainLoop();
		break;
	}
}

static int siLastButtonStatus = 0;

void motionCB( int x, int y) {
	if (gpSWF) {
		gpSWF->notifyMouse(siLastButtonStatus, x, y);
		//gpSWF->notifyMouse(siLastButtonStatus, 100, 200);
	}
}

void mouseCB(int button,int stat,int x,int y) {
	siLastButtonStatus = (GLUT_DOWN == stat)?1:0;
	motionCB(x,y);
}

void myURLCallback( MonkSWF::MovieClip&, bool isFSCommand, const char *url, const char *target )
{
	printf("fs:%d, url:%s, targt:%s\n", isFSCommand, url, target);
}

int _tmain(int argc, char* argv[])
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); 
    if (argc < 2)
		return 0;

	// load swf
    char *filename = argv[1];
    FILE *fp = fopen(filename,"rb");
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *pBuffer = new char[size];
    fread(pBuffer,size,1,fp);
    fclose(fp);
    MonkSWF::Reader reader(pBuffer, size);
    MonkSWF::SWF::initialize(myLoadAssetCallback);
    gpSWF = new MonkSWF::SWF;

	// trim unsupported tags
	if (2 < argc && 0 == strcmp(argv[2], "--trim")) {
		char output[1024];
		if (3 < argc) {
			strcpy(output, argv[3]);
		} else {
			strcpy(output, filename);
			//char *ptr = strrchr(output, '.');
			//strcpy(ptr,".nbwf");
		}
		gpSWF->trimSkippedTags(output, reader);
	    delete [] pBuffer;
		delete [] gpSWF;
		return 1;
	}

    atexit(_terminate_);
    glutInit(&argc, argv);

	int width =  640;
	int height = 480;

	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL | GLUT_DOUBLE);
	glutInitWindowSize (width, height); 
	glutInitWindowPosition (256, 32);
	glutCreateWindow (argv[0]);
	glutSetWindowTitle("swfView ver0.1");

	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc( mouseCB );
	glutPassiveMotionFunc( motionCB );
	glutTimerFunc(kMilliSecondPerFrame, Timer, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glDepthMask(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat colorFactor[4]={0};
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colorFactor);

    unsigned char temp[] = {255,0,0, 0,255,0, 0,0,255, 255,255,0 };
    suDefaultTexture = SOIL_create_OGL_texture (temp,2,2,SOIL_LOAD_RGB,SOIL_CREATE_NEW_ID,0);
    glBindTexture(GL_TEXTURE_2D, suDefaultTexture);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glEnable(GL_TEXTURE_2D);

#ifdef  USE_FMOD
    // initial fmod audio library
    FMOD_RESULT result;
    result = FMOD::System_Create(&gpFMOD);
    ERRCHECK(result);
    result = gpFMOD->init(32, FMOD_INIT_NORMAL, 0);
    ERRCHECK(result);
    MonkSWF::Speaker::setSpeaker(new fmodSpeaker);
#endif

#if 1
    MonkSWF::Renderer::setRenderer(new glRenderer);
	gpSWF->setGetURL( myURLCallback );
    bool ret = gpSWF->read(reader);
    delete [] pBuffer;

    if (ret) {
        width = (int)gpSWF->getFrameWidth();
        height = (int)gpSWF->getFrameHeight();
        glutReshapeWindow(width,height);

        MonkSWF::MovieClip * clip = gpSWF->duplicate("npc_animalA");
        if (clip)
        {
            const MonkSWF::RECT& rect = clip->getRectangle();
            clip->getTransform()->tx = 100;
            clip->getTransform()->ty = 200;
        }
    } else {
        MessageBox(NULL, "Not Support Compressed SWF","Eror Format", MB_OK);
        delete gpSWF;
        gpSWF = NULL;
    }
#endif

	siLastTick = clock();
	glutMainLoop();

	return 0;
}
