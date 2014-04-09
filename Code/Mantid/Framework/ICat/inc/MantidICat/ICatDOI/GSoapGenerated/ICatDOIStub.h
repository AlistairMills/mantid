/* ICatDOIStub.h
   Generated by gSOAP 2.8.15 from ICatDOIService.h

Copyright(C) 2000-2013, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under ONE of the following licenses:
GPL or Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#ifndef ICatDOIStub_H
#define ICatDOIStub_H
#include <vector>
#define SOAP_NAMESPACE_OF_ICatDOI2	""
#define SOAP_NAMESPACE_OF_ICatDOI1	"http://webservice.doi.stfc.ac.uk/"
#ifndef WITH_NONAMESPACES
#define WITH_NONAMESPACES
#endif
#ifndef WITH_NOGLOBAL
#define WITH_NOGLOBAL
#endif
#include "MantidICat/GSoap/stdsoap2.h"
#if GSOAP_VERSION != 20815
# error "GSOAP VERSION MISMATCH IN GENERATED CODE: PLEASE REINSTALL PACKAGE"
#endif


namespace ICatDOI {

/******************************************************************************\
 *                                                                            *
 * Enumerations                                                               *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Types with Custom Serializers                                              *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Classes and Structs                                                        *
 *                                                                            *
\******************************************************************************/


#if 0 /* volatile type: do not declare here, declared elsewhere */

#endif

#if 0 /* volatile type: do not declare here, declared elsewhere */

#endif

#ifndef SOAP_TYPE_ICatDOI_ICatDOI1__registerDatasetDOI
#define SOAP_TYPE_ICatDOI_ICatDOI1__registerDatasetDOI (8)
/* ICatDOI1:registerDatasetDOI */
class SOAP_CMAC ICatDOI1__registerDatasetDOI
{
public:
	std::string *arg0;	/* optional element of type xsd:string */
	LONG64 arg1;	/* required element of type xsd:long */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 8; } /* = unique id SOAP_TYPE_ICatDOI_ICatDOI1__registerDatasetDOI */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         ICatDOI1__registerDatasetDOI() { ICatDOI1__registerDatasetDOI::soap_default(NULL); }
	virtual ~ICatDOI1__registerDatasetDOI() { }
};
#endif

#ifndef SOAP_TYPE_ICatDOI_ICatDOI1__registerDatasetDOIResponse
#define SOAP_TYPE_ICatDOI_ICatDOI1__registerDatasetDOIResponse (9)
/* ICatDOI1:registerDatasetDOIResponse */
class SOAP_CMAC ICatDOI1__registerDatasetDOIResponse
{
public:
	std::string *return_;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 9; } /* = unique id SOAP_TYPE_ICatDOI_ICatDOI1__registerDatasetDOIResponse */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         ICatDOI1__registerDatasetDOIResponse() { ICatDOI1__registerDatasetDOIResponse::soap_default(NULL); }
	virtual ~ICatDOI1__registerDatasetDOIResponse() { }
};
#endif

#ifndef SOAP_TYPE_ICatDOI_ICatDOI1__DataNotFoundException
#define SOAP_TYPE_ICatDOI_ICatDOI1__DataNotFoundException (10)
/* ICatDOI1:DataNotFoundException */
class SOAP_CMAC ICatDOI1__DataNotFoundException
{
public:
	std::string *message;	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 10; } /* = unique id SOAP_TYPE_ICatDOI_ICatDOI1__DataNotFoundException */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         ICatDOI1__DataNotFoundException() { ICatDOI1__DataNotFoundException::soap_default(NULL); }
	virtual ~ICatDOI1__DataNotFoundException() { }
};
#endif

#ifndef SOAP_TYPE_ICatDOI_ICatDOI1__DoiException
#define SOAP_TYPE_ICatDOI_ICatDOI1__DoiException (11)
/* ICatDOI1:DoiException */
class SOAP_CMAC ICatDOI1__DoiException
{
public:
	std::string *message;	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 11; } /* = unique id SOAP_TYPE_ICatDOI_ICatDOI1__DoiException */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         ICatDOI1__DoiException() { ICatDOI1__DoiException::soap_default(NULL); }
	virtual ~ICatDOI1__DoiException() { }
};
#endif

#ifndef SOAP_TYPE_ICatDOI_ICatDOI1__InternalException
#define SOAP_TYPE_ICatDOI_ICatDOI1__InternalException (12)
/* ICatDOI1:InternalException */
class SOAP_CMAC ICatDOI1__InternalException
{
public:
	std::string *message;	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 12; } /* = unique id SOAP_TYPE_ICatDOI_ICatDOI1__InternalException */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         ICatDOI1__InternalException() { ICatDOI1__InternalException::soap_default(NULL); }
	virtual ~ICatDOI1__InternalException() { }
};
#endif

#ifndef SOAP_TYPE_ICatDOI_ICatDOI1__ICATException
#define SOAP_TYPE_ICatDOI_ICatDOI1__ICATException (13)
/* ICatDOI1:ICATException */
class SOAP_CMAC ICatDOI1__ICATException
{
public:
	std::string *message;	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 13; } /* = unique id SOAP_TYPE_ICatDOI_ICatDOI1__ICATException */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         ICatDOI1__ICATException() { ICatDOI1__ICATException::soap_default(NULL); }
	virtual ~ICatDOI1__ICATException() { }
};
#endif

#ifndef SOAP_TYPE_ICatDOI_ICatDOI1__registerInvestigationDOI
#define SOAP_TYPE_ICatDOI_ICatDOI1__registerInvestigationDOI (14)
/* ICatDOI1:registerInvestigationDOI */
class SOAP_CMAC ICatDOI1__registerInvestigationDOI
{
public:
	std::string *arg0;	/* optional element of type xsd:string */
	LONG64 arg1;	/* required element of type xsd:long */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 14; } /* = unique id SOAP_TYPE_ICatDOI_ICatDOI1__registerInvestigationDOI */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         ICatDOI1__registerInvestigationDOI() { ICatDOI1__registerInvestigationDOI::soap_default(NULL); }
	virtual ~ICatDOI1__registerInvestigationDOI() { }
};
#endif

#ifndef SOAP_TYPE_ICatDOI_ICatDOI1__registerInvestigationDOIResponse
#define SOAP_TYPE_ICatDOI_ICatDOI1__registerInvestigationDOIResponse (15)
/* ICatDOI1:registerInvestigationDOIResponse */
class SOAP_CMAC ICatDOI1__registerInvestigationDOIResponse
{
public:
	std::string *return_;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 15; } /* = unique id SOAP_TYPE_ICatDOI_ICatDOI1__registerInvestigationDOIResponse */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         ICatDOI1__registerInvestigationDOIResponse() { ICatDOI1__registerInvestigationDOIResponse::soap_default(NULL); }
	virtual ~ICatDOI1__registerInvestigationDOIResponse() { }
};
#endif

#ifndef SOAP_TYPE_ICatDOI_ICatDOI1__registerDatafileDOI
#define SOAP_TYPE_ICatDOI_ICatDOI1__registerDatafileDOI (16)
/* ICatDOI1:registerDatafileDOI */
class SOAP_CMAC ICatDOI1__registerDatafileDOI
{
public:
	std::string *arg0;	/* optional element of type xsd:string */
	LONG64 arg1;	/* required element of type xsd:long */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 16; } /* = unique id SOAP_TYPE_ICatDOI_ICatDOI1__registerDatafileDOI */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         ICatDOI1__registerDatafileDOI() { ICatDOI1__registerDatafileDOI::soap_default(NULL); }
	virtual ~ICatDOI1__registerDatafileDOI() { }
};
#endif

#ifndef SOAP_TYPE_ICatDOI_ICatDOI1__registerDatafileDOIResponse
#define SOAP_TYPE_ICatDOI_ICatDOI1__registerDatafileDOIResponse (17)
/* ICatDOI1:registerDatafileDOIResponse */
class SOAP_CMAC ICatDOI1__registerDatafileDOIResponse
{
public:
	std::string *return_;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 17; } /* = unique id SOAP_TYPE_ICatDOI_ICatDOI1__registerDatafileDOIResponse */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         ICatDOI1__registerDatafileDOIResponse() { ICatDOI1__registerDatafileDOIResponse::soap_default(NULL); }
	virtual ~ICatDOI1__registerDatafileDOIResponse() { }
};
#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_ICatDOI_SOAP_ENV__Detail
#define SOAP_TYPE_ICatDOI_SOAP_ENV__Detail (22)
/* SOAP-ENV:Detail */
struct SOAP_ENV__Detail
{
public:
	char *__any;
	ICatDOI1__DataNotFoundException *ICatDOI1__DataNotFoundException_;	/* optional element of type ICatDOI1:DataNotFoundException */
	ICatDOI1__DoiException *ICatDOI1__DoiException_;	/* optional element of type ICatDOI1:DoiException */
	ICatDOI1__ICATException *ICatDOI1__ICATException_;	/* optional element of type ICatDOI1:ICATException */
	ICatDOI1__InternalException *ICatDOI1__InternalException_;	/* optional element of type ICatDOI1:InternalException */
	int __type;	/* any type of element <fault> (defined below) */
	void *fault;	/* transient */
public:
	int soap_type() const { return 22; } /* = unique id SOAP_TYPE_ICatDOI_SOAP_ENV__Detail */
};
#endif

#endif

#ifndef SOAP_TYPE_ICatDOI___ICatDOI1__registerInvestigationDOI
#define SOAP_TYPE_ICatDOI___ICatDOI1__registerInvestigationDOI (32)
/* Operation wrapper: */
struct __ICatDOI1__registerInvestigationDOI
{
public:
	ICatDOI1__registerInvestigationDOI *ICatDOI1__registerInvestigationDOI_;	/* optional element of type ICatDOI1:registerInvestigationDOI */
public:
	int soap_type() const { return 32; } /* = unique id SOAP_TYPE_ICatDOI___ICatDOI1__registerInvestigationDOI */
};
#endif

#ifndef SOAP_TYPE_ICatDOI___ICatDOI1__registerDatafileDOI
#define SOAP_TYPE_ICatDOI___ICatDOI1__registerDatafileDOI (36)
/* Operation wrapper: */
struct __ICatDOI1__registerDatafileDOI
{
public:
	ICatDOI1__registerDatafileDOI *ICatDOI1__registerDatafileDOI_;	/* optional element of type ICatDOI1:registerDatafileDOI */
public:
	int soap_type() const { return 36; } /* = unique id SOAP_TYPE_ICatDOI___ICatDOI1__registerDatafileDOI */
};
#endif

#ifndef SOAP_TYPE_ICatDOI___ICatDOI1__registerDatasetDOI
#define SOAP_TYPE_ICatDOI___ICatDOI1__registerDatasetDOI (40)
/* Operation wrapper: */
struct __ICatDOI1__registerDatasetDOI
{
public:
	ICatDOI1__registerDatasetDOI *ICatDOI1__registerDatasetDOI_;	/* optional element of type ICatDOI1:registerDatasetDOI */
public:
	int soap_type() const { return 40; } /* = unique id SOAP_TYPE_ICatDOI___ICatDOI1__registerDatasetDOI */
};
#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_ICatDOI_SOAP_ENV__Header
#define SOAP_TYPE_ICatDOI_SOAP_ENV__Header (41)
/* SOAP Header: */
struct SOAP_ENV__Header
{
public:
	int soap_type() const { return 41; } /* = unique id SOAP_TYPE_ICatDOI_SOAP_ENV__Header */
#ifdef WITH_NOEMPTYSTRUCT
private:
	char dummy;	/* dummy member to enable compilation */
#endif
};
#endif

#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_ICatDOI_SOAP_ENV__Code
#define SOAP_TYPE_ICatDOI_SOAP_ENV__Code (42)
/* SOAP Fault Code: */
struct SOAP_ENV__Code
{
public:
	char *SOAP_ENV__Value;	/* optional element of type xsd:QName */
	struct SOAP_ENV__Code *SOAP_ENV__Subcode;	/* optional element of type SOAP-ENV:Code */
public:
	int soap_type() const { return 42; } /* = unique id SOAP_TYPE_ICatDOI_SOAP_ENV__Code */
};
#endif

#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_ICatDOI_SOAP_ENV__Reason
#define SOAP_TYPE_ICatDOI_SOAP_ENV__Reason (44)
/* SOAP-ENV:Reason */
struct SOAP_ENV__Reason
{
public:
	char *SOAP_ENV__Text;	/* optional element of type xsd:string */
public:
	int soap_type() const { return 44; } /* = unique id SOAP_TYPE_ICatDOI_SOAP_ENV__Reason */
};
#endif

#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_ICatDOI_SOAP_ENV__Fault
#define SOAP_TYPE_ICatDOI_SOAP_ENV__Fault (45)
/* SOAP Fault: */
struct SOAP_ENV__Fault
{
public:
	char *faultcode;	/* optional element of type xsd:QName */
	char *faultstring;	/* optional element of type xsd:string */
	char *faultactor;	/* optional element of type xsd:string */
	struct SOAP_ENV__Detail *detail;	/* optional element of type SOAP-ENV:Detail */
	struct SOAP_ENV__Code *SOAP_ENV__Code;	/* optional element of type SOAP-ENV:Code */
	struct SOAP_ENV__Reason *SOAP_ENV__Reason;	/* optional element of type SOAP-ENV:Reason */
	char *SOAP_ENV__Node;	/* optional element of type xsd:string */
	char *SOAP_ENV__Role;	/* optional element of type xsd:string */
	struct SOAP_ENV__Detail *SOAP_ENV__Detail;	/* optional element of type SOAP-ENV:Detail */
public:
	int soap_type() const { return 45; } /* = unique id SOAP_TYPE_ICatDOI_SOAP_ENV__Fault */
};
#endif

#endif

/******************************************************************************\
 *                                                                            *
 * Typedefs                                                                   *
 *                                                                            *
\******************************************************************************/

#ifndef SOAP_TYPE_ICatDOI__QName
#define SOAP_TYPE_ICatDOI__QName (5)
typedef char *_QName;
#endif

#ifndef SOAP_TYPE_ICatDOI__XML
#define SOAP_TYPE_ICatDOI__XML (6)
typedef char *_XML;
#endif


/******************************************************************************\
 *                                                                            *
 * Externals                                                                  *
 *                                                                            *
\******************************************************************************/


} // namespace ICatDOI


#endif

/* End of ICatDOIStub.h */