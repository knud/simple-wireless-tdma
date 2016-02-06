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
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("TdmaMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TdmaMac);

Time
TdmaMac::GetDefaultMaxPropagationDelay (void)
{
  // 1000m
  return Seconds (1000.0 / 300000000.0);
}

TypeId
TdmaMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3:TdmaMac")
    .SetParent<Object> ()
    .AddAttribute ("MaxPropagationDelay", "The maximum propagation delay. Unused for now.",
                   TimeValue (GetDefaultMaxPropagationDelay ()),
                   MakeTimeAccessor (&TdmaMac::m_maxPropagationDelay),
                   MakeTimeChecker ())
    .AddAttribute ("Ssid", "The ssid we want to belong to.",
                   SsidValue (Ssid ("default")),
                   MakeSsidAccessor (&TdmaMac::GetSsid,
                                     &TdmaMac::SetSsid),
                   MakeSsidChecker ())
#if 0
    // Not currently implemented in this device
    .AddTraceSource ("Sniffer",
                     "Trace source simulating a non-promiscuous packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&TdmaMac::m_snifferTrace))
#endif
  ;

  return tid;
}

void
TdmaMac::SetMaxPropagationDelay (Time delay)
{
  m_maxPropagationDelay = delay;
}
Time
TdmaMac::GetMaxPropagationDelay (void) const
{
  return m_maxPropagationDelay;
}
} // namespace ns3
