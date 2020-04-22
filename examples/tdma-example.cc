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
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/simple-wireless-tdma-module.h"
#include "ns3/dsdv-module.h"
#include <iostream>
#include <cmath>

using namespace ns3;

uint16_t port = 9;

NS_LOG_COMPONENT_DEFINE ("TdmaExample");

class TdmaExample
{
public:
  TdmaExample ();
  void CaseRun (uint32_t nWifis,
                uint32_t nSinks,
                double totalTime,
                std::string rate,
                std::string phyMode,
                uint32_t nodeSpeed,
                uint32_t periodicUpdateInterval,
                uint32_t settlingTime,
                double dataStart,
                std::string CSVfileName,
                bool usingWifi,
                double txpDistance,
                uint32_t nSlots,
                uint32_t slotTime,
                uint32_t guardTime,
                uint32_t interFrameGap);

private:
  uint32_t m_nWifis;
  uint32_t m_nSinks;
  double m_totalTime;
  std::string m_rate;
  std::string m_phyMode;
  uint32_t m_nodeSpeed;
  uint32_t m_periodicUpdateInterval;
  uint32_t m_settlingTime;
  double m_dataStart;
  uint32_t bytesTotal;
  uint32_t packetsReceived;
  std::string m_CSVfileName;
  uint32_t m_slots;
  uint32_t m_slotTime;
  uint32_t m_guardTime;
  uint32_t m_interFrameGap;

  std::map<double, double> m_transmitRangeMap;

  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;

private:
  void CreateNodes ();
  void CreateDevices (std::string tr_name, bool usingWifi, double txpDistance);
  void InstallInternetStack ();
  void InstallApplications ();
  void SetupMobility ();
  void ReceivePacket (Ptr <Socket> );
  Ptr <Socket> SetupPacketReceive (Ipv4Address, Ptr <Node> );
  void CheckThroughput ();
  void InsertIntoTxp (void);

};

int main (int argc, char **argv)
{
  TdmaExample test;
  uint32_t nWifis = 4;
  uint32_t nSinks = 1;
  double totalTime = 100.0;
  std::string rate ("8kbps");
  std::string phyMode ("DsssRate11Mbps");
  uint32_t nodeSpeed = 10; //in m/s
  std::string appl = "all";
  uint32_t periodicUpdateInterval = 15;
  uint32_t settlingTime = 6;
  double dataStart = 50.0;
  std::string CSVfileName = "TdmaExample.csv";
  bool usingWifi = false;
  double txpDistance = 400.0;

  // tdma parameters
  uint32_t nSlots = nWifis;
  // slotTime is at least the number of bytes in a packet * 8 bits/byte / bit rate * 1e6 microseconds
  uint32_t slotTime = 1000 * 8 / 8000 * 1000000; // us
  uint32_t interFrameGap = 0;
  uint32_t guardTime = 0;

  CommandLine cmd;
  cmd.AddValue ("nWifis", "Number of wifi nodes[Default:30]", nWifis);
  cmd.AddValue ("nSinks", "Number of wifi sink nodes[Default:10]", nSinks);
  cmd.AddValue ("usingWifi", "Do you want to use WifiMac(1/0)[Default:false(0)]", usingWifi);
  cmd.AddValue ("totalTime", "Total Simulation time[Default:100]", totalTime);
  cmd.AddValue ("phyMode", "Wifi Phy mode[Default:DsssRate11Mbps]", phyMode);
  cmd.AddValue ("rate", "CBR traffic rate[Default:8kbps]", rate);
  cmd.AddValue ("nodeSpeed", "Node speed in RandomWayPoint model[Default:10]", nodeSpeed);
  cmd.AddValue ("periodicUpdateInterval", "Periodic Interval Time[Default=15]", periodicUpdateInterval);
  cmd.AddValue ("settlingTime", "Settling Time before sending out an update for changed metric[Default=6]", settlingTime);
  cmd.AddValue ("dataStart", "Time at which nodes start to transmit data[Default=50.0]", dataStart);
  cmd.AddValue ("CSVfileName", "The name of the CSV output file name[Default:TdmaExample.csv]", CSVfileName);
  cmd.AddValue ("txpDistance", "MaxRange for the node transmissions [Default:400.0]", txpDistance);
  cmd.AddValue ("nSlots", "Number of slots per gframe [Default:nWifis]", nSlots);
  cmd.AddValue ("slotTime", "Slot transmission Time [Default(us):1000]", slotTime);
  cmd.AddValue ("guardTime", "Duration to wait between slots [Default(us):0]", guardTime);
  cmd.AddValue ("interFrameGap", "Duration between frames [Default(us):0]", interFrameGap);
  cmd.Parse (argc, argv);

  std::ofstream out (CSVfileName.c_str ());
  out << "SimulationSecond," <<
  "ReceiveRate," <<
  "PacketsReceived," <<
  "NumberOfSinks," <<
  std::endl;
  out.close ();

  SeedManager::SetSeed (12345);

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("1000")); // bytes!
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (rate));

  LogComponentEnable ("TdmaExample", LOG_LEVEL_DEBUG);

  test = TdmaExample ();
  test.CaseRun (nWifis, nSinks, totalTime, rate, phyMode, nodeSpeed, periodicUpdateInterval,
                settlingTime, dataStart,CSVfileName,usingWifi,txpDistance, nSlots, slotTime, guardTime, interFrameGap);

  return 0;
}

TdmaExample::TdmaExample ()
  : m_nWifis(30),
		m_nSinks(10),
		m_totalTime(100),
	  m_rate ("8kbps"),
	  m_phyMode ("DsssRate11Mbps"),
	  m_nodeSpeed (10),
	  m_periodicUpdateInterval (15),
		m_settlingTime (6),
	  m_dataStart (50.0),
		bytesTotal (10000),
    packetsReceived (0),
	  m_CSVfileName ("tdmaExample.csv"),
	  m_slots(30),
	  m_slotTime (1000),
    m_guardTime (0),
	  m_interFrameGap (0)

{
  NS_LOG_FUNCTION (this);
}

void
TdmaExample::ReceivePacket (Ptr <Socket> socket)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " Received one packet!");
  Ptr <Packet> packet;
  while ((packet = socket->Recv ()))
    {
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
    }
}

void
TdmaExample::CheckThroughput ()
{
  double kbs = (bytesTotal * 8.0) / 1000;
  bytesTotal = 0;

  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

  out << (Simulator::Now ()).GetSeconds () << "," << kbs << "," << packetsReceived << "," << m_nSinks << std::endl;

  out.close ();
  packetsReceived = 0;
  Simulator::Schedule (Seconds (1.0), &TdmaExample::CheckThroughput, this);
}

Ptr <Socket>
TdmaExample::SetupPacketReceive (Ipv4Address addr, Ptr <Node> node)
{

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr <Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback ( &TdmaExample::ReceivePacket, this));

  return sink;
}

void
TdmaExample::CaseRun (uint32_t nWifis, uint32_t nSinks, double totalTime, std::string rate,
                      std::string phyMode, uint32_t nodeSpeed, uint32_t periodicUpdateInterval, uint32_t settlingTime,
                      double dataStart, std::string CSVfileName, bool usingWifi, double txpDistance, uint32_t nSlots,
                      uint32_t slotTime, uint32_t guardTime, uint32_t interFrameGap)
{
  m_nWifis = nWifis;
  m_nSinks = nSinks;
  m_totalTime = totalTime;
  m_rate = rate;
  m_phyMode = phyMode;
  m_nodeSpeed = nodeSpeed;
  m_periodicUpdateInterval = periodicUpdateInterval;
  m_settlingTime = settlingTime;
  m_dataStart = dataStart;
  m_CSVfileName = CSVfileName;
  m_slots = nSlots;
  m_slotTime = slotTime;
  m_guardTime = guardTime;
  m_interFrameGap = interFrameGap;

  std::stringstream ss;
  ss << m_nWifis;
  std::string t_nodes = ss.str ();

  std::stringstream ss2;
  ss2 << m_totalTime;
  std::string sTotalTime = ss2.str ();

  std::stringstream ss3;
  ss3 << txpDistance;
  std::string t_txpDistance = ss3.str ();

  std::string macProtocol = "";
  if (usingWifi)
    {
      macProtocol = "wifi";
    }
  else
    {
      macProtocol = "tdma";
    }

  std::string tr_name = "Dsdv_Manet_"
    + t_nodes + "Nodes_"
    + macProtocol + "_"
    + t_txpDistance + "txDistance_"
    + sTotalTime + "SimTime";
  std::cout << "Trace file generated is " << tr_name << ".tr\n";

  InsertIntoTxp ();
  CreateNodes ();
  CreateDevices (tr_name,usingWifi,txpDistance);
  SetupMobility ();
  InstallInternetStack ();
  InstallApplications ();

  std::cout << "\nStarting simulation for " << m_totalTime << " s ...\n";

  CheckThroughput ();

  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  Simulator::Destroy ();
}

void
TdmaExample::CreateNodes ()
{
  std::cout << "Creating " << (unsigned) m_nWifis << " nodes.\n";
  nodes.Create (m_nWifis);
  NS_ASSERT_MSG (m_nWifis > m_nSinks, "Sinks must be less or equal to the number of nodes in network");
}

void
TdmaExample::SetupMobility ()
{
  MobilityHelper mobility;
/*  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", RandomVariableValue (UniformVariable (0, 1000)));
  pos.Set ("Y", RandomVariableValue (UniformVariable (0, 1000)));

  Ptr <PositionAllocator> positionAlloc = pos.Create ()->GetObject <PositionAllocator> ();
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel", "Speed", RandomVariableValue (ConstantVariable (m_nodeSpeed)),
      "Pause", RandomVariableValue (ConstantVariable (2.0)), "PositionAllocator", PointerValue (positionAlloc));
  mobility.SetPositionAllocator (positionAlloc);*/

  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));

  Ptr<PositionAllocator> positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
}

void
TdmaExample::CreateDevices (std::string tr_name, bool usingWifi, double txpDistance)
{
  if (usingWifi)
    {
      Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (m_phyMode));
      Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2000"));

      double txp;
      if (m_transmitRangeMap.find (txpDistance) == m_transmitRangeMap.end ())
        {
          txp = 0.0;
        }
      else
        {
          txp = m_transmitRangeMap[txpDistance];
        }

      // WifiMacHelper replaced NqosWifiMacHelper ~ v3.28
      WifiMacHelper wifiMac = WifiMacHelper();
      YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
      YansWifiChannelHelper wifiChannel;
      wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
      wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
      wifiPhy.SetChannel (wifiChannel.Create ());
      WifiHelper wifi;
      wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode", StringValue (m_phyMode),
                                    "ControlMode", StringValue (m_phyMode));
      //set the tx range based on txpDistance
      wifiPhy.Set ("TxPowerStart", DoubleValue (txp));
      wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));

      devices = wifi.Install (wifiPhy, wifiMac, nodes);

      AsciiTraceHelper ascii;
      wifiPhy.EnableAsciiAll (ascii.CreateFileStream (tr_name + ".tr"));
      wifiPhy.EnablePcapAll (tr_name);
    }
  else
    {
      Config::SetDefault ("ns3::SimpleWirelessChannel::MaxRange", DoubleValue (txpDistance));
      // default allocation, each node gets a slot to transmit
//      TdmaHelper tdma = TdmaHelper (nodes.GetN (),nodes.GetN ()); // in this case selected, numSlots = nodes
      /* can make custom allocation through simulation script
       * will override default allocation
       */
      /*tdma.SetSlots(4,
          0,1,1,0,0,
          1,0,0,0,0,
          2,0,0,1,0,
          3,0,0,0,1);*/
      // if TDMA slot assignment is through a file
      TdmaHelper tdma = TdmaHelper ("tdmaSlots.txt");
      TdmaControllerHelper controller;
      controller.Set ("SlotTime", TimeValue (MicroSeconds (1100)));
      controller.Set ("GuardTime", TimeValue (MicroSeconds (100)));
      controller.Set ("InterFrameTime", TimeValue (MicroSeconds (0)));
      tdma.SetTdmaControllerHelper (controller);
      devices = tdma.Install (nodes);

      AsciiTraceHelper ascii;
      Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (tr_name + ".tr");
      tdma.EnableAsciiAll (stream);
    }
}

void
TdmaExample::InstallInternetStack ()
{
  DsdvHelper dsdv;
  dsdv.Set ("PeriodicUpdateInterval", TimeValue (Seconds (m_periodicUpdateInterval)));
  dsdv.Set ("SettlingTime", TimeValue (Seconds (m_settlingTime)));
  InternetStackHelper stack;
  stack.SetRoutingHelper (dsdv);
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces = address.Assign (devices);
}

void
TdmaExample::InstallApplications ()
{
  for (uint32_t i = 0; i <= m_nSinks - 1; i++ )
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Ipv4Address nodeAddress = node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
      Ptr<Socket> sink = SetupPacketReceive (nodeAddress, node);
    }

  for (uint32_t clientNode = 0; clientNode <= m_nWifis - 1; clientNode++ )
    {
      for (uint32_t j = 0; j <= m_nSinks - 1; j++ )
        {
          OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces.GetAddress (j), port)));
          onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
          onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

          if (j != clientNode)
            {
              ApplicationContainer apps1 = onoff1.Install (nodes.Get (clientNode));
              Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
              apps1.Start (Seconds (var->GetValue (m_dataStart, m_dataStart + 1)));
              apps1.Stop (Seconds (m_totalTime));
            }
        }
    }
}

void
TdmaExample::InsertIntoTxp ()
{
  m_transmitRangeMap.insert (std::pair<double, double> (100.0, 1.2251));
  m_transmitRangeMap.insert (std::pair<double, double> (130.0, 3.5049));
  m_transmitRangeMap.insert (std::pair<double, double> (150.0, 4.7478));
  m_transmitRangeMap.insert (std::pair<double, double> (180.0, 6.3314));
  m_transmitRangeMap.insert (std::pair<double, double> (200.0, 7.2457));
  m_transmitRangeMap.insert (std::pair<double, double> (300.0, 10.7675));
  m_transmitRangeMap.insert (std::pair<double, double> (350.0, 12.1065));
  m_transmitRangeMap.insert (std::pair<double, double> (400.0, 13.2663));
  m_transmitRangeMap.insert (std::pair<double, double> (450.0, 14.2893));
  m_transmitRangeMap.insert (std::pair<double, double> (500.0, 15.2045));
  m_transmitRangeMap.insert (std::pair<double, double> (550.0, 16.0323));
  m_transmitRangeMap.insert (std::pair<double, double> (600.0, 16.7881));
  m_transmitRangeMap.insert (std::pair<double, double> (650.0, 17.4834));
  m_transmitRangeMap.insert (std::pair<double, double> (700.0, 18.1271));
  m_transmitRangeMap.insert (std::pair<double, double> (800.0, 19.2869));
  m_transmitRangeMap.insert (std::pair<double, double> (900.0, 20.3099));
  m_transmitRangeMap.insert (std::pair<double, double> (950.0, 20.7796));
  m_transmitRangeMap.insert (std::pair<double, double> (1000.0, 21.2251));
  m_transmitRangeMap.insert (std::pair<double, double> (27800.0, 50.0));
}

