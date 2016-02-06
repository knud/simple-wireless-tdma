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
#ifndef TDMA_CENTRAL_MAC_H
#define TDMA_CENTRAL_MAC_H

#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "tdma-mac.h"
#include "tdma-mac-low.h"
#include "tdma-mac-queue.h"

namespace ns3 {

class WifiMacHeader;
class TdmaController;
class TdmaMacLow;

/**
 * \brief Tdma Mac that supports centralized tdma controller
 *
 * Tdma Controller drives the transmission of packets in a round-robin
 * manner depending on the number of slots allocated to a node and the
 * slot interval. Mac also be made to request for more slots or change in
 * slot interval which would take affect from the next epoch.
 */
class TdmaCentralMac : public TdmaMac
{
public:
  static TypeId GetTypeId (void);

  TdmaCentralMac ();
  ~TdmaCentralMac ();

  // inherited from TdmaMac.
  virtual void Enqueue (Ptr<const Packet> packet, Mac48Address to, Mac48Address from);
  virtual void Enqueue (Ptr<const Packet> packet, Mac48Address to);
  virtual bool SupportsSendFrom (void) const;
  virtual void SetForwardUpCallback (Callback<void,Ptr<Packet>, Mac48Address, Mac48Address> upCallback);
  virtual void SetLinkUpCallback (Callback<void> linkUp);
  virtual void SetLinkDownCallback (Callback<void> linkDown);
  virtual Mac48Address GetAddress (void) const;
  virtual Ssid GetSsid (void) const;
  virtual void SetAddress (Mac48Address address);
  virtual void SetSsid (Ssid ssid);
  virtual Mac48Address GetBssid (void) const;
  virtual void SetTdmaController (Ptr<TdmaController> controller);
  virtual Ptr<TdmaController> GetTdmaController (void) const;
  virtual void SetDevice (Ptr<TdmaNetDevice> device);
  virtual Ptr<TdmaNetDevice> GetDevice (void) const;
  virtual void SetChannel (Ptr<SimpleWirelessChannel> channel);
  virtual void StartTransmission (uint64_t transmissionTime);
  virtual void NotifyTx (Ptr<const Packet> packet);
  virtual void NotifyTxDrop (Ptr<const Packet> packet);
  virtual void NotifyRx (Ptr<const Packet> packet);
  virtual void NotifyPromiscRx (Ptr<const Packet> packet);
  virtual void NotifyRxDrop (Ptr<const Packet> packet);
  virtual void SetTxQueueStartCallback (Callback<bool,uint32_t> queueStart);
  virtual void SetTxQueueStopCallback (Callback<bool,uint32_t> queueStop);
  virtual uint32_t GetQueueState (uint32_t index);
  virtual uint32_t GetNQueues (void);
  virtual void Initialize (void);

  /**
   * \param packet packet to send
   * \param hdr header of packet to send.
   *
   * Store the packet in the internal queue until it
   * can be sent safely.
   */
  void Queue (Ptr<const Packet> packet, const WifiMacHeader &hdr);
  void SetMaxQueueSize (uint32_t size);
  void SetMaxQueueDelay (Time delay);
  Ptr<SimpleWirelessChannel> GetChannel (void) const;
  Ptr<TdmaMacLow> GetTdmaMacLow (void) const;
  void RequestForChannelAccess (void);

private:
  void Receive (Ptr<Packet> packet, const WifiMacHeader *hdr);
  void ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to);
  void TxOk (const WifiMacHeader &hdr);
  void TxFailed (const WifiMacHeader &hdr);
  virtual void DoDispose (void);
//  virtual void DoInitialize (void);
  TdmaCentralMac (const TdmaCentralMac & ctor_arg);
  TdmaCentralMac &operator = (const TdmaCentralMac &o);
  void TxQueueStart (uint32_t index);
  void TxQueueStop (uint32_t index);
  void SendPacketDown (Time remainingTime);

  /**
   * The trace source fired when packets come into the "top" of the device
   * at the L3/L2 transition, before being queued for transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxTrace;

  /**
   * The trace source fired when packets coming into the "top" of the device
   * are dropped at the MAC layer during transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxDropTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3
   * transition).  This is a promiscuous trace.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macPromiscRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3
   * transition).  This is a non- promiscuous trace.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxTrace;

  /**
   * The trace source fired when packets coming into the "top" of the device
   * are dropped at the MAC layer during reception.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxDropTrace;

  Callback<void, Ptr<Packet>,Mac48Address, Mac48Address> m_upCallback;
  Callback<bool,uint32_t> m_queueStart;
  Callback<bool,uint32_t> m_queueStop;
  std::vector<uint32_t> m_slots;
  Ptr<TdmaController> m_tdmaController;
  Ptr<TdmaNetDevice> m_device;
  Ptr<TdmaMacQueue> m_queue;
  Ptr<TdmaMacLow> m_low;
  Ptr<SimpleWirelessChannel> m_channel;
  Ssid m_ssid;
  Ptr<Node> m_nodePtr;
  bool m_isTdmaRunning;
};

} // namespace ns3


#endif /* TDMA_CENTRAL_MAC_H */
