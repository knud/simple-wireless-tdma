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
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "tdma-mac-queue.h"

using namespace std;
NS_LOG_COMPONENT_DEFINE ("TdmaMacQueue");
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TdmaMacQueue);

TdmaMacQueue::Item::Item (Ptr<const Packet> packet,
                          const WifiMacHeader &hdr,
                          Time tstamp)
  : packet (packet),
    hdr (hdr),
    tstamp (tstamp)
{
}

TypeId
TdmaMacQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TdmaMacQueue")
    .SetParent<Object> ()
    .AddConstructor<TdmaMacQueue> ()
    .AddAttribute ("MaxPacketNumber", "If a packet arrives when there are already this number of packets, it is dropped.",
                   UintegerValue (400),
                   MakeUintegerAccessor (&TdmaMacQueue::m_maxSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxDelay", "If a packet stays longer than this delay in the queue, it is dropped.",
                   TimeValue (Seconds (10.0)),
                   MakeTimeAccessor (&TdmaMacQueue::m_maxDelay),
                   MakeTimeChecker ())
  ;
  return tid;
}

TdmaMacQueue::TdmaMacQueue ()
  : m_size (0),
		m_maxSize (0),
    m_count (0)
{
  NS_LOG_FUNCTION_NOARGS ();
//  LogComponentEnable ("TdmaMacQueue", LOG_LEVEL_DEBUG);
}

TdmaMacQueue::~TdmaMacQueue ()
{
  Flush ();
}

void
TdmaMacQueue::SetMaxSize (uint32_t maxSize)
{
  m_maxSize = maxSize;
}

void
TdmaMacQueue::SetMacPtr (Ptr<TdmaMac> macPtr)
{
  m_macPtr = macPtr;
}

void
TdmaMacQueue::SetMaxDelay (Time delay)
{
  m_maxDelay = delay;
}

void
TdmaMacQueue::SetTdmaMacTxDropCallback (Callback<void,Ptr<const Packet> > callback)
{
  m_txDropCallback = callback;
}

uint32_t
TdmaMacQueue::GetMaxSize (void) const
{
  return m_maxSize;
}

Time
TdmaMacQueue::GetMaxDelay (void) const
{
  return m_maxDelay;
}

bool
TdmaMacQueue::Enqueue (Ptr<const Packet> packet, const WifiMacHeader &hdr)
{
  NS_LOG_DEBUG ("Queue Size: " << GetSize () << " Max Size: " << GetMaxSize ());
  Cleanup ();
  if (m_size == m_maxSize)
    {
      return false;
    }
  Time now = Simulator::Now ();
  m_queue.push_back (Item (packet, hdr, now));
  m_size++;
  NS_LOG_DEBUG ("Inserted packet of size: " << packet->GetSize ()
                                            << " uid: " << packet->GetUid ());
  return true;
}

void
TdmaMacQueue::Cleanup (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (m_queue.empty ())
    {
      return;
    }
  Time now = Simulator::Now ();
  uint32_t n = 0;
  for (PacketQueueI i = m_queue.begin (); i != m_queue.end (); )
    {
      if (i->tstamp + m_maxDelay > now)
        {
          i++;
        }
      else
        {
          m_count++;
          NS_LOG_DEBUG (Simulator::Now ().GetSeconds () << "s Dropping this packet as its exceeded queue time, pid: " << i->packet->GetUid ()
                                                        << " macPtr: " << m_macPtr
                                                        << " queueSize: " << m_queue.size ()
                                                        << " count:" << m_count);
          m_txDropCallback (i->packet);
          i = m_queue.erase (i);
          n++;
        }
    }
  m_size -= n;
}

Ptr<const Packet>
TdmaMacQueue::Dequeue (WifiMacHeader *hdr)
{
  NS_LOG_FUNCTION_NOARGS ();
  Cleanup ();
  if (!m_queue.empty ())
    {
      Item i = m_queue.front ();
      m_queue.pop_front ();
      m_size--;
      *hdr = i.hdr;
      NS_LOG_DEBUG ("Dequeued packet of size: " << i.packet->GetSize ());
      return i.packet;
    }
  return 0;
}

Ptr<const Packet>
TdmaMacQueue::Peek (WifiMacHeader *hdr)
{
  NS_LOG_FUNCTION_NOARGS ();
  Cleanup ();
  if (!m_queue.empty ())
    {
      Item i = m_queue.front ();
      *hdr = i.hdr;
      return i.packet;
    }
  return 0;
}

bool
TdmaMacQueue::IsEmpty (void)
{
  Cleanup ();
  return m_queue.empty ();
}

uint32_t
TdmaMacQueue::GetSize (void)
{
  return m_size;
}

void
TdmaMacQueue::Flush (void)
{
  m_queue.erase (m_queue.begin (), m_queue.end ());
  m_size = 0;
}

Mac48Address
TdmaMacQueue::GetAddressForPacket (enum WifiMacHeader::AddressType type, PacketQueueI it)
{
  if (type == WifiMacHeader::ADDR1)
    {
      return it->hdr.GetAddr1 ();
    }
  if (type == WifiMacHeader::ADDR2)
    {
      return it->hdr.GetAddr2 ();
    }
  if (type == WifiMacHeader::ADDR3)
    {
      return it->hdr.GetAddr3 ();
    }
  return 0;
}

bool
TdmaMacQueue::Remove (Ptr<const Packet> packet)
{
  PacketQueueI it = m_queue.begin ();
  for (; it != m_queue.end (); it++)
    {
      if (it->packet == packet)
        {
          m_queue.erase (it);
          m_size--;
          return true;
        }
    }
  return false;
}
} // namespace ns3
