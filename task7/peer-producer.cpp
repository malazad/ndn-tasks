#include "peer-producer.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>

NS_LOG_COMPONENT_DEFINE("ndn.PeerProducer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(PeerProducer);

TypeId
PeerProducer::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::PeerProducer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<PeerProducer>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&PeerProducer::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&PeerProducer::m_postfix), MakeNameChecker())
	  .AddAttribute("PeerName", "Name of the Peer", StringValue("/"),
		              MakeNameAccessor(&PeerProducer::m_peerName), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&PeerProducer::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&PeerProducer::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&PeerProducer::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&PeerProducer::m_keyLocator), MakeNameChecker());
  return tid;
}

PeerProducer::PeerProducer()
	: m_isDataReceived(false)
{
  NS_LOG_FUNCTION_NOARGS();
}


void
PeerProducer::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();
  m_prefix.append(m_peerName);
  FibHelper::AddRoute(GetNode(),m_prefix , m_face, 0);
}

void
PeerProducer::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
PeerProducer::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest);

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;

  Name dataName(interest->getName());


  Name localhost = "/prefix/peer";
  localhost.append(m_peerName);
  localhost.append("local_sync");


  if(dataName.getSubName(0,4).equals(localhost) && m_isDataReceived) // Sending data to the node consumer app
  {

	   m_isDataReceived = false;
	   auto data = make_shared<Data>();
	   data->setName(dataName);
	   data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

	   const char* buff = m_Parameters.c_str();
	   size_t buff_size = m_Parameters.length();
	   data->setContent(make_shared< ::ndn::Buffer>(buff,buff_size));

	   m_Parameters = "";
	   Signature signature;
	   SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

	   if (m_keyLocator.size() > 0) {
	     signatureInfo.setKeyLocator(m_keyLocator);
	   }

	   signature.setInfo(signatureInfo);
	   signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

	   data->setSignature(signature);

	   NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());

	   // to create real wire encoding
	   data->wireEncode();

	   m_transmittedDatas(data, this, m_face);
	   m_appLink->onReceiveData(*data);

	   NS_LOG_INFO("Data is sent to Consumer "<< m_peerName<< ".");
  }

  else if(dataName.getSubName(0,3).equals(localhost.getSubName(0,3))) //Receiving data from Peer A
  {
	   Block parametersBlock = interest->getParameters();
	   m_Parameters = std::string((char*)parametersBlock.value()).substr(0,33);
	   NS_LOG_INFO("Parameters from Producer A = "<< m_Parameters);
	   m_isDataReceived = true;

  }

}

} // namespace ndn
} // namespace ns3
