//Control path elements
//TODO remove this
arp_tbl::MF_ARPTable;
nbr_tbl::MF_NeighborTable();
rtg_tbl::MF_RoutingTable(1, NEIGHBOR_TABLE nbr_tbl);
lp_hndlr::MF_LinkProbeHandler(1, NEIGHBOR_TABLE nbr_tbl, ROUTING_TABLE rtg_tbl, ARP_TABLE arp_tbl, ETHER_TYPE 0x27C0, tap0);
lsa_hndlr::MF_LSAHandler(1, NEIGHBOR_TABLE nbr_tbl, ROUTING_TABLE rtg_tbl)
rstats::MF_RouterStats;
//Data path elements
//TODO why arp table?
agg::MF_Aggregator(ARP_TABLE arp_tbl, ROUTER_STATS rstats);
cache_mngr::MF_CacheManager;
//TODO decouple forward table API from routing table element
intra_lkup::MF_IntraLookUp(1, FORWARDING_TABLE rtg_tbl);
seg::MF_Segmentor(rstats);

//Queues
inQ::ThreadSafeQueue(65535); //incoming L2 pkt Q
net_binderQ::ThreadSafeQueue(100); //chunk Q prior to NA resolution 
holdQ::Queue(100); //delayed chunk Q for destns w/ poor link/path conditions
outQ_ctrl::ThreadSafeQueue(100); //L2 outgoing high priority control pkt queue
outQ_data::Queue(65535); //L2 outgoing lower priority data pkt queue
outQ_sched::PrioSched; //priority sched for L2 pkts
outQ_core::Queue(65535); //L2 outgoing pkt Q for 'core' port

//L2 packet sources
//core port
fd_core::FromSimDevice($core_dev, PROMISC false, SNIFFER true);

//L2 out i/f
td_core::ToSimDevice($core_dev);

fd_core -> SetTimestamp -> Paint(0, 16) -> inQ;

//start incoming pkt processing
inQ 	-> Unqueue
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

mf_cla[6] -> Discard; //No client access in this config
mf_cla[7] -> Discard; //No client access in this config
mf_cla[8] -> Discard;

//routing control pkts

mf_cla[0] -> [0]lp_hndlr; // link probe
mf_cla[1] -> [1]lp_hndlr; // link probe ack
mf_cla[2] -> [0]lsa_hndlr; // lsa

// data and Hop signalling pkts

mf_cla[3] -> [0]agg; // data 
mf_cla[4] -> [1]agg; // csyn 
mf_cla[5] -> [1]seg; // csyn ack

// Net-level processing for aggregated data blocks (chunks)

// chunks assembled post Hop transfer from upstream node
agg[0] 
	// chunk queue prior to NA resolution
	-> mf_cachefile::MF_CacheFile(FOLDER "/home/shweta/cache")
	-> net_binderQ;

//no network binder/GNRS resolution in this basic version
//move packet directly into forward table lookup 
net_binderQ 
	-> Unqueue 
    -> intra_lkup;

//Forwarding decisions

intra_lkup[0] ->[0]seg; //send chunk to next hop
// Hold if (1) dest not found in FT or (2) poor net condns to dest
intra_lkup[1] -> holdQ; //no forwarding entry
intra_lkup[2] -> holdQ; //hold decision
 
//TODO: use a unified cache manager to hold delayed packets
//This is a hack for now that simply delays chunk processing by const. seconds 
//followed by rebinding
holdQ 
	-> DelayUnqueue(1) 
	-> net_binderQ;

//Outgoing csyn/csyn-ack pkts - place in high priority queue 
agg[1]
	//TODO: encap should happen just before ToDevice
	-> MF_EtherEncap(0x27C0, $core_dev, arp_tbl)
	-> outQ_ctrl;

//Outgoing data frame
seg[0]
	//TODO: encap should happen just before ToDevice
	-> MF_EtherEncap(0x27C0, $core_dev, arp_tbl)
	-> outQ_data;

//Rebind chunks that failed transfer to specified downstream node
seg[1] -> net_binderQ;

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
outQ_data -> [1]outQ_sched; 

//to switch outgoing L2 pkts to respective learnt ports
out_switch::PaintSwitch(ANNO 16);
outQ_sched 
	-> Unqueue 
	-> MF_DevColor(arp_tbl) 
	-> out_switch;

//Send pkts switch to corresponding to-devices
//port 0 is core bound

//core pkts
out_switch[0] -> outQ_core -> td_core;

//Thread/Task Scheduling
//re-balance tasks to threads every 10ms
//BalancedThreadSched(INTERVAL 10);
//StaticThreadSched(fd_core 1, td_core 2, rtg_tbl 3);
