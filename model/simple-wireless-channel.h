/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Washington
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
 * Modified by Hemanth Narra <hemanthnarra222@gmail.com> to suit the
 * TDMA implementation.
 */
#ifndef SIMPLE_WIRELESS_CHANNEL_H
#define SIMPLE_WIRELESS_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "tdma-mac-low.h"
#include "tdma-mac-net-device.h"
#include <vector>

namespace ns3 {

class TdmaMacLow;
class Packet;

/**
 * \ingroup channel
 * \brief A simple channel, for simple things and testing
 */
class SimpleWirelessChannel : public Channel
{
public:
  typedef std::vector<Ptr<TdmaMacLow> > TdmaMacLowList;
  static TypeId GetTypeId (void);
  SimpleWirelessChannel ();

  /**
   * Copy the packet to be received at a time equal to the transmission
   * time plus the propagation delay between sender and all receivers
   * on the channel that are within the range of the sender
   *
   * \param p Pointer to packet
   * \param sender sending NetDevice
   * \param txTime transmission time (seconds)
   */
  void Send (Ptr<const Packet> p, Ptr<TdmaMacLow> sender);

  /**
   * Add a device to the channel
   *
   * \param device Device to add
   */
  void Add (Ptr<TdmaMacLow> tdmaMacLow);
  double GetMaxRange (void) const;

  // inherited from ns3::Channel
  virtual uint32_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

private:
  TdmaMacLowList m_tdmaMacLowList;
  double m_range;
};

} // namespace ns3

#endif /* SIMPLE_WIRELESS_CHANNEL_H */
