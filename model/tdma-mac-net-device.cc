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
#include "tdma-mac.h"
#include "tdma-mac-net-device.h"
#include "ns3/llc-snap-header.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/node.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("TdmaNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TdmaNetDevice);

TypeId
TdmaNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TdmaNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<TdmaNetDevice> ()
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH),
                   MakeUintegerAccessor (&TdmaNetDevice::SetMtu,
                                         &TdmaNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> (1,MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH))
    .AddAttribute ("Mac", "The MAC layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&TdmaNetDevice::GetMac,
                                        &TdmaNetDevice::SetMac),
                   MakePointerChecker<TdmaMac> ())
    .AddAttribute ("Channel", "The channel attached to this device",
                   PointerValue (),
                   MakePointerAccessor (&TdmaNetDevice::DoGetChannel,
                                        &TdmaNetDevice::SetChannel),
                   MakePointerChecker<SimpleWirelessChannel> ())
    .AddAttribute ("TdmaController", "The tdma controller attached to this device",
                   PointerValue (),
                   MakePointerAccessor (&TdmaNetDevice::GetTdmaController,
                                        &TdmaNetDevice::SetTdmaController),
                   MakePointerChecker<TdmaController> ());
  return tid;
}

TdmaNetDevice::TdmaNetDevice ()
  : m_ifIndex (0),
		m_linkUp (false),
		m_mtu (0),
    m_configComplete (false)
{
  NS_LOG_FUNCTION_NOARGS ();
//  LogComponentEnable ("TdmaNetDevice", LOG_LEVEL_DEBUG);
}
TdmaNetDevice::~TdmaNetDevice ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
TdmaNetDevice::DoDispose (void)
{
  m_node = 0;
  m_mac->Dispose ();
  m_mac = 0;
  m_channel = 0;
  m_tdmaController = 0;
  // chain up.
  NetDevice::DoDispose ();
}

void
TdmaNetDevice::DoInitialize (void)
{
  m_mac->Initialize ();
  NetDevice::DoInitialize ();
}

void
TdmaNetDevice::CompleteConfig (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (m_mac == 0
      || m_node == 0
      || m_channel == 0
      || m_tdmaController == 0
      || m_configComplete)
    {
      return;
    }
  //setup mac
  m_mac->SetTdmaController (m_tdmaController);
  m_mac->SetChannel (m_channel);
  // setup callbacks
  m_mac->SetForwardUpCallback (MakeCallback (&TdmaNetDevice::ForwardUp, this));
  m_mac->SetLinkUpCallback (MakeCallback (&TdmaNetDevice::LinkUp, this));
  m_mac->SetLinkDownCallback (MakeCallback (&TdmaNetDevice::LinkDown, this));
  m_mac->SetTxQueueStartCallback (MakeCallback (&TdmaNetDevice::TxQueueStart, this));
  m_mac->SetTxQueueStopCallback (MakeCallback (&TdmaNetDevice::TxQueueStop, this));
  m_configComplete = true;
}

void
TdmaNetDevice::SetMac (Ptr<TdmaMac> mac)
{
  m_mac = mac;
  m_mac->SetDevice (this);
  CompleteConfig ();
}
Ptr<TdmaMac>
TdmaNetDevice::GetMac (void) const
{
  return m_mac;
}

void
TdmaNetDevice::SetTdmaController (Ptr<TdmaController> controller)
{
  m_tdmaController = controller;
  CompleteConfig ();
}

Ptr<TdmaController>
TdmaNetDevice::GetTdmaController (void) const
{
  return m_tdmaController;
}

Ptr<Node>
TdmaNetDevice::GetNode (void) const
{
  return m_node;
}

void
TdmaNetDevice::SetNode (Ptr<Node> node)
{
  m_node = node;
  CompleteConfig ();
}

void
TdmaNetDevice::SetChannel (Ptr<SimpleWirelessChannel> channel)
{
  if (channel != 0)
    {
      m_channel = channel;
      CompleteConfig ();
    }
}

Ptr<Channel>
TdmaNetDevice::GetChannel (void) const
{
  return m_channel;
}

Ptr<SimpleWirelessChannel>
TdmaNetDevice::DoGetChannel (void) const
{
  return m_channel;
}

void
TdmaNetDevice::SetIfIndex (const uint32_t index)
{
  m_ifIndex = index;
}

uint32_t
TdmaNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}

void
TdmaNetDevice::SetAddress (Address address)
{
  m_mac->SetAddress (Mac48Address::ConvertFrom (address));
}

Address
TdmaNetDevice::GetAddress (void) const
{
  return m_mac->GetAddress ();
}

bool
TdmaNetDevice::SetMtu (const uint16_t mtu)
{
  if (mtu > MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH)
    {
      return false;
    }
  m_mtu = mtu;
  return true;
}

uint16_t
TdmaNetDevice::GetMtu (void) const
{
  return m_mtu;
}

bool
TdmaNetDevice::IsLinkUp (void) const
{
  return m_linkUp;
}

void
TdmaNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  m_linkChanges.ConnectWithoutContext (callback);
}

bool
TdmaNetDevice::IsBroadcast (void) const
{
  return true;
}

Address
TdmaNetDevice::GetBroadcast (void) const
{
  return Mac48Address::GetBroadcast ();
}

bool
TdmaNetDevice::IsMulticast (void) const
{
  return true;
}

Address
TdmaNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  return Mac48Address::GetMulticast (multicastGroup);
}

Address TdmaNetDevice::GetMulticast (Ipv6Address addr) const
{
  return Mac48Address::GetMulticast (addr);
}

bool
TdmaNetDevice::IsPointToPoint (void) const
{
  return false;
}

bool
TdmaNetDevice::IsBridge (void) const
{
  return false;
}

bool
TdmaNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (*packet << " Dest:" << dest << " ProtocolNo:" << protocolNumber);
  NS_ASSERT (Mac48Address::IsMatchingType (dest));
  Mac48Address realTo = Mac48Address::ConvertFrom (dest);
//  Mac48Address realFrom = Mac48Address::ConvertFrom (GetAddress ());
  LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet->AddHeader (llc);
  m_mac->Enqueue (packet, realTo);
  return true;
}

bool
TdmaNetDevice::NeedsArp (void) const
{
  return true;
}

void
TdmaNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_forwardUp = cb;
}

void
TdmaNetDevice::ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
  NS_LOG_FUNCTION (*packet << from << to);
  LlcSnapHeader llc;
  packet->RemoveHeader (llc);
  enum NetDevice::PacketType type;
  if (to.IsBroadcast ())
    {
      NS_LOG_DEBUG ("NetDevice::PACKET_BROADCAST");
      type = NetDevice::PACKET_BROADCAST;
    }
  else if (to.IsGroup ())
    {
      NS_LOG_DEBUG ("NetDevice::PACKET_MULTICAST");
      type = NetDevice::PACKET_MULTICAST;
    }
  else if (to == m_mac->GetAddress ())
    {
      NS_LOG_DEBUG ("NetDevice::PACKET_HOST");
      type = NetDevice::PACKET_HOST;
    }
  else
    {
      NS_LOG_DEBUG ("NetDevice::PACKET_OTHERHOST");
      type = NetDevice::PACKET_OTHERHOST;
    }

  if (type != NetDevice::PACKET_OTHERHOST)
    {
      m_mac->NotifyRx (packet);
      m_forwardUp (this, packet, llc.GetType (), from);
    }

  if (!m_promiscRx.IsNull ())
    {
      m_mac->NotifyPromiscRx (packet);
      m_promiscRx (this, packet, llc.GetType (), from, to, type);
    }
}

void
TdmaNetDevice::LinkUp (void)
{
  m_linkUp = true;
  m_linkChanges ();
}

void
TdmaNetDevice::LinkDown (void)
{
  m_linkUp = false;
  m_linkChanges ();
}

bool
TdmaNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (Mac48Address::IsMatchingType (dest));
  NS_ASSERT (Mac48Address::IsMatchingType (source));
  Mac48Address realTo = Mac48Address::ConvertFrom (dest);
  Mac48Address realFrom = Mac48Address::ConvertFrom (source);
  LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet->AddHeader (llc);
  m_mac->Enqueue (packet, realTo, realFrom);
  return true;
}

void
TdmaNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  m_promiscRx = cb;
}

bool
TdmaNetDevice::SupportsSendFrom (void) const
{
  return m_mac->SupportsSendFrom ();
}

bool
TdmaNetDevice::TxQueueStart (uint32_t index)
{
  m_queueStateChanges (index);
  return true;
}

bool
TdmaNetDevice::TxQueueStop (uint32_t index)
{
  m_queueStateChanges (index);
  return true;
}

uint32_t
TdmaNetDevice::GetQueueState (uint32_t index)
{
  return m_mac->GetQueueState (index);
}

uint32_t
TdmaNetDevice::GetNQueues (void)
{
  return m_mac->GetNQueues ();
}

void
TdmaNetDevice::SetQueueStateChangeCallback (Callback<void,uint32_t> callback)
{
  m_queueStateChanges.ConnectWithoutContext (callback);
}

} // namespace ns3

