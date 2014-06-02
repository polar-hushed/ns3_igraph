// Parameters:
// my_GUID
// topo_file - GUID-based topology file 
// core_dev

// Maintains router-wide resource stats, etc. 
routerstats::MF_RouterStats;

//Control path elements
//TODO remove this
arp_tbl::MF_ARPTable;
nbr_tbl::MF_NeighborTable();
rtg_tbl::MF_RoutingTable(MY_GUID $my_GUID, NEIGHBOR_TABLE nbr_tbl);
lp_hndlr::MF_LinkProbeHandler(MY_GUID $my_GUID, NEIGHBOR_TABLE nbr_tbl, ROUTING_TABLE rtg_tbl, ARP_TABLE arp_tbl, ETHER_TYPE 0x27C0, SRC $core_dev);
lsa_hndlr::MF_LSAHandler(MY_GUID $my_GUID, NEIGHBOR_TABLE nbr_tbl, ROUTING_TABLE rtg_tbl)

//Data path elements
//TODO why arp table?
agg::MF_Aggregator(ARP_TABLE arp_tbl, ROUTER_STATS routerstats);

//enforces stated topology
topo_mngr::MF_TopologyManager(MY_GUID $my_GUID, TOPO_FILE $topo_file, ARP_TABLE arp_tbl);

//Counters and Statistics
//TODO: build a custom chunk counter to get proper data byte count
//incoming L2 pkt and byte count 
inCntr_pkt::Counter()
//incoming L3 chunk and byte count 
inCntr_chunk::Counter()
//outgoing L2 pkt and byte count 
outCntr_pkt::Counter

//Queues
inQ::ThreadSafeQueue(65535); //incoming L2 pkt Q
outQ_ctrl::ThreadSafeQueue(100); //L2 outgoing high priority control pkt queue
outQ_sched::PrioSched; //priority sched for L2 pkts
outQ_core::Queue(65535); //L2 outgoing pkt Q for 'core' port

//L2 packet sources
//core port
fd_core::FromDevice($core_dev, PROMISC false, SNIFFER true);

//L2 out i/f
td_core::ToDevice($core_dev);

fd_core -> SetTimestamp -> Paint(0, 16) -> inQ;

//start incoming pkt processing
inQ 	-> Unqueue
	-> topo_mngr
	-> inCntr_pkt
	//drop anything that isn't MF
	-> net_cla::Classifier(12/27C0, -);

net_cla[1] -> Discard;

net_cla[0] 
	-> Strip(14) // drop eth header
	-> mf_cla::Classifier(
			00/00000003, // p0 Link probe
			00/00000004, // p1 Link probe response
			00/00000005, // p2 LSA
			00/00000000, // p3 Data
			00/00000001, // p4 CSYN
			00/00000002, // p5 CSYN-ACK
			00/00000006, // p6 Client association
			00/00000007, // p7 Client dis-association 
			-);          // p8 Unhandled type, discard

mf_cla[5] -> Discard; // No csyn-acks in this config - only recv op
mf_cla[6] -> Discard; // No client access in this config
mf_cla[7] -> Discard; // No client access in this config
mf_cla[8] -> Discard;

//routing control pkts

mf_cla[0] -> [0]lp_hndlr; // link probe
mf_cla[1] -> [1]lp_hndlr; // link probe ack
mf_cla[2] -> [0]lsa_hndlr; // lsa

//data and Hop signalling pkts

mf_cla[3] -> [0]agg; // data 
mf_cla[4] -> [1]agg; // csyn 

//Net-level processing for aggregated data blocks (chunks)

//chunks assembled post Hop transfer from upstream node
agg[0] 
	-> inCntr_chunk 
	-> mf_tofile::MF_ToFile(FOLDER "/root/file");

//Outgoing csyn/csyn-ack pkts - place in high priority queue 
agg[1]
	//TODO: encap should happen just before ToDevice
	-> MF_EtherEncap(0x27C0, $core_dev, arp_tbl)
	-> outQ_ctrl;

//Outgoing control pkts
lp_hndlr[0] //outgoing link probe
	//TODO unify encapsulation
	-> EtherEncap(0x27C0, $core_dev, ff:ff:ff:ff:ff:ff)
	-> outQ_ctrl;

lp_hndlr[1] //outgoing link probe ack
	-> EtherEncap(0x27C0, $core_dev, ff:ff:ff:ff:ff:ff)
	-> outQ_ctrl;

lsa_hndlr[0] //outgoing lsa
	-> EtherEncap(0x27C0, $core_dev, ff:ff:ff:ff:ff:ff)
	-> outQ_ctrl;

//priority schedule data and control pkts
outQ_ctrl -> [0]outQ_sched; 

//to switch outgoing L2 pkts to respective learnt ports
out_switch::PaintSwitch(ANNO 16);
outQ_sched 
	-> outCntr_pkt 
	-> Unqueue 
	-> MF_DevColor(arp_tbl) 
	-> out_switch;

//Send pkts switch to corresponding to-devices
//port 0 is core bound

//core pkts
out_switch[0] -> outQ_core -> td_core;

//Thread/Task Scheduling
//re-balance tasks to threads every 10ms
BalancedThreadSched(INTERVAL 10);
//StaticThreadSched(fd_core 1, td_core 1, fd_edge 2, td_edge 2, rtg_tbl 3);
