#include "peer-consumer.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"

#include "utils/ndn-ns3-packet-tag.hpp"
#include "utils/ndn-rtt-mean-deviation.hpp"

#include <ndn-cxx/lp/tags.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>


NS_LOG_COMPONENT_DEFINE("ndn.PeerConsumer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(PeerConsumer);

TypeId
PeerConsumer::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::PeerConsumer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddAttribute("StartSeq", "Initial sequence number", IntegerValue(0),
                    MakeIntegerAccessor(&PeerConsumer::m_seq), MakeIntegerChecker<int32_t>())
	  .AddAttribute("PeerKey", "Peer validation key", IntegerValue(0),
		            MakeIntegerAccessor(&PeerConsumer::m_peerKey), MakeIntegerChecker<int32_t>())

      .AddAttribute("Prefix", "Name of the Interest", StringValue("/"),
                    MakeNameAccessor(&PeerConsumer::m_interestName), MakeNameChecker())
	  .AddAttribute("PeerName", "Name of the Peer", StringValue("/"),
              	    MakeNameAccessor(&PeerConsumer::m_peerName), MakeNameChecker())
      .AddAttribute("LifeTime", "LifeTime for interest packet", StringValue("2s"),
                    MakeTimeAccessor(&PeerConsumer::m_interestLifeTime), MakeTimeChecker())

      .AddAttribute("RetxTimer",
                    "Timeout defining how frequent retransmission timeouts should be checked",
                    StringValue("50ms"),
                    MakeTimeAccessor(&PeerConsumer::GetRetxTimer, &PeerConsumer::SetRetxTimer),
                    MakeTimeChecker())

      .AddTraceSource("LastRetransmittedInterestDataDelay",
                      "Delay between last retransmitted Interest and received Data",
                      MakeTraceSourceAccessor(&PeerConsumer::m_lastRetransmittedInterestDataDelay),
                      "ns3::ndn::PeerConsumer::LastRetransmittedInterestDataDelayCallback")

      .AddTraceSource("FirstInterestDataDelay",
                      "Delay between first transmitted Interest and received Data",
                      MakeTraceSourceAccessor(&PeerConsumer::m_firstInterestDataDelay),
                      "ns3::ndn::PeerConsumer::FirstInterestDataDelayCallback");

  return tid;
}

PeerConsumer::PeerConsumer()
  : m_rand(CreateObject<UniformRandomVariable>())
  , m_seq(0)
  , m_seqMax(0) // don't request anything
  , m_retxCount(0)
  , m_sendSync(false)
  , m_pushDataPacket(false)
  , m_getMetaData(false)
  , m_start(0)
  , m_end(0)
  , m_isValidPeer(true)
{
  NS_LOG_FUNCTION_NOARGS();

  m_rtt = CreateObject<RttMeanDeviation>();
}

void
PeerConsumer::SetRetxTimer(Time retxTimer)
{
  m_retxTimer = retxTimer;
  if (m_retxEvent.IsRunning()) {
    // m_retxEvent.Cancel (); // cancel any scheduled cleanup events
    Simulator::Remove(m_retxEvent); // slower, but better for memory
  }

  // schedule even with new timeout
  m_retxEvent = Simulator::Schedule(m_retxTimer, &PeerConsumer::CheckRetxTimeout, this);
}

Time
PeerConsumer::GetRetxTimer() const
{
  return m_retxTimer;
}

void
PeerConsumer::CheckRetxTimeout()
{
  Time now = Simulator::Now();

  Time rto = m_rtt->RetransmitTimeout();
  // NS_LOG_DEBUG ("Current RTO: " << rto.ToDouble (Time::S) << "s");

  while (!m_seqTimeouts.empty()) {
    SeqTimeoutsContainer::index<i_timestamp>::type::iterator entry =
      m_seqTimeouts.get<i_timestamp>().begin();
    if (entry->time + rto <= now) // timeout expired?
    {
      uint32_t seqNo = entry->seq;
      m_seqTimeouts.get<i_timestamp>().erase(entry);
      OnTimeout(seqNo);
    }
    else
      break; // nothing else to do. All later packets need not be retransmitted
  }

  m_retxEvent = Simulator::Schedule(m_retxTimer, &PeerConsumer::CheckRetxTimeout, this);
}

// Application Methods
void
PeerConsumer::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  // do base stuff
  App::StartApplication();

  ScheduleNextPacket();
}

void
PeerConsumer::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  // cancel periodic packet generation
  Simulator::Cancel(m_sendEvent);

  // cleanup base stuff
  App::StopApplication();
}

void
PeerConsumer::SendPacket()
{
  if (!m_active)
    return;

  NS_LOG_FUNCTION_NOARGS();
  uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid

  while (m_retxSeqs.size()) {
    seq = *m_retxSeqs.begin();
    m_retxSeqs.erase(m_retxSeqs.begin());
    break;
  }

  if (seq == std::numeric_limits<uint32_t>::max()) {
    if (m_seqMax != std::numeric_limits<uint32_t>::max()) {
      if (m_seq >= m_seqMax) {
        return; // we are totally done
      }
    }
    seq = m_currentFileNumber;
    //seq = m_seq++;
  }

  //
  shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);

  shared_ptr<Interest> interest = make_shared<Interest>();

  if(!m_getMetaData)
  {
	  nameWithSequence->append("metadata");
	  std::stringstream temp;
	  temp << m_peerKey;
	  std::string key = temp.str();
	  key.insert(0,"key=");
	  const char* buf = key.c_str();
	  size_t buf_size = key.length();
	  interest->setParameters(make_shared< ::ndn::Buffer>(buf,buf_size));
	  nameWithSequence->append(m_peerName);
	  nameWithSequence->appendSequenceNumber(m_currentFileNumber);


	  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
	  interest->setName(*nameWithSequence);
	  interest->setCanBePrefix(false);
	  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
	  interest->setInterestLifetime(interestLifeTime);

	  NS_LOG_INFO("> Interest for " << *nameWithSequence);

	  WillSendOutInterest(seq);

	  m_transmittedInterests(interest, this, m_face);
	  m_appLink->onReceiveInterest(*interest);

	  ScheduleNextPacket();

  }
  else if(m_currentFileNumber<=m_end)
  {
	  nameWithSequence->append("file");
	  nameWithSequence->append(m_peerName);









	  //nameWithSequence->appendSequenceNumber(m_currentFileNumber);

	  std::stringstream temp_file_number;

	  if(m_currentFileNumber<10)
	  {
		  temp_file_number << "00"<<m_currentFileNumber;
	  }
	  else if((m_currentFileNumber>9) && (m_currentFileNumber<100))
	  {
		  temp_file_number << "0"<<m_currentFileNumber;
	  }
	  else
	  {
		  temp_file_number <<m_currentFileNumber;
	  }

	  nameWithSequence->append(temp_file_number.str());











	  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
	  interest->setName(*nameWithSequence);
	  interest->setCanBePrefix(false);
	  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
	  interest->setInterestLifetime(interestLifeTime);

	  NS_LOG_INFO("> Interest for " << *nameWithSequence);

	  WillSendOutInterest(seq);

	  m_transmittedInterests(interest, this, m_face);
	  m_appLink->onReceiveInterest(*interest);

	  ScheduleNextPacket();
  }
  /*else
	  return;

  nameWithSequence->append(m_peerName);
  nameWithSequence->appendSequenceNumber(m_currentFileNumber);


  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  interest->setInterestLifetime(interestLifeTime);

  NS_LOG_INFO("> Interest for " << *nameWithSequence);

  WillSendOutInterest(seq);

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  ScheduleNextPacket();


  */

  if(m_sendSync)
  {

	  shared_ptr<Name> nameSyncWithSequence = make_shared<Name>("/prefix/peer");
	  nameSyncWithSequence->append(m_peerName);
	  nameSyncWithSequence->append("local_sync");
	  nameSyncWithSequence->appendSequenceNumber(seq);

	  shared_ptr<Interest> interestSync = make_shared<Interest>();
	  interestSync->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
	  interestSync->setName(*nameSyncWithSequence);
	  interestSync->setCanBePrefix(false);
	  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
	  interestSync->setInterestLifetime(interestLifeTime);

	  NS_LOG_INFO("> Interest for " << *nameSyncWithSequence);

	  WillSendOutInterest(seq);

	  m_transmittedInterests(interestSync, this, m_face);
	  m_appLink->onReceiveInterest(*interestSync);

	  ScheduleNextPacket();
	  m_sendSync = false;
  }

  if(m_pushDataPacket)
  {
	  int selectProxyName = (rand()%3);
	  NS_LOG_INFO("selectProxyName = "<< selectProxyName);
	  std::string SelectedProxyName;
	  if(selectProxyName == 0)
		  SelectedProxyName = "/cnn";
	  else if(selectProxyName == 1)
			  SelectedProxyName = "/bbc";
	  else if(selectProxyName == 2)
		  SelectedProxyName = "/nytimes";

	  shared_ptr<Name> namePushDataWithSequence = make_shared<Name>(SelectedProxyName);
	  namePushDataWithSequence->appendSequenceNumber(seq);

	  shared_ptr<Interest> interestPushData = make_shared<Interest>();
	  interestPushData->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
	  interestPushData->setName(*namePushDataWithSequence);
	  interestPushData->setCanBePrefix(false);
	  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
	  interestPushData->setInterestLifetime(interestLifeTime);

	  //std::string interestParameter = "This is a real data packet";

	  NS_LOG_INFO("Push m_DataPacket = "<< m_DataPacket);
	  const char* buf = m_DataPacket.c_str();
	  size_t buf_size = m_DataPacket.length();
	  interestPushData->setParameters(make_shared< ::ndn::Buffer>(buf,buf_size));


	  NS_LOG_INFO("> Interest for " << *namePushDataWithSequence);

	  WillSendOutInterest(m_currentFileNumber-1); // could be a problem

	  m_transmittedInterests(interestPushData, this, m_face);
	  m_appLink->onReceiveInterest(*interestPushData);

	  ScheduleNextPacket();

	  m_pushDataPacket = false;
	  m_DataPacket = "";

  }
}

///////////////////////////////////////////////////
//          Process incoming packets             //
///////////////////////////////////////////////////

void
PeerConsumer::OnData(shared_ptr<const Data> data)
{
  if (!m_active)
    return;

  App::OnData(data); // tracing inside

  NS_LOG_FUNCTION(this << data);

  //uint32_t seq = data->getName().at(-1).toSequenceNumber();
  Name dataName = data->getName();
  NS_LOG_INFO("< DATA for " << dataName);


  if(dataName.getSubName(0,2).equals("/prefix/metadata"))
  {

	  NS_LOG_INFO("< Get Metadata!!! " );
	  Block content = data->getContent();
	  std::string content2 = std::string((char*)content.value());
	  if(content2.substr(0,5)== "start")
	  {
		  m_start = std::strtoul(content2.substr(6,3).c_str(),nullptr,10);
		  m_end = std::strtoul(content2.substr(13,3).c_str(),nullptr,10);
		  m_currentFileNumber = m_start;
		  NS_LOG_INFO("start =  "<<m_start<<" end = "<< m_end);
		  m_getMetaData = true;

	  }

	  else
	  {
		  NS_LOG_INFO("Content is = " << content2 );
		  NS_LOG_INFO(m_peerName <<" is an Invalid Peer !!!" );
		  m_isValidPeer = false;
	  }

  }

  else if(dataName.getSubName(0,2).equals("/prefix/file") && m_getMetaData)

	  //add consumer name  ?? may be no
	  // check data validity also
	  // somewhere in the code check the file number limit

  {
	  Block contentBlock = data->getContent();
	  std::string content = std::string((char*)contentBlock.value()).substr(0,28);
	  NS_LOG_INFO("< Data content is : " << content);
	  if(content.substr(0,8) == "Original") // should never be true here but must be true later
	  {

		  m_currentFileNumber++;
		  m_pushDataPacket = true;
	  }

	  else
	  {
		  NS_LOG_INFO("The retxCount is = "<< m_retxCount);
		  //m_DataPacket = "";
		  m_retxCount++;
	  }
  }

  else if(dataName.getSubName(0,2).equals("/prefix/peer"))
  {
	  Block contentBlock = data->getContent();
	  m_DataPacket = std::string((char*)contentBlock.value()).substr(0,33);
	  NS_LOG_INFO("< Data content from PeerProducer is : " << m_DataPacket);
	  if(m_DataPacket.substr(0,8) == "Original")
	  {
		  m_pushDataPacket = true;
	  }
  }




  if(m_retxCount==3)
  {
	  m_retxCount = 0;
	  //m_seq++;
	  m_currentFileNumber++;
	  m_sendSync = true;
  }

  /*
  else if(data from /prefix/peer/B/local)
  {
	  //original data packet
	  Block contentBlock = data->getContent();
	  m_DataPacket = std::string((char*)contentBlock.value());
	  NS_LOG_INFO("< Data content is : " << m_DataPacket);
	  m_currentFileNumber++;
	  m_pushDataPacket = true;
  }

  if(m_retxCount==3)
  {
	  m_retxCount = 0;
	  m_seq++;
	  m_sendSync = true;
  }
  int hopCount = 0;
  auto hopCountTag = data->getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  NS_LOG_DEBUG("Hop count: " << hopCount);

  SeqTimeoutsContainer::iterator entry = m_seqLastDelay.find(seq);
  if (entry != m_seqLastDelay.end()) {
    m_lastRetransmittedInterestDataDelay(this, seq, Simulator::Now() - entry->time, hopCount);
  }

  entry = m_seqFullDelay.find(seq);
  if (entry != m_seqFullDelay.end()) {
    m_firstInterestDataDelay(this, seq, Simulator::Now() - entry->time, m_seqRetxCounts[seq], hopCount);
  }

  m_seqRetxCounts.erase(seq);
  m_seqFullDelay.erase(seq);
  m_seqLastDelay.erase(seq);

  m_seqTimeouts.erase(seq);
  m_retxSeqs.erase(seq);

  m_rtt->AckSeq(SequenceNumber32(seq));




  */
}

void
PeerConsumer::OnNack(shared_ptr<const lp::Nack> nack)
{
  /// tracing inside
  App::OnNack(nack);

  NS_LOG_INFO("NACK received for: " << nack->getInterest().getName()
              << ", reason: " << nack->getReason());
}

void
PeerConsumer::OnTimeout(uint32_t sequenceNumber)
{
  NS_LOG_FUNCTION(sequenceNumber);
  // std::cout << Simulator::Now () << ", TO: " << sequenceNumber << ", current RTO: " <<
  // m_rtt->RetransmitTimeout ().ToDouble (Time::S) << "s\n";

  m_rtt->IncreaseMultiplier(); // Double the next RTO
  m_rtt->SentSeq(SequenceNumber32(sequenceNumber),
                 1); // make sure to disable RTT calculation for this sample
  m_retxSeqs.insert(sequenceNumber);
  ScheduleNextPacket();
}

void
PeerConsumer::WillSendOutInterest(uint32_t sequenceNumber)
{
  //NS_LOG_DEBUG("Trying to add " << sequenceNumber << " with " << Simulator::Now() << ". already "
  //                              << m_seqTimeouts.size() << " items");

  m_seqTimeouts.insert(SeqTimeout(sequenceNumber, Simulator::Now()));
  m_seqFullDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  m_seqLastDelay.erase(sequenceNumber);
  m_seqLastDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  m_seqRetxCounts[sequenceNumber]++;

  m_rtt->SentSeq(SequenceNumber32(sequenceNumber), 1);
}

} // namespace ndn
} // namespace ns3
