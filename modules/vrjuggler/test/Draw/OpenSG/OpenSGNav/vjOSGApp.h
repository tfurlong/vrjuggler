#ifndef _VJ_OSG_APP__
#define _VJ_OSG_APP__

#include <iostream>
#include <iomanip>

#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGRenderAction.h>
#include <OpenSG/OSGPassiveWindow.h>

#include <vrj/vrjConfig.h>

#include <vrj/Draw/OGL/GlApp.h>

namespace vrj
{

class /*VJ_CLASS_API*/ OpenSGApp : public vrj::GlApp
{
  public:
    OpenSGApp(vrj::Kernel* kern) : GlApp(kern) { }

    virtual ~OpenSGApp() { }

    virtual void initScene() = 0;
    virtual OSG::RenderAction * getAction() = 0;

    virtual void draw();

    // Initialize
    // Make sure to call initScene if you override this function
    virtual void init()
    {
        std::cout << "vjOSGApp::init called\n";
        // initScene();
    }

    /*
    virtual void contextInit()    { }

    virtual void contextClose()   { }

    virtual void contextPreDraw() { }

    virtual void bufferPreDraw()  { }

    virtual void pipePreDraw()    { }
    */

  protected:
    OSG::PassiveWindowPtr            _win; //the renderer!!!

};

inline void OpenSGApp::draw()
{
   std::cout << "vjOSGApp::draw called\n";

    /*
    std::cout << "vjOSGApp::draw called\n";
    _win->render(getAction());
    */
}

};

#endif
