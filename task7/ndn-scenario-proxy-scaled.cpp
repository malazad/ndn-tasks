

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
  nodes.Create(40);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(1), nodes.Get(2));
  p2p.Install(nodes.Get(1), nodes.Get(3));
  //p2p.Install(nodes.Get(2), nodes.Get(4));
  p2p.Install(nodes.Get(1), nodes.Get(8));
  p2p.Install(nodes.Get(8), nodes.Get(4));




  p2p.Install(nodes.Get(0), nodes.Get(10));
  p2p.Install(nodes.Get(0), nodes.Get(11));
  p2p.Install(nodes.Get(0), nodes.Get(12));

  p2p.Install(nodes.Get(10), nodes.Get(18));
  p2p.Install(nodes.Get(10), nodes.Get(19));
  p2p.Install(nodes.Get(10), nodes.Get(20));

  p2p.Install(nodes.Get(11), nodes.Get(21));
  p2p.Install(nodes.Get(11), nodes.Get(22));
  p2p.Install(nodes.Get(11), nodes.Get(23));

  p2p.Install(nodes.Get(12), nodes.Get(24));
  p2p.Install(nodes.Get(12), nodes.Get(25));
  p2p.Install(nodes.Get(12), nodes.Get(26));


  p2p.Install(nodes.Get(1), nodes.Get(13));
  p2p.Install(nodes.Get(1), nodes.Get(14));
  p2p.Install(nodes.Get(1), nodes.Get(15));

  p2p.Install(nodes.Get(3), nodes.Get(9));
  p2p.Install(nodes.Get(3), nodes.Get(16));
  p2p.Install(nodes.Get(3), nodes.Get(17));

  p2p.Install(nodes.Get(9), nodes.Get(27));
  p2p.Install(nodes.Get(9), nodes.Get(28));
  p2p.Install(nodes.Get(9), nodes.Get(29));

  p2p.Install(nodes.Get(17), nodes.Get(30));
  p2p.Install(nodes.Get(17), nodes.Get(31));
  p2p.Install(nodes.Get(17), nodes.Get(32));


  //Proxy connections
  p2p.Install(nodes.Get(3), nodes.Get(5));
  p2p.Install(nodes.Get(2), nodes.Get(6));
  p2p.Install(nodes.Get(1), nodes.Get(7));
  p2p.Install(nodes.Get(20), nodes.Get(33));
  p2p.Install(nodes.Get(19), nodes.Get(34));
  p2p.Install(nodes.Get(11), nodes.Get(35));
  p2p.Install(nodes.Get(24), nodes.Get(36));
  p2p.Install(nodes.Get(16), nodes.Get(37));
  p2p.Install(nodes.Get(27), nodes.Get(38));
  p2p.Install(nodes.Get(30), nodes.Get(39));





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

  ndn::AppHelper consumerBHelper("ns3::ndn::PeerConsumerCbr");
  consumerBHelper.SetPrefix("/prefix/");
  consumerBHelper.SetAttribute("PeerKey", StringValue("10001"));
  consumerBHelper.SetAttribute("PeerName", StringValue("B"));
  consumerBHelper.SetAttribute("Frequency", StringValue("3"));
  consumerBHelper.Install(nodes.Get(0));

  ndn::AppHelper producerBHelper("ns3::ndn::PeerProducer");
  producerBHelper.SetPrefix("/prefix/peer");
  producerBHelper.SetAttribute("PeerName", StringValue("B"));
  producerBHelper.Install(nodes.Get(0));

  // node 9 = Peer C
  ndn::AppHelper consumerCHelper("ns3::ndn::PeerConsumerCbr");
  consumerCHelper.SetPrefix("/prefix/");
  consumerCHelper.SetAttribute("PeerKey", StringValue("10002"));
  consumerCHelper.SetAttribute("PeerName", StringValue("C"));
  consumerCHelper.SetAttribute("Frequency", StringValue("3"));
  auto appsC = consumerCHelper.Install(nodes.Get(9));
  //apps.Stop(Seconds(10.0));

  ndn::AppHelper producerCHelper("ns3::ndn::PeerProducer");
  producerCHelper.SetPrefix("/prefix/peer");
  producerCHelper.SetAttribute("PeerName", StringValue("C"));
  //producerCensorHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerCHelper.Install(nodes.Get(9));


  // node 10 = Peer D
  ndn::AppHelper consumerDHelper("ns3::ndn::PeerConsumerCbr");
  consumerDHelper.SetPrefix("/prefix/");
  consumerDHelper.SetAttribute("PeerKey", StringValue("10003"));
  consumerDHelper.SetAttribute("PeerName", StringValue("D"));
  consumerDHelper.SetAttribute("Frequency", StringValue("3"));
  consumerDHelper.Install(nodes.Get(10));

  ndn::AppHelper producerDHelper("ns3::ndn::PeerProducer");
  producerDHelper.SetPrefix("/prefix/peer");
  producerDHelper.SetAttribute("PeerName", StringValue("D"));
  producerDHelper.Install(nodes.Get(10));


  // node 11 = Peer E
  ndn::AppHelper consumerEHelper("ns3::ndn::PeerConsumerCbr");
  consumerEHelper.SetPrefix("/prefix/");
  consumerEHelper.SetAttribute("PeerKey", StringValue("10004"));
  consumerEHelper.SetAttribute("PeerName", StringValue("E"));
  consumerEHelper.SetAttribute("Frequency", StringValue("3"));
  consumerEHelper.Install(nodes.Get(11));

  ndn::AppHelper producerEHelper("ns3::ndn::PeerProducer");
  producerEHelper.SetPrefix("/prefix/peer");
  producerEHelper.SetAttribute("PeerName", StringValue("E"));
  producerEHelper.Install(nodes.Get(11));



  // node 12 = Peer F
  ndn::AppHelper consumerFHelper("ns3::ndn::PeerConsumerCbr");
  consumerFHelper.SetPrefix("/prefix/");
  consumerFHelper.SetAttribute("PeerKey", StringValue("10005"));
  consumerFHelper.SetAttribute("PeerName", StringValue("F"));
  consumerFHelper.SetAttribute("Frequency", StringValue("3"));
  consumerFHelper.Install(nodes.Get(12));

  ndn::AppHelper producerFHelper("ns3::ndn::PeerProducer");
  producerFHelper.SetPrefix("/prefix/peer");
  producerFHelper.SetAttribute("PeerName", StringValue("F"));
  producerFHelper.Install(nodes.Get(12));

  // node 13 = Peer G
  ndn::AppHelper consumerGHelper("ns3::ndn::PeerConsumerCbr");
  consumerGHelper.SetPrefix("/prefix/");
  consumerGHelper.SetAttribute("PeerKey", StringValue("10006"));
  consumerGHelper.SetAttribute("PeerName", StringValue("G"));
  consumerGHelper.SetAttribute("Frequency", StringValue("3"));
  consumerGHelper.Install(nodes.Get(13));

  ndn::AppHelper producerGHelper("ns3::ndn::PeerProducer");
  producerGHelper.SetPrefix("/prefix/peer");
  producerGHelper.SetAttribute("PeerName", StringValue("G"));
  producerGHelper.Install(nodes.Get(13));


  // node 14 = Peer H
  ndn::AppHelper consumerHHelper("ns3::ndn::PeerConsumerCbr");
  consumerHHelper.SetPrefix("/prefix/");
  consumerHHelper.SetAttribute("PeerKey", StringValue("10007"));
  consumerHHelper.SetAttribute("PeerName", StringValue("H"));
  consumerHHelper.SetAttribute("Frequency", StringValue("3"));
  consumerHHelper.Install(nodes.Get(14));

  ndn::AppHelper producerHHelper("ns3::ndn::PeerProducer");
  producerHHelper.SetPrefix("/prefix/peer");
  producerHHelper.SetAttribute("PeerName", StringValue("H"));
  producerHHelper.Install(nodes.Get(14));

  // node 15 = Peer I
  ndn::AppHelper consumerIHelper("ns3::ndn::PeerConsumerCbr");
  consumerIHelper.SetPrefix("/prefix/");
  consumerIHelper.SetAttribute("PeerKey", StringValue("10008"));
  consumerIHelper.SetAttribute("PeerName", StringValue("I"));
  consumerIHelper.SetAttribute("Frequency", StringValue("3"));
  consumerIHelper.Install(nodes.Get(15));

  ndn::AppHelper producerIHelper("ns3::ndn::PeerProducer");
  producerIHelper.SetPrefix("/prefix/peer");
  producerIHelper.SetAttribute("PeerName", StringValue("I"));
  producerIHelper.Install(nodes.Get(15));


  // node 16 = Peer J
  ndn::AppHelper consumerJHelper("ns3::ndn::PeerConsumerCbr");
  consumerJHelper.SetPrefix("/prefix/");
  consumerJHelper.SetAttribute("PeerKey", StringValue("10009"));
  consumerJHelper.SetAttribute("PeerName", StringValue("J"));
  consumerJHelper.SetAttribute("Frequency", StringValue("3"));
  consumerJHelper.Install(nodes.Get(16));

  ndn::AppHelper producerJHelper("ns3::ndn::PeerProducer");
  producerJHelper.SetPrefix("/prefix/peer");
  producerJHelper.SetAttribute("PeerName", StringValue("J"));
  producerJHelper.Install(nodes.Get(16));


  // node 17 = Peer K
  ndn::AppHelper consumerKHelper("ns3::ndn::PeerConsumerCbr");
  consumerKHelper.SetPrefix("/prefix/");
  consumerKHelper.SetAttribute("PeerKey", StringValue("10010"));
  consumerKHelper.SetAttribute("PeerName", StringValue("K"));
  consumerKHelper.SetAttribute("Frequency", StringValue("3"));
  consumerKHelper.Install(nodes.Get(17));

  ndn::AppHelper producerKHelper("ns3::ndn::PeerProducer");
  producerKHelper.SetPrefix("/prefix/peer");
  producerKHelper.SetAttribute("PeerName", StringValue("K"));
  producerKHelper.Install(nodes.Get(17));

  // node 18 = Peer L
  ndn::AppHelper consumerLHelper("ns3::ndn::PeerConsumerCbr");
  consumerLHelper.SetPrefix("/prefix/");
  consumerLHelper.SetAttribute("PeerKey", StringValue("10011"));
  consumerLHelper.SetAttribute("PeerName", StringValue("L"));
  consumerLHelper.SetAttribute("Frequency", StringValue("3"));
  consumerLHelper.Install(nodes.Get(18));

  ndn::AppHelper producerLHelper("ns3::ndn::PeerProducer");
  producerLHelper.SetPrefix("/prefix/peer");
  producerLHelper.SetAttribute("PeerName", StringValue("L"));
  producerLHelper.Install(nodes.Get(18));

  // node 19 = Peer M
  ndn::AppHelper consumerMHelper("ns3::ndn::PeerConsumerCbr");
  consumerMHelper.SetPrefix("/prefix/");
  consumerMHelper.SetAttribute("PeerKey", StringValue("10012"));
  consumerMHelper.SetAttribute("PeerName", StringValue("M"));
  consumerMHelper.SetAttribute("Frequency", StringValue("3"));
  consumerMHelper.Install(nodes.Get(19));

  ndn::AppHelper producerMHelper("ns3::ndn::PeerProducer");
  producerMHelper.SetPrefix("/prefix/peer");
  producerMHelper.SetAttribute("PeerName", StringValue("M"));
  producerMHelper.Install(nodes.Get(19));

  // node 20 = Peer N
  ndn::AppHelper consumerNHelper("ns3::ndn::PeerConsumerCbr");
  consumerNHelper.SetPrefix("/prefix/");
  consumerNHelper.SetAttribute("PeerKey", StringValue("10013"));
  consumerNHelper.SetAttribute("PeerName", StringValue("N"));
  consumerNHelper.SetAttribute("Frequency", StringValue("3"));
  consumerNHelper.Install(nodes.Get(20));

  ndn::AppHelper producerNHelper("ns3::ndn::PeerProducer");
  producerNHelper.SetPrefix("/prefix/peer");
  producerNHelper.SetAttribute("PeerName", StringValue("N"));
  producerNHelper.Install(nodes.Get(20));

  // node 21 = Peer O
  ndn::AppHelper consumerOHelper("ns3::ndn::PeerConsumerCbr");
  consumerOHelper.SetPrefix("/prefix/");
  consumerOHelper.SetAttribute("PeerKey", StringValue("10014"));
  consumerOHelper.SetAttribute("PeerName", StringValue("O"));
  consumerOHelper.SetAttribute("Frequency", StringValue("3"));
  consumerOHelper.Install(nodes.Get(21));

  ndn::AppHelper producerOHelper("ns3::ndn::PeerProducer");
  producerOHelper.SetPrefix("/prefix/peer");
  producerOHelper.SetAttribute("PeerName", StringValue("O"));
  producerOHelper.Install(nodes.Get(21));

  // node 22 = Peer P
  ndn::AppHelper consumerPHelper("ns3::ndn::PeerConsumerCbr");
  consumerPHelper.SetPrefix("/prefix/");
  consumerPHelper.SetAttribute("PeerKey", StringValue("10015"));
  consumerPHelper.SetAttribute("PeerName", StringValue("P"));
  consumerPHelper.SetAttribute("Frequency", StringValue("3"));
  consumerPHelper.Install(nodes.Get(22));

  ndn::AppHelper producerPHelper("ns3::ndn::PeerProducer");
  producerPHelper.SetPrefix("/prefix/peer");
  producerPHelper.SetAttribute("PeerName", StringValue("P"));
  producerPHelper.Install(nodes.Get(22));

  // node 23 = Peer Q
  ndn::AppHelper consumerQHelper("ns3::ndn::PeerConsumerCbr");
  consumerQHelper.SetPrefix("/prefix/");
  consumerQHelper.SetAttribute("PeerKey", StringValue("10016"));
  consumerQHelper.SetAttribute("PeerName", StringValue("Q"));
  consumerQHelper.SetAttribute("Frequency", StringValue("3"));
  consumerQHelper.Install(nodes.Get(23));

  ndn::AppHelper producerQHelper("ns3::ndn::PeerProducer");
  producerQHelper.SetPrefix("/prefix/peer");
  producerQHelper.SetAttribute("PeerName", StringValue("Q"));
  producerQHelper.Install(nodes.Get(23));

  // node 24 = Peer R
  ndn::AppHelper consumerRHelper("ns3::ndn::PeerConsumerCbr");
  consumerRHelper.SetPrefix("/prefix/");
  consumerRHelper.SetAttribute("PeerKey", StringValue("10017"));
  consumerRHelper.SetAttribute("PeerName", StringValue("R"));
  consumerRHelper.SetAttribute("Frequency", StringValue("3"));
  consumerRHelper.Install(nodes.Get(24));

  ndn::AppHelper producerRHelper("ns3::ndn::PeerProducer");
  producerRHelper.SetPrefix("/prefix/peer");
  producerRHelper.SetAttribute("PeerName", StringValue("R"));
  producerRHelper.Install(nodes.Get(24));

  // node 25 = Peer S
  ndn::AppHelper consumerSHelper("ns3::ndn::PeerConsumerCbr");
  consumerSHelper.SetPrefix("/prefix/");
  consumerSHelper.SetAttribute("PeerKey", StringValue("10018"));
  consumerSHelper.SetAttribute("PeerName", StringValue("S"));
  consumerSHelper.SetAttribute("Frequency", StringValue("3"));
  consumerSHelper.Install(nodes.Get(25));

  ndn::AppHelper producerSHelper("ns3::ndn::PeerProducer");
  producerSHelper.SetPrefix("/prefix/peer");
  producerSHelper.SetAttribute("PeerName", StringValue("S"));
  producerSHelper.Install(nodes.Get(25));

  // node 26 = Peer T
  ndn::AppHelper consumerTHelper("ns3::ndn::PeerConsumerCbr");
  consumerTHelper.SetPrefix("/prefix/");
  consumerTHelper.SetAttribute("PeerKey", StringValue("10019"));
  consumerTHelper.SetAttribute("PeerName", StringValue("T"));
  consumerTHelper.SetAttribute("Frequency", StringValue("3"));
  consumerTHelper.Install(nodes.Get(26));

  ndn::AppHelper producerTHelper("ns3::ndn::PeerProducer");
  producerTHelper.SetPrefix("/prefix/peer");
  producerTHelper.SetAttribute("PeerName", StringValue("T"));
  producerTHelper.Install(nodes.Get(26));

  // node 27 = Peer U
  ndn::AppHelper consumerUHelper("ns3::ndn::PeerConsumerCbr");
  consumerUHelper.SetPrefix("/prefix/");
  consumerUHelper.SetAttribute("PeerKey", StringValue("10020"));
  consumerUHelper.SetAttribute("PeerName", StringValue("U"));
  consumerUHelper.SetAttribute("Frequency", StringValue("3"));
  consumerUHelper.Install(nodes.Get(27));

  ndn::AppHelper producerUHelper("ns3::ndn::PeerProducer");
  producerUHelper.SetPrefix("/prefix/peer");
  producerUHelper.SetAttribute("PeerName", StringValue("U"));
  producerUHelper.Install(nodes.Get(27));


  // node 28 = Peer V (Invalid Peer)
  ndn::AppHelper consumerVHelper("ns3::ndn::PeerConsumerCbr");
  consumerVHelper.SetPrefix("/prefix/");
  consumerVHelper.SetAttribute("PeerKey", StringValue("20001"));
  consumerVHelper.SetAttribute("PeerName", StringValue("V"));
  consumerVHelper.SetAttribute("Frequency", StringValue("3"));
  consumerVHelper.Install(nodes.Get(28));

  ndn::AppHelper producerVHelper("ns3::ndn::PeerProducer");
  producerVHelper.SetPrefix("/prefix/peer");
  producerVHelper.SetAttribute("PeerName", StringValue("V"));
  producerVHelper.Install(nodes.Get(28));

  // node 29 = Peer W (Invalid Peer)
  ndn::AppHelper consumerWHelper("ns3::ndn::PeerConsumerCbr");
  consumerWHelper.SetPrefix("/prefix/");
  consumerWHelper.SetAttribute("PeerKey", StringValue("20002"));
  consumerWHelper.SetAttribute("PeerName", StringValue("W"));
  consumerWHelper.SetAttribute("Frequency", StringValue("3"));
  consumerWHelper.Install(nodes.Get(29));

  ndn::AppHelper producerWHelper("ns3::ndn::PeerProducer");
  producerWHelper.SetPrefix("/prefix/peer");
  producerWHelper.SetAttribute("PeerName", StringValue("W"));
  producerWHelper.Install(nodes.Get(29));

  // node 30 = Peer X (Invalid Peer)
  ndn::AppHelper consumerXHelper("ns3::ndn::PeerConsumerCbr");
  consumerXHelper.SetPrefix("/prefix/");
  consumerXHelper.SetAttribute("PeerKey", StringValue("20003"));
  consumerXHelper.SetAttribute("PeerName", StringValue("X"));
  consumerXHelper.SetAttribute("Frequency", StringValue("3"));
  consumerXHelper.Install(nodes.Get(30));

  ndn::AppHelper producerXHelper("ns3::ndn::PeerProducer");
  producerXHelper.SetPrefix("/prefix/peer");
  producerXHelper.SetAttribute("PeerName", StringValue("X"));
  producerXHelper.Install(nodes.Get(30));

  // node 31 = Peer Y (Invalid Peer)
  ndn::AppHelper consumerYHelper("ns3::ndn::PeerConsumerCbr");
  consumerYHelper.SetPrefix("/prefix/");
  consumerYHelper.SetAttribute("PeerKey", StringValue("20004"));
  consumerYHelper.SetAttribute("PeerName", StringValue("Y"));
  consumerYHelper.SetAttribute("Frequency", StringValue("3"));
  consumerYHelper.Install(nodes.Get(31));

  ndn::AppHelper producerYHelper("ns3::ndn::PeerProducer");
  producerYHelper.SetPrefix("/prefix/peer");
  producerYHelper.SetAttribute("PeerName", StringValue("Y"));
  producerYHelper.Install(nodes.Get(31));

  // node 32 = Peer Y (Invalid Peer)
  ndn::AppHelper consumerZHelper("ns3::ndn::PeerConsumerCbr");
  consumerZHelper.SetPrefix("/prefix/");
  consumerZHelper.SetAttribute("PeerKey", StringValue("20005"));
  consumerZHelper.SetAttribute("PeerName", StringValue("Z"));
  consumerZHelper.SetAttribute("Frequency", StringValue("3"));
  consumerZHelper.Install(nodes.Get(32));

  ndn::AppHelper producerZHelper("ns3::ndn::PeerProducer");
  producerZHelper.SetPrefix("/prefix/peer");
  producerZHelper.SetAttribute("PeerName", StringValue("Z"));
  producerZHelper.Install(nodes.Get(32));


  // node 2 =  Censor Producer
  ndn::AppHelper producerCensorHelper("ns3::ndn::ProducerCensor");
  producerCensorHelper.SetPrefix("/prefix/file");
  //producerCensorHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerCensorHelper.Install(nodes.Get(2));

  // node 4 =  Peer A
  ndn::AppHelper producerAMetaDataHelper("ns3::ndn::ProducerA");
  producerAMetaDataHelper.SetPrefix("/prefix/metadata");
  producerAMetaDataHelper.Install(nodes.Get(4));

  ndn::AppHelper producerAFileHelper("ns3::ndn::ProducerA");
  producerAFileHelper.SetPrefix("/prefix/file");
  producerAFileHelper.Install(nodes.Get(4));


  ndn::AppHelper consumerAHelper("ns3::ndn::ConsumerACbr");
  consumerAHelper.SetPrefix("/prefix/file/sync");  // could be problem
  //consumerAHelper.SetAttribute("Frequency", StringValue("3"));
  consumerAHelper.Install(nodes.Get(4));

  // node 5 =  Proxy 1
  ndn::AppHelper producerProxy11Helper("ns3::ndn::ProxyProducer");
  producerProxy11Helper.SetPrefix("/cnn");
  producerProxy11Helper.Install(nodes.Get(5));

  ndn::AppHelper producerProxy12Helper("ns3::ndn::ProxyProducer");
  producerProxy12Helper.SetPrefix("/bbc");
  producerProxy12Helper.Install(nodes.Get(5));

  ndn::AppHelper producerProxy13Helper("ns3::ndn::ProxyProducer");
  producerProxy13Helper.SetPrefix("/nytimes");
  producerProxy13Helper.Install(nodes.Get(5));

  // node 6 =  Proxy 2
  ndn::AppHelper producerProxy21Helper("ns3::ndn::ProxyProducer");
  producerProxy21Helper.SetPrefix("/cnn");
  producerProxy21Helper.Install(nodes.Get(6));

  ndn::AppHelper producerProxy22Helper("ns3::ndn::ProxyProducer");
  producerProxy22Helper.SetPrefix("/bbc");
  producerProxy22Helper.Install(nodes.Get(6));

  ndn::AppHelper producerProxy23Helper("ns3::ndn::ProxyProducer");
  producerProxy23Helper.SetPrefix("/nytimes");
  producerProxy23Helper.Install(nodes.Get(6));


  // node 7 =  Proxy 3
  ndn::AppHelper producerProxy31Helper("ns3::ndn::ProxyProducer");
  producerProxy31Helper.SetPrefix("/cnn");
  producerProxy31Helper.Install(nodes.Get(7));

  ndn::AppHelper producerProxy32Helper("ns3::ndn::ProxyProducer");
  producerProxy32Helper.SetPrefix("/bbc");
  producerProxy32Helper.Install(nodes.Get(7));

  ndn::AppHelper producerProxy33Helper("ns3::ndn::ProxyProducer");
  producerProxy33Helper.SetPrefix("/nytimes");
  producerProxy33Helper.Install(nodes.Get(7));

  // node 33 =  Proxy 4
  ndn::AppHelper producerProxy41Helper("ns3::ndn::ProxyProducer");
  producerProxy41Helper.SetPrefix("/cnn");
  producerProxy41Helper.Install(nodes.Get(33));

  ndn::AppHelper producerProxy42Helper("ns3::ndn::ProxyProducer");
  producerProxy42Helper.SetPrefix("/bbc");
  producerProxy42Helper.Install(nodes.Get(33));

  ndn::AppHelper producerProxy43Helper("ns3::ndn::ProxyProducer");
  producerProxy43Helper.SetPrefix("/nytimes");
  producerProxy43Helper.Install(nodes.Get(33));

  // node 34 =  Proxy 5
  ndn::AppHelper producerProxy51Helper("ns3::ndn::ProxyProducer");
  producerProxy51Helper.SetPrefix("/cnn");
  producerProxy51Helper.Install(nodes.Get(34));

  ndn::AppHelper producerProxy52Helper("ns3::ndn::ProxyProducer");
  producerProxy52Helper.SetPrefix("/bbc");
  producerProxy52Helper.Install(nodes.Get(34));

  ndn::AppHelper producerProxy53Helper("ns3::ndn::ProxyProducer");
  producerProxy53Helper.SetPrefix("/nytimes");
  producerProxy53Helper.Install(nodes.Get(34));


  // node 35 =  Proxy 6
  ndn::AppHelper producerProxy61Helper("ns3::ndn::ProxyProducer");
  producerProxy61Helper.SetPrefix("/cnn");
  producerProxy61Helper.Install(nodes.Get(35));

  ndn::AppHelper producerProxy62Helper("ns3::ndn::ProxyProducer");
  producerProxy62Helper.SetPrefix("/bbc");
  producerProxy62Helper.Install(nodes.Get(35));

  ndn::AppHelper producerProxy63Helper("ns3::ndn::ProxyProducer");
  producerProxy63Helper.SetPrefix("/nytimes");
  producerProxy63Helper.Install(nodes.Get(35));


  // node 36 =  Proxy 7
  ndn::AppHelper producerProxy71Helper("ns3::ndn::ProxyProducer");
  producerProxy71Helper.SetPrefix("/cnn");
  producerProxy71Helper.Install(nodes.Get(36));

  ndn::AppHelper producerProxy72Helper("ns3::ndn::ProxyProducer");
  producerProxy72Helper.SetPrefix("/bbc");
  producerProxy72Helper.Install(nodes.Get(36));

  ndn::AppHelper producerProxy73Helper("ns3::ndn::ProxyProducer");
  producerProxy73Helper.SetPrefix("/nytimes");
  producerProxy73Helper.Install(nodes.Get(36));


  // node 37 =  Proxy 8
  ndn::AppHelper producerProxy81Helper("ns3::ndn::ProxyProducer");
  producerProxy81Helper.SetPrefix("/cnn");
  producerProxy81Helper.Install(nodes.Get(37));

  ndn::AppHelper producerProxy82Helper("ns3::ndn::ProxyProducer");
  producerProxy82Helper.SetPrefix("/bbc");
  producerProxy82Helper.Install(nodes.Get(37));

  ndn::AppHelper producerProxy83Helper("ns3::ndn::ProxyProducer");
  producerProxy83Helper.SetPrefix("/nytimes");
  producerProxy83Helper.Install(nodes.Get(37));

  // node 38 =  Proxy 9
  ndn::AppHelper producerProxy91Helper("ns3::ndn::ProxyProducer");
  producerProxy91Helper.SetPrefix("/cnn");
  producerProxy91Helper.Install(nodes.Get(38));

  ndn::AppHelper producerProxy92Helper("ns3::ndn::ProxyProducer");
  producerProxy92Helper.SetPrefix("/bbc");
  producerProxy92Helper.Install(nodes.Get(38));

  ndn::AppHelper producerProxy93Helper("ns3::ndn::ProxyProducer");
  producerProxy93Helper.SetPrefix("/nytimes");
  producerProxy93Helper.Install(nodes.Get(38));


  // node 39 =  Proxy 10
  ndn::AppHelper producerProxy101Helper("ns3::ndn::ProxyProducer");
  producerProxy101Helper.SetPrefix("/cnn");
  producerProxy101Helper.Install(nodes.Get(39));

  ndn::AppHelper producerProxy102Helper("ns3::ndn::ProxyProducer");
  producerProxy102Helper.SetPrefix("/bbc");
  producerProxy102Helper.Install(nodes.Get(39));

  ndn::AppHelper producerProxy103Helper("ns3::ndn::ProxyProducer");
  producerProxy103Helper.SetPrefix("/nytimes");
  producerProxy103Helper.Install(nodes.Get(39));

  Simulator::Stop(Seconds(40.0));

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
