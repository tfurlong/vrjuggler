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

#include <cluster/Plugins/PluginConfig.h>

#include <cluster/Plugins/SwapLockTCPPlugin/SwapLockTCPPlugin.h> // my header...

#include <gadget/Util/Debug.h>
#include <cluster/ClusterDepChecker.h>

#include <cluster/ClusterManager.h>
#include <cluster/ClusterNetwork/ClusterNetwork.h>
#include <cluster/ClusterNetwork/ClusterNode.h>

//#include <cluster/Plugins/SwapLockPlugin/ClusterBarrier.h>
//#include <cluster/ClusterNetwork/ClusterNetwork.h>
#include <cluster/Packets/PacketFactory.h>
#include <cluster/Packets/SyncRequest.h>
#include <cluster/Packets/SyncAck.h>

#include <vpr/Thread/Thread.h>

#include <vpr/Util/ReturnStatus.h>
#include <vpr/IO/Socket/SocketStream.h>
#include <vpr/IO/Port/SerialPort.h>

#include <boost/concept_check.hpp>


cluster::ClusterPlugin* initPlugin()
{
   return cluster::SwapLockTCPPlugin::instance();
}

namespace cluster
{
   vprSingletonImp( SwapLockTCPPlugin );

   /** Add the pending element to the configuration.
   *  PRE: configCanHandle (element) == true.
   *  @return true iff element was successfully added to configuration.
   */
   bool SwapLockTCPPlugin::configAdd(jccl::ConfigElementPtr element)
   {
      // We need to make sure that the ClusterManager has been configured so that we
      // can ensure that the barrier master's config element has also been configured.
      if(!ClusterManager::instance()->isClusterActive())
      {
         // XXX: This could be made into a dependancy also.
         return false;
      }

      /////////////////////////////////////////
      //  SwapLock Barrier Stuff
      //
      // -Set flag we have started configuring the cluster
      // -Get Sync Machine Chunk Name
      // -Get ChunkPtr to this element
      // -Get the Hostname of this node

      std::string barrier_machine_element_name = element->getProperty<std::string>(std::string("sync_server"));
      jccl::ConfigElementPtr barrier_machine_element = ClusterManager::instance()->getConfigElementPointer(barrier_machine_element_name);
      vprASSERT(NULL != barrier_machine_element.get() && "ConfigManager Chunk MUST have a barrier_master.");
      mBarrierMasterHostname = barrier_machine_element->getProperty<std::string>(std::string("host_name"));
      
      mTCPport = element->getProperty<int>(std::string("listen_port"));

      vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL) << clrOutBOLD(clrCYAN,"[SwapLockTCPPlugin] ")
         << "SwapLock Master Chunk Name is: " << barrier_machine_element_name << std::endl << vprDEBUG_FLUSH;         
      vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL) << clrOutBOLD(clrCYAN,"[SwapLockTCPPlugin] ")
         << "SwapLock Master Hostname is: " << mBarrierMasterHostname << std::endl << vprDEBUG_FLUSH;         
      // Starting Barrier Stuff
      /////////////////////////////////////         
      
      if (ClusterNetwork::isLocalHost(mBarrierMasterHostname))
      {
         mIsMaster = true;
         // TODO: Get this from config file.
         startListening();
         mActive = true;
      }
      else
      {
         mIsMaster = false;         
         ConnectToMasterSocket();
      }
      return true;
   }
   
   /** Remove the pending element from the current configuration.
   *  PRE: configCanHandle (element) == true.
   *  @return true iff the element (and any objects it represented)
   *          were successfully removed.
   */
   bool SwapLockTCPPlugin::configRemove(jccl::ConfigElementPtr element)
   {
      if (recognizeSwapLockTCPPluginConfig(element))
      {
         vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL) << "[SwapLockTCPPlugin] Disabling SwapLock: " << element->getName() 
         << "\n" << vprDEBUG_FLUSH;
         return(true);
      }
      else
      {
         vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL) << "[SwapLockTCPPlugin::configRemove] ERROR, Something is seriously wrong, we should never get here\n"
         << vprDEBUG_FLUSH;
         return(false);
      }
   }
   
   /** Checks if this handler can process element.
   *  Typically, an implementation of handler will check the element's
   *  description name/token to decide if it knows how to deal with
   *  it.
   *  @return true iff this handler can process element.
   */
   bool SwapLockTCPPlugin::configCanHandle(jccl::ConfigElementPtr element)
   {
      return( recognizeSwapLockTCPPluginConfig(element) );   
   }
      
   bool SwapLockTCPPlugin::recognizeSwapLockTCPPluginConfig(jccl::ConfigElementPtr element)
   {
      return(element->getID() == getElementType());
   }

   std::string SwapLockTCPPlugin::getElementType()
   {
      return "swap_lock_tcp_plugin";
   }

   void SwapLockTCPPlugin::preDraw()
   {;
   }
   void SwapLockTCPPlugin::postPostFrame()
   {;
   }

   void SwapLockTCPPlugin::handlePacket(Packet* packet, ClusterNode* node)
   {
      boost::ignore_unused_variable_warning(packet);
      boost::ignore_unused_variable_warning(node);
   }     

   bool SwapLockTCPPlugin::isPluginReady()
   {
      // We could do some sort of signal here I guess?
      return (true);
   }
   bool SwapLockTCPPlugin::createBarrier()
   {
      // If Slave
      // - send ready to Master
      // - read ready from Server
      // Else if Master
      // - for each slave
      //   - read ready
      // - for each slave
      //   - send ready

      //vpr::Interval first_time, second_time;
      //first_time.setNow();

      if (mIsMaster)
      {
         masterReceive();
         masterSend();
      }
      else
      {
         // If we are not currently connected try to connect to the master.
         if (!mActive)
         {
            ConnectToMasterSocket();
         }

         // Make sure that we are connected.
         if (mActive)
         {
            slaveSend();
            slaveReceive();
         }
      }
      //second_time.setNow();
      //vpr::Interval diff_time(second_time-first_time);
      //vprDEBUG(gadgetDBG_RIM,vprDBG_CRITICAL_LVL) << clrSetBOLD(clrCYAN) << "Latency: " 
      //   << diff_time.getBaseVal() << " usecs\n"<< clrRESET << vprDEBUG_FLUSH;

      return(true);
   }

   vpr::ReturnStatus SwapLockTCPPlugin::ConnectToMasterSocket()
   {
      vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL) << "[SwapLockTCPPlugin]: Attempting to connect to SyncServer:  " 
         << mBarrierMasterHostname <<":"<< mTCPport << "\n"<< vprDEBUG_FLUSH;

      vpr::InetAddr inet_addr;
      
         // Set the address that we want to connect to
      if ( !inet_addr.setAddress(mBarrierMasterHostname, mTCPport).success() )
      {
         vprDEBUG(gadgetDBG_RIM,vprDBG_CRITICAL_LVL) << clrOutBOLD(clrRED,"[SwapLockTCPPlugin]: Failed to set address\n") 
            << vprDEBUG_FLUSH;
         mActive = false;

         return vpr::ReturnStatus::Fail;
      }
         // Create a new socket stream to this address
      mSyncServerSocket = new vpr::SocketStream(vpr::InetAddr::AnyAddr, inet_addr);
      
         // If we can successfully open the socket and connect to the server
      mSyncServerSocket->open();
      mSyncServerSocket->setBlocking(true);
      
      if (mSyncServerSocket->connect().success())
      {   
         vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL) << "[SwapLockTCPPlugin]: Successfully connected to sync server: " 
            << mBarrierMasterHostname <<":"<< mTCPport << "\n"<< vprDEBUG_FLUSH;
         
         // Send request
         // Wait for responce
         // return result
         
         // Get the localhost name.
         vpr::InetAddr local;
         vpr::InetAddr::getLocalHost(local);
         
         std::string local_host_name = local.getHostname();
         
         SyncRequest sync_request(local_host_name, mTCPport);
         
         ClusterNode* temp_node = new ClusterNode(std::string("Unknown"), std::string("Unknown"), vpr::Uint16(0), mSyncServerSocket);
         temp_node->send(&sync_request);
         Packet* packet = temp_node->recvPacket();
         
         delete temp_node;
         temp_node = NULL;
         
         SyncAck* ack_packet = static_cast<SyncAck*>(packet);
   
         if (ack_packet->getAck())
         {
            // add to list
            // respond true
            mActive = true;
            return(vpr::ReturnStatus::Succeed);
         }
      }
      // Free unused memory since we could not connect
      delete mSyncServerSocket;
      mSyncServerSocket = NULL;
      return(vpr::ReturnStatus::Fail);
   }

   void SwapLockTCPPlugin::masterSend()
   {
      //vprASSERT(mSyncClients!=NULL && "Sync Clients Vector is NULL!");
      vprASSERT(mActive==true && "Barrier is not active!");

      vpr::Uint32 bytes_read;
      vpr::Guard<vpr::Mutex> guard(mSyncClientsLock);

      vpr::Uint8 temp;

      for (std::vector<vpr::SocketStream*>::iterator i = this->mSyncClients.begin();
        i < this->mSyncClients.end();i++)
      {
         while ((*i)->availableBytes() > 0)
         {
            vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
               << clrOutBOLD(clrMAGENTA,"[SwapLockTCPPlugin]")
               << "Buffer overrun..." << (*i)->availableBytes() << std::endl << vprDEBUG_FLUSH;

            (*i)->recvn(&temp , 1, bytes_read);
         }
         (*i)->send(&SYNC_SIGNAL , 1, bytes_read, vpr::Interval::NoWait);
      }

   }
   void SwapLockTCPPlugin::masterReceive()
   {
      vprASSERT(mActive==true && "Barrier is not active!");

      vpr::Uint32 bytes_read;
      vpr::Uint8 temp;
      
      vpr::Guard<vpr::Mutex> guard(mSyncClientsLock);

      for (std::vector<vpr::SocketStream*>::iterator i = mSyncClients.begin();
           i < mSyncClients.end();i++)
      {
         if((*i)->recv(&temp , 1, bytes_read,read_timeout) == vpr::ReturnStatus::Timeout)
         {
            vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
               << clrOutBOLD(clrMAGENTA,"[SwapLockTCPPlugin]")
               << "SwapBarrier slip...\n" << vprDEBUG_FLUSH;
            return;            
/*            static int numTimeouts = 0;
            numTimeouts++;
            vprDEBUG(gadgetDBG_RIM,vprDBG_CRITICAL_LVL) << "[SwapLockTCPPlugin]: Received a timeout from a cluster node, it was removed" 
               << std::endl << vprDEBUG_FLUSH;
            if (numTimeouts > 5)
            {
               mSyncClients.erase(i);
               vprDEBUG(gadgetDBG_RIM,vprDBG_CRITICAL_LVL) << "[SwapLockTCPPlugin]: Received too many timeouts from a cluster node,"
                  <<" so it was removed from the list of machines to syncronize with." 
                  << std::endl << vprDEBUG_FLUSH;
               vpr::System::sleep(1);
            }
*/            
         }
         //vprASSERT(1==bytes_read && "SwapLockTCPPlugin: Master Barrier received timeout");
      } 
   }
   void SwapLockTCPPlugin::slaveSend()
   {
      vprASSERT(mSyncServerSocket!=NULL && "mSyncServerSocket is NULL!");
      vprASSERT(mActive==true && "Barrier is not active!");

      vpr::Uint8 temp;

      vpr::Uint32 bytes_read;
      
      while (mSyncServerSocket->availableBytes() > 0)
      {
         vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
            << clrOutBOLD(clrMAGENTA,"[SwapLockTCPPlugin]")
            << "Buffer overrun..." << mSyncServerSocket->availableBytes() << std::endl << vprDEBUG_FLUSH;
         mSyncServerSocket->recvn(&temp , 1, bytes_read);
      }
      
      mSyncServerSocket->send(&SYNC_SIGNAL , 1, bytes_read, vpr::Interval::NoWait);
   }
   void SwapLockTCPPlugin::slaveReceive()
   {
      vprASSERT(mSyncServerSocket!=NULL && "mSyncServerSocket is NULL!");
      vprASSERT(mActive==true && "Barrier is not active!");
      
      vpr::Uint32 bytes_read;
      vpr::Uint8 temp;
      if (!mSyncServerSocket->recv(&temp , 1, bytes_read,read_timeout).success())
      {
         vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
            << clrOutBOLD(clrMAGENTA,"[SwapLockTCPPlugin]")
            << "SwapBarrier slip...\n" << vprDEBUG_FLUSH;
         //vpr::System::sleep(read_timeout);
      }
      //vprASSERT(1==bytes_read && "SwapLockTCPPlugin: Slave Barrier received timeout");
   }


   bool SwapLockTCPPlugin::startListening()
   {
      if ( mAcceptThread == NULL )  // If we haven't already started the listening thread
      {     
         if ( mTCPport > 0 )     // If the listen port is valid
         {
            mListenAddr.setPort(mTCPport);

            vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
               << clrOutBOLD(clrMAGENTA,"[SwapLockTCPPlugin]")
               << "Starting the listening thread...\n" << vprDEBUG_FLUSH;

               // Start a thread to monitor port
            vpr::ThreadMemberFunctor<SwapLockTCPPlugin>* memberFunctor =
            new vpr::ThreadMemberFunctor<SwapLockTCPPlugin>
            (this, &SwapLockTCPPlugin::acceptLoop, NULL);
            
            mAcceptThread = new vpr::Thread(memberFunctor);
            vprASSERT(mAcceptThread->valid());
            return true;
         }
         else
         {
            vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
               << clrOutBOLD(clrMAGENTA,"[SwapLockTCPPlugin]")
               << "startListening() Can NOT listen on port: "
               << mTCPport << "\n" << vprDEBUG_FLUSH;
            return false;
         }
      }
      else
      {
         vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
               << clrOutBOLD(clrMAGENTA,"[SwapLockTCPPlugin]")
               << "startListening() Listening thread already active.\n" << vprDEBUG_FLUSH;
         return false;
      }
   }
   
   void SwapLockTCPPlugin::acceptLoop(void* nullParam)
   {
      boost::ignore_unused_variable_warning(nullParam);

      //////////////////////////// Create an acceptor socket that listens on port. //////////////////////////////

      vpr::SocketStream sock(mListenAddr, vpr::InetAddr::AnyAddr);
        // Open in server mode.
      if ( sock.openServer().success() )
      {
         sock.setReuseAddr(true);
      }
      else
      {
         vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
            << clrSetBOLD(clrRED) << "[SwapLockTCPPlugin]"
            << " Unable to open listening socket on port: " << mListenAddr.getPort() << std::endl << clrRESET << vprDEBUG_FLUSH;
         exit(0);
      }
      vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
         << clrOutBOLD(clrMAGENTA,"[SwapLockTCPPlugin]")
         << " Listening on Port: " << mListenAddr.getPort() << std::endl << vprDEBUG_FLUSH;      

      ///////////////////////////  Wait for a connection //////////////////////////////

      vpr::SocketStream* client_sock = new vpr::SocketStream;

      while ( 1 )
      {    // Wait for an incoming connection.   
         vpr::ReturnStatus status = sock.accept(*client_sock, vpr::Interval::NoTimeout);
         vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
            << clrOutBOLD(clrMAGENTA,"[SwapLockTCPPlugin]")
            << " Received a connection attempt on Port: " << mListenAddr.getPort() << std::endl << vprDEBUG_FLUSH;
         if ( status.success() )
         {               
            client_sock->setNoDelay(true);  
               // Lock Cluster Nodes while working on them
              // Try to receive a Packet
            Packet* temp_packet;
            try
            {
               ClusterNode* temp_node = new ClusterNode(std::string("Unknown"), std::string("Unknown"), vpr::Uint16(0), client_sock);

               temp_packet = temp_node->recvPacket();
               delete temp_node;
            }
            catch(cluster::ClusterException cluster_exception)
            {
               vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
                  << clrOutBOLD(clrRED,"ERROR:")
                  << "SwapLockTCPPlugin::acceptLoop() Caught an exception: " 
                  << cluster_exception.getMessage() << std::endl << vprDEBUG_FLUSH;

               delete client_sock;
               client_sock = new vpr::SocketStream;
               continue;
            }
            
            SyncRequest* sync_request;

              // Make sure it is a ConnectionRequest
            try
            {
               sync_request = static_cast<SyncRequest*>(temp_packet);
               sync_request->printData(vprDBG_CONFIG_LVL);
            }
            catch(...)
            {
               vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
                  << clrOutBOLD(clrRED,"ERROR:")
                  << " SwapLockTCPPlugin::acceptLoop() The first packet read on the socket is NOT a Sync Request" << std::endl << vprDEBUG_FLUSH;
               exit(0);
            }
            /////////////////
            
            // Add a new slave to list of slaves
            {
               vpr::Guard<vpr::Mutex> guard(mSyncClientsLock);
               mSyncClients.push_back(client_sock);
            }           
            
            SwapLockTCPPlugin::instance()->setActive(true);

            // Get the localhost name.
            vpr::InetAddr local;
            vpr::InetAddr::getLocalHost(local);
            
            std::string host = local.getHostname();
            vpr::Uint16 temp_port = mTCPport;
            
            SyncAck temp_ack(host, temp_port, true);


            vprDEBUG(gadgetDBG_RIM,vprDBG_CONFIG_LVL)
               << clrOutBOLD(clrMAGENTA,"[SwapLockTCPPlugin]")
               << "Sending responce: " << std::endl << vprDEBUG_FLUSH;
            
            temp_ack.printData(vprDBG_CONFIG_LVL);
            
            ClusterNode* temp_node = new ClusterNode(std::string("Unknown"), std::string("Unknown"), vpr::Uint16(0), client_sock);

            temp_node->send(&temp_ack);
            delete temp_node;

            // We need a new SocketStream since the old one is now being used by a ClusterNode
            client_sock = new vpr::SocketStream;
            
            //
            /////////////////////////////////////////////
            
         }
         else if ( status == vpr::ReturnStatus::Timeout )
         {
               // Should never happen since timeout is infinite
            client_sock->close();
            delete client_sock;
            client_sock = new vpr::SocketStream;
         }
      }   // end infinite while      
   }
} // End of cluster namespace
