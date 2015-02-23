#ifndef _RSRC_MGR_API_SERVER_H_
#define _RSRC_MGR_API_SERVER_H_

#include "ovsTcpServer.h"
#include "ovsLists.h"

typedef struct rsrcMgrApiResponse_t
{
    int seq;
    ovsPktHdr hdr;
    int fd;
    int vtyfd;
    int nodetype;
    int cmdtype;
    bool operator==(rsrcMgrApiResponse_t& d){ return (seq == d.seq);}
    bool operator!=(rsrcMgrApiResponse_t& d){ return (!(*this == d));}
    rsrcMgrApiResponse_t& operator=(const rsrcMgrApiResponse_t& p)
    {
	seq = p.seq;
      hdr = p.hdr;
      fd = p.fd;
	vtyfd = p.vtyfd;
       nodetype = p.nodetype;
	cmdtype = p.cmdtype;
	
    	return(*this);
    }
}rsrcMgrApiResponse_t;
typedef ovsValueList<rsrcMgrApiResponse_t> rsrcMgrApiResponseList;

class rsrcMgrApiServerThreadCb;
class rsrcMgrApiServer : public ovsTcpServer
{
public:
	
    rsrcMgrApiServer(uint32 port);
    ~rsrcMgrApiServer();

    static rsrcMgrApiServer* instance();
	
    /* 
      * virtual funovsions derived from ovsTcpServer.
      */
    int serverReadCb(int arg);
    void clientReadCb(int arg, int fd);
    void closeClient(int fd);
    void cmdResponse(int seq, const char* str);

    rsrcMgrApiServerThreadCb *_rsrcMgrApiServerThreadCb;

    /* API calls */
    int rsrcMgrUserSubscribe(rsrcMgrUser_t& userConfig, uint16 callbacks, uint32& userHandle);

private:

    void processReadUserSubscribe(ovsPktHdr& hdr, ovsINetworkBuffer& ibuffer, int fd);
    void processReadUserUnsubscribe(ovsPktHdr& hdr, ovsINetworkBuffer& ibuffer, int fd);
	
    void processCliCmd(ovsPktHdr& hdr, ovsINetworkBuffer& ibuffer, int fd);
    void readCmdInfo(ovsINetworkBuffer& ibuffer, int& vtyfd, int& nodetype, int& cmdtype);
    void writeCmdInfo(ovsONetworkBuffer& obuffer, int vtyfd, int nodetype, int cmdtype);
    void WriteResponse(ovsPktHdr& hdr, int ret, int vtyfd, int nodetype, int cmdtype, int fd);

    int processShowResources(ovsPktHdr& hdr, ovsINetworkBuffer& ibuffer, int fd, int vtyfd, int nodetype, int cmdtype);

    int getCmdSeq(void);

    rsrcMgrApiResponseList _apiRespList;
    int _seq;
};

class rsrcMgrApiServerThreadCb : public ovsThreadCb
{

public:
  ~rsrcMgrApiServerThreadCb() {}	
  rsrcMgrApiServerThreadCb(rsrcMgrApiServer* svr) :
  	_svr(svr) {}
	
  /* 
    * virtual funovsions derived from ovsThreadCb.
    */
  void readCb(int arg, int fd)  { _svr->clientReadCb(arg, fd); }

private:
  rsrcMgrApiServer* _svr;	

};

#endif /* _RSRC_MGR_API_SERVER_H_ */





