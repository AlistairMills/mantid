/* ICatDOIDOIPortBindingProxy.h
   Generated by gSOAP 2.8.15 from ICatDOIService.h

Copyright(C) 2000-2013, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under ONE of the following licenses:
GPL or Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#ifndef ICatDOIDOIPortBindingProxy_H
#define ICatDOIDOIPortBindingProxy_H
#include "ICatDOIH.h"

namespace ICatDOI {

class SOAP_CMAC DOIPortBindingProxy : public soap
{ public:
	/// Endpoint URL of service 'DOIPortBindingProxy' (change as needed)
	const char *soap_endpoint;
	/// Constructor
	DOIPortBindingProxy();
	/// Construct from another engine state
	DOIPortBindingProxy(const struct soap&);
	/// Constructor with endpoint URL
	DOIPortBindingProxy(const char *url);
	/// Constructor with engine input+output mode control
	DOIPortBindingProxy(soap_mode iomode);
	/// Constructor with URL and input+output mode control
	DOIPortBindingProxy(const char *url, soap_mode iomode);
	/// Constructor with engine input and output mode control
	DOIPortBindingProxy(soap_mode imode, soap_mode omode);
	/// Destructor frees deserialized data
	virtual	~DOIPortBindingProxy();
	/// Initializer used by constructors
	virtual	void DOIPortBindingProxy_init(soap_mode imode, soap_mode omode);
	/// Delete all deserialized data (with soap_destroy and soap_end)
	virtual	void destroy();
	/// Delete all deserialized data and reset to default
	virtual	void reset();
	/// Disables and removes SOAP Header from message
	virtual	void soap_noheader();
	/// Get SOAP Header structure (NULL when absent)
	virtual	const SOAP_ENV__Header *soap_header();
	/// Get SOAP Fault structure (NULL when absent)
	virtual	const SOAP_ENV__Fault *soap_fault();
	/// Get SOAP Fault string (NULL when absent)
	virtual	const char *soap_fault_string();
	/// Get SOAP Fault detail as string (NULL when absent)
	virtual	const char *soap_fault_detail();
	/// Close connection (normally automatic, except for send_X ops)
	virtual	int soap_close_socket();
	/// Force close connection (can kill a thread blocked on IO)
	virtual	int soap_force_close_socket();
	/// Print fault
	virtual	void soap_print_fault(FILE*);
#ifndef WITH_LEAN
	/// Print fault to stream
#ifndef WITH_COMPAT
	virtual	void soap_stream_fault(std::ostream&);
#endif

	/// Put fault into buffer
	virtual	char *soap_sprint_fault(char *buf, size_t len);
#endif

	/// Web service operation 'registerInvestigationDOI' (returns error code or SOAP_OK)
	virtual	int registerInvestigationDOI(ICatDOI1__registerInvestigationDOI *ICatDOI1__registerInvestigationDOI_, ICatDOI1__registerInvestigationDOIResponse *ICatDOI1__registerInvestigationDOIResponse_) { return this->registerInvestigationDOI(NULL, NULL, ICatDOI1__registerInvestigationDOI_, ICatDOI1__registerInvestigationDOIResponse_); }
	virtual	int registerInvestigationDOI(const char *endpoint, const char *soap_action, ICatDOI1__registerInvestigationDOI *ICatDOI1__registerInvestigationDOI_, ICatDOI1__registerInvestigationDOIResponse *ICatDOI1__registerInvestigationDOIResponse_);

	/// Web service operation 'registerDatafileDOI' (returns error code or SOAP_OK)
	virtual	int registerDatafileDOI(ICatDOI1__registerDatafileDOI *ICatDOI1__registerDatafileDOI_, ICatDOI1__registerDatafileDOIResponse *ICatDOI1__registerDatafileDOIResponse_) { return this->registerDatafileDOI(NULL, NULL, ICatDOI1__registerDatafileDOI_, ICatDOI1__registerDatafileDOIResponse_); }
	virtual	int registerDatafileDOI(const char *endpoint, const char *soap_action, ICatDOI1__registerDatafileDOI *ICatDOI1__registerDatafileDOI_, ICatDOI1__registerDatafileDOIResponse *ICatDOI1__registerDatafileDOIResponse_);

	/// Web service operation 'registerDatasetDOI' (returns error code or SOAP_OK)
	virtual	int registerDatasetDOI(ICatDOI1__registerDatasetDOI *ICatDOI1__registerDatasetDOI_, ICatDOI1__registerDatasetDOIResponse *ICatDOI1__registerDatasetDOIResponse_) { return this->registerDatasetDOI(NULL, NULL, ICatDOI1__registerDatasetDOI_, ICatDOI1__registerDatasetDOIResponse_); }
	virtual	int registerDatasetDOI(const char *endpoint, const char *soap_action, ICatDOI1__registerDatasetDOI *ICatDOI1__registerDatasetDOI_, ICatDOI1__registerDatasetDOIResponse *ICatDOI1__registerDatasetDOIResponse_);
};

} // namespace ICatDOI

#endif