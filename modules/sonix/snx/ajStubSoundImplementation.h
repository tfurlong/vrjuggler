/* Generated by Together */

#ifndef AJSTUBSOUNDIMPLEMENTATION_H
#define AJSTUBSOUNDIMPLEMENTATION_H
#include "ajSoundImplementation.h"
#include "ajSoundInfo.h"
class ajStubSoundImplementation : public ajSoundImplementation
{
public:
   /**
    * @input alias of the sound to trigger, and number of times to play
    * @preconditions alias does not have to be associated with a loaded sound.
    * @postconditions if it is, then the loaded sound is triggered.  if it isn't then nothing happens.
    * @semantics Triggers a sound
    */
   virtual void trigger(const std::string & alias, const unsigned int & looping = 0)
   {
      SoundImplementation::trigger( alias, looping );
      // do nothing
   }

   /**
    * @semantics stop the sound
    * @input alias of the sound to be stopped
    */
   virtual void stop(const std::string & name)
   {
      SoundImplementation::stop( name );
      // do nothing
   }

   /**
    * take a time step of [timeElapsed] seconds.
    * @semantics call once per sound frame (doesn't have to be same as your graphics frame)
    * @input time elapsed since last frame
    */
   virtual void step( const float & timeElapsed )
   {
      SoundImplementation::step( timeElapsed );
      // do nothing
   }


  /**
    * associate a name (alias) to the description
    * @preconditions provide an alias and a SoundInfo which describes the sound
    * @postconditions alias will point to loaded sound data
    * @semantics associate an alias to sound data.  later this alias can be used to operate on this sound data.
    */
   virtual void associate( const std::string& alias, const ajSoundInfo& description )
   {
      SoundImplementation::associate( alias, description );
      // do nothing
   }

   /**
    * remove alias->sounddata association 
    */
   virtual void remove( const std::string alias )
   {
      SoundImplementation::remove( alias );
      // do nothing
   }

   /**
    * set sound's 3D position 
    */
   virtual void setPosition( const std::string& alias, float x, float y, float z )
   {
      SoundImplementation::setPosition( alias, x, y, z );
   }

   /**
    * get sound's 3D position
    * @input alias is a name that has been associate()d with some sound data
    * @output x,y,z are returned in OpenGL coordinates.
    */
   virtual void getPosition( const std::string& alias, float& x, float& y, float& z )
   {
      SoundImplementation::getPosition( alias, x, y, z );
   }
private:  

   /** @link dependency */
   /*#  ajSoundInfo lnkSoundInfo; */
};
#endif //AJSTUBSOUNDIMPLEMENTATION_H