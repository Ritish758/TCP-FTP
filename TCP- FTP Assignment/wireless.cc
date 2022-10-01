#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/gnuplot.h"
#include <string>
#include <fstream>


using namespace std;
using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("wireless");

class SimulatorApp : public Application
{
	public:
  		SimulatorApp ();
  		virtual ~SimulatorApp ();
  		void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

	private:
  		virtual void StartApplication (void);
  		virtual void StopApplication (void);

		  void ScheduleTx (void);
		  void SendPacket (void);

		  Ptr<Socket>     m_socket;
		  Address         m_peer;
		  uint32_t        m_packetSize;
		  uint32_t        m_nPackets;
		  DataRate        m_dataRate;
		  EventId         m_sendEvent;
		  bool            m_running;
		  uint32_t        m_packetsSent;
};


//Constructor
SimulatorApp::SimulatorApp ()
{
    m_socket=0;
    m_packetSize=0;
    m_nPackets=0;
    m_dataRate=0;
    m_running=false;
    m_packetsSent=0;
}


//Deconstructor
SimulatorApp::~SimulatorApp ()
{
  m_socket = 0;
}


// Initialise Parameters
void SimulatorApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}


// Start Application
void SimulatorApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
  m_socket->Connect (m_peer);
  SendPacket ();
}


// Stop Application
void SimulatorApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}


// Sending and scheduling Packets
void SimulatorApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}


// Packet Scheduler
void SimulatorApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &SimulatorApp::SendPacket, this);
    }
}

int main(int argc, char *argv[])
{
	
	//Set time resolution
	Time::SetResolution (Time::NS);
	
	//enabling the log component
	LogComponentEnable("wireless" , LOG_INFO);
	
	//obtaining TCP agent
	string socket_type;
	CommandLine cmd;
	cmd.AddValue ("agent", "The TCP agent you want to use:", socket_type);
	cmd.Parse (argc, argv);
	
	// setting socket type (according to TCP-agent)
	if(socket_type=="Westwood")
	{
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpWestwood"));
	}
	else if (socket_type=="Veno")
	{
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVeno"));
	}
	else if (socket_type=="Vegas")
	{
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
	}
	else
	{
		NS_LOG_INFO("Invalid TCP agent, please enter one among {Westwood, Veno, Vegas}");
		exit(1);
	}
			
	
	NS_LOG_INFO("Wireless_TCP - "+ socket_type);
	NS_LOG_INFO("+-----------------------------------------------+");
	NS_LOG_INFO("|Packet Size  |   Throughput   |  Fairness Index|");
	NS_LOG_INFO("+-----------------------------------------------+");
	
	
	//Generating Plots
	string graphicsFileName = "Wireless_TCP_"+socket_type+".png";
	string plotFileName = "Wireless_TCP_"+socket_type+".plt";
	string plotTitle = "Throughput vs Packet size for TCP-"+socket_type;
	string dataTitle = "TCP-"+socket_type;
	
	Gnuplot plot (graphicsFileName);
	plot.SetTitle(plotTitle);
	plot.SetTerminal("png");
	plot.SetLegend ("Packet Size (in bytes)", "Throughput (in Kbps)");
	plot.AppendExtra ("set xrange [20:1520]");
	
	Gnuplot2dDataset dataset;
  	dataset.SetTitle (dataTitle);
  	dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
  	
  	
	int packet_sizes[10]={40, 44, 48, 52, 60, 552, 576, 628, 1420, 1500};
	
	for(int i=0;i<10;i++)
	{
	
		//setting segment size
		int segment_size=packet_sizes[i];
		Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (segment_size));	
		
		
		//creating nodes
		NodeContainer nodes;
		nodes.Create(4);
		
		//creating wired-links
		PointToPointHelper BaseToBase;
  		BaseToBase.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  		BaseToBase.SetChannelAttribute ("Delay", StringValue ("100ms"));
  		int mxPacketsInQueue = (10*100*1000)/(8*segment_size);
  		BaseToBase.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue(to_string(mxPacketsInQueue)+"p"));
  		
  		  		
  		
  		//setting up wired-links between base-stations
  		NetDeviceContainer path_BS1BS2;
  		path_BS1BS2 = BaseToBase.Install( nodes.Get(1), nodes.Get(2));
  		
  		
  		//setting up wireless-links between host and base-station
  		YansWifiChannelHelper N0BS1 = YansWifiChannelHelper::Default ();
  		YansWifiPhyHelper phy_N0BS1;
  		phy_N0BS1.SetChannel (N0BS1.Create ());
  		
  		YansWifiChannelHelper N1BS2 = YansWifiChannelHelper::Default ();
  		YansWifiPhyHelper phy_N1BS2;
  		phy_N1BS2.SetChannel (N1BS2.Create ());
  		
  		// creating and setting wifi
  		WifiHelper wifi;
  		wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  		WifiMacHelper mac;
  		Ssid ssid = Ssid ("ns-3-ssid");
  		
  		
  		//installing wifi on basestations
  		mac.SetType ("ns3::StaWifiMac",
               	"Ssid", SsidValue (ssid),
               	"ActiveProbing", BooleanValue (false));           
               NetDeviceContainer accessPoint_BS1, accessPoint_BS2;
  		accessPoint_BS1 = wifi.Install(phy_N0BS1, mac, nodes.Get(1));
  		accessPoint_BS2 = wifi.Install(phy_N1BS2, mac, nodes.Get(2));
               
               
               // installing wifi on hosts
               mac.SetType ("ns3::ApWifiMac",
               	"Ssid", SsidValue (ssid));

  		NetDeviceContainer endPoint_N0, endPoint_N1;
  		endPoint_N0 = wifi.Install (phy_N0BS1, mac, nodes.Get(0));
  		endPoint_N1 = wifi.Install (phy_N1BS2, mac, nodes.Get(3));
  		
  		
  		// Setting wireless links between hosts and basestations
  		NetDeviceContainer path_N0BS1(endPoint_N0, accessPoint_BS1);
  		NetDeviceContainer path_N1BS2(accessPoint_BS2, endPoint_N1);        
  		
  		
  		//Setting positions of devices
  		MobilityHelper mobility;

  		mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 		"MinX", DoubleValue (0.0),
                                 		"MinY", DoubleValue (0.0),
                                 		"DeltaX", DoubleValue (5.0),
                                 		"DeltaY", DoubleValue (10.0),
                                 		"GridWidth", UintegerValue (3),
                                 		"LayoutType", StringValue ("RowFirst"));

               mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
               
         	mobility.Install(nodes.Get(0));
    	 	mobility.Install(nodes.Get(1));
    		mobility.Install(nodes.Get(2));
    		mobility.Install(nodes.Get(3));

               
  		//building Internet stack
  		InternetStackHelper stack;
  		stack.Install(nodes);
  		
  		
  		//assigning Ip addresses
  		Ipv4AddressHelper ipv4_N0BS1;
	      	ipv4_N0BS1.SetBase( "10.1.1.0" , "255.255.255.0" );
	      	Ipv4InterfaceContainer N0BS1Interface = ipv4_N0BS1.Assign (path_N0BS1);

	      	Ipv4AddressHelper ipv4_BS1BS2;
	      	ipv4_BS1BS2.SetBase( "10.1.2.0" , "255.255.255.0" );
	      	Ipv4InterfaceContainer BS1BS2Interface = ipv4_BS1BS2.Assign (path_BS1BS2);

	      	Ipv4AddressHelper ipv4_N1BS2;
	      	ipv4_N1BS2.SetBase( "10.1.3.0" , "255.255.255.0" );
	      	Ipv4InterfaceContainer N1BS2Interface = ipv4_N1BS2.Assign (path_N1BS2);
	      	
	      	
	      	//Assigning port Number;
	      	uint16_t sinkPort = 9897;

	
	      	Address sinkAddress = InetSocketAddress (N1BS2Interface.GetAddress (1), sinkPort);
      		Address anyAddress = InetSocketAddress (Ipv4Address::GetAny (), sinkPort);
      		//Creating application container
      		PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", anyAddress);
      		ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (3));
      		//Set the start and stop times for the server-side
      		sinkApps.Start (Seconds (0.));
  		sinkApps.Stop (Seconds (20.));


      		// Create a client socket
      		Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
      		//Create an Application
      		Ptr<SimulatorApp> simulApp = CreateObject<SimulatorApp> ();
      		simulApp->Setup(ns3TcpSocket, sinkAddress, segment_size, 10000, DataRate ("100Mbps"));  //can we change 1000, 20Mbps
      		nodes.Get(0)->AddApplication(simulApp);
      		//Set the start and stop times for the client-side
      		simulApp->SetStartTime (Seconds (0.));
      		simulApp->SetStopTime (Seconds (20.));


		//populating routing tables         
      		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
      		

		//Flow monitor
		Ptr<FlowMonitor> flowMonitor;
		FlowMonitorHelper flowHelper;
		flowMonitor = flowHelper.InstallAll();
		
		Simulator::Stop (Seconds (20));
  		Simulator::Run ();
  		
  		// Output the data in xml format
		flowMonitor->SerializeToXmlFile("wireless_TCP_"+socket_type+"_"+std::to_string(segment_size)+".xml", true, true);
  		
  		//Obtaining statistics 
  		auto statistics=flowMonitor->GetFlowStats().begin();
  		
  		double totData = 8.0 * statistics->second.rxBytes;
  		double totTime = statistics->second.timeLastRxPacket.GetSeconds()-statistics->second.timeFirstRxPacket.GetSeconds();
  		
  		double throughput = totData/(1000*totTime);
  		
  		double sumThroughput=0;
  		double sumSqThroughput=0;
  		int n=0;
  		
  		sumThroughput += throughput;
  		sumSqThroughput += throughput*throughput;
  		n++;
  		
  		double jain_fairness = (sumThroughput*sumThroughput)/((n+0.0)*sumSqThroughput);
  		
  		Simulator::Destroy ();
  		  		
  		
  		//Output results
  		if(segment_size>=1000)
  			NS_LOG_INFO("|    "+ to_string(segment_size) +"     |   "+ to_string(throughput) +"   |    "+ to_string(jain_fairness)+"    |");
  		else if(segment_size<100)
  			NS_LOG_INFO("|    "+ to_string(segment_size) +"       |   "+ to_string(throughput) +"   |    "+ to_string(jain_fairness)+"    |");
  		else
  			NS_LOG_INFO("|    "+ to_string(segment_size) +"      |   "+ to_string(throughput) +"   |    "+ to_string(jain_fairness)+"    |");
  			
  		//adding values to dataset
      		dataset.Add (segment_size, throughput);

	}
	NS_LOG_INFO("+-----------------------------------------------+");
	
	//adding dataset and generating output file
  	plot.AddDataset (dataset);

  	ofstream plotFile (plotFileName.c_str());

  	plot.GenerateOutput (plotFile);

  	plotFile.close ();
  	return 0;

}


