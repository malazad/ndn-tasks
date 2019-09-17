

#include "peer-consumer-cbr.hpp"
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

NS_LOG_COMPONENT_DEFINE("ndn.PeerConsumerCbr");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(PeerConsumerCbr);

TypeId
PeerConsumerCbr::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::PeerConsumerCbr")
      .SetGroupName("Ndn")
      .SetParent<PeerConsumer>()
      .AddConstructor<PeerConsumerCbr>()

      .AddAttribute("Frequency", "Frequency of interest packets", StringValue("1.0"),
                    MakeDoubleAccessor(&PeerConsumerCbr::m_frequency), MakeDoubleChecker<double>())

      .AddAttribute("Randomize",
                    "Type of send time randomization: none (default), uniform, exponential",
                    StringValue("none"),
                    MakeStringAccessor(&PeerConsumerCbr::SetRandomize, &PeerConsumerCbr::GetRandomize),
                    MakeStringChecker())

      .AddAttribute("MaxSeq", "Maximum sequence number to request",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&PeerConsumerCbr::m_seqMax), MakeIntegerChecker<uint32_t>())

    ;

  return tid;
}

PeerConsumerCbr::PeerConsumerCbr()
  : m_frequency(1.0)
  , m_firstTime(true)
{
  NS_LOG_FUNCTION_NOARGS();
  m_seqMax = std::numeric_limits<uint32_t>::max();
}

PeerConsumerCbr::~PeerConsumerCbr()
{
}

void
PeerConsumerCbr::ScheduleNextPacket()
{

	NS_LOG_FUNCTION_NOARGS();

  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &PeerConsumer::SendPacket, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning() && m_isValidPeer)
    m_sendEvent = Simulator::Schedule( Seconds(1.0),&PeerConsumer::SendPacket, this);
}

void
PeerConsumerCbr::SetRandomize(const std::string& value)
{
  if (value == "uniform") {
    m_random = CreateObject<UniformRandomVariable>();
    m_random->SetAttribute("Min", DoubleValue(0.0));
    m_random->SetAttribute("Max", DoubleValue(2 * 1.0 / m_frequency));
  }
  else if (value == "exponential") {
    m_random = CreateObject<ExponentialRandomVariable>();
    m_random->SetAttribute("Mean", DoubleValue(1.0 / m_frequency));
    m_random->SetAttribute("Bound", DoubleValue(50 * 1.0 / m_frequency));
  }
  else
    m_random = 0;

  m_randomType = value;
}

std::string
PeerConsumerCbr::GetRandomize() const
{
  return m_randomType;
}

} // namespace ndn
} // namespace ns3
