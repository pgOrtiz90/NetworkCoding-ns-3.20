/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Universidad de Cantabria
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: David Gómez Fernández <dgomez@tlmat.unican.es>
 *		   Ramón Agüero Calvo <ramon@tlmat.unican.es>
 */

#ifndef NETWORK_CODING_L4_PROTOCOL_H_
#define NETWORK_CODING_L4_PROTOCOL_H_


#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include "ns3/trace-stats.h"

#include <stdio.h>
#include <math.h>

const bool g_debug = false;

/**
 * \brief Dumps a memory block in hexadecimal
 *
 * This prints a chunk of memory in hexadecimal format, this may be appropriate
 * e.g. for debugging transmitted/received packets, as well as addresses (MAC), etc
 *
 * \param label Reference of what it is being dumped
 * \param data Pointer to the first memory position
 * \param len Lenght of the data to be dumped
 */
#define PRINT   	printf
#define DBG_DUMP(label, data, len) \
	{if(g_debug) \
	{ int i; \
	PRINT("%-4s [%2d] ", label, len) ; \
	for (i=0;i<len;i++) { \
		if(i%8 == 0 && i!=0) { \
			PRINT("\n") ; PRINT("          ") ; } \
			PRINT("%02X ", ((unsigned char*) data)[i]) ; } \
			PRINT("\n") ; \
	}}

namespace ns3 {


typedef u_int16_t hashId_t;

/**
 * Classed used for defining the Network Coding layer which will be in charge of handling the encoding/decoding decisions. The current implementation carries on a XOR coding mechanism (we envisage to substitute with a
 * more complex one, i.e. Random Linear Coding
 */
class NetworkCodingL4Protocol: public IpL4Protocol {
public:
	//Callbacks definition
	typedef Callback<enum IpL4Protocol::RxStatus, Ptr<Packet>, Ipv4Header const &, Ptr<Ipv4Interface> > UpTargetCallback;

	/**
	 * Attribute handler
	 */
	static TypeId GetTypeId (void);

	/**
	 * Default constructor
	 */
	NetworkCodingL4Protocol();

	/**
	 * Default destructor
	 */
	virtual ~NetworkCodingL4Protocol();

    /**
     *	Locate in which node the SimpleNetworkCodingObject is stored
     *	\param node Pointer to the node that contains this object (or any of its derived classes)
     */
    virtual void SetNode (Ptr <Node> node);

    /**
     * \returns Pointer to the node that contains this object (or any of its derived classes)
     */
    virtual Ptr<Node> GetNode ();

	/**
	 * \param packet packet to send
	 * \param source source address of packet
	 * \param destination address of packet
	 * \param protocol number of packet
	 * \param route route entry
	 *
	 * Higher-level layers call this method to send a packet
	 * down the stack to the IP level
	 */
	virtual void ReceiveFromUpperLayer (Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route) = 0;
	/**
	   * \param p packet to forward up
	   * \param header IPv4 Header information
	   * \param incomingInterface the Ipv4Interface on which the packet arrived
	   * Called from lower-level layers to send the packet up
	   * in the stack.
	   */
	virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> packet, Ipv4Header const &header, Ptr<Ipv4Interface> incomingInterface) = 0;  	//Old version, inherited form base clase Ipv4L4Protocol
	/**
	 * As we need to overhear all the packets incoming from the node's surrounding area, we must configure promiscuous nodes. For that purpose, we need to hook a method through the method
	 * WifiNetDevice::SetPromiscReceiveCallback, which will invoke this method (since we have linked them). Therefore, a "promiscuous" packet will be bypassed directly from the Link Layer
	 * the Network Coding level, thus passing through the network layer. Therefore, we must discard the IP header in order to get execution exceptions.
	 * NOTE: It is worth highlighting that the callback method will deliver every received frame (including if the node is the expected receiver), hence he have to be careful of parsing
	 * duplicated packets
	 * \param device The NetDevice object from which we will receive the packet
	 * \param packet The overheard packet
	 * \param protocol LLC Header's protocol type
	 * \param from Source address (class Address)
	 * \param to Destination address (class Address)
	 * \param packetType NetDevice::PacketType enumerate
	 * \returns True if success; false otherwise
	 */
	virtual bool ReceivePromiscuous (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType) = 0;

	/**
	 * This method allows a caller to set the current down target callback set for this L4 protocol
	 * \param cb current Callback for the L4 protocol
	 */
	virtual inline void SetDownTarget (IpL4Protocol::DownTargetCallback cb) {m_downTarget = cb;}
	/**
	 * This method allows a caller to get the current down target callback set for this L4 protocol
	 * \return current Callback for the L4 protocol
	 */
	virtual inline IpL4Protocol::DownTargetCallback GetDownTarget (void) const {return m_downTarget;}

	/**
	 * This method allows a caller to get the current down target callback
	 * set for this L4 protocol (IPv6 case)
	 *
	 * \return current Callback for the L4 protocol
	 */
	virtual DownTargetCallback6 GetDownTarget6 (void) const {return m_downTarget6;};

	/**
	 * This method allows a caller to set the current down target callback
	 * set for this L4 protocol (IPv6 case)
	 *
	 * \param cb current Callback for the L4 protocol
	 */
	virtual void SetDownTarget6 (DownTargetCallback6 cb){m_downTarget6 = cb;};

	/**
	 * Return the NC statistics
	 */
	NetworkCodingLayerStats_t GetStats(){return m_stats;};

protected:
	IpL4Protocol::DownTargetCallback m_downTarget;
	IpL4Protocol::DownTargetCallback6 m_downTarget6; //!< Callback to send packets over IPv6 (not implemented yet)

	Ptr<Node> m_node;								//Pointer to the node that contains the NC layer
	NetworkCodingLayerStats_t m_stats;
};


}	//End namespace ns3
#endif /* NETWORK_CODING_L4_PROTOCOL_H_ */
