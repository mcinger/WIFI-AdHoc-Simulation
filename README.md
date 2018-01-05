# WIFI-AdHoc-Simulation
Using ns3 to evaluate performance of WiFi in adhoc mode


-------------------Abstract-------------------

Two experiments were conducted using NS-3 to evaluate the performance of WiFi in adhoc mode under different path loss. Two Systems were set up for each experiment that differ in area size. A node was set at the center of the area and acts as AP (access point). All the remaining nodes were connected directly with it and sent data to it at the same time. Throughput and PCBR (percentage of CBR packets) were used to evaluate the performance of the system. 


-------------------Instruction-------------------

1.Download the adhoc.cc file and put it into scratch directory under ns3.
2.Build ns3 and run the adhoc.cc file.


-------------------System Description-------------------

Topology: 
Two experiments are to be conducted that differ in size of the area under consideration. Consider the two area sizes to be: Area1 - 100 x 100 m2 and Area2 - 250 x 250 m2.
In each experiment, one node is declared as AP and located at the center of the given area and all nodes are connected directly with the AP in 1 hop (hint: use the transmit power sufficiently high to cover the entire given range in one hop from the centrally located AP).
The source-destinations pairs are fixed, with one AP node and the rest of the nodes as source nodes. The AP node is the only destination node and all source nodes transmitting at same time.

Traffic type: 
CBR (Constant Bit Rate) with packet sending rate fixed at 200 packets per second. Let the total simulation time be 150 seconds. Number of packets to be transmitted in total per node – 20000 packets.

Propagation loss model: 
Choose from log distance path loss model, 2-ray ground propagation loss model, and Friis propagation loss model.

Mobility model: 
Random waypoint node mobility model (only for nodes as AP is consider to be in fixed position). In random waypoint model, the nodes move from one waypoint to another independently from each other. Chose a random speed for each node, with which the node would move during simulation and a random waypoint, towards which the nodes would move. After staying for a selfinitiated pause time, the nodes would then choose a random waypoint and would start moving towards that. The chosen speed of mobility is uniformly distributed in [1, 12] m/s.


-------------------Note-------------------

1.We use randomRectangle model to randomly distribute the nodes, it can be changed to other models.
2.The area size, number of nodes, mobility model can be set as need.
3.The ns3 sets a seed to generate random numbers so that you will get the same random numbers if you run it multiple times. Therefore, we could set different seeds and use them to generate random numbers. 

