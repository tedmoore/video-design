#ifndef FBO_RENDERER_H
#define FBO_RENDERER_H

#include "ofMain.h"

class FboRenderer
{
public:
    ofFbo fbo;
    ofPixels pix;

    void allocate(int width, int height, int internalformat = GL_RGBA)
    {
        fbo.allocate(width, height, internalformat);
        pix.allocate(width, height, internalformat);
    }

    void begin()
    {
        fbo.begin();
    }

    void end()
    {
        fbo.end();
    }

    void draw(int x, int y)
    {
        fbo.draw(x, y);
    }

    void draw(int x, int y, int w, int h)
    {
        fbo.draw(x, y, w, h);
    }
    
    void write(string path){
        fbo.readToPixels(pix);
        ofSaveImage(pix, path, OF_IMAGE_QUALITY_BEST);
    }

    int getWidth()
    {
        return fbo.getWidth();
    }

    int getHeight()
    {
        return fbo.getHeight();
    }
};

#endif // FBO_RENDERER_H