//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE protocol IP driver for Linux

   For details cf. documentation in .h file.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscIpDispatcherLinuxSock.hpp"
#include "C_SclString.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::scl;
using namespace stw::tgl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
#define INVALID_SOCKET     (-1)

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
std::map<C_OscIpDispatcherLinuxSock::C_BufferIdentifier,
         std::list<std::vector<uint8_t> > > C_OscIpDispatcherLinuxSock::mhc_TcpBuffer;
C_TglCriticalSection C_OscIpDispatcherLinuxSock::mhc_LockBuffer;

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if current smaller than orc_Cmp

   \param[in] orc_Cmp Compared instance

   \return
   Current smaller than orc_Cmp
   Else false
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscIpDispatcherLinuxSock::C_BufferIdentifier::operator <(
   const C_OscIpDispatcherLinuxSock::C_BufferIdentifier & orc_Cmp) const
{
   bool q_Return;

   if (this->u8_ServerBusIdentifier < orc_Cmp.u8_ServerBusIdentifier)
   {
      q_Return = true;
   }
   else if (this->u8_ServerBusIdentifier == orc_Cmp.u8_ServerBusIdentifier)
   {
      if (this->u8_ServerNodeIdentifier < orc_Cmp.u8_ServerNodeIdentifier)
      {
         q_Return = true;
      }
      else if (this->u8_ServerNodeIdentifier == orc_Cmp.u8_ServerNodeIdentifier)
      {
         if (this->u8_ClientBusIdentifier < orc_Cmp.u8_ClientBusIdentifier)
         {
            q_Return = true;
         }
         else if (this->u8_ClientBusIdentifier == orc_Cmp.u8_ClientBusIdentifier)
         {
            if (this->u8_ClientNodeIdentifier < orc_Cmp.u8_ClientNodeIdentifier)
            {
               q_Return = true;
            }
            else
            {
               q_Return = false;
            }
         }
         else
         {
            q_Return = false;
         }
      }
      else
      {
         q_Return = false;
      }
   }
   else
   {
      q_Return = false;
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set up class

   Initialize class elements
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscIpDispatcherLinuxSock::C_OscIpDispatcherLinuxSock(void) :
   C_OscIpDispatcher(),
   ms32_SocketUdpServer(INVALID_SOCKET)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clean up class

   Release allocated ressources
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscIpDispatcherLinuxSock::~C_OscIpDispatcherLinuxSock(void)
{
   //make sure to release resources in case the user forgot to
   for (uint16_t u16_Index = 0U; u16_Index < this->mc_SocketsTcp.size(); u16_Index++)
   {
      this->CloseTcp(u16_Index);
   }
   this->CloseUdp();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Init TCP communication

   Jobs to perform:
   - create one non-blocking TCP client socket

   Strategy for "book keeping":
   * new connections will always be added at the end
   * closing connection closes the handle bus does not reduce the list of handles
   * so we don't have any trouble with existing handles getting invalid

   \param[in]     orau8_Ip      IP address of server to connect to
   \param[out]    oru32_Handle  handle to new TCP connection (to be used in subsequent calls of TCP functions)

   \return
   C_NO_ERR   Connection created
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::InitTcp(const uint8_t (&orau8_Ip)[4], uint32_t & oru32_Handle)
{
   C_TcpConnection c_NewConnection;

   (void)memcpy(&c_NewConnection.au8_IpAddress[0], &orau8_Ip[0], 4U);
   c_NewConnection.s32_Socket = INVALID_SOCKET;

   this->mc_SocketsTcp.push_back(c_NewConnection);
   oru32_Handle = static_cast<uint32_t>(mc_SocketsTcp.size() - 1U);

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: get IP of all local interfaces

   Gets list of IP addresses of all local IP interfaces.
   Does not report "localhost".
   Results will be placed in mc_LocalInterfaceIps

   \return
   C_NO_ERR     IPs listed
   C_NOACT      could not get IP information
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::m_GetAllInstalledInterfaceIps(void)
{
   int32_t sn_Ret;
   int32_t s32_Return;
   struct ifaddrs * pt_IfAddr;
   struct ifaddrs * pt_IfAddrFirst;

   mc_LocalInterfaceIps.resize(0);

   sn_Ret = getifaddrs(&pt_IfAddrFirst);
   if (sn_Ret == 0)
   {
      // Walk through linked list, maintaining head pointer so we can free list later
      for (pt_IfAddr = pt_IfAddrFirst; pt_IfAddr != NULL; pt_IfAddr = pt_IfAddr->ifa_next)
      {
         if (pt_IfAddr->ifa_addr == NULL)
         {
            continue;
         }

         // Only report IPv4 interfaces
         if (pt_IfAddr->ifa_addr->sa_family == AF_INET)
         {
            struct sockaddr_in * pt_InAddr = reinterpret_cast<struct sockaddr_in *>(pt_IfAddr->ifa_addr);

            //ignore localhost; otherwise "locally" running servers will get requests via the localhost and
            // a physical local interface
            if (ntohl(pt_InAddr->sin_addr.s_addr) != 0x7F000001U)
            {
               C_SclString c_IpStr = inet_ntoa(pt_InAddr->sin_addr);
               osc_write_log_info("openSYDE IP-TP", "Got a interface: " + c_IpStr);

               mc_LocalInterfaceIps.push_back(ntohl(pt_InAddr->sin_addr.s_addr));
            }
         }
      }

      freeifaddrs(pt_IfAddrFirst);
      s32_Return = C_NO_ERR;
   }
   else
   {
      C_SclString c_ErrnoStr = strerror(errno);
      osc_write_log_error("openSYDE IP-TP", "getifaddrs() failed. Error: " + c_ErrnoStr);
      s32_Return = C_NOACT;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: connect TCP socket to server

   Jobs to perform:
   - connect already created client socket to port 13400 of the specified server node

   Function shall return when connected or after timeout.
   The function will try the TCP connection for the configured timeout time.

   \param[in,out]  orc_Connection     TCP connection status and configuration

   \return
   C_NO_ERR   connected ...
   C_BUSY     connection failed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::m_ConnectTcp(C_TcpConnection & orc_Connection) const
{
   int x_Return; //lint !e8080 !e970 //using type to match library interface
   bool q_Error = false;

   //create TCP socket:
   orc_Connection.s32_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (orc_Connection.s32_Socket < 0)
   {
      C_SclString c_ErrnoStr = strerror(errno);
      osc_write_log_error("openSYDE IP-TP", "Error at TCP socket(): " + c_ErrnoStr +
                          " IP-Address: " + mh_IpToText(orc_Connection.au8_IpAddress));
      q_Error = true;
   }
   else
   {
      osc_write_log_info("openSYDE IP-TP", "TCP socket OK IP-Address: " + mh_IpToText(orc_Connection.au8_IpAddress));
   }

   if (q_Error == false)
   {
      //make socket non-blocking:
      int x_Flags; //lint !e8080 !e970 //using type to match library interface

      x_Return = -1;
      x_Flags = fcntl(orc_Connection.s32_Socket, F_GETFL, 0);
      if (x_Flags != -1)
      {
         x_Flags = (x_Flags | O_NONBLOCK);
         x_Return = fcntl(orc_Connection.s32_Socket, F_SETFL, x_Flags);
      }
      if (x_Return != 0)
      {
         C_SclString c_ErrnoStr = strerror(errno);
         osc_write_log_error("openSYDE IP-TP",
                             "TCP socket set non-blocking failed. Error: " + c_ErrnoStr +
                             "IP-Address: " + mh_IpToText(orc_Connection.au8_IpAddress));
         q_Error = true;
      }
   }

   if (q_Error == false)
   {
      sockaddr_in t_RemoteAddr;
      uint32_t u32_IpAddr;

      t_RemoteAddr.sin_family = AF_INET;
      u32_IpAddr = (orc_Connection.au8_IpAddress[0] << 24) |
                   (orc_Connection.au8_IpAddress[1] << 16) |
                   (orc_Connection.au8_IpAddress[2] << 8)  |
                   orc_Connection.au8_IpAddress[3];
      t_RemoteAddr.sin_addr.s_addr = htonl(u32_IpAddr);
      t_RemoteAddr.sin_port = htons(mhu16_UDP_TCP_PORT); //server port

      //lint -e{929,740}  Side-effect of the POSIX-style API. Match is guaranteed by the API.
      x_Return =
         connect(orc_Connection.s32_Socket, reinterpret_cast<const sockaddr *>(&t_RemoteAddr), sizeof(t_RemoteAddr));
      //for this non-blocking TCP socket the function should immediately return with EINPROGRESS:
      if ((x_Return == -1) && (errno != EINPROGRESS))
      {
         C_SclString c_ErrnoStr = strerror(errno);
         osc_write_log_error("openSYDE IP-TP", "TCP connect() failed. Error: " + c_ErrnoStr);
         q_Error = true;
      }
      else
      {
         C_SclString c_ErrnoStr;
         fd_set t_SocketWriteSet;
         fd_set t_SocketErrorSet;
         timeval t_TimeOut;

         FD_ZERO(&t_SocketWriteSet);
         FD_ZERO(&t_SocketErrorSet);
         FD_SET(orc_Connection.s32_Socket, &t_SocketWriteSet);
         FD_SET(orc_Connection.s32_Socket, &t_SocketErrorSet);
         t_TimeOut.tv_sec = mu32_ConnectionTimeoutSeconds;
         t_TimeOut.tv_usec = 0;
         x_Return = select(orc_Connection.s32_Socket + 1, NULL, &t_SocketWriteSet, &t_SocketErrorSet, &t_TimeOut);
         switch (x_Return)
         {
         case -1:
            c_ErrnoStr = strerror(errno);
            osc_write_log_error("openSYDE IP-TP",
                                "TCP connect select() failed. IP-Address: " + mh_IpToText(
                                   orc_Connection.au8_IpAddress) + " Error: " + c_ErrnoStr);
            q_Error = true;
            break;
         case 0:
            //no event -> timeout
            osc_write_log_warning("openSYDE IP-TP",
                                  "TCP connect select() failed. IP-Address: " + mh_IpToText(
                                     orc_Connection.au8_IpAddress) + " No connection within timeout");
            // No error. The connection to the concrete target can be established later
            q_Error = false;
            break;
         case 1:
            //lint -e{1924,929,909} //macro defined by API; no problem
            if (FD_ISSET(orc_Connection.s32_Socket, &t_SocketWriteSet))
            {
               // Get port of client for logging (the byte order must be changed of the read port by ntohs)
               sockaddr_in t_SocketAddr;
               socklen_t un_Size = sizeof(t_SocketAddr);

               //lint -e{929,740}  Side-effect of the POSIX-style API. Match is guaranteed by the API.
               getsockname(orc_Connection.s32_Socket, reinterpret_cast<sockaddr *>(&t_SocketAddr), &un_Size);

               //event caused by write (= connect finished)
               osc_write_log_info("openSYDE IP-TP",
                                  "TCP connect select() OK. IP-Address: " + mh_IpToText(orc_Connection.au8_IpAddress) +
                                  " on client port: " + C_SclString::IntToStr(ntohs(t_SocketAddr.sin_port)));
            }
            else
            {
               //event caused by error (= connect failed; e.g. rejected)
               osc_write_log_error("openSYDE IP-TP",
                                   "TCP connect select() failed. IP-Address: " +
                                   mh_IpToText(orc_Connection.au8_IpAddress));
               q_Error = true;
            }
            break;
         default:
            osc_write_log_error("openSYDE IP-TP",
                                "TCP connect select() failed. Unknown problem: " + C_SclString::IntToStr(
                                   x_Return) + " IP-Address: " + mh_IpToText(orc_Connection.au8_IpAddress));
            q_Error = true;
            break;
         }
      }
   }
   return (q_Error == true) ? C_BUSY : C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: configure UDP socket

   \param[in]   oq_ServerPort    true: set up server socket
                                 false: set up client socket
   \param[in]   ou32_IpToBindTo  IP to bind the socket to
   \param[out]  orun_Socket      Resulting socket handle

   \return
   C_NO_ERR   socket set up
   C_NOACT    could not set up socket
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::m_ConfigureUdpSocket(const bool oq_ServerPort, const uint32_t ou32_IpToBindTo,
                                                         int32_t & ors32_Socket) const
{
   bool q_Error = false;
   int x_Return; //lint !e8080 !e970 //using type to match library interface

   //create UDP socket:
   ors32_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (ors32_Socket < 0)
   {
      C_SclString c_ErrnoStr = strerror(errno);
      osc_write_log_error("openSYDE IP-TP", "Error at UDP socket(): " + c_ErrnoStr);
      q_Error = true;
   }
   else
   {
      osc_write_log_info("openSYDE IP-TP", "UDP socket OK");
   }

   if ((q_Error == false) && (oq_ServerPort == false))
   {
      //set broadcast permission for UDP client socket
      int x_Enabled = 1; //lint !e8080 !e970 //using type to match library interface
      x_Return = setsockopt(ors32_Socket, SOL_SOCKET, SO_BROADCAST, &x_Enabled, sizeof(x_Enabled));
      if (x_Return < 0)
      {
         C_SclString c_ErrnoStr = strerror(errno);
         osc_write_log_error("openSYDE IP-TP", "UDP set broadcast permission failed. Error: " + c_ErrnoStr);
         (void)close(ors32_Socket);
         q_Error = true;
      }
   }

   if (q_Error == false)
   {
      //bind UDP:
      sockaddr_in t_LocalAddr;
      t_LocalAddr.sin_family = AF_INET;
      t_LocalAddr.sin_addr.s_addr = htonl(ou32_IpToBindTo);
      if (oq_ServerPort == true)
      {
         // Make port 13400 reusable to be able to run osy client and server simultaneously on same target
         int x_Enabled = 1; //lint !e8080 !e970 //using type to match library interface
         setsockopt(ors32_Socket, SOL_SOCKET, SO_REUSEADDR, &x_Enabled, sizeof(x_Enabled));

         t_LocalAddr.sin_port = htons(mhu16_UDP_TCP_PORT); //provide port
      }
      else
      {
         //we want to bind to the IP address of the interface but NOT to the port
         // (this shall be assigned dynamically)
         t_LocalAddr.sin_port = 0U;
      }

      //lint -e{929,740}  Side-effect of the POSIX-style API. Match is guaranteed by the API.
      x_Return = bind(ors32_Socket, reinterpret_cast<const sockaddr *>(&t_LocalAddr), sizeof(t_LocalAddr));
      if (x_Return < 0)
      {
         C_SclString c_ErrnoStr = strerror(errno);
         osc_write_log_error("openSYDE IP-TP", "UDP bind() failed. Error: " + c_ErrnoStr);
         q_Error = true;
      }
   }

   if (q_Error == false)
   {
      //make socket non-blocking:
      int x_Flags; //lint !e8080 !e970 //using type to match library interface

      x_Return = -1;
      x_Flags = fcntl(ors32_Socket, F_GETFL, 0);
      if (x_Flags != -1)
      {
         x_Flags = (x_Flags | O_NONBLOCK);
         x_Return = fcntl(ors32_Socket, F_SETFL, x_Flags);
      }
      if (x_Return != 0)
      {
         C_SclString c_ErrnoStr = strerror(errno);
         osc_write_log_error("openSYDE IP-TP", "TCP socket ioctlsocket() failed. Error: " + c_ErrnoStr);
         q_Error = true;
      }
   }

   return (q_Error == true) ? C_NOACT : C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: compose textual representation of IP

   Format: xxx.xxx.xxx.xxx

   \return
   test representation of IP
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_OscIpDispatcherLinuxSock::mh_IpToText(const uint8_t (&orau8_Ip)[4])
{
   C_SclString c_Text;

   c_Text.PrintFormatted("%d.%d.%d.%d", orau8_Ip[0], orau8_Ip[1], orau8_Ip[2], orau8_Ip[3]);
   return c_Text;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize UDP communication

   Jobs to perform:
   * create one non-blocking UDP "server" socket to collect incoming UDP messages (broadcast and unicast)
   ** bind them to INADDR_ANY and port 13400
   * create non-blocking UDS "client" sockets on each local interface to send UDP messages (broadcast and unicast)
   ** bind them to the IP of the interface but don't assign a fixed port
   ** set permissions to send broadcasts

   \return
   C_NO_ERR   connected ...
   C_NOACT    connection failed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::InitUdp(void)
{
   bool q_Error = false;
   int32_t s32_Return;

   s32_Return = m_GetAllInstalledInterfaceIps();
   if (s32_Return != C_NO_ERR)
   {
      //we could not find out which interfaces we have: try "any"
      osc_write_log_warning("openSYDE IP-TP", "UDP init: could not find IP(s) of local interfaces. Trying \"any\" ...");
      mc_LocalInterfaceIps.resize(1);
      mc_LocalInterfaceIps[0] = INADDR_ANY;
   }
   mc_SocketsUdpClient.resize(mc_LocalInterfaceIps.size());

   for (uint32_t u32_Interface = 0U; u32_Interface < mc_LocalInterfaceIps.size(); u32_Interface++)
   {
      mc_SocketsUdpClient[u32_Interface] = INVALID_SOCKET;
   }
   ms32_SocketUdpServer = INVALID_SOCKET;

   for (uint32_t u32_Interface = 0U; u32_Interface < mc_LocalInterfaceIps.size(); u32_Interface++)
   {
      //create UDP socket:
      s32_Return = m_ConfigureUdpSocket(false, mc_LocalInterfaceIps[u32_Interface], mc_SocketsUdpClient[u32_Interface]);
      if (s32_Return != C_NO_ERR)
      {
         q_Error = true;
         break;
      }
   }

   if (q_Error == false)
   {
      // Create one UDP server socket, bind to INADDR_ANY, port 13400
      s32_Return = m_ConfigureUdpSocket(true, INADDR_ANY, ms32_SocketUdpServer);
      if (s32_Return != C_NO_ERR)
      {
         q_Error = true;
      }
   }

   //close sockets in case of error:
   if (q_Error == true)
   {
      this->CloseUdp();
   }

   return (q_Error == true) ? C_NOACT : C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Checks the connection of the TCP socket

   \param[in]   ou32_Handle   handle obtained by InitTcp()

   \return
   C_NO_ERR   is connected
   C_NOACT    is not connected
   C_RANGE    invalid handle
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::IsTcpConnected(const uint32_t ou32_Handle)
{
   int32_t s32_Return = C_NOACT;

   if (ou32_Handle >= this->mc_SocketsTcp.size())
   {
      osc_write_log_error("openSYDE IP-TP", "IsTcpConnected called with invalid handle.");
      s32_Return = C_RANGE;
   }
   else if (this->mc_SocketsTcp[ou32_Handle].s32_Socket == INVALID_SOCKET)
   {
      // Socket not opened yet
      s32_Return = C_NOACT;
   }
   else
   {
      char_t cn_Byte;
      // Dummy read to check the connection.
      // The parameter MSG_PEEK avoid that received data will removed from the queue when read
      //lint !e8080 !e970 //using type to match library interface
      const int x_BytesRead = recv(this->mc_SocketsTcp[ou32_Handle].s32_Socket, &cn_Byte, 1, MSG_PEEK);

      if (x_BytesRead > 0)
      {
         s32_Return = C_NO_ERR;
      }
      else if (x_BytesRead == 0)
      {
         // Connection closed by remote peer
         s32_Return = C_NOACT;
      }
      else
      {
         // Resource temporarily unavailable. It means that no bytes culd be read, because of an empty queue.
         // But the connection is already established.
         if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
         {
            s32_Return = C_NO_ERR;
         }
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reconnect TCP socket

   This function can be used by a client to reconnect after the connection was lost (e.g. dropped by server).
   If there is still an active connection the function shall close it before reconnecting.
   Function shall return when connected or after timeout.

   Jobs to perform:
   - re-connect TCP socket to port 13400 of specified server node

   \param[in]   ou32_Handle   handle obtained by InitTcp()

   \return
   C_NO_ERR   reconnected
   C_BUSY     connection failed
   C_RANGE    invalid handle
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::ReConnectTcp(const uint32_t ou32_Handle)
{
   int32_t s32_Return;

   if (ou32_Handle >= this->mc_SocketsTcp.size())
   {
      osc_write_log_error("openSYDE IP-TP", "ReConnectTcp called with invalid handle.");
      s32_Return = C_RANGE;
   }
   else
   {
      //still connected ?
      if (this->mc_SocketsTcp[ou32_Handle].s32_Socket != INVALID_SOCKET)
      {
         //disconnect first:
         (void)close(this->mc_SocketsTcp[ou32_Handle].s32_Socket);
         this->mc_SocketsTcp[ou32_Handle].s32_Socket = INVALID_SOCKET;
      }
      //connect:
      s32_Return = this->m_ConnectTcp(this->mc_SocketsTcp[ou32_Handle]);
      if (s32_Return != C_NO_ERR)
      {
         (void)close(this->mc_SocketsTcp[ou32_Handle].s32_Socket);
         this->mc_SocketsTcp[ou32_Handle].s32_Socket = INVALID_SOCKET;
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Close TCP communication

   Jobs to perform:
   - close opened TCP client socket

   \param[in]   ou32_Handle   handle obtained by InitTcp()

   \return
   C_NO_ERR   disconnected ...
   C_RANGE    handle invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::CloseTcp(const uint32_t ou32_Handle)
{
   int32_t s32_Return;

   if ((ou32_Handle >= this->mc_SocketsTcp.size()) ||
       (this->mc_SocketsTcp[ou32_Handle].s32_Socket == INVALID_SOCKET))
   {
      osc_write_log_error("openSYDE IP-TP", "CloseTcp called with invalid handle.");
      s32_Return = C_RANGE;
   }
   else
   {
      sockaddr_in t_SocketInfo;
      socklen_t un_Size = sizeof(t_SocketInfo);

      //lint -e{929,740}  Side-effect of the POSIX-style API. Match is guaranteed by the API.
      getsockname(this->mc_SocketsTcp[ou32_Handle].s32_Socket, reinterpret_cast<sockaddr *>(&t_SocketInfo), &un_Size);

      shutdown(this->mc_SocketsTcp[ou32_Handle].s32_Socket, SHUT_RDWR);

      s32_Return = close(this->mc_SocketsTcp[ou32_Handle].s32_Socket);
      this->mc_SocketsTcp[ou32_Handle].s32_Socket = INVALID_SOCKET;

      osc_write_log_info("openSYDE IP-TP",
                         "TCP closesocket() OK. IP-Address: " +
                         mh_IpToText(this->mc_SocketsTcp[ou32_Handle].au8_IpAddress) +
                         " on client port: " + C_SclString::IntToStr(ntohs(t_SocketInfo.sin_port)));
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Close UDP communication

   Jobs to perform:
   - close opened UDP client sockets

   \return
   C_NO_ERR   disconnected ...
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::CloseUdp(void)
{
   for (uint32_t u32_Interface = 0U; u32_Interface < mc_SocketsUdpClient.size(); u32_Interface++)
   {
      if (mc_SocketsUdpClient[u32_Interface] != INVALID_SOCKET)
      {
         (void)close(mc_SocketsUdpClient[u32_Interface]);
      }
   }

   if (ms32_SocketUdpServer != INVALID_SOCKET)
   {
      (void)close(ms32_SocketUdpServer);
   }

   mc_SocketsUdpClient.resize(0);
   ms32_SocketUdpServer = INVALID_SOCKET;

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send package on TCP socket

   Jobs to perform:
   - send specified data to configured TCP socket

   \param[in]   ou32_Handle   handle obtained by InitTcp()
   \param[in]   orc_Data      data to sent

   \return
   C_NO_ERR   data sent successfully
   C_CONFIG   required socket not initialized
   C_RD_WR    error sending data
   C_RANGE    handle invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::SendTcp(const uint32_t ou32_Handle, const std::vector<uint8_t> & orc_Data)
{
   int32_t s32_Return;

   if (ou32_Handle >= this->mc_SocketsTcp.size())
   {
      osc_write_log_error("openSYDE IP-TP", "SendTcp called with invalid handle.");
      s32_Return = C_RANGE;
   }
   else
   {
      s32_Return = C_NO_ERR;

      if (this->mc_SocketsTcp[ou32_Handle].s32_Socket == INVALID_SOCKET)
      {
         osc_write_log_error("openSYDE IP-TP", "SendTcp called with invalid socket.");
         s32_Return = C_CONFIG;
      }
      else
      {
         //lint -e{926}  Side-effect of the "char"-based API. No side effects as long as we are on the Windows platform.
         //lint !e970 //using type to match library interface
         const ssize_t x_BytesSent = send(this->mc_SocketsTcp[ou32_Handle].s32_Socket,
                                          reinterpret_cast<const char_t *>(&orc_Data[0]), orc_Data.size(), 0);
         if (x_BytesSent != static_cast<ssize_t>(orc_Data.size()))
         {
            if (x_BytesSent == -1)
            {
               C_SclString c_ErrnoStr = strerror(errno);
               osc_write_log_error("openSYDE IP-TP",
                                   "SendTcp: Could not send TCP service. Data lost. Error: " + c_ErrnoStr +
                                   " IP-Address: " + mh_IpToText(this->mc_SocketsTcp[ou32_Handle].au8_IpAddress));
               if ((errno == ENOTCONN) || (errno == ECONNRESET))
               {
                  //we got kicked out; we'll remember that ...
                  osc_write_log_warning("openSYDE IP-TP", "SendTcp: Connection aborted or reset ... IP-Address: " +
                                        mh_IpToText(this->mc_SocketsTcp[ou32_Handle].au8_IpAddress));
                  (void)close(this->mc_SocketsTcp[ou32_Handle].s32_Socket);
                  this->mc_SocketsTcp[ou32_Handle].s32_Socket = INVALID_SOCKET;
                  m_OnTcpConnectionDropped(ou32_Handle);
               }
            }
            else
            {
               osc_write_log_error("openSYDE IP-TP",
                                   "SendTcp: Could not send all data: tried: " +
                                   C_SclString::IntToStr(static_cast<int32_t>(orc_Data.size())) +
                                   "sent: " + C_SclString::IntToStr(x_BytesSent));
            }
            s32_Return = C_RD_WR;
         }
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read data from TCP socket

   Jobs to perform:
   - check whether TCP receive buffer contains required number of data bytes
   - read those bytes

    The function shall not return any data unless it can provide as many data bytes as specified.

   \param[in]      ou32_Handle   handle obtained by InitTcp()
   \param[in,out]  orc_Data      in: the expected number of bytes is set by the caller (orc_Data.size());
                                 out: received data

   \return
   C_NO_ERR   data read successfully
   C_CONFIG   required socket not initialized
   C_NOACT    not enough bytes
   C_RD_WR    error reading data
   C_RANGE    handle invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::ReadTcp(const uint32_t ou32_Handle, std::vector<uint8_t> & orc_Data)
{
   int32_t s32_Return;

   if (ou32_Handle >= this->mc_SocketsTcp.size())
   {
      osc_write_log_error("openSYDE IP-TP", "ReadTcp called with invalid handle.");
      s32_Return = C_RANGE;
   }
   else
   {
      s32_Return = C_NOACT;

      if (this->mc_SocketsTcp[ou32_Handle].s32_Socket == INVALID_SOCKET)
      {
         osc_write_log_error("openSYDE IP-TP", "ReadTcp called with invalid socket.");
         s32_Return = C_CONFIG;
      }
      else
      {
         int x_Return; //lint !e8080 !e970 //using type to match library interface
         int x_SizeInBuffer;
         //do we have enough bytes in RX buffer ?
         x_Return = ioctl(this->mc_SocketsTcp[ou32_Handle].s32_Socket, FIONREAD, &x_SizeInBuffer);
         //lint !e970 //using type to match library interface
         if ((x_Return == 0) && (x_SizeInBuffer >= static_cast<int>(orc_Data.size())))
         {
            //enough bytes: read
            //lint -e{926}  Side-effect of the "char"-based API. No side effects as long as we are on the Windows
            // platform.
            //lint !e970 //using type to match library interface
            ssize_t x_BytesRead = recv(this->mc_SocketsTcp[ou32_Handle].s32_Socket,
                                       reinterpret_cast<char_t *>(&orc_Data[0]), orc_Data.size(), 0);
            if (x_BytesRead == static_cast<ssize_t>(orc_Data.size()))
            {
               s32_Return = C_NO_ERR;
            }
            else
            {
               //comm error: no data read even though it was reported by ioctl
               osc_write_log_error("openSYDE IP-TP",
                                   "TCP unexpected error: data reported as available but reading failed. IP-Address: " +
                                   mh_IpToText(this->mc_SocketsTcp[ou32_Handle].au8_IpAddress));
               s32_Return = C_RD_WR;
            }
         }
         else if (x_Return == -1)
         {
            C_SclString c_ErrnoStr = strerror(errno);
            osc_write_log_error("openSYDE IP-TP", "Could not read TCP: buffer count could not be read. Error: " +
                                c_ErrnoStr + " IP-Address: " +
                                mh_IpToText(this->mc_SocketsTcp[ou32_Handle].au8_IpAddress));
         }
         else
         {
            //not enough data -> done here
         }
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read data from TCP socket

   Jobs to perform:
   - check whether TCP receive buffer contains required number of data bytes
   - Check if the identifier are matching (starting at byte 0). The header must be already read separately.
      If the identifier are not matching, the data will be stored in the buffer of the dispatcher.
   - read those bytes

    The function shall not return any data unless it can provide as many data bytes as specified.

   \param[in]      ou32_Handle               handle obtained by InitTcp()
   \param[in]      ou8_ClientBusIdentifier   client identifier of bus
   \param[in]      ou8_ClientNodeIdentifier  client identifier of node
   \param[in]      ou8_ServerBusIdentifier   server identifier of bus
   \param[in]      ou8_ServerNodeIdentifier  server identifier of node
   \param[in,out]  orc_Data                  in: the expected number of bytes is set by the caller (orc_Data.size());
                                             out: received data

   \return
   C_NO_ERR   data read successfully
   C_CONFIG   required socket not initialized
   C_NOACT    not enough bytes
   C_RD_WR    error reading data
   C_RANGE    handle invalid
   C_WARN     data is not for the server with the node identifier and bus identifier
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::ReadTcp(const uint32_t ou32_Handle, const uint8_t ou8_ClientBusIdentifier,
                                            const uint8_t ou8_ClientNodeIdentifier,
                                            const uint8_t ou8_ServerBusIdentifier,
                                            const uint8_t ou8_ServerNodeIdentifier, std::vector<uint8_t> & orc_Data)
{
   int32_t s32_Return;

   s32_Return = this->ReadTcp(ou32_Handle, orc_Data);

   if (s32_Return == C_NO_ERR)
   {
      if (orc_Data.size() > 4)
      {
         //are source and target address correct ?
         //lint -e{864} //false positive due to const/non-const misinterpretation
         const uint16_t u16_SourceAddress =
            (static_cast<uint16_t>(static_cast<uint16_t>(orc_Data[0]) << 8U) + orc_Data[1]) - 1U;
         //lint -e{864} //false positive due to const/non-const misinterpretation
         const uint16_t u16_TargetAddress =
            (static_cast<uint16_t>(static_cast<uint16_t>(orc_Data[2]) << 8U) + orc_Data[3]) - 1U;
         const uint8_t u8_SourceNodeId = static_cast<uint8_t>(u16_SourceAddress & 0x7FU);
         const uint8_t u8_SourceBusId = static_cast<uint8_t>((u16_SourceAddress >> 7U) & 0x0FU);
         const uint8_t u8_TargetNodeId = static_cast<uint8_t>(u16_TargetAddress & 0x7FU);
         const uint8_t u8_TargetbusId = static_cast<uint8_t>((u16_TargetAddress >> 7U) & 0x0FU);

         if ((u8_SourceNodeId != ou8_ServerNodeIdentifier) ||
             (u8_SourceBusId != ou8_ServerBusIdentifier) ||
             (u8_TargetNodeId != ou8_ClientNodeIdentifier) ||
             (u8_TargetbusId != ou8_ClientBusIdentifier))
         {
            // Save the message for other client server connections over the same IP address
            std::map<C_BufferIdentifier, std::list<std::vector<uint8_t> > >::iterator c_ItBuffer;
            C_BufferIdentifier c_Id(u8_TargetbusId, u8_TargetNodeId, u8_SourceBusId, u8_SourceNodeId);

            mhc_LockBuffer.Acquire();

            // Search for already existing data of this identifier
            c_ItBuffer = mhc_TcpBuffer.find(c_Id);

            if (c_ItBuffer != mhc_TcpBuffer.end())
            {
               c_ItBuffer->second.push_back(orc_Data);
            }
            else
            {
               std::list<std::vector<uint8_t> > c_List;
               c_List.push_back(orc_Data);
               mhc_TcpBuffer.insert(
                  std::pair<C_BufferIdentifier, std::list<std::vector<uint8_t> > >(c_Id, c_List));
            }

            mhc_LockBuffer.Release();

            s32_Return = C_WARN;
         }
         else
         {
            s32_Return = C_NO_ERR;
         }
      }
      else
      {
         s32_Return = C_NOACT;
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read data from TCP buffer of dispatcher

   Jobs to perform:
   - Check if the identifier are matching (starting at byte 0). The header must be already read separately.
   - read those bytes

    The function shall not return any data unless it can provide as many data bytes as specified.

   \param[in]      ou8_ClientBusIdentifier   client identifier of bus
   \param[in]      ou8_ClientNodeIdentifier  client identifier of node
   \param[in]      ou8_ServerBusIdentifier   server identifier of bus
   \param[in]      ou8_ServerNodeIdentifier  server identifier of node
   \param[in,out]  orc_Data                  in: the expected number of bytes is set by the caller (orc_Data.size());
                                             out: received data

   \return
   C_NO_ERR   data read successfully
   C_NOACT    no data for these identifier
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::ReadTcpBuffer(const uint8_t ou8_ClientBusIdentifier,
                                                  const uint8_t ou8_ClientNodeIdentifier,
                                                  const uint8_t ou8_ServerBusIdentifier,
                                                  const uint8_t ou8_ServerNodeIdentifier,
                                                  std::vector<uint8_t> & orc_Data)
{
   int32_t s32_Return = C_NOACT;

   std::map<C_BufferIdentifier, std::list<std::vector<uint8_t> > >::iterator c_ItBuffer;
   C_BufferIdentifier c_Id(ou8_ClientBusIdentifier, ou8_ClientNodeIdentifier, ou8_ServerBusIdentifier,
                           ou8_ServerNodeIdentifier);

   mhc_LockBuffer.Acquire();

   // Search for saved data in the buffer
   c_ItBuffer = mhc_TcpBuffer.find(c_Id);

   if (c_ItBuffer != mhc_TcpBuffer.end())
   {
      std::list<std::vector<uint8_t> > & rc_List = c_ItBuffer->second;

      if (rc_List.size() > 0)
      {
         // Copy the data
         orc_Data = *rc_List.begin();
         // Remove the read data package
         rc_List.erase(rc_List.begin());

         s32_Return = C_NO_ERR;
      }
   }
   mhc_LockBuffer.Release();

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Send broadcast package on UDP request socket

   Jobs to perform:
   - send specified data as broadcast using the configured UDP request socket

   \param[in]  orc_Data   data to sent

   \return
   C_NO_ERR   data sent successfully
   C_CONFIG   required socket not initialized
   C_RD_WR    error sending data
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::SendUdp(const std::vector<uint8_t> & orc_Data)
{
   int32_t s32_Return = C_NO_ERR;

   if (mc_SocketsUdpClient.size() == 0)
   {
      osc_write_log_error("openSYDE IP-TP", "SendUdp called with no socket.");
      s32_Return = C_CONFIG;
   }
   else
   {
      for (uint32_t u32_Interface = 0U; u32_Interface < mc_SocketsUdpClient.size(); u32_Interface++)
      {
         if (mc_SocketsUdpClient[u32_Interface] != INVALID_SOCKET)
         {
            sockaddr_in t_TargetAddress;
            t_TargetAddress.sin_family = AF_INET;
            t_TargetAddress.sin_port = htons(mhu16_UDP_TCP_PORT);      //target port [REQ DoIp-011]
            t_TargetAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST); //lint !e1960 //constant defined by API; no
                                                                       // problem
            //lint -e{740,926,929}  Side-effect of the POSIX-style API. Match is guaranteed by the API.
            //lint !e8080 !e970 //using type to match library interface
            const ssize_t x_Retval = sendto(mc_SocketsUdpClient[u32_Interface],
                                            reinterpret_cast<const char_t *>(&orc_Data[0]),
                                            orc_Data.size(), 0, reinterpret_cast<const sockaddr *>(&t_TargetAddress),
                                            sizeof(t_TargetAddress));
            //lint !e970 //using type to match library interface
            if (x_Retval != static_cast<ssize_t>(orc_Data.size()))
            {
               C_SclString c_ErrnoStr = strerror(errno);
               osc_write_log_error("openSYDE IP-TP", "SendUdp sendto error: " + c_ErrnoStr);
               s32_Return = C_RD_WR;
            }
         }
         else
         {
            // Write error to log, then ignore the error and continue
            osc_write_log_error("openSYDE IP-TP", "SendUdp called with invalid socket(s).");
         }
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read package from UDP socket

   Jobs to perform:
   Try to read incoming datagram from UDP "server" socket.
   Report datagrams sent from:
   * any IP
   * port 13400 (to ignore reception of our own broadcasts and unexpected traffic)
   Return the read data.

   \param[out]  orc_Data   received data
   \param[out]  orau8_Ip   IP address the request was received from

   \return
   C_NO_ERR   datagram read successfully
   C_CONFIG   required socket not initialized
   C_NOACT    no data received
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscIpDispatcherLinuxSock::ReadUdp(std::vector<uint8_t> & orc_Data, uint8_t (&orau8_Ip)[4])
{
   int32_t s32_Return = C_NOACT;

   if (ms32_SocketUdpServer == INVALID_SOCKET)
   {
      osc_write_log_error("openSYDE IP-TP", "ReadUdp called with no socket.");
      s32_Return = C_CONFIG;
   }
   else
   {
      //do we have a package in RX buffer ?
      //lint !e8080 !e970 //using type to match library interface
      int x_SizeInBuffer;
      //lint !e8080 !e970 //using type to match library interface
      int x_Return = ioctl(ms32_SocketUdpServer, FIONREAD, &x_SizeInBuffer);

      if ((x_Return == 0) && (x_SizeInBuffer >= 1))
      {
         sockaddr_in t_Sender;
         socklen_t t_AddressSize = sizeof(t_Sender);
         //enough bytes: read
         orc_Data.resize(x_SizeInBuffer);

         //lint -e{926,929} Side-effect of the "char"-based API. No problems as long as we are on Windows.
         //lint -e{740}     Side-effect of the POSIX-style API. Match is guaranteed by the API.
         x_Return = recvfrom(ms32_SocketUdpServer, reinterpret_cast<char_t *>(&orc_Data[0]),
                             orc_Data.size(), 0, reinterpret_cast<sockaddr *>(&t_Sender),
                             &t_AddressSize);

         //there might be more than one package in the buffer; recvfrom only reads one
         //so we check whether we have more than zero bytes:
         if (x_Return > 0)
         {
            //extract sender address
            const uint32_t u32_IpAddr = ntohl(t_Sender.sin_addr.s_addr);
            orau8_Ip[0] = (u32_IpAddr >> 24) & 0x0FF;
            orau8_Ip[1] = (u32_IpAddr >> 16) & 0x0FF;
            orau8_Ip[2] = (u32_IpAddr >> 8) & 0x0FF;
            orau8_Ip[3] = u32_IpAddr & 0x0FF;

            if (x_Return != static_cast<int>(orc_Data.size())) //lint !e970 //using type to match library interface
            {
               orc_Data.resize(x_Return); //we only need the data we really received
            }

            //filter out local reception of broadcasts we sent ourselves
            //strategy to identify those:
            //Responses from real nodes are sent from Port 13400
            //Our broadcasts are sent with dynamically assigned Port != 13400
            //Also: we should not just accept anything that is thrown upon us ...
            if (t_Sender.sin_port == htons(13400))
            {
               s32_Return = C_NO_ERR;
            }
            else
            {
               s32_Return = C_NOACT;
            }
         }
         else
         {
            //comm error: no data read even though it was reported by ioctl
            osc_write_log_error("openSYDE IP-TP",
                                "ReadUdp unexpected error: data reported as available but reading failed. Reported size: " +
                                C_SclString::IntToStr(
                                   orc_Data.size()) + " Read size: " + C_SclString::IntToStr(s32_Return));
            s32_Return = C_RD_WR;
         }
      }
      else if (x_Return < 0)
      {
         // Write error to log, then ignore the error and continue
         osc_write_log_error("openSYDE IP-TP", "ReadUdp called with invalid socket (ioctl(FIONREAD) failed).");
      }
      else
      {
         // No data available, nothing to do...
      }
   }

   return s32_Return;
}
