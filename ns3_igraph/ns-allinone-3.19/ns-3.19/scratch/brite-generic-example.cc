/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */
//Modify this file to create Internet tcp traffic
#include <sstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/stats-module.h"
#include "ns3/applications-module.h"
#include "ns3/brite-module.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include <iostream>
#include <fstream>
#include <time.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("IGraph");

int
main (int argc, char *argv[])
{

  double SimTime        = 1000.00;
  double SinkStartTime  = 1.0001;
  double SinkStopTime   = 999.90001;
  double AppStartTime   = 10.0001;
  double AppStopTime    = 999.80001;

  std::string AppPacketRate ("10Kbps");
  Config::SetDefault  ("ns3::OnOffApplication::PacketSize",StringValue ("1000"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (AppPacketRate));
  std::string LinkRate ("10Mbps");
  std::string LinkDelay ("2ms");
 
  uint16_t port = 9;

  //LogComponentEnable ("UdpClient", LOG_LEVEL_ALL);
  //LogComponentEnable ("UdpServer", LOG_LEVEL_ALL);

  LogComponentEnable ("IGraph", LOG_LEVEL_INFO);

  // BRITE needs a configuration file to build its graph. By default, this
  // example will use the TD_ASBarabasi_RTWaxman.conf file. There are many others
  // which can be found in the BRITE/conf_files directory
  std::string confFile = "src/brite/examples/conf_files/TD_ASBarabasi_RTWaxman.conf";
  bool tracing = true;
  bool nix = false;

  CommandLine cmd;
  cmd.AddValue ("confFile", "BRITE conf file", confFile);
  cmd.AddValue ("tracing", "Enable or disable ascii tracing", tracing);
  cmd.AddValue ("nix", "Enable or disable nix-vector routing", nix);

  cmd.Parse (argc,argv);

  nix = false;

  // Invoke the BriteTopologyHelper and pass in a BRITE
  // configuration file and a seed file. This will use
  // BRITE to build a graph from which we can build the ns-3 topology
  BriteTopologyHelper bth (confFile);
  bth.AssignStreams (int(time(0))%20);


  Ipv4StaticRoutingHelper staticRouting;
  Ipv4GlobalRoutingHelper globalRouting;
  Ipv4ListRoutingHelper listRouting;
  Ipv4NixVectorHelper nixRouting;

  InternetStackHelper stack;

  if (nix)
    {
      listRouting.Add (staticRouting, 0);
      listRouting.Add (nixRouting, 10);
    }
  else
    {
      listRouting.Add (staticRouting, 0);
      listRouting.Add (globalRouting, 10);
    }

  stack.SetRoutingHelper (listRouting);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.252");

  bth.BuildBriteTopology (stack);
  bth.AssignIpv4Addresses (address);

  int nAS = bth.GetNAs();
  NS_LOG_INFO ("[SHWETA] Number of AS created " << nAS);
  NS_LOG_INFO ("[SHWETA] I have created " << nAS);

  int nLeafNodes = 0;
  for (int i = 0; i < nAS; ++i)
  {
       int nLeafNodesThisAs = 0;

       nLeafNodesThisAs = bth.GetNLeafNodesForAs(i);
       nLeafNodes += nLeafNodesThisAs;
       NS_LOG_INFO ("[SHWETA] Number of leaf nodes in AS " << i << " is " << nLeafNodesThisAs); 
       NS_LOG_INFO ("[SHWETA] Number of nodes in AS " << i << " is " << bth.GetNNodesForAs(i)); 
 }
       NS_LOG_INFO ("[SHWETA] Total number of leaf nodes is " << nLeafNodes); 
 
  //The BRITE topology generator generates a topology of routers.  Here we create
  //two subnetworks which we attach to router leaf nodes generated by BRITE
  //Any NS3 topology may be used to attach to the BRITE leaf nodes but here we
  //use just one node

  NodeContainer client;
  NodeContainer server;
  
  int numClients = 0.75 * nLeafNodes;
  int numServers = 0.25 * nLeafNodes;
  client.Create (numClients);
  stack.Install (client);
  server.Create (numServers);
  stack.Install (server);
  NS_LOG_INFO ("[SHWETA] clients = " << numClients << " servers = " << numServers); 

  //install client node on last leaf node of AS 0
  //int numLeafNodesInAsZero = bth.GetNLeafNodesForAs (0);
  //client.Add (bth.GetLeafNodeForAs (0, numLeafNodesInAsZero - 1));

  //install clients and servers on leaf nodes in random ASs

  int server_count = 0;
  int client_count = 0;
  for (unsigned int i = 0; i < bth.GetNAs(); ++i)
    for (unsigned int j = 0 ; j < bth.GetNLeafNodesForAs(i); ++j)
	{
		int random = rand();
		if (random%100 > 75 && server_count < numServers)
                {
  			server.Add (bth.GetLeafNodeForAs (i, j));
  			NS_LOG_INFO ("[SHWETA] server added in node " << j << " in AS " << i );
			++server_count;
				
                }
	        else if(client_count < numClients)
		{
			client.Add(bth.GetLeafNodeForAs (i, j));		 
  			NS_LOG_INFO ("[SHWETA] client added in node " << j << " in AS " << i );
			++client_count;
		}
	}
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue (LinkRate));
  p2p.SetChannelAttribute ("Delay", StringValue (LinkDelay));
  address.SetBase ("10.1.0.0", "255.255.0.0");

  for (unsigned int i = 0 ; i < client.GetN(); ++i)
  {
  	for (unsigned int j = 0 ; j < server.GetN(); ++j)
	{
              NodeContainer n_links = NodeContainer (client.Get(i), server.Get (j));
              NetDeviceContainer n_devs = p2p.Install (n_links);
	      address.Assign(n_devs);

	}

  }
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  for (unsigned int i = 0 ; i < client.GetN(); ++i)
  {
      PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer apps_sink = sink.Install (client.Get (i));   // sink is installed on all nodes
      apps_sink.Start (Seconds (SinkStartTime));
      apps_sink.Stop (Seconds (SinkStopTime));
  }

  for (unsigned int i = 0; i < client.GetN(); i++)
    {
      for (unsigned int j = 0; j < server.GetN(); j++)
        {

              // We needed to generate a random number (rn) to be used to eliminate
              // the artificial congestion caused by sending the packets at the
              // same time. This rn is added to AppStartTime to have the sources
              // start at different time, however they will still send at the same rate.

              Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
              x->SetAttribute ("Min", DoubleValue (0));
              x->SetAttribute ("Max", DoubleValue (1));
              double rn = x->GetValue ();
              Ptr<Node> c = client.Get(i);
              Ptr<Ipv4> ipv4 = c->GetObject<Ipv4>();
              OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (ipv4->GetAddress(1,0).GetLocal(), port)); // traffic flows from node[j] to node[i]
              onoff.SetConstantRate (DataRate (AppPacketRate));
              ApplicationContainer apps = onoff.Install (server.Get (j));  // traffic sources are installed on all nodes
              apps.Start (Seconds (AppStartTime + rn));
              apps.Stop (Seconds (AppStopTime));
            }
        }

  if (!nix)
    {
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    }

  if (tracing)
    {
      AsciiTraceHelper ascii;
      p2p.EnableAsciiAll (ascii.CreateFileStream ("briteLeaves.tr"));
      //p2p.EnablePcapAll ("briteLeaves");
    }
  // Run the simulator
  Simulator::Stop (Seconds (SimTime));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
