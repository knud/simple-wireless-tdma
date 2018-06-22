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
#include "tdma-helper.h"
#include "ns3/tdma-mac-net-device.h"
#include "ns3/simple-wireless-channel.h"
#include "ns3/channel.h"
#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/names.h"

NS_LOG_COMPONENT_DEFINE ("TdmaHelper");

namespace ns3 {

static void AsciiMacTxEventWithContext (Ptr<OutputStreamWrapper> stream, std::string context,
                                        Ptr<const Packet> packet)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << context << " "
                        << *packet << std::endl;
}

static void AsciiMacRxOkEventWithContext (Ptr<OutputStreamWrapper> stream, std::string context,
                                          Ptr<const Packet> packet)
{
  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << context << " "
                        << *packet << std::endl;
}

static void AsciiMacTxDropEventWithContext (Ptr<OutputStreamWrapper> stream, std::string context,
                                            Ptr<const Packet> packet)
{
  *stream->GetStream () << "d " << Simulator::Now ().GetSeconds () << " " << context << " "
                        << *packet << std::endl;
}

static void AsciiMacTxEventWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << *packet << std::endl;
}

static void AsciiMacRxOkEventWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet)
{
  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << *packet << std::endl;
}

static void AsciiMacTxDropEventWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet)
{
  *stream->GetStream () << "d " << Simulator::Now ().GetSeconds () << " " << *packet << std::endl;
}

TdmaHelper::TdmaHelper (uint32_t numNodes, uint32_t numSlots) : m_controller (0),
                                                                m_controllerHelper (0),
                                                                m_slotAllotmentArray (0),
                                                                m_numRows (numNodes),
                                                                m_numCols (numSlots + 1)
{
  NS_LOG_FUNCTION (this << numNodes << numSlots);
  m_mac.SetTypeId ("ns3::TdmaCentralMac");
  m_channel = CreateObject<SimpleWirelessChannel> ();
  Allocate2D ();
  SetDefaultSlots ();
}

TdmaHelper::TdmaHelper (std::string filename) : m_controller (0),
                                                m_controllerHelper (0),
                                                m_slotAllotmentArray (0),
                                                m_filename (filename)
{
  m_mac.SetTypeId ("ns3::TdmaCentralMac");
  m_channel = CreateObject<SimpleWirelessChannel> ();
  m_parser = CreateObject<TdmaSlotAssignmentFileParser> (m_filename);
  m_numRows = m_parser->GetNodeCount ();
  m_numCols = m_parser->GetTotalSlots () + 1; // +1 for nodeid in first column
  Allocate2D ();
  SetSlots ();
}

TdmaHelper::~TdmaHelper ()
{
  NS_LOG_FUNCTION (this);
  Deallocate2D ();
}

void
TdmaHelper::SetFileName (std::string filename)
{
  m_filename = filename;
}

void
TdmaHelper::SetTdmaControllerHelper (const TdmaControllerHelper &controllerHelper)
{
  NS_LOG_FUNCTION (this);
  delete m_controllerHelper;
  m_controllerHelper = controllerHelper.Copy ();
  m_controller = m_controllerHelper->Create ();
  m_controller->SetTotalSlotsAllowed (m_numCols - 1);
}

void TdmaHelper::Deallocate2D (void)
{
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      free (m_slotAllotmentArray[i]);
    }
  free (m_slotAllotmentArray);
}

void TdmaHelper::Allocate2D (void)
{
  NS_LOG_FUNCTION (this);
  m_slotAllotmentArray = (uint32_t**)malloc (m_numRows * sizeof(int*));
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      m_slotAllotmentArray[i] = (uint32_t*)malloc (m_numCols * sizeof(int));
    }
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      m_slotAllotmentArray[i][0] = i; //put in nodeId
      for (uint32_t j = 1; j < m_numCols; j++)
        {
          m_slotAllotmentArray[i][j] = 0;
        }
    }
}

void
TdmaHelper::SetDefaultSlots (void)
{
  NS_LOG_FUNCTION (this);
  //PrintSlotAllotmentArray ();
  int continuousSlots = (m_numCols - 1) / m_numRows;
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      NS_LOG_DEBUG ("i:" << i << " continuousSlots:" << continuousSlots
                         << " m_numRows:" << m_numRows << " m_numCols:" << m_numCols);
      int tmp = 0;
      while (tmp < continuousSlots)
        {
          tmp++;
          if (continuousSlots * i + tmp > m_numCols)
            {
              break;
            }
          m_slotAllotmentArray[i][continuousSlots * i + tmp] = 1;
        }
    }
  uint32_t remainingSlots = m_numCols - 1 - (continuousSlots * m_numRows);
  NS_LOG_DEBUG ("remainingSlots:" << remainingSlots);
  for (uint32_t i = 0; i < remainingSlots; i++)
    {
      m_slotAllotmentArray[i][continuousSlots * m_numRows + i + 1] = 1;
    }
  PrintSlotAllotmentArray ();
}

void
TdmaHelper::SetSlots (void)
{
  //PrintSlotAllotmentArray();
  std::vector<SlotArray> & slotArrays = m_parser->GetSlotArrays ();
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      m_slotAllotmentArray[i][0] = slotArrays[i][0]; //put in nodeId
      for (uint32_t j = 1; j < m_numCols; j++)
        {
          m_slotAllotmentArray[i][j] = slotArrays[i][j];
        }
    }
  PrintSlotAllotmentArray();
}

void
TdmaHelper::SetSlots (int numNodes, ...)
{
  NS_LOG_FUNCTION (this);
  va_list args;
  va_start (args, numNodes);
  NS_LOG_DEBUG ("Rows:" << m_numRows << " columns: " << m_numCols);
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      NS_LOG_DEBUG ("new row");
      m_slotAllotmentArray[i][0] = va_arg (args, int);
      for (uint32_t j = 1; j < m_numCols; j++)
        {
          NS_LOG_DEBUG ("new column");
          NS_LOG_DEBUG ("before: value at i: " << i << " j: " << j << " is " << m_slotAllotmentArray[i][j]);
          m_slotAllotmentArray[i][j] = va_arg (args, int);
          NS_LOG_DEBUG ("after: value at i: " << i << " j: " << j << " is " << m_slotAllotmentArray[i][j]);
        }
    }
  va_end (args);
  NS_LOG_DEBUG ("Rows:" << m_numRows << " columns: " << m_numCols);
  //PrintSlotAllotmentArray();
}

void
TdmaHelper::AssignTdmaSlots (Ptr<TdmaMac> mac, uint32_t nodeId) const
{
  NS_LOG_FUNCTION (this << mac << nodeId);
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      if (m_slotAllotmentArray[i][0] == nodeId)
        {
          for (uint32_t j = 1; j < m_numCols; j++)
            {
              //validation of the slots
              NS_ASSERT_MSG (((m_slotAllotmentArray[i][j] == 0) || (m_slotAllotmentArray[i][j] == 1)),
                             "Tdma slots should be assigned with only 0 or 1");
              for (uint32_t k = 0; k < m_numRows; k++)
                {
                  if (k == i)
                    {
                      continue;
                    }
                  NS_ASSERT_MSG (!(m_slotAllotmentArray[k][j] == 1 && m_slotAllotmentArray[i][j] == 1),
                                 "Slot exclusivity is not maintained");
                }
              if (m_slotAllotmentArray[i][j] == 1)
                {
                  m_controller->AddTdmaSlot (j - 1,mac);
                }
            }
        }
    }
}

void
TdmaHelper::PrintSlotAllotmentArray (void) const
{
  std::cout << "Printing SlotAllotmentArray\n";
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      std::cout << m_slotAllotmentArray[i][0] << " : \t";
      for (uint32_t j = 1; j < m_numCols; j++)
        {
          std::cout << m_slotAllotmentArray[i][j] << ",\t";
        }
      std::cout << "\n";
    }
}

NetDeviceContainer
TdmaHelper::Install (NodeContainer c) const
{
  NS_LOG_FUNCTION (this);
  NetDeviceContainer devices;
  NS_ASSERT (m_controller != 0);
  uint32_t slotUserId = 0;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<TdmaNetDevice> device = CreateObject<TdmaNetDevice> ();
      Ptr<TdmaMac> mac = m_mac.Create<TdmaMac> ();
      mac->SetAddress (Mac48Address::Allocate ());
      AssignTdmaSlots (mac,slotUserId);
      device->SetMac (mac);
      device->SetChannel (m_channel);
      device->SetTdmaController (m_controller);
      node->AddDevice (device);
      devices.Add (device);
      slotUserId++;
    }
  return devices;
}

NetDeviceContainer
TdmaHelper::Install (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);
  return Install (NodeContainer (node));
}

NetDeviceContainer
TdmaHelper::Install (std::string nodeName) const
{
  NS_LOG_FUNCTION (this << nodeName);
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return Install (NodeContainer (node));
}

void
TdmaHelper::EnableLogComponents (void)
{
  LogComponentEnable ("TdmaHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("TdmaControllerHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("TdmaCentralMac", LOG_LEVEL_ALL);
  LogComponentEnable ("TdmaMacLow", LOG_LEVEL_ALL);
  LogComponentEnable ("TdmaController", LOG_LEVEL_ALL);
  LogComponentEnable ("TdmaMacQueue", LOG_LEVEL_ALL);
  LogComponentEnable ("TdmaNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("SimpleWirelessChannel", LOG_LEVEL_ALL);
}

void
TdmaHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream,
  std::string prefix,
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  Ptr<TdmaNetDevice> device = nd->GetObject<TdmaNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("TdmaHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::TdmaNetDevice");
      return;
    }

  //
  // Our trace sinks are going to use packet printing, so we have to make sure
  // that is turned on.
  //
  Packet::EnablePrinting ();

  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create
  // one using the usual trace filename conventions and write our traces
  // without a context since there will be one file per context and therefore
  // the context would be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);
      //
      // We could go poking through the phy and the state looking for the
      // correct trace source, but we can let Config deal with that with
      // some search cost.  Since this is presumably happening at topology
      // creation time, it doesn't seem much of a price to pay.
      //
      oss.str ("");
      oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::TdmaNetDevice/Mac/MacRx";
      Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiMacRxOkEventWithoutContext, theStream));

      oss.str ("");

      oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::TdmaNetDevice/Mac/MacTx";
      Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiMacTxEventWithoutContext, theStream));

      oss.str ("");

      oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::TdmaNetDevice/Mac/MacTxDrop";
      Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiMacTxDropEventWithoutContext, theStream));

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to provide a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with coming up with a context.
  //
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::TdmaNetDevice/Mac/MacRx";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiMacRxOkEventWithContext, stream));

  oss.str ("");

  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::TdmaNetDevice/Mac/MacTx";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiMacTxEventWithContext, stream));

  oss.str ("");

  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::TdmaNetDevice/Mac/MacTxDrop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiMacTxDropEventWithContext, stream));
}
} // namespace ns3
