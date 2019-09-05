/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/



#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {


int
main(int argc, char* argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create(4);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(0), nodes.Get(2));
  p2p.Install(nodes.Get(0), nodes.Get(3));

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");

  // Installing applications

  // node 0 = Consumer A
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix("/prefix/node/D");
  consumerHelper.SetAttribute("Frequency", StringValue("1"));
  auto apps = consumerHelper.Install(nodes.Get(0));
  apps.Stop(Seconds(5.0));

  //node 1 = Producer B
  ndn::AppHelper producerBHelper("ns3::ndn::Producer");
  producerBHelper.SetPrefix("/prefix/node");
  producerBHelper.SetAttribute("ProducerName", StringValue("/B"));
  producerBHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerBHelper.Install(nodes.Get(1));


  // node 2 = Producer C
  ndn::AppHelper producerCHelper("ns3::ndn::Producer");
  producerCHelper.SetPrefix("/prefix/node");
  producerCHelper.SetAttribute("ProducerName", StringValue("/C"));
  producerCHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerCHelper.Install(nodes.Get(2));


  // node 3 = Producer D
  ndn::AppHelper producerDHelper("ns3::ndn::Producer");
  producerDHelper.SetPrefix("/prefix/node");
  producerDHelper.SetAttribute("ProducerName", StringValue("/D"));
  producerDHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerDHelper.Install(nodes.Get(3));

  Simulator::Stop(Seconds(20.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
