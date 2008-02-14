#ifndef MANTID_PYTHONAPI_PYTHONINTERFACE_H_
#define MANTID_PYTHONAPI_PYTHONINTERFACE_H_

#include <string>
#include <map>
#include <vector>
#include "MantidKernel/System.h"
#include <boost/shared_ptr.hpp>

namespace Mantid
{

namespace API
{
	class FrameworkManager;
	class IAlgorithm;
}

namespace PythonAPI
{

class DLLExport PythonInterface
{
private:
	Mantid::API::FrameworkManager* fwMgr;

	typedef std::map< const std::string,  boost::shared_ptr<Mantid::API::IAlgorithm> > algMap;
	algMap algs;

public:
	PythonInterface();
	~PythonInterface();

	void InitialiseFrameworkManager();
	bool CreateAlgorithm(const std::string&);
	bool ExecuteAlgorithm(const std::string&);

	int LoadIsisRawFile(const std::string&, const std::string&);
	std::vector<double> GetXData(const std::string&, int const );
	std::vector<double> GetYData(const std::string&, int const );

	unsigned long GetAddressXData(const std::string&, int const );
	unsigned long GetAddressYData(const std::string&, int const );
	

};

}
}

#endif //MANTID_PYTHONAPI_PYTHONINTERFACE_H_

