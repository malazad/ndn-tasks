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
  ndn::StrategyChoiceHelper::InstallAll("/file", "/localhost/nfd/strategy/multicast");

  // Installing applications

  // node 0 =  A
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerACbr");
  consumerHelper.SetPrefix("/prefix/node/metadata");
  consumerHelper.SetAttribute("StartSeq", StringValue("0"));
  consumerHelper.SetAttribute("Frequency", StringValue("3"));
  //consumerHelper.SetAttribute("MaxSeq",StringValue("3"));
  auto appsAconsumer = consumerHelper.Install(nodes.Get(0));
  appsAconsumer.Stop(Seconds(2.0));

  ndn::AppHelper producerAHelper("ns3::ndn::ProducerA");
  producerAHelper.SetPrefix("/file");
  //producerAHelper.SetAttribute("ProducerName", StringValue("/A"));
  producerAHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerAHelper.Install(nodes.Get(0));

  //node 1 =  B
  ndn::AppHelper producerBHelper("ns3::ndn::Producer");
  producerBHelper.SetPrefix("/prefix/node");
  producerBHelper.SetAttribute("ProducerName", StringValue("/B"));
  //producerBHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerBHelper.Install(nodes.Get(1));

  ndn::AppHelper consumerBHelper("ns3::ndn::ConsumerCbr");
  consumerBHelper.SetPrefix("/prefix/node/");
  consumerBHelper.SetAttribute("ConsumerName", StringValue("/B"));
  consumerBHelper.SetAttribute("Frequency", StringValue("5"));
  auto appsBconsumer = consumerBHelper.Install(nodes.Get(1));
  appsBconsumer.Stop(Seconds(0));
  appsBconsumer.Start(Seconds(2.0));


  // node 2 =  C
  ndn::AppHelper producerCHelper("ns3::ndn::Producer");
  producerCHelper.SetPrefix("/prefix/node");
  producerCHelper.SetAttribute("ProducerName", StringValue("/C"));
  //producerCHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerCHelper.Install(nodes.Get(2));


  ndn::AppHelper consumerCHelper("ns3::ndn::ConsumerCbr");
  consumerCHelper.SetPrefix("/prefix/node/");
  consumerCHelper.SetAttribute("ConsumerName", StringValue("/C"));
  consumerCHelper.SetAttribute("Frequency", StringValue("5"));
  auto appsCconsumer = consumerCHelper.Install(nodes.Get(2));
  appsCconsumer.Stop(Seconds(0));
  appsCconsumer.Start(Seconds(2.0));


  // node 3 =  D
  ndn::AppHelper producerDHelper("ns3::ndn::Producer");
  producerDHelper.SetPrefix("/prefix/node");
  producerDHelper.SetAttribute("ProducerName", StringValue("/D"));
  //producerDHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerDHelper.Install(nodes.Get(3));

  ndn::AppHelper consumerDHelper("ns3::ndn::ConsumerCbr");
  consumerDHelper.SetPrefix("/prefix/node/");
  consumerDHelper.SetAttribute("ConsumerName", StringValue("/D"));
  consumerDHelper.SetAttribute("Frequency", StringValue("5"));
  auto appsDconsumer = consumerDHelper.Install(nodes.Get(3));
  appsDconsumer.Stop(Seconds(0));
  appsDconsumer.Start(Seconds(2.0));

  Simulator::Stop(Seconds(15.0));

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
