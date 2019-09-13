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

// ndn-simple.cpp

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
  nodes.Create(8);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(1), nodes.Get(2));
  p2p.Install(nodes.Get(1), nodes.Get(3));
  p2p.Install(nodes.Get(3), nodes.Get(4));
  p2p.Install(nodes.Get(4), nodes.Get(5));
  p2p.Install(nodes.Get(2), nodes.Get(6));
  p2p.Install(nodes.Get(1), nodes.Get(7));


  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Nocache");
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/cnn", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/bbc", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/nytimes", "/localhost/nfd/strategy/multicast");

  // Installing applications

  // node 0 = Peer B
  ndn::AppHelper consumerBHelper("ns3::ndn::ConsumerBCbr");
  consumerBHelper.SetPrefix("/prefix/data");
  consumerBHelper.SetAttribute("Frequency", StringValue("3"));
  auto apps = consumerBHelper.Install(nodes.Get(0));
  //apps.Stop(Seconds(10.0));

  ndn::AppHelper producerBHelper("ns3::ndn::ProducerB");
  producerBHelper.SetPrefix("/prefix/sync");
  //producerCensorHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerBHelper.Install(nodes.Get(0));

  // node 2 =  Censor Producer
  ndn::AppHelper producerCensorHelper("ns3::ndn::ProducerCensor");
  producerCensorHelper.SetPrefix("/prefix/data");
  //producerCensorHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerCensorHelper.Install(nodes.Get(2));

  // node 4 =  Peer A
  ndn::AppHelper producerAHelper("ns3::ndn::ProducerA");
  producerAHelper.SetPrefix("/prefix/data");
  //producerCensorHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerAHelper.Install(nodes.Get(4));

  ndn::AppHelper consumerAHelper("ns3::ndn::ConsumerACbr");
  consumerAHelper.SetPrefix("/prefix");
  //consumerAHelper.SetAttribute("Frequency", StringValue("3"));
  consumerAHelper.Install(nodes.Get(4));

  // node 5 =  Proxy
  ndn::AppHelper producerProxy11Helper("ns3::ndn::ProxyProducer");
  producerProxy11Helper.SetPrefix("/cnn");
  producerProxy11Helper.Install(nodes.Get(5));

  ndn::AppHelper producerProxy12Helper("ns3::ndn::ProxyProducer");
  producerProxy12Helper.SetPrefix("/bbc");
  producerProxy12Helper.Install(nodes.Get(5));

  ndn::AppHelper producerProxy13Helper("ns3::ndn::ProxyProducer");
  producerProxy13Helper.SetPrefix("/nytimes");
  producerProxy13Helper.Install(nodes.Get(5));

  // node 6 =  Proxy
  ndn::AppHelper producerProxy21Helper("ns3::ndn::ProxyProducer");
  producerProxy21Helper.SetPrefix("/cnn");
  producerProxy21Helper.Install(nodes.Get(6));

  ndn::AppHelper producerProxy22Helper("ns3::ndn::ProxyProducer");
  producerProxy22Helper.SetPrefix("/bbc");
  producerProxy22Helper.Install(nodes.Get(6));

  ndn::AppHelper producerProxy23Helper("ns3::ndn::ProxyProducer");
  producerProxy23Helper.SetPrefix("/nytimes");
  producerProxy23Helper.Install(nodes.Get(6));


  // node 6 =  Proxy
  ndn::AppHelper producerProxy31Helper("ns3::ndn::ProxyProducer");
  producerProxy31Helper.SetPrefix("/cnn");
  producerProxy31Helper.Install(nodes.Get(7));

  ndn::AppHelper producerProxy32Helper("ns3::ndn::ProxyProducer");
  producerProxy32Helper.SetPrefix("/bbc");
  producerProxy32Helper.Install(nodes.Get(7));

  ndn::AppHelper producerProxy33Helper("ns3::ndn::ProxyProducer");
  producerProxy33Helper.SetPrefix("/nytimes");
  producerProxy33Helper.Install(nodes.Get(7));
  Simulator::Stop(Seconds(12.0));

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
