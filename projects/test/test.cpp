// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif//_DEBUG

#include <stdint.h>
#include <GL/freeglut.h>
#include <map>
#include <string>
#include <time.h>
#include "SOIL.h"

#if 1
#include "mkSWF.h"

MonkSWF::SWF *gpSWF = NULL;
float sfSWF_HEIGHT = 0;

class glRenderer : public MonkSWF::Renderer
{
    typedef std::map<std::string, GLuint> TextureCache;
    TextureCache moCache;

public:
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

    void applyTexture( unsigned int texture )
    {
        if (0 != texture)
        {
            glBindTexture(GL_TEXTURE_2D, texture);
        }
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
    void drawQuad( const MonkSWF::RECT& rect, const MonkSWF::CXFORM& cxform  )
    {
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
    }

    unsigned int getTexture( const char *filename )
    {
        unsigned int ret = 0;
        char path[256];
        sprintf(path, "%s.png", filename);
        TextureCache::iterator it = moCache.find(path);
        if (moCache.end()!=it)
            return it->second;

        ret = SOIL_load_OGL_texture(
					path,
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
        moCache[path] = ret;
        return ret;
    }

};


void _terminate_(void)
{
    delete gpSWF->GetRenderer();
	delete gpSWF;
    gpSWF = NULL;
}

#endif

//Called to update the display.
//You should call glutSwapBuffers after all of your rendering to display what you rendered.
//If you need continuous updates of the screen, call glutPostRedisplay() at the end of the function.
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    if (gpSWF) gpSWF->draw();

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

//Called whenever a key on the keyboard was pressed.
//The key is given by the ''key'' parameter, which is in ASCII.
//It's often a good idea to have the escape key (ASCII value 27) call glutLeaveMainLoop() to 
//exit the program.
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
    case 'z':
        if (gpSWF) {
            gpSWF->step();
            printf("frame[%d]\n",gpSWF->getFrame());
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

const int kMilliSecondPerFrame = 16;
static clock_t siLastTick = 0;

void Timer(int)
{
	clock_t tick = clock();
	float delta = float(tick - siLastTick) *0.001f;
	siLastTick = tick;

    if(gbUpdate)
    {
        if (gpSWF)
            gpSWF->play(delta);
    }

	glutPostRedisplay();
	glutTimerFunc(kMilliSecondPerFrame, Timer, 0);
}

int _tmain(int argc, char* argv[])
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); 
    if (argc < 2) return 0;

    atexit(_terminate_);

    glutInit(&argc, argv);

	int width =  960;
	int height = 960;
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize (width, height); 
	glutInitWindowPosition (256, 32);
	glutCreateWindow (argv[0]);
	glutSetWindowTitle("swfView ver0.1");

	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(kMilliSecondPerFrame, Timer, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glDepthMask(false);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat colorFactor[4]={0};
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colorFactor);

#if 1
    char *filename = argv[1];
    FILE *fp = fopen(filename,"rb");
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *pBuffer = new char[size];
    fread(pBuffer,size,1,fp);
    fclose(fp);
    MonkSWF::Reader reader(pBuffer, size);
    gpSWF = new MonkSWF::SWF;
    gpSWF->initialize(new glRenderer);
    gpSWF->read(&reader);
    sfSWF_HEIGHT = gpSWF->getFrameHeight();
    delete [] pBuffer;

    const MonkSWF::COLOR4f& color = gpSWF->getBackgroundColor();
   	glClearColor(color.r, color.g, color.b, color.a);
#endif

	siLastTick = clock();
	glutMainLoop();

	return 0;
}
