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
#ifndef TDMA_MAC_LOW_H
#define TDMA_MAC_LOW_H

#include <vector>
#include <stdint.h>
#include <ostream>
#include <map>

#include "ns3/wifi-mac-header.h"
#include "simple-wireless-channel.h"
#include "ns3/data-rate.h"
#include "ns3/mac48-address.h"
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/qos-utils.h"
#include "ns3/node.h"

namespace ns3 {


class TdmaMac;
class SimpleWirelessChannel;
class TdmaNetDevice;

class TdmaMacLow : public Object
{
public:
  typedef Callback<void, Ptr<Packet>, const WifiMacHeader*> TdmaMacLowRxCallback;

  TdmaMacLow ();
  virtual ~TdmaMacLow ();
  void SetDevice (Ptr<TdmaNetDevice> device);
  void SetAddress (Mac48Address ad);
  void SetBssid (Mac48Address ad);
  Mac48Address GetAddress (void) const;
  Mac48Address GetBssid (void) const;
  Ptr<TdmaNetDevice> GetDevice (void) const;
  /**
   * Associate the device with a channel
   *
   * \param channel Pointer to the channel
   */
  void SetChannel (Ptr<SimpleWirelessChannel> channel);
  /**
   * \param callback the callback which receives every incoming packet.
   *
   * This callback typically forwards incoming packets to
   * an instance of ns3::TdmaCentralMac.
   */
  void SetRxCallback (Callback<void,Ptr<Packet>,const WifiMacHeader *> callback);
  /**
   * \param packet packet to send
   * \param hdr 802.11 header for packet to send
   *
   * Start the transmission of the input packet and notify the listener
   * of transmission events.
   */
  void StartTransmission (Ptr<const Packet> packet,
                          const WifiMacHeader* hdr);

  /**
   * \param packet packet received
   * \param rxSnr snr of packet received
   * \param txMode transmission mode of packet received
   * \param preamble type of preamble used for the packet received
   *
   * This method is typically invoked by the lower PHY layer to notify
   * the MAC layer that a packet was successfully received.
   */
  void Receive (Ptr<Packet> packet);
private:
  uint32_t GetSize (Ptr<const Packet> packet, const WifiMacHeader *hdr) const;
  void ForwardDown (Ptr<const Packet> packet, const WifiMacHeader *hdr);
  virtual Ptr<SimpleWirelessChannel> GetChannel (void) const;
  virtual void DoDispose (void);
  TdmaMacLowRxCallback m_rxCallback;
  Ptr<Packet> m_currentPacket;
  Ptr<SimpleWirelessChannel> m_channel;
  Ptr<TdmaNetDevice> m_device;
  WifiMacHeader m_currentHdr;
  Mac48Address m_self;
  Mac48Address m_bssid;
};

} // namespace ns3

#endif /* TDMA_MAC_LOW_H */
