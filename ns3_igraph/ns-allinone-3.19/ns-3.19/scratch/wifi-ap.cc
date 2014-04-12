/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/athstats-helper.h"

#include <iostream>
#include <vector>

using namespace ns3;
struct mov_avg
{
   long count;
   double value[2];
   double time;
};
const double WINDOW_SIZE = 50.0;

static bool g_verbose = true;
NodeContainer stas;
NodeContainer ap;
std::map<int, mov_avg> rss_avg;
 
std::ofstream fs;

 static void
SetPosition (Ptr<Node> node, Vector position)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  mobility->SetPosition (position);
}

static Vector
GetPosition (Ptr<Node> node)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  return mobility->GetPosition ();
}

static void 
AdvancePosition (Ptr<Node> node) 
{
  Vector pos = GetPosition (node);
  pos.x += 5.0;
  if (pos.x >= 210.0) 
    {
      return;
    }
  SetPosition (node, pos);

  if (g_verbose)
    {
      //std::cout << "x="<<pos.x << std::endl;
    }
  Simulator::Schedule (Seconds (1.0), &AdvancePosition, node);
}


void
DevTxTrace (std::string context, Ptr<const Packet> p)
{
  if (g_verbose)
    {
      std::cout << " TX p: " << *p << std::endl;
    }
}
void
DevRxTrace (std::string context, Ptr<const Packet> p)
{
  if (g_verbose)
    {
      std::cout << " RX p: " << *p << std::endl;
    }
}
void
PhyRxOkTrace (std::string context, Ptr<const Packet> packet, double snr, WifiMode mode, enum WifiPreamble preamble)
{
  if (g_verbose)
    {
      std::cout << Simulator::Now() << " " << context <<  " PHYRXOK mode=" << mode << " snr=" << snr << " " <<  std::endl;
    }
}
void
PhyRxErrorTrace (std::string context, Ptr<const Packet> packet, double snr)
{
  if (g_verbose)
    {
      std::cout << "PHYRXERROR snr=" << snr << " " << *packet << std::endl;
    }
}
void
PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower)
{
  if (g_verbose)
    {
      std::cout << "PHYTX mode=" << mode << " " << *packet << std::endl;
    }
}
void
PhyStateTrace (std::string context, Time start, Time duration, enum WifiPhy::State state)
{
  if (g_verbose)
    {
      std::cout << " state=" << state << " start=" << start << " duration=" << duration << std::endl;
    }
}
void
RSSITrace(std::string context, Ptr<const Packet> packet, uint16_t channelFreqMhz, uint16_t channelNumber, uint32_t rate, bool isShortPreamble, double signalDbm, double noiseDbm)
{
  std::istringstream iss(context.substr(9,context.length()-10), std::ios_base::in);
  std::string str;
  char c;
  int nodeId;
  iss >> c >>  nodeId >> str; 
  Vector position = GetPosition(stas.Get(nodeId));
 
   double time = Simulator::Now().GetSeconds()-rss_avg[nodeId].time ; 
   rss_avg[nodeId].count += 1;
   rss_avg[nodeId].value[0] += signalDbm;
   rss_avg[nodeId].value[1] += noiseDbm;
   
if (time >= WINDOW_SIZE)
{
if (g_verbose)
    {
      std::cout << Simulator::Now().GetSeconds() << " " <<  nodeId << " position (" << position.x << "," << position.y ; 
      std::cout << ") signalStrength=" << signalDbm << " Noise=" << noiseDbm << std::endl;
    }
   
   fs << Simulator::Now().GetSeconds() << " " <<  nodeId << " position (" << position.x << "," << position.y ; 
   fs << ") signalStrength=" << rss_avg[nodeId].value[0]/rss_avg[nodeId].count << " Noise=" << rss_avg[nodeId].value[1]/rss_avg[nodeId].count << std::endl;

rss_avg[nodeId].time=Simulator::Now().GetSeconds();

rss_avg[nodeId].value[0]=0;

rss_avg[nodeId].value[1]=0;

rss_avg[nodeId].count = 0;

}    
    
}
int main (int argc, char *argv[])
{
  std::string p_model = "ns3::JakesPropagationLossModel";
  CommandLine cmd;
  cmd.AddValue ("verbose", "Print trace information if true", g_verbose);
  cmd.AddValue ("pmodel", "Propagation Loss Model to use", p_model);

  cmd.Parse (argc, argv);
  AsciiTraceHelper ascii;

  Packet::EnablePrinting ();

  // enable rts cts all the time.
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
  // disable fragmentation
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  Config::SetDefault("ns3::YansWifiChannel::PropagationLossModel", StringValue(p_model));
  WifiHelper wifi = WifiHelper::Default ();
  MobilityHelper mobility;
 NetDeviceContainer staDevs;
  PacketSocketHelper packetSocket;

  stas.Create (5);
  ap.Create (3);
  for (int i = 0; i < 5; ++i) {
		rss_avg[i].count = 0;
		rss_avg[i].time = 0;
		rss_avg[i].value[0] = 0;
		rss_avg[i].value[1] = 0;
  }

  // give packet socket powers to nodes.
  packetSocket.Install (stas);
  packetSocket.Install (ap);

  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  Ssid ssid = Ssid ("wifi-default");
  wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
  // setup stas.
  wifiMac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue (false));
  staDevs = wifi.Install (wifiPhy, wifiMac, stas);
  // setup ap.
  wifiMac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));
  wifi.Install (wifiPhy, wifiMac, ap);

  // mobility.
  mobility.Install (stas);
  mobility.Install (ap);
  //wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("rss_trace.tr"));
  //Simulator::Schedule (Seconds (1.0), &AdvancePosition, ap.Get (0));

  PacketSocketAddress socket[4];
  ApplicationContainer apps[4];
  OnOffHelper onoff[4]= {OnOffHelper ("ns3::PacketSocketFactory", Address(socket[0])), OnOffHelper ("ns3::PacketSocketFactory", Address(socket[0])),OnOffHelper ("ns3::PacketSocketFactory", Address(socket[0])),OnOffHelper ("ns3::PacketSocketFactory", Address(socket[0]))};

  Vector pos;
  pos.x = 0;
  pos.y = 0;
  pos.z = 10;
  for (int i = 0; i < 3; ++i)
  {
  	SetPosition (ap.Get(i), pos);
  	pos.x = pos.x + 10;
  	pos.y = pos.y + 10;
  }
  pos.x = 0;
  pos.y = 10;
  pos.z = 0;
  
  for (unsigned int i = 0; i < stas.GetN()-1; ++i)
  {
  	socket[i].SetSingleDevice (staDevs.Get (i)->GetIfIndex ());
  	socket[i].SetPhysicalAddress (staDevs.Get (4)->GetAddress ());
  	socket[i].SetProtocol (1);
	onoff[i].SetAttribute("Protocol", StringValue("ns3::PacketSocketFactory"));
        onoff[i].SetAttribute("Remote", AddressValue(Address( (socket[i]))));


  	onoff[i].Install (stas.Get (4));
	onoff[i].SetConstantRate (DataRate ("500kb/s"));
  	apps[i].Start (Seconds (0.5));
  	apps[i].Stop (Seconds (43.0));
	if (i >= 2)
        {
  		pos.x = 100;
        }
  	SetPosition (stas.Get(i), pos);
  }
        pos.x = 50;
         pos.y = 10;
  	SetPosition (stas.Get(4), pos);

//  Config::Connect ("/NodeList/*/DeviceList/*/Mac/MacTx", MakeCallback (&DevTxTrace));
 // Config::Connect ("/NodeList/*/DeviceList/*/Mac/MacRx", MakeCallback (&DevRxTrace));
  std::ostringstream oss[5];
  fs.open(p_model.c_str());
  for (unsigned int i=0; i < stas.GetN();++i)
  {
    oss[i] << "NodeList/"<< stas.Get(i)->GetId() << "/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx";
  //  oss << "NodeList/"<< stas.Get(i)->GetId() << "/DeviceList/*/Phy/State/RxOk";
    Config::Connect (oss[i].str(), MakeCallback (&RSSITrace));
  //  Config::Connect (oss.str(), MakeCallback (&PhyRxOkTrace));
  }
  //Config::Connect ("/NodeList/*/DeviceList/*/Phy/State/RxError", MakeCallback (&PhyRxErrorTrace));
  //Config::Connect ("/NodeList/*/DeviceList/*/Phy/State/Tx", MakeCallback (&PhyTxTrace));
 // Config::Connect ("/NodeList/*/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace));
 // Config::Connect ("/NodeList/*/DeviceList/*/WifiNetDevice/Channel/YansRSS", MakeCallback (&RSSITrace));


  
/*
  AthstatsHelper athstats;
  athstats.EnableAthstats ("athstats-sta", stas);
  athstats.EnableAthstats ("athstats-ap", ap);
*/
  Simulator::Run ();

  Simulator::Stop (Seconds (1000.0));
  Simulator::Destroy ();
  fs.close();
  return 0;
}
