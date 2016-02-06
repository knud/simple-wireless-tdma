/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Hemanth Narra
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
 * Author: Hemanth Narra <hemanthnarra222@gmail.com>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */
#ifndef TDMA_NET_DEVICE_H
#define TDMA_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/mac48-address.h"
#include "ns3/tdma-mac.h"
#include "ns3/simple-wireless-channel.h"
#include "tdma-controller.h"
#include <string>

namespace ns3 {

class SimpleWirelessChannel;
class TdmaController;
/**
 * \brief Hold together all Tdma-related objects.
 *
 * This class holds together ns3::SimpleWirelessChannel and
 * ns3::TdmaMac
 */
class TdmaNetDevice : public NetDevice
{
public:
  static TypeId GetTypeId (void);

  TdmaNetDevice ();
  virtual ~TdmaNetDevice ();

  /**
   * \param mac the mac layer to use.
   */
  void SetMac (Ptr<TdmaMac> mac);
  /**
   * \returns the mac we are currently using.
   */
  Ptr<TdmaMac> GetMac (void) const;
  /**
   * \param channel The channel this device is attached to
   */
  void SetChannel (Ptr<SimpleWirelessChannel> channel);
  /**
   * \param controller The tdma controller this device is attached to
   */
  void SetTdmaController (Ptr<TdmaController> controller);

  // inherited from NetDevice base class.
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool IsBridge (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);

  virtual Address GetMulticast (Ipv6Address addr) const;

  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

  virtual uint32_t GetQueueState (uint32_t index);
  virtual uint32_t GetNQueues (void);
  virtual void SetQueueStateChangeCallback (Callback<void,uint32_t> callback);

private:
  // This value conforms to the 802.11 specification
  static const uint16_t MAX_MSDU_SIZE = 2304;

  virtual void DoDispose (void);
  virtual void DoInitialize (void);
  void ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to);
  void LinkUp (void);
  void LinkDown (void);
  void Setup (void);
  Ptr<SimpleWirelessChannel> DoGetChannel (void) const;
  Ptr<TdmaController> GetTdmaController (void) const;
  void CompleteConfig (void);

  Ptr<Node> m_node;
  Ptr<TdmaMac> m_mac;
  Ptr<SimpleWirelessChannel> m_channel;
  Ptr<TdmaController> m_tdmaController;
  NetDevice::ReceiveCallback m_forwardUp;
  NetDevice::PromiscReceiveCallback m_promiscRx;

  TracedCallback<Ptr<const Packet>, Mac48Address> m_rxLogger;
  TracedCallback<Ptr<const Packet>, Mac48Address> m_txLogger;

  bool TxQueueStart (uint32_t index);
  bool TxQueueStop (uint32_t index);
  TracedCallback<uint32_t> m_queueStateChanges;

  uint32_t m_ifIndex;
  bool m_linkUp;
  TracedCallback<> m_linkChanges;
  mutable uint16_t m_mtu;
  bool m_configComplete;
};

} // namespace ns3

#endif /* TDMA_NET_DEVICE_H */
