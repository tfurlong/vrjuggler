/* Generated by Together */

#ifndef AUDIOJUGGLER_H
#define AUDIOJUGGLER_H
#include <string>
#include "ajSoundInfo.h"
#include "IAudioJuggler.h"
#include "ajSoundFactory.h"
#include "ajSoundImplementation.h"
#include "ajSoundAPIInfo.h"

class AudioJuggler : public IAudioJuggler
{
public:

   AudioJuggler() : IAudioJuggler(), mImplementation( NULL )
   {
   }

   virtual ~AudioJuggler()
   {
      // release the implementation
      if (mImplementation != NULL)
      {
         // unload all sound data
         mImplementation->unbindAll();
         
         // shutdown old api if exists
         mImplementation->shutdownAPI();
         delete mImplementation;
         mImplementation = NULL;
      }
   }

   /**
    * @input alias of the sound to trigger, and number of times to play
    * @preconditions alias does not have to be associated with a loaded sound.
    * @postconditions if it is, then the loaded sound is triggered.  if it isn't then nothing happens.
    * @semantics Triggers a sound
    */
   virtual void trigger( const std::string& alias, const unsigned int& repeat = 1 )
   {
      this->impl().trigger( alias, repeat );
   }

   /*
    * when sound is already playing then you call trigger,
    * does the sound restart from beginning?
    * (if a tree falls and no one is around to hear it, does it make sound?)
    */
   virtual void setRetriggerable( const std::string& alias, bool onOff )
   {
      this->impl().setRetriggerable( alias, onOff );
   }

   /**
    * ambient or positional sound.
    * is the sound ambient - attached to the listener, doesn't change volume
    * when listener moves...
    * or is the sound positional - changes volume as listener nears or retreats..
    */
   void setAmbient( const std::string& alias, bool setting = false )
   {
      this->impl().setAmbient( alias, setting );
   }

   /**
    * @semantics stop the sound
    * @input alias of the sound to be stopped
    */
   virtual void stop( const std::string& name )
   {
      this->impl().stop( name );
   }

   /**
    * pause the sound, use unpause to return playback where you left off...
    */
   virtual void pause( const std::string& alias )
   {
      this->impl().pause( alias );
   }

   /**
    * resume playback from a paused state.  does nothing if sound was not paused.
    */
   virtual void unpause( const std::string& alias )
   {
      this->impl().unpause( alias );
   }

   /**
    * mute, sound continues to play, but you can't hear it...
    */
   virtual void mute( const std::string& alias )
   {
      this->impl().mute( alias );
   }

   /**
    * unmute, let the muted-playing sound be heard again
    */
   virtual void unmute( const std::string& alias )
   {
      this->impl().unmute( alias );
   }

   /**
    * set sound's 3D position 
    * @input x,y,z are in OpenGL coordinates.  alias is a name that has been associate()d with some sound data
    */
   virtual void setPosition( const std::string& alias, const float& x, const float& y, const float& z )
   {
      this->impl().setPosition( alias, x, y, z );
   }

   /**
    * get sound's 3D position
    * @input alias is a name that has been associate()d with some sound data
    * @output x,y,z are returned in OpenGL coordinates.
    */
   virtual void getPosition( const std::string& alias, float& x, float& y, float& z )
   {
      this->impl().getPosition( alias, x, y, z );
   }

   /**
    * set the position of the listener
    */
   virtual void setListenerPosition( const vjMatrix& mat )
   {
      this->impl().setListenerPosition( mat );
   }

   /**
    * get the position of the listener
    */
   virtual void getListenerPosition( vjMatrix& mat ) const
   {
      this->impl().getListenerPosition( mat );
   }


   /**
    * change the underlying sound API to something else.
    * @input usually a name of a valid registered sound API implementation
    * @preconditions sound implementation should be registered
    * @postconditions underlying API is changed to the requested API name.   if apiName's implementation is not registered, then underlying API is changed to the stub version.
    * @semantics function is safe: always returns a valid implementation.
    * @time O(1)
    * @output a valid sound API.  if apiName is invalid, then a stub implementation is returned.
    */
   virtual void changeAPI( const std::string& apiName )
   {
      ajSoundImplementation* oldImpl = mImplementation;
      ajSoundFactory::createImplementation( apiName, mImplementation );

      // copy sound state (doesn't do binding here)
      mImplementation->copy( *oldImpl );

      if (oldImpl != NULL)
      {
         // unload all sound data
         oldImpl->unbindAll();
         
         // shutdown old api if exists
         oldImpl->shutdownAPI();
         delete oldImpl;
         oldImpl = NULL;
      }

      // startup the new API
      mImplementation->startAPI();

      // load all sound data
      mImplementation->bindAll();
   }

   /*
    * configure the sound API global settings
    */
   virtual void configure( const ajSoundAPIInfo& sai )
   {
      this->impl().configure( sai );
   }   

   /**
     * configure/reconfigure a sound
     * configure: associate a name (alias) to the description if not already done
     * reconfigure: change properties of the sound to the descriptino provided.
     * @preconditions provide an alias and a SoundInfo which describes the sound
     * @postconditions alias will point to loaded sound data
     * @semantics associate an alias to sound data.  later this alias can be used to operate on this sound data.
     */
   virtual void configure( const std::string& alias, const ajSoundInfo& description )
   {
      this->impl().associate( alias, description );
   }   

   /**
     * remove a configured sound, any future reference to the alias will not
     * cause an error, but will not result in a rendered sound
     */
   virtual void remove( const std::string alias )
   {
      this->impl().remove( alias );
   }   

   /**
     * @semantics call once per sound frame (doesn't have to be same as your graphics frame)
     * @input time elapsed since last frame
     */
   virtual void step( const float& timeElapsed )
   {
      this->impl().step( timeElapsed );
   }
   
protected:
   ajSoundImplementation& impl()
   {
      if (mImplementation == NULL)
      {
         ajSoundFactory::createImplementation( "stub", mImplementation );
         mImplementation->startAPI();
         mImplementation->bindAll();
      }
      return *mImplementation;
   }
private:
   /** @link dependency */
   /*#  ajSoundFactory lnkSoundFactory; */

   /** @link aggregation 
    * @clientCardinality 1
    * @supplierCardinality 1*/
   ajSoundImplementation* mImplementation;
        
   /** AudioJuggler API includes objects of this type
    * @link dependency */
   /*#  ajSoundInfo lnkSoundInfo; */

   /** @link dependency */
   /*#  ajSoundAPIInfo lnkajSoundAPIInfo; */
};
#endif //AUDIOJUGGLER_H
