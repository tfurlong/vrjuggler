#ifndef _VJ_GLX_WIN_H
#define _VJ_GLX_WIN_H
#pragma once

/* vjGLXWindow.h
 *
 */

#include <vjConfig.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#include <Kernel/GL/vjGlWindow.h>
#include <Kernel/vjKernel.h>
#include <Kernel/vjDebug.h>

// this simple motif-esqe definition was taken from GLUT
typedef struct {
#define MWM_HINTS_DECORATIONS   2
  long flags;
  long functions;
  long decorations;
  long input_mode;
} MotifWmHints;



//------------------------------------
//: A GLX specific glWindow
//------------------------------------
// Has all information specific
// to dealing with a GLX window
// in OpenGL
//------------------------------------
class vjGlxWindow: public vjGlWindow
{

public:
   vjGlxWindow() {
      window_is_open = 0;
      window_width = window_height = -1;
   }

   void swapBuffers() {
      glXSwapBuffers (x_display,  x_window);
   }


   int open() {
      /* attempts to open the glxWindow & create the gl context.  Does nothing
       * if the window is already open (& returns true).
       * returns true for success, false for failure.
       * The newly opened window will be set as the calling proccess'
       * current gl context.
       */

      XEvent fooevent;
      XSetWindowAttributes w_attrib;
      int screen;

      vjDEBUG(2) << "glxWindow: Open window" << endl << vjDEBUG_FLUSH;

      if (window_is_open)
         return true;

      if (window_width == -1)
         vjDEBUG(1) << "ERROR: Window has not been configured\n" << vjDEBUG_FLUSH;

      if (! (x_display = XOpenDisplay (display_name)))
      {
         vjDEBUG(0) << "Unable to open display " << display_name << endl << vjDEBUG_FLUSH;
         return false;
      }

      screen = DefaultScreen (x_display);

      // get an XVisualInfo*, which we'll need below
      if ((visual_info = GetGlxVisInfo (x_display, screen)) == NULL)
      {
         vjDEBUG(0) << "glXChooseVisual failed\n" << flush << vjDEBUG_FLUSH;
         return false;
      }

      // window attributes.
      if ((w_attrib.colormap = XCreateColormap (x_display,
                                                RootWindow(x_display, screen),
                                                visual_info->visual,
                                                AllocNone)) == NULL)
      {
         vjDEBUG(0) << "ERROR: XCreateColorMap failed on " << display_name << endl << vjDEBUG_FLUSH;
         return false;
      }
      w_attrib.event_mask = ExposureMask | StructureNotifyMask
                            | PointerMotionMask | KeyPressMask;
      w_attrib.border_pixel = 0x0;

      // create window
      if ((x_window = XCreateWindow (x_display, RootWindow(x_display, screen),
                                     origin_x, origin_y - window_height,
                                     window_width, window_height,
                                     0, visual_info->depth,
                                     InputOutput,
                                     visual_info->visual,
                                     CWEventMask | CWColormap | CWBorderPixel,
                                     /* ^--attrib mask*/
                                     &w_attrib))  /* Attributes */
          == NULL)
      {
         vjDEBUG(0) << "ERROR: Couldn't create window for " << display_name << endl << vjDEBUG_FLUSH;
         return false;
      }

      /* Before we map the window, we need a name for it (this is also useful for
       * the resource cruft that'll get rid of the borders).
       */
      XClassHint *classhint = XAllocClassHint();
      classhint->res_name = window_name;
      classhint->res_class = "vj GLX";
      XSetClassHint (x_display, x_window, classhint);
      XFree (classhint);


      /* Get rid of window border, if configured to do so.
       * This technique doesn't require any modifications to the .XDefaults file
       * or anything, but it will only work with window managers based on MWM
       * (the Motif window manager).  That covers most cases.
       * Unfortunately, the generic X resources for communicating with a window
       * manager don't support this feature.
       */
      if (!border) {
	cout << "attempting to make window borderless" << endl;
	Atom vjMotifHints = XInternAtom (x_display, "_MOTIF_WM_HINTS", 0);
	if (vjMotifHints == None) {
	  vjDEBUG(2) << "ERROR: Could not get X atom for _MOTIF_WM_HINTS." << endl << vjDEBUG_FLUSH;
	}
	else {
	  MotifWmHints hints;
	  hints.flags = MWM_HINTS_DECORATIONS;
	  hints.decorations = 0;
	  XChangeProperty(x_display, x_window,
			  vjMotifHints, vjMotifHints, 32,
			  PropModeReplace, (unsigned char *) &hints, 4);
	}
      }
	/* END_BORDERLESS_STUFF*/


      /* The next few lines of crud are needed to get it through the window
       * manager's skull that yes, we DO have a REASON for wanting the window
       * to be positioned WHERE WE TOLD YOU TO, and not where YOU think it should
       * go, thank you very much, I'M the APPLICATION so stop $%^*&%#@! SECOND
       * GUESSING ME!
       */
      XSizeHints *sizehints = XAllocSizeHints();
      sizehints->flags = USPosition;
      XSetWMNormalHints (x_display, x_window, sizehints);
      XFree (sizehints);

      /* Now that we've straightened that out with a minimum of bloodshed,
       * we can actually map the window.  The XIfEvent lets us wait until
       * it's actually on screen.
       */

      XMapWindow (x_display, x_window);
      XIfEvent (x_display, &fooevent, EventIsMapNotify, (XPointer)x_window);

      vjDEBUG(1) << "glxcWindow: done map" << endl << vjDEBUG_FLUSH;

      if (! (glx_context = glXCreateContext (x_display,
                                             visual_info, NULL, True)))
      {
         vjDEBUG(0) << "ERROR: Couldn't create GlxContext for " << display_name << vjDEBUG_FLUSH
         << endl;
         return false;
      }

      if (!glXMakeCurrent ( x_display, x_window, glx_context  ))
      {
         vjDEBUG(0) << "ERROR: Couldn't set GlxContext for " << display_name << endl << vjDEBUG_FLUSH;
         return false;
      }

      /* allen found a hint online that if we have the controlling process
       * "disown" the gl context we just created, we could then have another
       * process pick it up and use it.  However, it doesn't seem to have
       * worked for us.
       */
      // glXMakeCurrent (x_display,0,0);

      window_is_open = 1;

      /*
      glEnable (GL_DEPTH_TEST);
      glClearDepth (1.0);
      glFlush();
      */
      return true;
   }


   int close() {
      /* closes the window.
       * always returns true
       */
      /* note: this function mucks with the current rendering context */

      if (!window_is_open)
         return true;

      window_is_open = false;

      if (glx_context)
      {
         makeCurrent();
         glXDestroyContext (x_display,  glx_context);
         glFlush();
         glx_context = NULL;
      }
      if (x_window)
      {
         XDestroyWindow (x_display, x_window);
         x_window = NULL;
      }
      if (visual_info)
      {
         XFree (visual_info);
         visual_info = NULL;
      }
      if (x_display)
      {
         XCloseDisplay (x_display);
         x_display = NULL;
      }

      return true;

   } /* close() */


   bool makeCurrent() {
      /* returns true for success,
       * false for failure (eg window not open or glXMakeCurrent fails)
       */
      if (!window_is_open)
         return false;

      return glXMakeCurrent ( x_display, x_window, glx_context  );
   }

   void config(vjDisplay* _display) {
      vjGlWindow::config(_display);

/*    try_stereo = false;
    border = true;
    display_name = ":0.0";
    window_width = 400;
    window_height = 400;
*/

       // Get the vector of display chunks
      std::vector<vjConfigChunk*>* dispSysChunk;
      dispSysChunk = vjKernel::instance()->getChunkDB()->getMatching("displaySystem");

      window_name = display->getName();
      pipe = display->pipe();

      vjConfigChunk *cfg = display->configChunk();
      display_name = (*dispSysChunk)[0]->getProperty("xpipes", pipe);
      if(strcmp(display_name, "-1") == 0)    // Use display env
         strcpy(display_name, getenv("DISPLAY"));
      vjDEBUG(0) << "glxWindow display name is: " << display_name << endl << vjDEBUG_FLUSH;
      try_stereo = display->inStereo();
   }


public:  /**** Static Helpers *****/
   /* static */ virtual bool createHardwareSwapGroup(std::vector<vjGlWindow*> wins)
   {
      // Convert to glx windows
      std::vector<vjGlxWindow*> glx_wins;

      if(wins.size() <= 0)
         vjDEBUG(0) << "WARNING: createHardwareSwapGroup called with no windows\n" << vjDEBUG_FLUSH;

      for(int i=0;i<wins.size();i++)
      {
         vjGlxWindow* glx_win = dynamic_cast<vjGlxWindow*>(wins[i]);
         vjASSERT(glx_win != NULL);    // Make sure we have the right type
         glx_wins.push_back(glx_win);
      }

      // Create hardware group
#ifdef VJ_OS_SGI
      for(i=0;i<glx_wins.size();i++)      // For each window
      {
         if(glx_wins[i] != this)    // If not me
         {                          //then add with me to the swap group
            glXJoinSwapGroupSGIX(x_display, x_window, glx_wins[i]->x_window);
         }
      }

#else
      vjDEBUG(0) << "WARNING: createHardwareSwapGroup not supported.\n" << vjDEBUG_FLUSH;
#endif

   }

private:

   Display         *x_display;
   XVisualInfo     *visual_info;
   GLXContext      glx_context;
   Window          x_window;
   char            *window_name;
   int             pipe;

   /* private member functions.  these get profoundly painful */

   XVisualInfo *GetGlxVisInfo (Display *display, int screen) {
      /* pre:  screen is a screen on the current XDisplay, and
       *       XDisplay is already defined and valid.
       * post: returns a pointer to an XVisualInfo to be used for GLX.
       *       Note that it doesn't necessarily have _all_ of the requested
       *       features... it will attempt to get a visual in mono or
       *       without alpha if it's virst attempts fail.
       *       Returns NULL if it can't come up with a reasonably close
       *       XVisualInfo, or if the display in question doesn't support
       *       GLX
       */


      XVisualInfo *vi;
      const int MaxAttributes = 32;
      int viattrib[MaxAttributes] = {
         GLX_DOUBLEBUFFER,
         GLX_RGBA,
         GLX_DEPTH_SIZE, 1,
         GLX_RED_SIZE, 1,
         GLX_GREEN_SIZE, 1,
         GLX_BLUE_SIZE, 1,
         GLX_ALPHA_SIZE, 1,
      };
      /* Notes on viattrib:  by using 1 for GLX_RED_SIZE et.al. we ask
       * for the _largest_ available buffers.  If this fails,  we might
       * want to try setting alpha size to 0 (smallest possible, maybe 0)
       * which is required eg. for alpha on the indys.
       *
       * Implementation note: the code below makes assumptions about the
       * exact order of the arguments in viattrib.  Alter those, & you'll
       * Need to redo the indices used.
       */
      int NumAttribs = 12;

      if (!glXQueryExtension (display, NULL, NULL))
      {
         vjDEBUG(0) << "ERROR: Display "<< display_name <<
         "doesn't support GLX.\n  Aborting.\n" <<flush << vjDEBUG_FLUSH;
         return NULL;
      }

      if (try_stereo)
      {
         viattrib[NumAttribs++] = GLX_STEREO;
         viattrib[NumAttribs] = None;
         in_stereo = true;
      }
      else
         in_stereo = false;


      // first, see if we can get exactly what we want.
      if (vi = glXChooseVisual (display, screen, viattrib))
         return vi;

      // still no luck. if we were going for stereo, let's try without.
      if (try_stereo)
      {
         vjDEBUG(0) << "WARNING: Display process for " << display_name
                    << "\n  Couldn't get display in stereo."
                    << "\n  Trying mono.\n" << vjDEBUG_FLUSH;
         in_stereo = false;
         viattrib[12] = GLX_USE_GL; // should be a reasonable 'ignore' tag
         if (vi = glXChooseVisual (display, screen, viattrib))
            return vi;
      }

      // if we reach here, we didn't.  Maybe we should make alpha optional.
      vjDEBUG(0) << "WARNING: Display process for " << display_name
                 << "\n  Couldn't get display with alpha channel."
                 << "\n  Trying without." << endl << vjDEBUG_FLUSH;
      viattrib[11] = 0;
      if (vi = glXChooseVisual (display, screen, viattrib))
         return vi;

      // But they told me to please go f___ myself
      // You know you just can't win.  -d. gilmour
      return NULL;
   }

   //!PRE:  window is an xwindow under display
   //!POST: returns true if e is a mapnotify event for window, else false
   //!NOTE: this is a utility function for InitGfx,  used to wait
   //+       until a window has actually been mapped.
   static int EventIsMapNotify (Display *display,  XEvent *e,  XPointer window) {

      return ((e->type == MapNotify) && (e->xmap.window == (Window)window));
   }

};

#endif






