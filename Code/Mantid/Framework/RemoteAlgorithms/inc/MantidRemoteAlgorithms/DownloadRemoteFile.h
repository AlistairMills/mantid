#ifndef DOWNLOADREMOTEFILE_H_
#define DOWNLOADREMOTEFILE_H_

#include "MantidAPI/Algorithm.h"

namespace Mantid {
namespace RemoteAlgorithms {

class DownloadRemoteFile : public Mantid::API::Algorithm
{
public:
  /// (Empty) Constructor
  DownloadRemoteFile() : Mantid::API::Algorithm() {}
  /// Virtual destructor
  virtual ~DownloadRemoteFile() {}
  /// Algorithm's name
  virtual const std::string name() const { return "DownloadRemoteFile"; }
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
#endif /*DOWNLOADREMOTEFILE_H_*/
