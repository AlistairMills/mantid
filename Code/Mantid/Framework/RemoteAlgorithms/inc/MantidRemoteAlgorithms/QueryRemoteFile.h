#ifndef QUERYREMOTEFILE_H_
#define QUERYREMOTEFILE_H_

#include "MantidAPI/Algorithm.h"

namespace Mantid {
namespace RemoteAlgorithms {

class QueryRemoteFile : public Mantid::API::Algorithm
{
public:
  /// (Empty) Constructor
  QueryRemoteFile() : Mantid::API::Algorithm() {}
  /// Virtual destructor
  virtual ~QueryRemoteFile() {}
  /// Algorithm's name
  virtual const std::string name() const { return "QueryRemoteFile"; }
  /// Algorithm's version
  virtual int version() const { return (1); }
  /// Algorithm's category for identification
  virtual const std::string category() const { return "Remote"; }

private:
  /// Initialisation code
  void initDocs();
  void init();
  ///Execution code
  void exec();

};

} // end namespace RemoteAlgorithms
} // end namespace Mantid
#endif /*QUERYREMOTEFILE_H_*/
