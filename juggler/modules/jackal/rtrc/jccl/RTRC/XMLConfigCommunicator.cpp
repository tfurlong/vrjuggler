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

#include <jccl/RTRC/XMLConfigCommunicator.h>
#include <jccl/RTRC/ConfigManager.h>
#include <jccl/RTRC/ConfigCommand.h>
#include <jccl/Net/Connect.h>
#include <jccl/Net/JackalServer.h>
#include <jccl/XMLUtil/XercesXMLParserPool.h>
#include <jccl/Config/ConfigIO.h>
#include <jccl/Config/XMLConfigIOHandler.h>
#include <jccl/Config/ChunkFactory.h>


// a totally cheezy kludge to avoid asserts becasue of some disagreement
// between xerces & jackal/juggler/etc.  Doing this causes the loader code
// to leak memory like a sieve.
//  #ifdef VPR_OS_Win32

//  #define delete[] //

//  #endif




namespace jccl
{


XMLConfigCommunicator::XMLConfigCommunicator (ConfigManager* _config_manager):
   NetCommunicator()
{

   vprASSERT (_config_manager != 0);

   mConfigManager = _config_manager;
   
   mConfigIOHandler = (XMLConfigIOHandler*)ConfigIO::instance()->getHandler ();
   mXMLParser = XercesXMLParserPool::instance()->getParser();
}


/*virtual*/ XMLConfigCommunicator::~XMLConfigCommunicator ()
{
   ConfigIO::instance()->releaseHandler (mConfigIOHandler);
   XercesXMLParserPool::instance()->releaseParser (mXMLParser);
}


/*virtual*/ void XMLConfigCommunicator::initConnection(Connect* _connection)
{
   NetCommunicator::initConnection (_connection);
}


/*virtual*/ void XMLConfigCommunicator::shutdownConnection()
{
   NetCommunicator::shutdownConnection();
}


/*virtual*/ bool XMLConfigCommunicator::acceptsStreamIdentifier (const std::string& id)
{
   return !strcasecmp (id.c_str(), "xml_config");
}


/*virtual*/ bool XMLConfigCommunicator::readStream (Connect* con, std::istream& instream, const std::string& id)
{
   DOM_Node doc;
   bool retval = mXMLParser->readStream (instream, doc);
   if (retval)
   {
      retval = interpretDOM_Node (con, doc);
   }
   return retval;
}


bool XMLConfigCommunicator::interpretDOM_Node (Connect* con, DOM_Node& doc)
{
   bool retval = true;
   DOMString node_name = doc.getNodeName();
   DOMString node_value = doc.getNodeValue();
   char* name = node_name.transcode();
   DOM_Node child;
   DOM_NamedNodeMap attributes;
//     int attrCount;
//     int i;
   //ConfigChunk* ch = 0;
   ConfigChunkDB newchunkdb;
    //      cout << "ok, we've got a node named '" << name << "' with a value of '" << node_value << "'." << endl;
    
   switch (doc.getNodeType())
   {
      case DOM_Node::DOCUMENT_NODE:
         //cout << "document node..." << endl;
         child = doc.getFirstChild();
         while (child != 0)
         {
            retval = retval && (interpretDOM_Node(con, child));
            child = child.getNextSibling();
         }
         break;
      case DOM_Node::ELEMENT_NODE:
         //cout << "command is '" << name << "'." << endl;
         if (!strcasecmp (name, "apply_chunks"))
         {
            // we've received a set of configchunks to apply.
            newchunkdb.clear();
            child = doc.getFirstChild();
            while (child != 0)
            {
               retval = retval && mConfigIOHandler->buildChunkDB
                  (newchunkdb, child);
               child = child.getNextSibling();
            }
            if (retval)
            {
               mConfigManager->addPendingAdds(&newchunkdb);
            }
         }
         else if (!strcasecmp (name, "remove_chunks"))
         {
            child = doc.getFirstChild();
            while (child != 0)
            {
               // i really want to just send names of chunks, but right now
               // the ConfigManager actually needs a full chunkdb :(
               retval =
                  retval && mConfigIOHandler->buildChunkDB (newchunkdb, child);
               child = child.getNextSibling();
            }
            if (retval)
            {
               mConfigManager->addPendingRemoves (&newchunkdb);
            }
         }
         else if (!strcasecmp (name, "remove_descs"))
         {
            // that could be dangerous, so we quietly refuse to honor
            // this request.
         }
         else if (!strcasecmp (name, "request_current_chunks"))
         {
            mConfigManager->lockActive();
            ConfigChunkDB* db = new ConfigChunkDB((*(mConfigManager->getActiveConfig())));   // Make a copy
            mConfigManager->unlockActive();
            
            //vprDEBUG(jcclDBG_SERVER,4) << "Connect: Sending (requested) chunkdb.\n" << vprDEBUG_FLUSH;
            //vprDEBUG(jcclDBG_SERVER,5) << *db << std::endl << vprDEBUG_FLUSH;
            con->addCommand (new CommandSendChunkDB (db, true));
         }
         else if (!strcasecmp (name, "request_current_descs"))
         {
            ChunkDescDB* db = ChunkFactory::instance()->getChunkDescDB();
            vprDEBUG(jcclDBG_SERVER,4) << "Connect: Sending (requested) chunkdesc.\n" << vprDEBUG_FLUSH;
            vprDEBUG(jcclDBG_SERVER,5) << *db << std::endl << vprDEBUG_FLUSH;
            con->addCommand (new CommandSendDescDB (db));
         }
         else
         {
            vprDEBUG (jcclDBG_SERVER,0) << "Connect: Unrecognized command: '"
                                        << name << "'\n" << vprDEBUG_FLUSH;
         }
         break; // ELEMENT_NODE
      default:
         //cout << "confused" << endl;
         break;
   }
   return retval;
} // interpretDOM_Node ();


//----------------------- ConfigStatus Stuff ----------------------------
    
void XMLConfigCommunicator::configChanged ()
{
   mConfigManager->lockActive();
   ConfigChunkDB* db = new ConfigChunkDB(*(mConfigManager->getActiveConfig()));   // Make a copy
   mConfigManager->unlockActive();
   
   std::vector<Connect*>& connections 
      = JackalServer::instance()->getConnections();
   for (unsigned int i = 0, n = connections.size(); i < n; i++) 
   {
      if (connections[i]->getConnectMode() == INTERACTIVE_CONNECT) 
      {
         connections[i]->addCommand (new CommandSendChunkDB (db, true));
      }
   }
   JackalServer::instance()->releaseConnections();
}


};
