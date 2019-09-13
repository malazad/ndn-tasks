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

#include "consumerB.hpp"
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


NS_LOG_COMPONENT_DEFINE("ndn.ConsumerB");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ConsumerB);

TypeId
ConsumerB::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ConsumerB")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddAttribute("StartSeq", "Initial sequence number", IntegerValue(0),
                    MakeIntegerAccessor(&ConsumerB::m_seq), MakeIntegerChecker<int32_t>())

      .AddAttribute("Prefix", "Name of the Interest", StringValue("/"),
                    MakeNameAccessor(&ConsumerB::m_interestName), MakeNameChecker())
      .AddAttribute("LifeTime", "LifeTime for interest packet", StringValue("2s"),
                    MakeTimeAccessor(&ConsumerB::m_interestLifeTime), MakeTimeChecker())

      .AddAttribute("RetxTimer",
                    "Timeout defining how frequent retransmission timeouts should be checked",
                    StringValue("50ms"),
                    MakeTimeAccessor(&ConsumerB::GetRetxTimer, &ConsumerB::SetRetxTimer),
                    MakeTimeChecker())

      .AddTraceSource("LastRetransmittedInterestDataDelay",
                      "Delay between last retransmitted Interest and received Data",
                      MakeTraceSourceAccessor(&ConsumerB::m_lastRetransmittedInterestDataDelay),
                      "ns3::ndn::ConsumerB::LastRetransmittedInterestDataDelayCallback")

      .AddTraceSource("FirstInterestDataDelay",
                      "Delay between first transmitted Interest and received Data",
                      MakeTraceSourceAccessor(&ConsumerB::m_firstInterestDataDelay),
                      "ns3::ndn::ConsumerB::FirstInterestDataDelayCallback");

  return tid;
}

ConsumerB::ConsumerB()
  : m_rand(CreateObject<UniformRandomVariable>())
  , m_seq(0)
  , m_seqMax(0) // don't request anything
  , m_retxCount(0)
  , m_sendSync(false)
  , m_pushDataPacket(false)
{
  NS_LOG_FUNCTION_NOARGS();

  m_rtt = CreateObject<RttMeanDeviation>();
}

void
ConsumerB::SetRetxTimer(Time retxTimer)
{
  m_retxTimer = retxTimer;
  if (m_retxEvent.IsRunning()) {
    // m_retxEvent.Cancel (); // cancel any scheduled cleanup events
    Simulator::Remove(m_retxEvent); // slower, but better for memory
  }

  // schedule even with new timeout
  m_retxEvent = Simulator::Schedule(m_retxTimer, &ConsumerB::CheckRetxTimeout, this);
}

Time
ConsumerB::GetRetxTimer() const
{
  return m_retxTimer;
}

void
ConsumerB::CheckRetxTimeout()
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

  m_retxEvent = Simulator::Schedule(m_retxTimer, &ConsumerB::CheckRetxTimeout, this);
}

// Application Methods
void
ConsumerB::StartApplication() // Called at time specified by Start
{
  NS_LOG_FUNCTION_NOARGS();

  // do base stuff
  App::StartApplication();

  ScheduleNextPacket();
}

void
ConsumerB::StopApplication() // Called at time specified by Stop
{
  NS_LOG_FUNCTION_NOARGS();

  // cancel periodic packet generation
  Simulator::Cancel(m_sendEvent);

  // cleanup base stuff
  App::StopApplication();
}

void
ConsumerB::SendPacket()
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
    seq = m_seq;
    //seq = m_seq++;
  }

  //
  shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);

  nameWithSequence->appendSequenceNumber(seq);
  //

  shared_ptr<Interest> interest = make_shared<Interest>();
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

  if(m_sendSync)
  {

	  shared_ptr<Name> nameSyncWithSequence = make_shared<Name>("/prefix/sync/local");

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
	  const char* buf = m_DataPacket.c_str();
	  size_t buf_size = m_DataPacket.length();
	  interestPushData->setParameters(make_shared< ::ndn::Buffer>(buf,buf_size));


	  NS_LOG_INFO("> Interest for " << *namePushDataWithSequence);

	  WillSendOutInterest(seq);

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
ConsumerB::OnData(shared_ptr<const Data> data)
{
  if (!m_active)
    return;

  App::OnData(data); // tracing inside

  NS_LOG_FUNCTION(this << data);

  uint32_t seq = data->getName().at(-1).toSequenceNumber();
  Name dataName = data->getName();
  NS_LOG_INFO("< DATA for " << dataName);





  if(dataName.getSubName(0,3).equals("/prefix/sync/local"))
  {
	  Block contentBlock = data->getContent();
	  m_DataPacket = std::string((char*)contentBlock.value());
	  NS_LOG_INFO("< Data content is : " << m_DataPacket);
	  m_pushDataPacket = true;
  }
  else
  {
	  NS_LOG_INFO("The retxCount is = "<< m_retxCount);
	  m_retxCount++;
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
}

void
ConsumerB::OnNack(shared_ptr<const lp::Nack> nack)
{
  /// tracing inside
  App::OnNack(nack);

  NS_LOG_INFO("NACK received for: " << nack->getInterest().getName()
              << ", reason: " << nack->getReason());
}

void
ConsumerB::OnTimeout(uint32_t sequenceNumber)
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
ConsumerB::WillSendOutInterest(uint32_t sequenceNumber)
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
