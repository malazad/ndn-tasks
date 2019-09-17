#ifndef NDN_PRODUCERA_H
#define NDN_PRODUCERA_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ndn-app.hpp"
#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/nstime.h"
#include "ns3/ptr.h"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * @brief A simple Interest-sink applia simple Interest-sink application
 *
 * A simple Interest-sink applia simple Interest-sink application,
 * which replying every incoming Interest with Data packet with a specified
 * size and name same as in Interest.cation, which replying every incoming Interest
 * with Data packet with a specified size and name same as in Interest.
 */
class ProducerA : public App {
public:
  static TypeId
  GetTypeId(void);

  ProducerA();

  // inherited from NdnApp
  virtual void
  OnInterest(shared_ptr<const Interest> interest);

protected:
  // inherited from Application base class.
  virtual void
  StartApplication();

  virtual void
  StopApplication();

private:
  Name m_prefix;
  Name m_postfix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;
  //int m_sameInterestCount;
  Name m_previousInterestName[255];
  uint32_t m_sameInterestCount[255];
  bool m_isSyncedNeeded[255];

  uint32_t m_signature;
  Name m_keyLocator;

  //std::string m_previousInterestName[255];

};

} // namespace ndn
} // namespace ns3

#endif // NDN_PRODUCERA_H
