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
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/wifi-mac-header.h"
#include "tdma-central-mac.h"

NS_LOG_COMPONENT_DEFINE ("TdmaCentralMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TdmaCentralMac);

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT if (m_low != 0) {std::clog << "[TdmaMac=" << m_low->GetAddress () << "] "; }

TypeId
TdmaCentralMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TdmaCentralMac")
    .SetParent<TdmaMac> ()
    .AddConstructor<TdmaCentralMac> ()
    .AddTraceSource ("MacTx",
                     "A packet has been received from higher layers and is being processed in preparation for "
                     "queueing for transmission.",
                     MakeTraceSourceAccessor (&TdmaCentralMac::m_macTxTrace),
										 "ns3::TdmaCentralMac::MacTxCallback")
    .AddTraceSource ("MacTxDrop",
                     "A packet has been dropped in the MAC layer before being queued for transmission.",
                     MakeTraceSourceAccessor (&TdmaCentralMac::m_macTxDropTrace),
										 "ns3::TdmaCentralMac::MacTxDropCallback")
    .AddTraceSource ("MacPromiscRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&TdmaCentralMac::m_macPromiscRxTrace),
										 "ns3::TdmaCentralMac::MacPromiscRxCallback")
    .AddTraceSource ("MacRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&TdmaCentralMac::m_macRxTrace),
										 "ns3::TdmaCentralMac::MacRxCallback")
    .AddTraceSource ("MacRxDrop",
                     "A packet has been dropped in the MAC layer after it has been passed up from the physical "
                     "layer.",
                     MakeTraceSourceAccessor (&TdmaCentralMac::m_macRxDropTrace),
										 "ns3::TdmaCentralMac::MacRxDropCallback")
  ;
  return tid;
}

TdmaCentralMac::TdmaCentralMac ()
{
  NS_LOG_FUNCTION (this);
  m_isTdmaRunning = false;
  m_low = CreateObject<TdmaMacLow> ();
  m_queue = CreateObject<TdmaMacQueue> ();
  m_queue->SetTdmaMacTxDropCallback (MakeCallback (&TdmaCentralMac::NotifyTxDrop, this));
//  LogComponentEnable ("TdmaCentralMac", LOG_LEVEL_DEBUG);
}
TdmaCentralMac::~TdmaCentralMac ()
{
}

void
TdmaCentralMac::DoDispose (void)
{
  m_low->Dispose ();
  m_low = 0;
  m_device = 0;
  m_queue = 0;
  m_tdmaController = 0;
  TdmaMac::DoDispose ();
}

void
TdmaCentralMac::NotifyTx (Ptr<const Packet> packet)
{
  m_macTxTrace (packet);
}

void
TdmaCentralMac::NotifyTxDrop (Ptr<const Packet> packet)
{
  m_macTxDropTrace (packet);
}

void
TdmaCentralMac::NotifyRx (Ptr<const Packet> packet)
{
  m_macRxTrace (packet);
}

void
TdmaCentralMac::NotifyPromiscRx (Ptr<const Packet> packet)
{
  m_macPromiscRxTrace (packet);
}

void
TdmaCentralMac::NotifyRxDrop (Ptr<const Packet> packet)
{
  m_macRxDropTrace (packet);
}

void
TdmaCentralMac::SetChannel (Ptr<SimpleWirelessChannel> channel)
{
  if (channel != 0)
    {
      m_channel = channel;
      m_tdmaController->SetChannel (channel);
      m_low->SetChannel (m_channel);
    }
}

Ptr<SimpleWirelessChannel>
TdmaCentralMac::GetChannel (void) const
{
  return m_channel;
}

void
TdmaCentralMac::SetTdmaController (Ptr<TdmaController> controller)
{
  m_tdmaController = controller;
}

Ptr<TdmaController>
TdmaCentralMac::GetTdmaController (void) const
{
  return m_tdmaController;
}

void
TdmaCentralMac::SetDevice (Ptr<TdmaNetDevice> device)
{
  m_device = device;
  m_low->SetDevice (m_device);
}

Ptr<TdmaNetDevice>
TdmaCentralMac::GetDevice (void) const
{
  return m_device;
}

Ptr<TdmaMacLow>
TdmaCentralMac::GetTdmaMacLow (void) const
{
  return m_low;
}

void
TdmaCentralMac::SetForwardUpCallback (Callback<void,Ptr<Packet>, Mac48Address, Mac48Address> upCallback)
{
  NS_LOG_FUNCTION (this);
  m_upCallback = upCallback;
}

void
TdmaCentralMac::SetLinkUpCallback (Callback<void> linkUp)
{
  linkUp ();
}

void
TdmaCentralMac::SetTxQueueStartCallback (Callback<bool,uint32_t> queueStart)
{
  NS_LOG_FUNCTION (this);
  m_queueStart = queueStart;
}

void
TdmaCentralMac::SetTxQueueStopCallback (Callback<bool,uint32_t> queueStop)
{
  NS_LOG_FUNCTION (this);
  m_queueStop = queueStop;
}

uint32_t
TdmaCentralMac::GetQueueState (uint32_t index)
{
  if (m_queue->GetMaxSize () == m_queue->GetSize ())
    {
      return 0;
    }
  else
    {
      return 1;
    }
}

uint32_t
TdmaCentralMac::GetNQueues (void)
{
  //TDMA currently has only one queue
  return 1;
}

void
TdmaCentralMac::SetLinkDownCallback (Callback<void> linkDown)
{
}

void
TdmaCentralMac::SetMaxQueueSize (uint32_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_queue->SetMaxSize (size);
}
void
TdmaCentralMac::SetMaxQueueDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_queue->SetMaxDelay (delay);
}


Mac48Address
TdmaCentralMac::GetAddress (void) const
{
  return m_low->GetAddress ();
}
Ssid
TdmaCentralMac::GetSsid (void) const
{
  return m_ssid;
}
void
TdmaCentralMac::SetAddress (Mac48Address address)
{
  NS_LOG_FUNCTION (address);
  m_low->SetAddress (address);
  m_low->SetBssid (address);
}
void
TdmaCentralMac::SetSsid (Ssid ssid)
{
  NS_LOG_FUNCTION (ssid);
  m_ssid = ssid;
}
Mac48Address
TdmaCentralMac::GetBssid (void) const
{
  return m_low->GetBssid ();
}

void
TdmaCentralMac::ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << from);
  //NotifyRx(packet);
  m_upCallback (packet, from, to);
}

void
TdmaCentralMac::Queue (Ptr<const Packet> packet, const WifiMacHeader &hdr)
{
  NS_LOG_FUNCTION (this << packet << &hdr);
  if (!m_queue->Enqueue (packet, hdr))
    {
      NotifyTxDrop (packet);
    }
  //Cannot request for channel access in tdma. Tdma schedules every node in round robin manner
  //RequestForChannelAccess();
}

void
TdmaCentralMac::StartTransmission (uint64_t transmissionTimeUs)
{
  NS_LOG_DEBUG (transmissionTimeUs << " usec");
  Time totalTransmissionSlot = MicroSeconds (transmissionTimeUs);
  if (m_queue->IsEmpty ())
    {
      NS_LOG_DEBUG ("queue empty");
      return;
    }
  WifiMacHeader header;
  Ptr<const Packet> peekPacket = m_queue->Peek (&header);
  Time packetTransmissionTime = m_tdmaController->CalculateTxTime (peekPacket);
  NS_LOG_DEBUG ("Packet TransmissionTime(microSeconds): " << packetTransmissionTime.GetMicroSeconds () << "usec");
  if (packetTransmissionTime < totalTransmissionSlot)
    {
      totalTransmissionSlot -= packetTransmissionTime;
      Simulator::Schedule (packetTransmissionTime, &TdmaCentralMac::SendPacketDown, this,totalTransmissionSlot);
    }
  else
    {
      NS_LOG_DEBUG ("Packet takes more time to transmit than the slot allotted. Will send in next slot");
    }
}

void
TdmaCentralMac::SendPacketDown (Time remainingTime)
{
  WifiMacHeader header;
  Ptr<const Packet> packet = m_queue->Dequeue (&header);
  m_low->StartTransmission (packet, &header);
  TxQueueStart (0);
  NotifyTx (packet);
  TxQueueStart (0);
  StartTransmission (remainingTime.GetMicroSeconds ());
}

void
TdmaCentralMac::Enqueue (Ptr<const Packet> packet, Mac48Address to, Mac48Address from)
{
  NS_LOG_FUNCTION (this << packet << to << from);
  WifiMacHeader hdr;
  //hdr.SetTypeData ();
  hdr.SetType(WifiMacType::WIFI_MAC_DATA);
  hdr.SetAddr1 (to);
  hdr.SetAddr2 (GetAddress ());
  hdr.SetAddr3 (from);
  hdr.SetDsFrom ();
  hdr.SetDsNotTo ();
  Queue (packet, hdr);
}
void
TdmaCentralMac::Enqueue (Ptr<const Packet> packet, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << to);
  WifiMacHeader hdr;
  //hdr.SetTypeData ();
  hdr.SetType(WifiMacType::WIFI_MAC_DATA);
  hdr.SetAddr1 (to);
  hdr.SetAddr2 (GetAddress ());
  hdr.SetAddr3 (m_low->GetAddress ());
  hdr.SetDsFrom ();
  hdr.SetDsNotTo ();
  Queue (packet, hdr);
  NS_LOG_FUNCTION (this << packet << to);
}
bool
TdmaCentralMac::SupportsSendFrom (void) const
{
  return true;
}

void
TdmaCentralMac::TxOk (const WifiMacHeader &hdr)
{
}
void
TdmaCentralMac::TxFailed (const WifiMacHeader &hdr)
{
}

void
TdmaCentralMac::TxQueueStart (uint32_t index)
{
  NS_ASSERT (index < GetNQueues ());
  m_queueStart (index);
}
void
TdmaCentralMac::TxQueueStop (uint32_t index)
{
  NS_ASSERT (index < GetNQueues ());
  m_queueStop (index);
}

void
TdmaCentralMac::Receive (Ptr<Packet> packet, const WifiMacHeader *hdr)
{
  ForwardUp (packet, hdr->GetAddr3 (), hdr->GetAddr1 ());
}

void
TdmaCentralMac::Initialize ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_isTdmaRunning = true;
  m_queue->SetMacPtr (this);
  m_tdmaController->Start ();
  m_low->SetRxCallback (MakeCallback (&TdmaCentralMac::Receive, this));
  TdmaMac::DoInitialize ();
}

} // namespace ns3
