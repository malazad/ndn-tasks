#ifndef NDN_PEERPRODUCER_H
#define NDN_PEERPRODUCER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ndn-app.hpp"
#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/nstime.h"
#include "ns3/ptr.h"

namespace ns3 {
namespace ndn {


class PeerProducer : public App {
public:
  static TypeId
  GetTypeId(void);

  PeerProducer();

  virtual void
  OnInterest(shared_ptr<const Interest> interest);

protected:
  virtual void
  StartApplication();

  virtual void
  StopApplication();

private:
  Name m_prefix;
  Name m_postfix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;

  uint32_t m_signature;
  Name m_keyLocator;
  std::string m_Parameters;
  bool m_isDataReceived;
  Name m_peerName;
};

} // namespace ndn
} // namespace ns3

#endif // NDN_PEERPRODUCER_H
