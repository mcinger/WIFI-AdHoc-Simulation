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
 */

// Authors:
//Mingyang Zhou; Boyuan Zhang

#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/propagation-module.h"
#include "ns3/trace-helper.h"
#include "ns3/wifi-module.h"
#include <string>
#include <vector>

using namespace ns3;


struct Results {
        double PCBR;
        double averageThroughput;
};

uint32_t receivedPackets = 0;
uint32_t CBRPackets = 0;
uint32_t accessPointPacket = 0;
uint32_t accessPointPacketReceived = 0;

//Initialize data rate and propagation loss model.
std::string phyMode("DsssRate1Mbps");

//Choose from one of the following model.
std::string propagation("ns3::FriisPropagationLossModel");


//std::string propagation("ns3::TwoRayGroundPropagationLossModel");


//std::string propagation("ns3::LogDistancePropagationLossModel");


//Choose the random speed for each node.
std::string mobilitySpeed("ns3::UniformRandomVariable[Min=1.0|Max=12.0]");

//Set the area size.
double distance = 250;

//Set iteration times
int times = 30;

//Set the packet size.
uint32_t packetSize = 597;

//Set the total number of packets per node.
uint32_t numPackets = 20000;

//Set interval between transmitting packets to satisify the fixed 200 sending packets rate.
double interval = 0.005;

//Set the number of nodes as 50, 100 or 200.
uint32_t numNodes = 200;

//Count number for AP.
uint32_t acessPoint = 0;

//This is the stop time for simulation.
uint32_t totalTime = 150;

void acessPointPacket(std::string context, const Ptr<const Packet> p) {
        accessPointPacket++;
}

void acessPointPacketReceived(std::string context, const Ptr<const Packet> p) {
        accessPointPacketReceived++;
}

void TransmittedPackets(std::string context,
                                  const Ptr<const Packet> p) {
        // add counter for UDP packet Transmitted
        std::string packetSerialized = p->ToString();
        std::string udpHeaderString = "UdpHeader";
        if (packetSerialized.find(udpHeaderString) != std::string::npos) {
                CBRPackets++;
        }
        receivedPackets++;
}


Results start(void) {
        CBRPackets = 0.0;
        receivedPackets = 0.0;

        Results results;
        results.averageThroughput = 0.0;
        results.PCBR = 0.0;

        NodeContainer n;
        n.Create(numNodes);

        //Use wifiHelper to set the wifi standard as 802.11b.
        WifiHelper wifi;
        wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

        //Use YansWifiPhyHelper to set up the PHY layer.
        YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
        wifiPhy.Set("TxGain", DoubleValue(10));
        wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

        YansWifiChannelHelper wifiChannel;
        wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
        wifiChannel.AddPropagationLoss(propagation);
        wifiPhy.SetChannel(wifiChannel.Create());

        //Set up MAC layer.
        NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
        wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                                     StringValue(phyMode), "ControlMode",
                                     StringValue(phyMode));
        //SetType as adhoc.
        wifiMac.SetType("ns3::AdhocWifiMac");
        NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, n);

        MobilityHelper mobility;
        //Set AP at the center of the area.
        Ptr<ListPositionAllocator> positionAP = CreateObject<ListPositionAllocator>();
        positionAP->Add(Vector(distance / 2, distance / 2, 0));
        mobility.SetPositionAllocator(positionAP);
        mobility.Install(n.Get(acessPoint));

        Ptr<RandomRectanglePositionAllocator> randomPosition = CreateObject<RandomRectanglePositionAllocator>();
        
        //Set initial positions for other nodes using randomRectangle model.
        randomPosition->SetAttribute("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=250.0]"));
        randomPosition->SetAttribute("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=250.0]"));

        mobility.SetPositionAllocator(randomPosition);

        std::cout << "AP Node is set as (125, 125), other nodes are initialized at x = UniformRandomVariable[Min=0.0|Max=250.0] and y = UniformRandomVariable[Min=0.0|Max=250.0] \n";
        std::cout << "Other nodes are moving within x=[0, 250], y=[0,250], AP node is fixed. \n";

        mobility.SetMobilityModel(
                "ns3::RandomWalk2dMobilityModel",
                "Speed", StringValue(mobilitySpeed), "Bounds",
                RectangleValue(Rectangle(0, distance, 0, distance)));

        for (uint32_t i = 0; i < numNodes; ++i) {
                if (i != acessPoint) {
                        mobility.Install(n.Get(i));
                }
        }

        InternetStackHelper internet;
        internet.Install(n);

        Ipv4AddressHelper ipv4;
        ipv4.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer i = ipv4.Assign(devices);

        uint16_t port = 135;
        UdpServerHelper server(port);
        ApplicationContainer apCon = server.Install(n.Get(acessPoint));
        apCon.Start(Seconds(1.0));
        apCon.Stop(Seconds(totalTime));

        UdpClientHelper client(i.GetAddress(acessPoint), port);
        client.SetAttribute("MaxPackets", UintegerValue(numPackets));
        client.SetAttribute("Interval", TimeValue(Seconds(interval)));
        client.SetAttribute("PacketSize", UintegerValue(packetSize));

        for (uint32_t i = 0; i < numNodes; ++i) {
                if (i != acessPoint) {
                        ApplicationContainer app = client.Install(n.Get(i));
                        app.Start(Seconds(5.0));
                        app.Stop(Seconds(totalTime));
                }
        }


        AsciiTraceHelper ascii;
        wifiPhy.EnableAsciiAll(ascii.CreateFileStream("adhoc.tr"));

        std::ostringstream sinkNodePath_tx;
        sinkNodePath_tx << "/NodeList/" << acessPoint
                        << "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyTxBegin";
        Config::Connect(sinkNodePath_tx.str(),
                        MakeCallback(&acessPointPacket));

        std::ostringstream sinkNodePath_rx;
        sinkNodePath_rx << "/NodeList/" << acessPoint
                        << "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyRxEnd";
        Config::Connect(sinkNodePath_rx.str(), MakeCallback(&acessPointPacketReceived));

        for (uint32_t i = 0; i < numNodes; ++i) {
                if (i != acessPoint) {
                        std::ostringstream sourceNodePath_tx;
                        sourceNodePath_tx << "/NodeList/" << i
                                          << "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyTxBegin";
                        Config::Connect(sourceNodePath_tx.str(),
                                        MakeCallback(&TransmittedPackets));
                }
        }

        for (uint32_t i = 0; i < numNodes; ++i) {
                if (i != acessPoint) {
                        std::ostringstream sourceNodePath_rx;
                        sourceNodePath_rx << "/NodeList/" << i
                                          << "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyRxEnd";
                        Config::Connect(sourceNodePath_rx.str(),
                                        MakeCallback(&TransmittedPackets));
                }
        }

        std::cout << "-----------------Simulation begins-------------------" << std::endl;

        FlowMonitorHelper flowHelper;
        Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

        Simulator::Stop(Seconds(totalTime));
        Simulator::Run();


        monitor->CheckForLostPackets();
        Ptr<Ipv4FlowClassifier> classifier =
                DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
        std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

        uint32_t counterPackets = 0;
        double sum = 0;

        for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator it =
                     stats.begin();
                     it != stats.end(); it++) {
                counterPackets = counterPackets + it->second.txPackets;
                sum += it->second.rxBytes * 8.0 /
                       (it->second.timeLastRxPacket.GetSeconds() -
                       it->second.timeFirstTxPacket.GetSeconds()) /
                       1024;
        }


        std::cout << "Received total packets: " << receivedPackets
                  << std::endl;
        std::cout << "Received CBR packets: " <<CBRPackets
                  << std::endl;


        std::cout << "PCBR percentage is: "<<CBRPackets <<" / " <<
        receivedPackets << " = " << CBRPackets * 1.0 / receivedPackets * 100
        << "%" << std::endl;


        double aveThroughput = sum / numNodes;

        std::cout << "Average throughput is: " << aveThroughput << " kbps" << std::endl;
        
        std::cout << std::endl;

        Simulator::Destroy();

        results.averageThroughput = aveThroughput;
        results.PCBR =
        CBRPackets * 1.0 / receivedPackets;
        return results;
}

int main(int argc, char *argv[]) {
//Array to store PCBR.
double arr[times] = {};

double throughput;
                for (int i = 0; i < times; i++) {
                        Results results;

                       std::cout << "-------------------This is the " << i+1 << " time.----------------------"<< std::endl;
                                 
                                  std::cout<< "We choose propagation model: " << propagation<< std::endl;
                                  std::cout<< "The area size is: " << distance << "m x " << distance<<"m. "<< std::endl;
                                  std::cout<<"Total nodes: "<< numNodes << ". "<< std::endl;
                                  std::cout<<"Mobility speed of other node is: " << mobilitySpeed << std::endl;
                        results = start();
                        //Obtain the results for further calculation.
                        arr[i] = results.PCBR;
                        throughput = results.averageThroughput; 
                }

        double sum = 0.0;
        
        for(int i = 0; i < times; i++){
                sum += arr[i];
        }
        sum /= times;


        float sd1 = 0.0;
        for(int i = 0; i < times; i++){
                sd1 += (arr[i] - sum) * (arr[i] - sum);
        }
        sd1 /= times;
        sd1 = pow(sd1, 0.5);

        std::cout<<"Number of times: "<<times<<std::endl;
        std::cout<<"Average PCBR: "<<sum<<std::endl;
        std::cout<<"Standard Deviation of PCBR: "<<sd1<<std::endl;
        std::cout<<std::endl;
        std::cout<<"Average throughput: "<<throughput<<std::endl;

}
