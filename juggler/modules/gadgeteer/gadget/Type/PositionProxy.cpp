#include <vjConfig.h>
#include <Input/InputManager/vjPosProxy.h>
#include <Kernel/vjKernel.h>


//: Set the transform for this vjPosProxy
// Sets the transformation matrix to
//    mMatrixTransform = M<sub>trans</sub>.post(M<sub>rot</sub>)
//! NOTE: This means that to set transform, you specific the translation
//+       followed by rotation that takes the device from where it physically
//+       is in space to where you want it to be.
void vjPosProxy::SetTransform( float xoff, float yoff, float zoff,    // Translate
                   float xrot, float yrot, float zrot)   // Rotate
{
   etrans = true;
   if((xoff != 0.0f) || (yoff != 0.0f) || (zoff != 0.0f))
      m_matrixTransform.makeTrans(xoff, yoff, zoff);
   if((xrot != 0.0f) || (yrot != 0.0f) || (xrot != 0.0f))
      m_matrixTransform.postXYZEuler(m_matrixTransform, xrot, yrot, zrot);
}

//: Set the vjPosProxy to now point to another device and subDevicenumber
void vjPosProxy::Set(vjPosition* posPtr, int unitNum, int useTransform)
{
   vjASSERT( posPtr->FDeviceSupport(DEVICE_POSITION) );
   vjDEBUG(0) << "posPtr: " << posPtr << endl
              << "unit  : " << unitNum << endl << endl << vjDEBUG_FLUSH;
   m_posPtr = posPtr;
   m_unitNum = unitNum;
   etrans = useTransform;
}


bool vjPosProxy::config(vjConfigChunk* chunk)
{
   vjDEBUG_BEGIN(1) << "------------------ POS PROXY -----------------\n"
                    << "   config()" << endl << vjDEBUG_FLUSH;
   vjASSERT(((string)(char*)chunk->getType()) == "PosProxy");

   int unitNum = chunk->getProperty("unit");
   string proxy_name = (char*)chunk->getProperty("name");
   string dev_name = (char*)chunk->getProperty("device");

   if (1 == (int)chunk->getProperty("etrans") )
   {
      vjDEBUG(2) << "Position Transform enabled..." << endl << vjDEBUG_FLUSH;
      SetTransform
      ( chunk->getProperty("translate",0) , // xtrans
        chunk->getProperty("translate",1) , // ytrans
        chunk->getProperty("translate",2) , // ztrans
        chunk->getProperty("rotate",0) , // xrot
        chunk->getProperty("rotate",1) , // yrot
        chunk->getProperty("rotate",2) );// zrot
      vjDEBUG(2) << "Transform Matrix: " << endl << GetTransform() << endl << vjDEBUG_FLUSH;
   }

   int proxy_num = vjKernel::instance()->getInputManager()->AddPosProxy(dev_name,unitNum,proxy_name,this);

   if ( proxy_num != -1)
   {
      vjDEBUG_END(1) << "   PosProxy config()'ed" << endl << vjDEBUG_FLUSH;
      return true;
   }
   else
   {
      vjDEBUG_END(1) << "   PosProxy config() failed" << endl << vjDEBUG_FLUSH;
      return false;
   }
}

