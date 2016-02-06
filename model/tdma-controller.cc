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
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "tdma-central-mac.h"
#include "tdma-controller.h"
#include "tdma-mac.h"
#include "tdma-mac-low.h"
#include "ns3/abort.h"

NS_LOG_COMPONENT_DEFINE ("TdmaController");

#define MY_DEBUG(x) \
  NS_LOG_DEBUG (Simulator::Now () << " " << this << " " << x)

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (TdmaController);

Time
TdmaController::GetDefaultSlotTime (void)
{
  return MicroSeconds (1100);
}

Time
TdmaController::GetDefaultGuardTime (void)
{
  return MicroSeconds (100);
}

DataRate
TdmaController::GetDefaultDataRate (void)
{
  NS_LOG_DEBUG ("Setting default");
  return DataRate ("11000000b/s");
}

/*************************************************************
 * Tdma Controller Class Functions
 ************************************************************/
TypeId
TdmaController::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3:TdmaController")
    .SetParent<Object> ()
    .AddConstructor<TdmaController> ()
    .AddAttribute ("DataRate",
                   "The default data rate for point to point links",
                   DataRateValue (GetDefaultDataRate ()),
                   MakeDataRateAccessor (&TdmaController::SetDataRate,
                                         &TdmaController::GetDataRate),
                   MakeDataRateChecker ())
    .AddAttribute ("SlotTime", "The duration of a Slot in microseconds.",
                   TimeValue (GetDefaultSlotTime ()),
                   MakeTimeAccessor (&TdmaController::SetSlotTime,
                                     &TdmaController::GetSlotTime),
                   MakeTimeChecker ())
    .AddAttribute ("GuardTime", "GuardTime between TDMA slots in microseconds.",
                   TimeValue (GetDefaultGuardTime ()),
                   MakeTimeAccessor (&TdmaController::SetGuardTime,
                                     &TdmaController::GetGuardTime),
                   MakeTimeChecker ())
    .AddAttribute ("InterFrameTime", "The wait time between consecutive tdma frames.",
                   TimeValue (MicroSeconds (0)),
                   MakeTimeAccessor (&TdmaController::SetInterFrameTimeInterval,
                                     &TdmaController::GetInterFrameTimeInterval),
                   MakeTimeChecker ())
    .AddAttribute ("TdmaMode","Tdma Mode, Centralized",
                   EnumValue (CENTRALIZED),
                   MakeEnumAccessor (&TdmaController::m_tdmaMode),
                   MakeEnumChecker (CENTRALIZED, "Centralized"));
  return tid;
}

TdmaController::TdmaController ()
  : m_slotTime (0),
		m_guardTime (0),
		m_tdmaFrameLength (0),
		m_tdmaInterFrameTime (0),
		m_totalSlotsAllowed (10000),
		m_activeEpoch (false),
		m_tdmaMode (CENTRALIZED),
    m_channel (0)
{
  NS_LOG_FUNCTION (this);
//  LogComponentEnable ("TdmaController", LOG_LEVEL_DEBUG);
  NS_LOG_DEBUG("TdmaController constructor");
}

TdmaController::~TdmaController ()
{
  m_channel = 0;
  m_bps = 0;
  m_slotPtrs.clear ();
}

void
TdmaController::Start (void)
{
  NS_LOG_FUNCTION (this);
  if (!m_activeEpoch)
    {
      m_activeEpoch = true;
      Simulator::Schedule (NanoSeconds (10),&TdmaController::StartTdmaSessions, this);
    }
}

void
TdmaController::StartTdmaSessions (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  ScheduleTdmaSession (0);
}

void
TdmaController::AddTdmaSlot (uint32_t slotPos, Ptr<TdmaMac> macPtr)
{
  NS_LOG_FUNCTION (slotPos << macPtr);
  std::pair<std::map<uint32_t, Ptr<TdmaMac> >::iterator, bool> result =
    m_slotPtrs.insert (std::make_pair (slotPos,macPtr));
  if (result.second == true)
    {
      NS_LOG_DEBUG ("Added mac : " << macPtr << " in slot " << slotPos);
    }
  else
    {
      NS_LOG_WARN ("Could not add mac: " << macPtr << " to slot " << slotPos);
    }
}

void
TdmaController::SetSlotTime (Time slotTime)
{
  NS_LOG_FUNCTION (this << slotTime);
  m_slotTime = slotTime.GetMicroSeconds ();
}

Time
TdmaController::GetSlotTime (void) const
{
  return MicroSeconds (m_slotTime);
}

void
TdmaController::SetDataRate (DataRate bps)
{
  NS_LOG_FUNCTION (this << bps);
  m_bps = bps;
}

DataRate
TdmaController::GetDataRate (void) const
{
  return m_bps;
}

void
TdmaController::SetChannel (Ptr<SimpleWirelessChannel> c)
{
  NS_LOG_FUNCTION (this << c);
  m_channel = c;
}


Ptr<SimpleWirelessChannel>
TdmaController::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_channel;
}

void
TdmaController::SetGuardTime (Time guardTime)
{
  NS_LOG_FUNCTION (this << guardTime);
  //guardTime is based on the SimpleWirelessChannel's max range
  if (m_channel != 0)
    {
      m_guardTime = Seconds (m_channel->GetMaxRange () / 300000000.0).GetMicroSeconds ();
    }
  else
    {
      m_guardTime = guardTime.GetMicroSeconds ();
    }
}

Time
TdmaController::GetGuardTime (void) const
{
  return MicroSeconds (m_guardTime);
}

void
TdmaController::SetInterFrameTimeInterval (Time interFrameTime)
{
  NS_LOG_FUNCTION (interFrameTime);
  m_tdmaInterFrameTime = interFrameTime.GetMicroSeconds ();
}

Time
TdmaController::GetInterFrameTimeInterval (void) const
{
  return MicroSeconds (m_tdmaInterFrameTime);
}

void
TdmaController::SetTotalSlotsAllowed (uint32_t slotsAllowed)
{
  m_totalSlotsAllowed = slotsAllowed;
  m_slotPtrs.clear ();
}

uint32_t
TdmaController::GetTotalSlotsAllowed (void) const
{
  return m_totalSlotsAllowed;
}

void
TdmaController::ScheduleTdmaSession (const uint32_t slotNum)
{
  NS_LOG_FUNCTION (slotNum);
  std::map<uint32_t, Ptr<TdmaMac> >::iterator it = m_slotPtrs.find (slotNum);
  if (it == m_slotPtrs.end ())
    {
      NS_LOG_WARN ("No MAC ptrs in TDMA controller");
    }
  uint32_t numOfSlotsAllotted = 1;
  while (1)
    {
      std::map<uint32_t, Ptr<TdmaMac> >::iterator j = m_slotPtrs.find (slotNum + numOfSlotsAllotted);
      if (j != m_slotPtrs.end ())
        {
          if (it->second == j->second)
            {
              numOfSlotsAllotted++;
            }
          else
            {
              break;
            }
        }
      else
        {
          break;
        }
    }
  NS_LOG_DEBUG ("Number of slots allotted for this node is: " << numOfSlotsAllotted);
  Time transmissionSlot = MicroSeconds (GetSlotTime ().GetMicroSeconds () * numOfSlotsAllotted);
  Time totalTransmissionTimeUs = GetGuardTime () + transmissionSlot;
  NS_ASSERT (it->second != NULL);
  it->second->StartTransmission (transmissionSlot.GetMicroSeconds ());
  if ((slotNum + numOfSlotsAllotted) == GetTotalSlotsAllowed ())
    {
      NS_LOG_DEBUG ("Starting over all sessions again");
      Simulator::Schedule ((totalTransmissionTimeUs + GetInterFrameTimeInterval ()), &TdmaController::StartTdmaSessions, this);
    }
  else
    {
      NS_LOG_DEBUG ("Scheduling next session");
      Simulator::Schedule (totalTransmissionTimeUs, &TdmaController::ScheduleTdmaSession, this, (slotNum + numOfSlotsAllotted));
    }
}

Time
TdmaController::CalculateTxTime (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (*packet);
  NS_ASSERT_MSG (packet->GetSize () < 1500,"PacketSize must be less than 1500B, it is: " << packet->GetSize ());
  return Seconds (m_bps.CalculateBytesTxTime (packet->GetSize ()));
}

} // namespace ns3
