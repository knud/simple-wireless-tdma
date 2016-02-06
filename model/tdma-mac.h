/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Hemanth Narra
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
 * Author: Hemanth Narra <hemanth@ittc.ku.com>
 *
 * Thanks to the students and our professor Dr.James P.G. Sterbenz in the ResiliNets group
 * at The University of Kansas, https://wiki.ittc.ku.edu/resilinets/Main_Page
 */
#ifndef TDMA_MAC_H
#define TDMA_MAC_H

#include <stdint.h>
#include "ns3/traced-callback.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/mac48-address.h"
#include "ns3/ssid.h"
#include "ns3/node.h"
#include "tdma-controller.h"

namespace ns3 {

class SimpleWirelessChannel;
class TdmaNetDevice;
class TdmaController;

/**
 * \brief base class for all MAC-level wifi objects.
 *
 * This class encapsulates all the low-level MAC functionality
 * DCA, EDCA, etc) and all the high-level MAC functionality
 * (association/disassociation state machines).
 *
 */
class TdmaMac : public Object
{
public:
  static TypeId GetTypeId (void);

  /**
   * \param delay the max propagation delay.
   *
   * Unused for now.
   */
  void SetMaxPropagationDelay (Time delay);
  Time GetMaxPropagationDelay (void) const;

  /**
   * \returns the MAC address associated to this MAC layer.
   */
  virtual Mac48Address GetAddress (void) const = 0;
  /**
   * \returns the ssid which this MAC layer is going to try to stay in.
   */
  virtual Ssid GetSsid (void) const = 0;
  /**
   * \param address the current address of this MAC layer.
   */
  virtual void SetAddress (Mac48Address address) = 0;
  /**
   * \param ssid the current ssid of this MAC layer.
   */
  virtual void SetSsid (Ssid ssid) = 0;
  /**
   * \returns the bssid of the network this device belongs to.
   */
  virtual Mac48Address GetBssid (void) const = 0;
  virtual void SetTdmaController (Ptr<TdmaController> controller) = 0;
  virtual Ptr<TdmaController> GetTdmaController (void) const = 0;
  virtual void SetChannel (Ptr<SimpleWirelessChannel> channel) = 0;
  virtual void SetDevice (Ptr<TdmaNetDevice> device) = 0;

  /**
   * \param packet the packet to send.
   * \param to the address to which the packet should be sent.
   * \param from the address from which the packet should be sent.
   *
   * The packet should be enqueued in a tx queue, and should be
   * dequeued as soon as the DCF function determines that
   * access it granted to this MAC.  The extra parameter "from" allows
   * this device to operate in a bridged mode, forwarding received
   * frames without altering the source address.
   */
  virtual void Enqueue (Ptr<const Packet> packet, Mac48Address to, Mac48Address from) = 0;
  /**
   * \param packet the packet to send.
   * \param to the address to which the packet should be sent.
   *
   * The packet should be enqueued in a tx queue, and should be
   * dequeued as soon as the DCF function determines that
   * access it granted to this MAC.
   */
  virtual void Enqueue (Ptr<const Packet> packet, Mac48Address to) = 0;
  virtual bool SupportsSendFrom (void) const = 0;
  /**
   * \param upCallback the callback to invoke when a packet must be forwarded up the stack.
   */
  virtual void SetForwardUpCallback (Callback<void,Ptr<Packet>, Mac48Address, Mac48Address> upCallback) = 0;
  /**
   * \param linkUp the callback to invoke when the link becomes up.
   */
  virtual void SetLinkUpCallback (Callback<void> linkUp) = 0;
  /**
   * \param linkDown the callback to invoke when the link becomes down.
   */
  virtual void SetLinkDownCallback (Callback<void> linkDown) = 0;
  /**
   * \param queuespace the callback to invoke network protocol to senddown packet
   * when there is space in the mac queue.
   */
  virtual void SetTxQueueStartCallback (Callback<bool,uint32_t> queueStart) = 0;
  virtual void SetTxQueueStopCallback (Callback<bool,uint32_t> queueStop) = 0;
  virtual uint32_t GetQueueState (uint32_t index) = 0;
  virtual uint32_t GetNQueues (void) = 0;
  virtual void StartTransmission (uint64_t transmissionTime) = 0;
  /**
   * Public method used to fire a MacTx trace.  Implemented for encapsulation
   * purposes.
   */
  virtual void NotifyTx (Ptr<const Packet> packet) = 0;

  /**
   * Public method used to fire a MacTxDrop trace.  Implemented for encapsulation
   * purposes.
   */
  virtual void NotifyTxDrop (Ptr<const Packet> packet) = 0;

  /**
   * Public method used to fire a MacRx trace.  Implemented for encapsulation
   * purposes.
   */
  virtual void NotifyRx (Ptr<const Packet> packet) = 0;

  /**
   * Public method used to fire a MacPromiscRx trace.  Implemented for encapsulation
   * purposes.
   */
  virtual void NotifyPromiscRx (Ptr<const Packet> packet) = 0;

  /**
   * Public method used to fire a MacRxDrop trace.  Implemented for encapsulation
   * purposes.
   */
  virtual void NotifyRxDrop (Ptr<const Packet> packet) = 0;

  virtual void Initialize () = 0;

private:
  static Time GetDefaultMaxPropagationDelay (void);
  Time m_maxPropagationDelay;
  uint32_t m_nodeId;

};

} // namespace ns3

#endif /* AERO_MAC_H */
