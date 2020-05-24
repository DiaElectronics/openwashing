#ifndef _LEA_OPENGLES_RENDERER_H
#define _LEA_OPENGLES_RENDERER_H
#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

#include "interfaces.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>

#include "bcm_host.h"

namespace DiaDSP {
  class OpenGLESRenderer {
    private:
      uint32_t GScreenWidth;
      uint32_t GScreenHeight;
      EGLDisplay GDisplay;
      EGLSurface GSurface;
      EGLContext DGContext;
    public: 
    int InitScreen(DiaApp::FloatPair logicalSize) {
      bcm_host_init();
	    int32_t success = 0;
	    EGLBoolean result;
	    EGLint num_config;
      
      static EGL_DISPMANX_WINDOW_T nativewindow;
      DISPMANX_ELEMENT_HANDLE_T dispman_element;
	    DISPMANX_DISPLAY_HANDLE_T dispman_display;
	    DISPMANX_UPDATE_HANDLE_T dispman_update;
	    VC_RECT_T dst_rect;
	    VC_RECT_T src_rect;

	    static const EGLint attribute_list[] =
	    {
		    EGL_RED_SIZE, 8,
		    EGL_GREEN_SIZE, 8,
		    EGL_BLUE_SIZE, 8,
		    EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,   // You need this line for depth buffering to work
		    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		    EGL_NONE
	    };

	    static const EGLint context_attributes[] = 
	    {
		    EGL_CONTEXT_CLIENT_VERSION, 2,
		    EGL_NONE
	    };
	    EGLConfig config;

      // get an EGL display connection
      GDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      assert(GDisplay!=EGL_NO_DISPLAY);
      if (glGetError()) return glGetError();

      // initialize the EGL display connection
      result = eglInitialize(GDisplay, NULL, NULL);
      assert(EGL_FALSE != result);
      if (glGetError()) return glGetError();

      // get an appropriate EGL frame buffer configuration
      result = eglChooseConfig(GDisplay, attribute_list, &config, 1, &num_config);
      assert(EGL_FALSE != result);
      if (glGetError()) return glGetError();

      // get an appropriate EGL frame buffer configuration
      result = eglBindAPI(EGL_OPENGL_ES_API);
      assert(EGL_FALSE != result);
      if (glGetError()) return glGetError();

      // create an EGL rendering context
      DGContext = eglCreateContext(GDisplay, config, EGL_NO_CONTEXT, context_attributes);
      assert(DGContext!=EGL_NO_CONTEXT);
      if (glGetError()) return glGetError();

      // create an EGL window surface
      success = graphics_get_display_size(0 /* LCD */, &GScreenWidth, &GScreenHeight);
      assert( success >= 0 );

      dst_rect.x = 0;
      dst_rect.y = 0;
      dst_rect.width = GScreenWidth;
      dst_rect.height = GScreenHeight;

      src_rect.x = 0;
      src_rect.y = 0;
      src_rect.width = GScreenWidth << 16;
      src_rect.height = GScreenHeight << 16;        

      dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
      dispman_update = vc_dispmanx_update_start( 0 );

      dispman_element = vc_dispmanx_element_add (dispman_update, dispman_display,
        0/*layer*/, &dst_rect, 0/*src*/,
        &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);

      nativewindow.element = dispman_element;
      nativewindow.width = GScreenWidth;
      nativewindow.height = GScreenHeight;
      vc_dispmanx_update_submit_sync( dispman_update );

      if (glGetError()) return glGetError();

      GSurface = eglCreateWindowSurface(GDisplay, config, &nativewindow, NULL);
      assert(GSurface != EGL_NO_SURFACE);
      if (glGetError()) return glGetError();

      // connect the context to the surface
      result = eglMakeCurrent(GDisplay, GSurface, GSurface, DGContext);
      assert(EGL_FALSE != result);
      if (glGetError()) return glGetError();

      // Set background color and clear buffers
      glClearColor(0.15f, 0.25f, 0.35f, 1.0f);
      glClear( GL_COLOR_BUFFER_BIT );

      glViewport ( 0, 0, logicalSize.X(), logicalSize.Y());

      if (glGetError()) return glGetError();
      return 0;
    }
    static int _initScreen(void *obj, DiaApp::FloatPair logicalSize) {
      OpenGLESRenderer *r = (OpenGLESRenderer *) obj;
      return r->InitScreen(logicalSize);
    }

    int DisplayImage(DiaApp::Image *img, DiaApp::FloatPair offset, DiaApp::FloatPair size) {
        return -1;
    }
    static int _displayImage(void *obj, DiaApp::Image *img, DiaApp::FloatPair offset, DiaApp::FloatPair size) {
      OpenGLESRenderer *r = (OpenGLESRenderer *) obj;
      return r->DisplayImage(img, offset, size);
    }

    void SwapFrame() {
        eglSwapBuffers(GDisplay,GSurface);
    }
    static void _swapFrame(void *obj) {
      OpenGLESRenderer *r = (OpenGLESRenderer *) obj;
      r->SwapFrame();
    }

    DiaApp::Renderer ToAppRenderer() {
      DiaApp::Renderer renderer(this, _initScreen, _displayImage, _swapFrame);
      return renderer;
    }        
  };
}
#endif
