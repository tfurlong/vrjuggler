/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2003 by Iowa State University
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

#ifndef _GADGET_ASCENSION_MOTION_STAR_EXCEPTIONS_H_
#define _GADGET_ASCENSION_MOTION_STAR_EXCEPTIONS_H_

#include <exception>
#include <string>


namespace mstar
{

/**
 * Basic exception that may be thrown by the standalone MotionStar driver.
 */
class MotionStarException : public std::exception
{
public:
   MotionStarException(const std::string& msg) : mMessage(msg)
   {
      ;
   }

   virtual ~MotionStarException() throw()
   {
      ;
   }

   const std::string& getMessage() const
   {
      return mMessage;
   }

private:
   std::string mMessage;
};

/**
 * A generic network-related exception.
 */
class NetworkException : public MotionStarException
{
public:
   NetworkException(const std::string& msg = "") : MotionStarException(msg)
   {
   }
};

/**
 * A generic network exception caused by trying to read from a socket.
 */
class NetworkReadException : public NetworkException
{
public:
   NetworkReadException(const std::string& msg = "") : NetworkException(msg)
   {
   }
};

/**
 * No data was read from the socket.
 */
class NoDataReadException : public NetworkReadException
{
public:
   NoDataReadException(const std::string& msg = "") : NetworkReadException(msg)
   {
   }
};

/**
 * A generic network exception caused by trying to write to a socket.
 */
class NetworkWriteException : public NetworkException
{
public:
   NetworkWriteException(const std::string& msg = "") : NetworkException(msg)
   {
   }
};

/**
 * No data was written to the socket.
 */
class NoDataWrittenException : public NetworkWriteException
{
public:
   NoDataWrittenException(const std::string& msg = "")
      : NetworkWriteException(msg)
   {
   }
};

/**
 * A socket connection exception.
 */
class ConnectException : public NetworkException
{
public:
   ConnectException(const std::string& msg = "") : NetworkException(msg)
   {
   }
};

/**
 * An exception uesd when no status information could be read from a device
 * on the Fast Bird Bus (FBB).
 */
class NoDeviceStatusException : public MotionStarException
{
public:
   /**
    * @param deviceNum The device address on the FBB.
    *        msg       An optional message to associate with this exception.
    */
   NoDeviceStatusException(const unsigned int deviceNum,
                           const std::string& msg = "")
      : MotionStarException(msg), mDevNum(deviceNum)
   {
   }

   unsigned int mDevNum;
};

/**
 * An exception used when something went wrong in trying to send a network
 * command to the MotionStar chassis.
 */
class CommandException : public MotionStarException
{
public:
   CommandException(const std::string& msg = "") : MotionStarException(msg)
   {
   }
};

/**
 * An exception used when data read from the MotionStar is unexcepted or
 * otherwise invalid.
 */
class DataException : public MotionStarException
{
public:
   DataException(const std::string& msg = "") : MotionStarException(msg)
   {
   }
};

/**
 * An exception used when the scaling factor being used by the MotionStar is
 * unknown.  A special exception is created for this case because the
 * MotionStar driver depends on the MotionStar hardware to tell it what
 * scaling factor to use when reading data.  Without this information, the
 * data being read from the device is useless.
 */
class ScaleFactorUnknownException : public DataException
{
public:
   ScaleFactorUnknownException(const std::string& msg = "") : DataException(msg)
   {
   }
};

} // End of mstar namespace


#endif /* _GADGET_ASCENSION_MOTION_STAR_EXCEPTIONS_H_ */
