/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#include <gadget/gadgetConfig.h>
#include <gadget/Devices/Sim/SimAnalog.h>
#include <jccl/Config/ConfigChunk.h>

namespace gadget
{

//: Default Constructor
SimAnalog::SimAnalog() : Analog(), SimInput()
{
   //vprDEBUG(vprDBG_ALL,4)<<"*** SimAnalog::SimPinchGlove()\n"<< vprDEBUG_FLUSH;
}

//: Destructor
SimAnalog::~SimAnalog()
{
   //vprDEBUG(vprDBG_ALL,4)<<"*** SimAnalog::~SimPinchGlove()\n"<< vprDEBUG_FLUSH;
}

bool SimAnalog::config(jccl::ConfigChunkPtr chunk)
{
   //vprDEBUG(vprDBG_ALL,4)<<"*** SimAnalog::config()\n"<< vprDEBUG_FLUSH;
   if(! (Input::config(chunk) && Analog::config(chunk) && SimInput::config(chunk)))
      return false;

   std::vector<jccl::VarValue*> key_inc_list = chunk->getAllProperties("keyPairsInc");
   std::vector<jccl::VarValue*> key_dec_list = chunk->getAllProperties("keyPairsDec");

   mSimKeysUp = readKeyList(key_inc_list);
   mSimKeysDown = readKeyList(key_dec_list);

   int num_pairs = mSimKeysUp.size();

   mAnaData = std::vector<AnalogData>( num_pairs ); // Initialize to all zeros
   mAnaStep = chunk->getProperty( "anastep" );

   return true;
}

void SimAnalog::updateData()
{
   //vprDEBUG(vprDBG_ALL,4)<<"*** SimAnalog::updateData()\n"<< vprDEBUG_FLUSH;

   // -- Update analog data --- //
   for (unsigned int i = 0; i < mSimKeysUp.size(); i++)
   {
      mAnaData[i] = (float)mAnaData[i] + (float)checkKeyPair(mSimKeysUp[i]) * mAnaStep;
      mAnaData[i] = (float)mAnaData[i] - (float)checkKeyPair(mSimKeysDown[i]) * mAnaStep;

      if ((float)mAnaData[i] < 0.0f) mAnaData[i] = 0.0f;
      if ((float)mAnaData[i] > 255.0f) mAnaData[i] = 255.0f;

      float f;
      this->normalizeMinToMax( (float)mAnaData[i], f );
      mAnaData[i] = f;
   }
}

};
