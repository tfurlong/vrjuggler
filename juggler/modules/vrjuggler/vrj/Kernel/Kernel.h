#ifndef _VJ_KERNEL_
#define _VJ_KERNEL_
#pragma once

#include <vjConfig.h>
#include <Kernel/vjApp.h>
#include <Kernel/vjSystemFactory.h>
//#include <Kernel/vjAPIFactory.h>
#include <Kernel/vjDisplayManager.h>
#include <Kernel/vjDrawManager.h>
#include <Threads/vjThread.h>
#include <Kernel/vjDebug.h>
#include <Input/InputManager/vjInputManager.h>
#include <Kernel/vjSystemData.h>
#include <Environment/vjEnvironmentManager.h>
#include <Performance/vjPerfDataBuffer.h>

#include <Kernel/vjUser.h>

	// --- HACK --- //
	// Is this really needed??
#include <Kernel/vjSGISystemFactory.h>

    // Config stuff
#include <Config/vjConfigChunkDB.h>


//-------------------------------------------------------
//: Main control class for all vj applications.
//
//	Takes care of all initialization and object creation
//	for the system.
//	This class is the only class that MUST be instantiated
//	for all applications.
//
// @author  Allen Bierbaum
//
// Date: 9-7-97
//-------------------------------------------------------
//!PUBLIC_API:
class vjKernel
{
public:

   //: Start the Kernel running
   int start();

   //: Load initial configuration data for the managers
   //! POST: Managers set up
   //+       Config files loaded and handed to configAdd
   void initConfig();

   //: The Kernel loop
   void controlLoop(void* nullParam);

   // Set the application object for the Kernel to deal with
   //  If there is another app active, it has to stop that
   //  application first then restart all API specific Managers.
   void setApplication(vjApp* _app);

   // Stops the current application but leaves the kernel running.
   // It closes all API specific stuff (DrawManager,  etc.)
   int stopApplication()
   { return 1; }

   /* Load configuration data for Kernel
    * POST: Config data has been read in
    *       All Managers are configured
    */
   void loadConfigFile(std::string filename);

public:  // --- Config interface --- //
   //: Get the chunks that are actually running in the system
   vjConfigChunkDB* getChunkDB()
   {
      vjASSERT(NULL != mChunkDB);
      return mChunkDB;
   }

   //: Get the initial chunk database
   // Returns the chunks that were originally used to configure
   // the system.
   //! NOTE: This may contain chunks that are not currently running.
   vjConfigChunkDB* getInitialChunkDB()
   {
      vjASSERT(NULL != mInitialChunkDB);
      return mInitialChunkDB;
   }

      // Add a group of config chunks
   void configAdd(vjConfigChunkDB* chunkDB);

      // Remove a group of config chunks
   void configRemove(vjConfigChunkDB* chunkDB);

protected:     // Config chunks local to kernel
   bool configKernelHandle(vjConfigChunk* chunk);
   bool configKernelAdd(vjConfigChunk* chunk);
   bool configKernelRemove(vjConfigChunk* chunk);

protected:
   //: Updates any data that needs updated once a frame (Trackers, etc.)
   //! POST: All tracker data is ready for next frame
   void updateFrameData();

protected:      // --- STARTUP ROUTINES --- //
         // --- Manager Initial setup functions ---- //
   void initialSetupInputManager();
   void setupEnvironmentManager();
   void initialSetupDisplayManager();
   void initialSetupDrawManager();

   // Starts the drwa manager running
   // Calls the app callbacks for the draw manager
   void startDrawManager();

   // Stop the draw manager and close it's resources
   void stopDrawManager()
   {;}

public:      // Global "get" interface

      //: Get the system Factory
   vjSystemFactory* getSysFactory()
   { return sysFactory; }

      //: Get the input manager
   vjInputManager* getInputManager()
   { return mInputManager; }

   //: Get the user associated with given name
   //! RETURNS: NULL - Not found
   vjUser*  getUser(std::string userName);

   //: Get a list of the users back
   std::vector<vjUser*> getUsers()
   { return mUsers; }

private:
   vjSystemData    data;   //: Global system data

protected:

   vjApp*      app;                        //: The app object
   vjThread*   mControlThread;             //: The thread in control of me.

   /// Factories and Managers
   vjSystemFactory*  sysFactory;          //: The current System factory
   vjInputManager*   mInputManager;       //: The input manager for the system
   vjDrawManager*    drawManager;         //: The Draw Manager we are currently using
   vjDisplayManager* displayManager;      //: The Display Manager we are currently using
   vjEnvironmentManager* environmentManager; //: The Environment Manager object

   /// Performance information
   vjPerfDataBuffer* perfBuffer;          //: store perfdata for kernel main
   bool performanceEnabled;               //: is performance measurement for kernel active?

   /// Config Stuff
   vjChunkDescDB*    mConfigDesc;
   vjConfigChunkDB*  mChunkDB;            //: The current chunk db for the system
   vjConfigChunkDB*  mInitialChunkDB;     //: Initial chunks added to system before it is started

   /// Shared Memory stuff
   vjMemPool*       sharedMemPool;

   /// Multi-user information
   std::vector<vjUser*>   mUsers;         //: A list of user objects in system

   // ----------------------- //
   // --- SINGLETON STUFF --- //
   // ----------------------- //
protected:
   //: Constructor:  Hidden, so no instantiation is allowed
   vjKernel()
   {
      app = NULL;
      mInitialChunkDB = NULL;
      mConfigDesc = NULL;
   }

public:
   //: Get instance of singleton object
   static vjKernel* instance()
   {
      if (_instance == NULL)
         _instance = new vjKernel;
      return _instance;
   }

private:
   static vjKernel* _instance;   //: The instance
};


#endif
