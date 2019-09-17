#include "producerA.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>

NS_LOG_COMPONENT_DEFINE("ndn.ProducerA");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ProducerA);

TypeId
ProducerA::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ProducerA")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<ProducerA>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&ProducerA::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&ProducerA::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&ProducerA::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&ProducerA::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&ProducerA::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&ProducerA::m_keyLocator), MakeNameChecker());
  return tid;
}

ProducerA::ProducerA()
	//: m_sameInterestCount(1)
	//, m_previousInterestName("")
	//, m_isSyncedNeeded(false)
{
  NS_LOG_FUNCTION_NOARGS();
}

// inherited from Application base class.
void
ProducerA::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
}

void
ProducerA::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
ProducerA::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;


  Name dataName(interest->getName());
  //auto intendedProducer  = dataName.getSubName(2,1);

  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  if(dataName.getSubName(0,2).equals("/prefix/metadata"))
  {
	   Block parametersBlock = interest->getParameters();
	   std::string temp = std::string((char*)parametersBlock.value());
	   NS_LOG_INFO("temp = "<< temp );
	   //std::string peerKey = temp.substr(4,5);
	   uint32_t peerKey = std::strtoul(temp.substr(4,5).c_str(),nullptr,10);
	   NS_LOG_INFO("peerKey = "<< peerKey);
	   if((peerKey > 10000) && (peerKey < 20000))
	   {
		   NS_LOG_INFO("Valid peer.");
		   uint32_t start = ((peerKey%1000)-1)*10;
		   uint32_t end = start + 9;

		   std::stringstream temp_content;
		   if(end < 10)
		   {
			   temp_content << "start=00"<< start << "end=00"<< end;
		   }
		   else if(end > 9 && end < 100)
		   {
			   temp_content << "start=0"<< start << "end=0"<< end;
		   }
		   else
			   temp_content << "start="<< start << "end="<< end;

		   std::string content = temp_content.str();
		   const char* buff = content.c_str();
		   size_t buff_size = content.length();
		   data->setContent(make_shared< ::ndn::Buffer>(buff,buff_size));


	   }

	   else
	   {
		   NS_LOG_INFO("invalid peer.");
		   std::string content = "Invalid Peer!!!";
		   const char* buff = content.c_str();
		   size_t buff_size = content.length();
		   data->setContent(make_shared< ::ndn::Buffer>(buff,buff_size));

	   }
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

  }

  else if(dataName.getSubName(0,3).equals("/prefix/file/sync"))
   {
 	  std::string content = "AllSynced";

 	  for(int i=0;i<256;i++)
 	  {
 		  if(m_isSyncedNeeded[i])
 		  {
 			  m_isSyncedNeeded[i] = false;
 			  content = m_previousInterestName[i].getSubName(2,2).toUri();
 			  content.insert(0,"/prefix/peer");
 			  break;
 		  }
 	  }

 	  const char* buff = content.c_str();
   	  size_t buff_size = content.length();
   	  data->setContent(make_shared< ::ndn::Buffer>(buff,buff_size));



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
   }

  else if(dataName.getSubName(0,2).equals("/prefix/file"))
  {
	  uint32_t file_number = std::strtoul(dataName.getSubName(3,2).toUri().substr(1,3).c_str(), nullptr,10);
	  NS_LOG_INFO("File Number is = "<< file_number);

	  //Name firstRequestCheck ==;
	  if(!m_isSyncedNeeded[file_number])
	  {
		  	  if(m_previousInterestName[file_number].equals("/"))
		  	  {
		  		  m_previousInterestName[file_number] = dataName;
		  		  m_sameInterestCount[file_number]++;
		  	  }
		  	  else if(m_previousInterestName[file_number].equals(dataName))
		  	  	  m_sameInterestCount[file_number]++;
		  	  if(m_sameInterestCount[file_number] == 3)
		  	  {
		  	  	  m_isSyncedNeeded[file_number] = true;
		  	  	  //m_sameInterestCount[file_number] = 1;
		  	  	  //m_previousInterestName = "";  						// need to understand
		  	  	  //return;
		  	  }

		  	  std::stringstream temp_content;
		  	  if(file_number <10)
		  		  temp_content << "Original data packet for file 00" << file_number;
		  	  else if(file_number >9 && file_number<100)
		  		temp_content << "Original data packet for file 0" << file_number;
		  	  else
		  		temp_content << "Original data packet for file " << file_number;
		  	  std::string content = temp_content.str();
		  	  const char* buff = content.c_str();
		  	  size_t buff_size = content.length();
		  	  data->setContent(make_shared< ::ndn::Buffer>(buff,buff_size));

		  	  NS_LOG_INFO("m_previousInterestName  = "<< m_previousInterestName[10] << ";  m_sameInterestCount = " << m_sameInterestCount[10]<<";  m_isSyncedNeeded = "<< m_isSyncedNeeded[10]);

	  }


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

  }







  //old code from here. need to modify for scaling

  /*
  if(dataName.getSubName(2,1).equals("/sync"))
  {
	  std::string payloadString;
	  if(m_isSyncedNeeded)
	  {
		  payloadString = "1";
		  m_isSyncedNeeded = false;
	  }
	  else
		  payloadString = "0";

	  const char* buff = payloadString.c_str();
	  size_t buff_size = payloadString.length();
	  data->setContent(make_shared< ::ndn::Buffer>(buff,buff_size));
	  //if(m_sameInterestCount == 1)
	  //	  m_isSyncedNeeded = false;
  }

  else
  {
	  if(m_previousInterestName.equals(""))
	  	  m_previousInterestName = dataName;
	  else if(m_previousInterestName.equals(dataName))
	  	  m_sameInterestCount++;
	  if(m_sameInterestCount == 3)
	  {
	  	  m_isSyncedNeeded = true;
	  	  m_sameInterestCount = 1;
	  	  m_previousInterestName = "";
	  	  return;
	  }

	  data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));

  }


	*/




}

} // namespace ndn
} // namespace ns3
