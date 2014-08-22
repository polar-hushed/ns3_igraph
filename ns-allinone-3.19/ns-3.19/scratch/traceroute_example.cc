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
#include "ns3/queue.h"
#include "ns3/netanim-module.h"
#include "ns3/icmpv4.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "BRITE/BriteNode.h"
#include "ns3/point-to-point-channel.h"
using namespace ns3;
using namespace std;
using namespace brite;


NS_LOG_COMPONENT_DEFINE ("IGraph");

int insert_parsed_node(BriteNode* b, std::list<BriteNode*>& bnl);
int isClient(uint32_t id, NodeContainer c, NodeContainer s);

void
DevRxTrace (std::string context, Ptr<const Packet> p)
{
/*  std::cout << "Got a packet" << std::endl;   
  std::cout << context << std::endl;   
  Ptr<Packet> copy = p->Copy();
  Icmpv4Header icmp;
  std::string node = context.substr(10,context.find('D')-1);
  copy->PeekHeader(icmp);
  {
	std::cout << "Got ICMP " << icmp.GetCode() << " type " << icmp.GetType()  << std::endl;
       if (icmp.GetType() == 'E')
	{
	std::cout << "Got ICMP " << std::endl;
        }
  }
*/
}

int
main (int argc, char *argv[])
{
  //NS_LOG_FUNCTION (this);

  double SimTime        = 201.00;
  //double SinkStartTime  = 1.0001;
  //double SinkStopTime   = 9.90001;
  double AppStartTime   = 2.0001;
  double AppStopTime    = 200.80001;

  std::string exampleRunID;
  exampleRunID = "RTBarabasi-2AS-10LeafNodes-9Clients-1Server-20Nodes";
  char *runID = strdup(exampleRunID.c_str()) ; 

  std::string AppPacketRate ("1Kbps");
//  Config::SetDefault ("ns3::TraceRoute::DataRate",  StringValue(AppPacketRate));
  std::string LinkRate ("1Mbps");
  std::string LinkDelay ("2ms");
 
  //uint16_t port = 9;

  LogComponentEnable ("UdpClient", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpServer", LOG_LEVEL_ALL);
  LogComponentEnable ("TraceRoute", LOG_LEVEL_INFO);

  //LogComponentEnable ("Igraph", LOG_LEVEL_INFO);

  // BRITE needs a configuration file to build its graph. By default, this
  // example will use the TD_ASBarabasi_RTWaxman.conf file. There are many others
  // which can be found in the BRITE/conf_files directory
  std::string confDir = "src/brite/examples/conf_files/";
  std::string confFile;
  std::string topoFile;
  bool tracing = false;
  bool pcap_tracing = false;
  bool nix = false;
  float percClients = 0.75;
  float percServers = 0.25;
  int type = 2;

  CommandLine cmd;
  cmd.AddValue ("confFile", "BRITE conf file", confFile);
  cmd.AddValue ("topoFile", "BRITE topology output file", topoFile);
  cmd.AddValue ("tracing", "Enable or disable ascii tracing", tracing);
  cmd.AddValue ("pcap", "Enable or disable pcap tracing", pcap_tracing);
  cmd.AddValue ("nix", "Enable or disable nix-vector routing", nix);
  cmd.AddValue ("percClients", "Percentage of leafnodes as clients", percClients);
  cmd.AddValue ("percServers", "Percentage of leafnodes as clients", percServers);
  cmd.AddValue ("rate", "Application packet rate", AppPacketRate);
  cmd.AddValue ("type", "Type of edge weights: 1=by wt degree, 2=src outdegree", type);

  cmd.Parse (argc,argv);

  NS_LOG_INFO("[SHWETA] Command line params \nconfFile=" <<  confFile << "\ntopoFile=" << topoFile << "\ntracing=" << tracing << "\nnix=" << nix << "\npercClient=" << percClients << "\npercServers=" << percServers);
  nix = false;

  // Invoke the BriteTopologyHelper and pass in a BRITE
  // configuration file and a seed file. This will use
  // BRITE to build a graph from which we can build the ns-3 topology
  BriteTopologyHelper bth (confDir + confFile, topoFile);
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
 /* 
  int numClients = percClients * nLeafNodes;
  int numServers = percServers * nLeafNodes;
*/

  int numClients = 1;
  int numServers = 1;

  client.Create (numClients);
  stack.Install (client);
  server.Create (numServers);
  stack.Install (server);

  NS_LOG_INFO ("[SHWETA] clients = " << numClients << " servers = " << numServers); 
  sprintf(runID, "%s%c%d%s%d%s%c%d%c%c%d%c%c%d%s%c%d%s", confFile.c_str(), '-', nAS,"AS-",nLeafNodes, "LeafNodes", '-', numClients,'c','-',numServers,'s','-',bth.GetNNodesTopology(),"Nodes",'-',bth.GetNEdgesTopology(),"Edges" );

  //install client node on last leaf node of AS 0
  //int numLeafNodesInAsZero = bth.GetNLeafNodesForAs (0);
  //client.Add (bth.GetLeafNodeForAs (0, numLeafNodesInAsZero - 1));

  //install clients and servers on leaf nodes in random ASs
  int numLeafNodesInAsZero = bth.GetNLeafNodesForAs (0);
  client.Add (bth.GetLeafNodeForAs (0, numLeafNodesInAsZero - 1));
  //install server node on last leaf node on AS 1
  server.Add (bth.GetLeafNodeForAs (0, numLeafNodesInAsZero - 2));



  // bth.AdjustWeights(type);
   DataCollector data;
   data.DescribeRun("Delay_graph", "brite", "Queueing_Delay", runID);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue (LinkRate));
  p2p.SetChannelAttribute ("Delay", StringValue (LinkDelay));


  NetDeviceContainer clientDev = p2p.Install(client);
  address.SetBase ("10.1.0.0", "255.255.0.0");
  Ipv4InterfaceContainer clientInterfaces;
  clientInterfaces = address.Assign (clientDev);

  NetDeviceContainer serverDev = p2p.Install(server);
  address.SetBase ("10.2.0.0", "255.255.0.0");
  Ipv4InterfaceContainer serverInterfaces;
  serverInterfaces = address.Assign (serverDev);
  if (!nix)
  {
     Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  }

  /*for (unsigned int i = 0 ; i < client.GetN(); ++i)
  {
      PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer apps_sink = sink.Install (client.Get (i));   // sink is installed on all nodes
      apps_sink.Start (Seconds (SinkStartTime));
      apps_sink.Stop (Seconds (SinkStopTime));
  }*/
  uint16_t port = 4000;
 
    unsigned int i = 0; 
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
	     std::cout << "client address " << i << "is " <<  clientInterfaces.GetAddress(i) << std::endl;
              /*UdpServerHelper udps  (
				port
				); // traffic flows from node[j] to node[i]

	      ApplicationContainer apps ;*/
	      ApplicationContainer appc ;

             // apps = udps.Install (server.Get (j));  // traffic sources are installed on all nodes
              uint32_t MaxPacketSize = 1024;
              Time interPacketInterval = Seconds (5);
              uint32_t maxPacketCount = 320;
              TraceRouteHelper udpc = TraceRouteHelper(serverInterfaces.GetAddress(j), port);
	      udpc.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
	      udpc.SetAttribute ("Interval", TimeValue (interPacketInterval));
	      udpc.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
	      appc = udpc.Install (client.Get (i));
	
	      Ptr<TraceRoute> c = udpc.GetClient();
              c->SetIpTtl(1);

             // apps.Start (Seconds (AppStartTime + rn));
              appc.Start (Seconds (AppStartTime + rn));
             // apps.Stop (Seconds (AppStopTime));
              appc.Stop (Seconds (AppStopTime));
	      ++i;
	      if (i >= client.GetN())
		break; 
            }

	Config::Connect("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacRx",MakeCallback(&DevRxTrace));
	std::list<brite::Edge*> el = bth.GetBriteEdgeList();
	std::list<brite::BriteNode*> bnl ;
	NS_LOG_INFO("Preparing to make stats ");

	for (std::list<brite::Edge*>::iterator e = el.begin(); e != el.end(); ++e)
	{

		brite::BriteNode *b = (*e)->GetSrc();
		
		int ret =  insert_parsed_node(b, bnl);
                if (ret == 0)
		{
		    NS_LOG_INFO("[insert_parse_node] node already parsed " << b->GetId());
		    continue;
		}
		Ptr<Node> n = bth.GetNode(b->GetId());
		for (uint32_t k= 1; k < n->GetNDevices(); ++k)
		{
	  		Ptr<NetDevice> net = n->GetDevice(k);
	  		if (net != NULL  )
	  		{
				PointerValue tmp;
				net->GetAttribute("TxQueue", tmp);
				Ptr<Object> tmpObject = tmp.GetObject();
				Ptr<Queue> txQueue = tmpObject->GetObject <Queue> ();
				NS_ASSERT(txQueue != 0);

				Ptr<TimeMinMaxAvgTotalCalculator> delayStat = 
					CreateObject<TimeMinMaxAvgTotalCalculator>();
				if (delayStat != NULL)
				{
					delayStat->Start(Time(AppStartTime));
					delayStat->SetKey("delay");
					char context[100] ;
					sprintf(context,"Node %d isClient? %d NetDevice %d WtDegree %f OutDegree %d LinkBW %f", b->GetId() ,isClient(b->GetId(), client, server), k,  b->GetWeight(), b->GetOutDegree(), (*e)->GetConf()->GetBW());
					delayStat->SetContext(context);

					txQueue->SetDelayTracker(delayStat);
					data.AddDataCalculator(delayStat);
				}

			}
			else
				NS_LOG_WARN("[SHWETA] No net device found on node.");
		}
	}
  if (!nix)
    {
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    }

  if (tracing)
    {
      AsciiTraceHelper ascii;
      p2p.EnableAsciiAll (ascii.CreateFileStream ("brite.tr"));
    }

  if (pcap_tracing)
    {
      AsciiTraceHelper ascii;
      p2p.EnablePcapAll ("briteLeaves");
    }

 
  // Run the simulator
  bth.PrintBriteTopology();
     // Generate Stats

   Ptr<DataOutputInterface> output = 0;
   //create data in omnet format
   NS_LOG_INFO ("Creating omnet formatted data.");
   output = CreateObject<OmnetDataOutput>();
   if (output !=0)
	output->Output(data);
   else
   NS_LOG_ERROR ("Stats Failed.");
  //Stats done
  Simulator::Stop (Seconds (SimTime));
  Simulator::Run ();


  Simulator::Destroy ();

  return 0;
}

int insert_parsed_node(BriteNode* b, std::list<BriteNode*>& bnl)
{
      int node = b->GetId();
      std::list<BriteNode*>::iterator it;
      if (bnl.empty() == true)
      {
	NS_LOG_INFO("[insert_parsed_node] inserting the first " << node  );
	bnl.push_back(b);
	return 1;
      }
      for (it = bnl.begin(); it != bnl.end(); ++it)
      {
	int id  = (*it)->GetId();
	if (id == node)
	{
		break;
	}
      }	
  
      if (it == bnl.end())
      {
	bnl.push_back(b);
	return 1;
      }
     return 0;
}

int isClient(uint32_t id, NodeContainer c, NodeContainer s)
{

  uint32_t nNodes = c.GetN ();
   for (uint32_t i = 0; i < nNodes; ++i)
  {
    Ptr<Node> p = c.Get(i);
    if (p->GetId() == id)
 	return 1;
  }
   nNodes = s.GetN();
   for (uint32_t i = 0; i < nNodes; ++i)
  {
    Ptr<Node> p = s.Get(i);
    if (p->GetId() == id)
 	return 2;
  }    
            
   return 0;
            
}
