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
#ifndef TDMA_HELPER_H
#define TDMA_HELPER_H

#include <string>
#include <stdarg.h>
#include "ns3/attribute.h"
#include "ns3/tdma-mac.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/trace-helper.h"
#include "ns3/tdma-controller-helper.h"
#include "tdma-slot-assignment-parser.h"

namespace ns3 {

class TdmaMac;
class TdmaNetDevice;
class Node;
class TdmaController;

/**
 * \brief create MAC objects
 *
 * This base class must be implemented by new MAC implementation which wish to integrate
 * with the \ref ns3::TdmaHelper class.
 */
class TdmaMacHelper
{
public:
  virtual ~TdmaMacHelper ();
  /**
   * \returns a new MAC object.
   *
   * Subclasses must implement this method to allow the ns3::TdmaHelper class
   * to create MAC objects from ns3::TdmaHelper::Install.
   */
  virtual Ptr<TdmaMac> Create (void) const = 0;
};

/**
 * \brief helps to create TdmaNetDevice objects and ensures creation of a centralized
 * TdmaController which takes care of the slot scheduling
 */
class TdmaHelper : public AsciiTraceHelperForDevice
{
public:
  /**
   * Constructor with two attributes
   *
   * \param numNodes number of nodes in the TDMA frame
   * \param numSlots total number of slots per frame
   */
  TdmaHelper (uint32_t numNodes, uint32_t numSlots);
  /**
   * Constructor with filename for slot assignment
   *
   * \param fileName file name that contains the TDMA slot assignment.
   * Slot file must be of the format specified below. nodeId followed by
   * ':' and then slot assignment with ',' seperating the slots
   * 0:1,1,0,0,0
   * 1:0,0,1,0,0
   * 2:0,0,0,1,0
   * 3:0,0,0,0,1
   */
  TdmaHelper (std::string fileName);
  ~TdmaHelper ();

  /**
   * \returns a device container which contains all the devices created by this method.
   */
  NetDeviceContainer Install (NodeContainer c) const;
  /**
   * \param mac the MAC helper to create MAC objects
   * \param node the node on which a aero device must be created
   * \param controller the TdmaController to add to these devices
   * \returns a device container which contains all the devices created by this method.
   */
  NetDeviceContainer Install (Ptr<Node> node) const;
  /**
   * \param nodeName the name of node on which a wifi device must be created
   * \returns a device container which contains all the devices created by this method.
   */
  NetDeviceContainer Install (std::string nodeName) const;

  void SetFileName (std::string filename);
  /**
   * \brief Set the TdmaController for this TdamHelper class
   */
  void SetTdmaControllerHelper (const TdmaControllerHelper &controllerHelper);
  /**
   * \brief used to set the TDMA slots from the simulation script
   * useful for low number of nodes. If the number of nodes is more, it is
   * advisable to use the external file to set TDMA slots.
   * For example, assume tdma is a TdmaHelper object, the slot assigment
   * for 4 nodes can be set as follows. We have a total of 5 slots,
   * out of which first two slots are assigned to node 1 and the other nodes
   * are assigned single slots as shown below
   * tdma.SetSlots(4,
   *               0,1,1,0,0,0,
   *               1,0,0,1,0,0,
   *               2,0,0,0,1,0,
   *               3,0,0,0,0,1);   *
   */
  void SetSlots (int NodeNum, ...);
  /**
   * Helper to enable all TdmaNetDevice log components with one statement
   */
  static void EnableLogComponents (void);
private:
  /**
   * \brief Enable ascii trace output on the indicated net device.
   * \internal
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param stream The output stream object to use when logging ascii traces.
   * \param prefix Filename prefix to use for ascii trace files.
   * \param nd Net device for which you want to enable tracing.
   */
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream,
                                    std::string prefix,
                                    Ptr<NetDevice> nd,
                                    bool explicitFilename);
  /**
   * \brief print the TDMA slot assignment for debugging purposes.
   */
  void PrintSlotAllotmentArray (void) const;
  /**
   * \brief Assigns a single TDMA slot in a frame for each node installed
   * by the TDMA helper.
   * \internal
   * Slot assignment is done in ascending order of the node id of nodes
   * present in the node container that was passed to the TdmaHelper
   * install method
   */
  void SetDefaultSlots (void);
  /**
   * \brief used for slot assignment from an external file
   */
  void SetSlots (void);
  /**
   *  \brief memory allocation of the m_slotAllotmentArray when number of
   *  nodes and total number of slots occupied by those nodes is specified
   *  by the user
   */
  void Allocate2D (void);
  /**
   * memory deallocation of the m_slotAllotmentArray during object deletion
   */
  void Deallocate2D (void);
  /**
   * \brief Populate the m_slotArray in the TdmaController class with the
   * mac pointers of nodes assigned to those slots
   *
   * \param mac mac pointer of the node assigned to this TDMA slot
   * \param nodeId node id assigned to this TDMA slot
   */
  void AssignTdmaSlots (Ptr<TdmaMac> mac, uint32_t nodeId) const;

  ObjectFactory m_mac;
  Ptr<SimpleWirelessChannel> m_channel;
  Ptr<TdmaController> m_controller;
  const TdmaControllerHelper *m_controllerHelper;
  uint32_t **m_slotAllotmentArray;
  uint32_t m_numRows;
  uint32_t m_numCols;
  std::string m_filename;
  Ptr<TdmaSlotAssignmentFileParser> m_parser;
};

} // namespace ns3

#endif /* AERO_HELPER_H */
