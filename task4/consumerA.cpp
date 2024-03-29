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

#include "consumerA.hpp"
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
#include "helper/ndn-fib-helper.hpp"

#include <ndn-cxx/lp/tags.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>

#include <string>
NS_LOG_COMPONENT_DEFINE("ndn.ConsumerA");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ConsumerA);

TypeId
ConsumerA::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ConsumerA")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddAttribute("StartSeq", "Initial sequence number", IntegerValue(0),
                    MakeIntegerAccessor(&ConsumerA::m_seq), MakeIntegerChecker<int32_t>())

      .AddAttribute("Prefix", "Name of the Interest", StringValue("/"),
                    MakeNameAccessor(&ConsumerA::m_interestName), MakeNameChecker())
      .AddAttribute("LifeTime", "LifeTime for interest packet", StringValue("2s"),
                    MakeTimeAccessor(&ConsumerA::m_interestLifeTime), MakeTimeChecker())

      .AddAttribute("RetxTimer",
                    "Timeout defining how frequent retransmission timeouts should be checked",
                    StringValue("50ms"),
                    MakeTimeAccessor(&ConsumerA::GetRetxTimer, &ConsumerA::SetRetxTimer),
                    MakeTimeChecker())

      .AddTraceSource("LastRetransmittedInterestDataDelay",
                      "Delay between last retransmitted Interest and received Data",
                      MakeTraceSourceAccessor(&ConsumerA::m_lastRetransmittedInterestDataDelay),
                      "ns3::ndn::ConsumerA::LastRetransmittedInterestDataDelayCallback")

      .AddTraceSource("FirstInterestDataDelay",
                      "Delay between first transmitted Interest and received Data",
                      MakeTraceSourceAccessor(&ConsumerA::m_firstInterestDataDelay),
                      "ns3::ndn::ConsumerA::FirstInterestDataDelayCallback");

  return tid;
}

ConsumerA::ConsumerA()
  : m_rand(CreateObject<UniformRandomVariable>())
  , m_seq(0)
  , m_seqMax(0) // don't request anything
{
  NS_LOG_FUNCTION_NOARGS();

  m_rtt = CreateObject<RttMeanDeviation>();
}

void
ConsumerA::SetRetxTimer(Time retxTimer)
{
  m_retxTimer = retxTimer;
  if (m_retxEvent.IsRunning()) {
    // m_retxEvent.Cancel (); // cancel any scheduled cleanup events
    Simulator::Remove(m_retxEvent); // slower, but better for memory
  }

  // schedule even with new timeout
  m_retxEvent = Simulator::Schedule(m_retxTimer, &ConsumerA::CheckRetxTimeout, this);
}

Time
ConsumerA::GetRetxTimer() const
{
  return m_retxTimer;
}

void
ConsumerA::CheckRetxTimeout()
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

  m_retxEvent = Simulator::Schedule(m_retxTimer, &ConsumerA::CheckRetxTimeout, this);
}

// Application Methods
void
ConsumerA::StartApplication() // Called at time specified by Start
{
  NS_LOG_FUNCTION_NOARGS();

  // do base stuff
  App::StartApplication();
  //FibHelper::AddRoute(GetNode(), "/prefix", m_face, 0);

  ScheduleNextPacket();
}

void
ConsumerA::StopApplication() // Called at time specified by Stop
{
  NS_LOG_FUNCTION_NOARGS();

  // cancel periodic packet generation
  Simulator::Cancel(m_sendEvent);

  // cleanup base stuff
  App::StopApplication();
}

void
ConsumerA::SendPacket()
{
  if (!m_active)
    return;
if(m_seq < 3)
{
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

	    seq = m_seq++;
	  }

	  //
	  shared_ptr<Interest> interest = make_shared<Interest>();
	  shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);
	  std::string interestParameter;

	  NS_LOG_INFO("m_seq = "<< m_seq<<" seq = "<< seq);
	  switch(seq)
			{
	  case 0: nameWithSequence->append("B");
			  interestParameter = "start= 00 end= 39 .";
	  	  	  break;
	  case 1: nameWithSequence->append("C");
			  interestParameter = "start= 40 end= 79 .";
	    	  break;
	  case 2: nameWithSequence->append("D");
			  interestParameter = "start= 80 end= 99 .";
	    	  break;
			}

	  nameWithSequence->appendSequenceNumber(seq);
	  //nameWithSequence->appendSequenceNumber("/D"+std::to_string(seq));
	  NS_LOG_INFO("> Intended peer is :" << *nameWithSequence);
	  // shared_ptr<Interest> interest = make_shared<Interest> ();

	  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
	  interest->setName(*nameWithSequence);
	  interest->setCanBePrefix(false);
	  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
	  interest->setInterestLifetime(interestLifeTime);

	  const char* buf = interestParameter.c_str();
	  size_t buf_size = interestParameter.length();

	  interest->setParameters(make_shared< ::ndn::Buffer>(buf,buf_size));


	  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
	  NS_LOG_INFO("> Interest for " << seq);

	  WillSendOutInterest(seq);

	  m_transmittedInterests(interest, this, m_face);
	  m_appLink->onReceiveInterest(*interest);

	  ScheduleNextPacket();
}

}

///////////////////////////////////////////////////
//          Process incoming packets             //
///////////////////////////////////////////////////

void
ConsumerA::OnData(shared_ptr<const Data> data)
{
  if (!m_active)
    return;

  App::OnData(data); // tracing inside

  NS_LOG_FUNCTION(this << data);

  // NS_LOG_INFO ("Received content object: " << boost::cref(*data));

  // This could be a problem......
  uint32_t seq = data->getName().at(-1).toSequenceNumber();
  NS_LOG_INFO("< DATA for " << seq);

  // Extracting the content
  Block contentBlock = data->getContent();
  std::string content = std::string((char*)contentBlock.value());
  //NS_LOG_INFO("< Data content is : " << content);

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
ConsumerA::OnNack(shared_ptr<const lp::Nack> nack)
{
  /// tracing inside
  App::OnNack(nack);

  NS_LOG_INFO("NACK received for: " << nack->getInterest().getName()
              << ", reason: " << nack->getReason());
}

void
ConsumerA::OnTimeout(uint32_t sequenceNumber)
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
ConsumerA::WillSendOutInterest(uint32_t sequenceNumber)
{
  NS_LOG_DEBUG("Trying to add " << sequenceNumber << " with " << Simulator::Now() << ". already "
                                << m_seqTimeouts.size() << " items");

  m_seqTimeouts.insert(SeqTimeout(sequenceNumber, Simulator::Now()));
  m_seqFullDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  m_seqLastDelay.erase(sequenceNumber);
  m_seqLastDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  m_seqRetxCounts[sequenceNumber]++;

  m_rtt->SentSeq(SequenceNumber32(sequenceNumber), 1);
}

} // namespace ndn
} // namespace ns3
