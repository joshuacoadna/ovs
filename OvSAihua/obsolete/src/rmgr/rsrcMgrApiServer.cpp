#include "rsrcMgrApiServer.h"
#include "ovsRsrcMgr.h"

rsrcMgrApiServer::rsrcMgrApiServer(uint32 port) :
	ovsTcpServer(port) 
{
     _rsrcMgrApiServerThreadCb = new rsrcMgrApiServerThreadCb(this);
     if (!_rsrcMgrApiServerThreadCb)
     {
         OVS_LOG_ERR(NULL, 
    	 	 OVS_LOG_TYPE_RSRCMGR, 
    	 	"Unable to instantiate thread callback for Resource Manager command server. Possiblly out of memory.");
         exit(-1);
     }

     _seq = 0;

     OVS_LOG_INFO(NULL, 
	 	 OVS_LOG_TYPE_RSRCMGR, 
	 	"Resource Manager command server started on port %d",
	 	port);
}

rsrcMgrApiServer::~rsrcMgrApiServer()
{
    if (_rsrcMgrApiServerThreadCb != NULL)
        delete(_rsrcMgrApiServerThreadCb);

    OVS_LOG_INFO(NULL, OVS_LOG_TYPE_RSRCMGR, "Resource Manager command server stopped.");
}

rsrcMgrApiServer *
rsrcMgrApiServer::instance(void)
{
    if (_instance == NULL)
        _instance = new rsrcMgrApiServer(OVS_RSRC_MGR_TCP_PORT);

    return((rsrcMgrApiServer *)_instance);
}

void
rsrcMgrApiServer::closeClient(int fd)
{
    if (fd <= 0)
        return;

    close(fd);
}

int 
rsrcMgrApiServer::getCmdSeq(void)
{
    if (_seq > 0x1FFFFFFF)
        _seq = 0;
	
    return(++_seq);
}

int
rsrcMgrApiServer::serverReadCb(int arg)
{
    int sock;
	
    sock = ovsTcpServer::serverReadCb(arg);
    if (sock <= 0)
        return(sock);

    /* 
      * Create new client read thread.
      */
    OVS_THR_ADD_READ(_rsrcMgrApiServerThreadCb, arg, sock);
  
    return(sock);
}

void 
rsrcMgrApiServer::clientReadCb(int arg, int fd)
{
    int readlen, bodylen;
    ovsPktHdr hdr;
    ovsINetworkBuffer ibuffer;

    if (fd <= 0)
    {
        OVS_LOG_ERR(NULL, OVS_LOG_TYPE_SOCKET, "clientReadCb(): invalid client socket %s", strerror (errno));
        return;
    }
  
    /*
      * Receive packet header.
      */
    readlen = read(fd, (void *)&hdr, sizeof(ovsPktHdr));
    if (readlen == 0)
    {
        OVS_LOG_WARN(NULL, OVS_LOG_TYPE_SOCKET, "clientReadCb(): client socket %d closed", fd);

	closeClient(fd);
        return;
    }

    /* 
      * Create repeating client read thread.
      */
    OVS_THR_ADD_READ(_rsrcMgrApiServerThreadCb, 0, fd);

    if (readlen != sizeof(ovsPktHdr)) 
    {
        OVS_LOG_ERR(NULL, OVS_LOG_TYPE_SOCKET, "clientReadCb(): read failed: %s", strerror (errno));
        return;
    }

    /*
      * Receive packet body.
      */
    bodylen = ntohs(hdr.length) - sizeof(ovsPktHdr);
    if ( bodylen > 0)
    {
        readlen = read(fd, (void*)(ibuffer.getWriteBuffer()), bodylen);
        if (readlen != bodylen) 
        {
            OVS_LOG_ERR(NULL, OVS_LOG_TYPE_SOCKET, "clientReadCb(): read failed: %s", strerror (errno));
            return;
        }

        ibuffer.setWriteLength(readlen);
    }
    else
	ibuffer.setWriteLength(0);


    /*
      * Process packet. Send response back to client.
      */
    switch (hdr.type)
    {
        case OVS_MSG_CLI_CMD:
	processCliCmd(hdr, ibuffer, fd);
        break;
		
        case OVS_MSG_RMGR_USER_SUBSCRIBE:
        processReadUserSubscribe(hdr, ibuffer, fd);
        break;

        case OVS_MSG_RMGR_USER_UNSUBSCRIBE:
        processReadUserUnsubscribe(hdr, ibuffer, fd);
        break;
        
        default:
        
        OVS_LOG_ERR(NULL, 
            OVS_LOG_TYPE_RSRCMGR, 
            "processReadCb(): unknown API request");
		
        break;
    
    }
}

void
rsrcMgrApiServer::processReadUserSubscribe(ovsPktHdr& hdr, ovsINetworkBuffer& ibuffer, int fd)
{
    int n;
    int ret;
    ovsONetworkBuffer obuffer, obuffdata;
    string userName;

    ovsRsrcMgr *svr = ovsRsrcMgr::instance();

    /*
      * read parameters from input buffer.
      */
    rsrcMgrUser_t user;
    ibuffer >> userName;
    strcpy(user.name, userName.c_str());
	
    ibuffer >> user.flags;
    ibuffer >> user.traceLevel;

    uint16 callbacks = 0;
    ibuffer >> callbacks;

    /*
      * call resource manager backend server.
      */
    rmptr_t pUserHandle = NULL;
    ret = svr->rsrcMgrUserSubscribe(user, callbacks, &pUserHandle);

    if (ret != OVS_RET_OK)
    {
          OVS_LOG_WARN(NULL, 
                             OVS_LOG_TYPE_RSRCMGR, 
                             "ovsRsrcMgr::rsrcMgrUserSubscribe() failed. Error code %d.", ret);
    }
 
    /*
      * write result to output buffer.
      */
    obuffdata << ret;
    obuffdata << (uint32)(pUserHandle);
    
	
    hdr.length = sizeof(hdr) + obuffdata.getUsedSize();

    obuffer << hdr;
    obuffer << obuffdata;

    /*
      * send result back to client.
      */
    n = write(fd, obuffer.data(), obuffer.getUsedSize());
      
    if (n <= 0) 
    {
       OVS_LOG_WARN(NULL, 
                          OVS_LOG_TYPE_RSRCMGR, 
                          "write() in rsrcMgrApiServer::processReadUserSubscribe() failed.");
    }
}

void
rsrcMgrApiServer::processReadUserUnsubscribe(ovsPktHdr& hdr, ovsINetworkBuffer& ibuffer, int fd)
{
    int n;
    int ret;
    ovsONetworkBuffer obuffer, obuffdata;
    string userName;

    ovsRsrcMgr *svr = ovsRsrcMgr::instance();

    /*
      * read parameters from input buffer.
      */
    uint32 userHandle;
    ibuffer >> userHandle;

    /*
      * call resource manager backend server.
      */
    rmptr_t pUserHandle = (rmptr_t)userHandle;
    if (!pUserHandle)
        return;
	
    ret = svr->rsrcMgrUserUnsubscribe(pUserHandle);

    if (ret != OVS_RET_OK)
    {
          OVS_LOG_WARN(NULL, 
                             OVS_LOG_TYPE_RSRCMGR, 
                             "ovsRsrcMgr::rsrcMgrUserUnsubscribe() failed. Error code %d.", ret);
    }
 
    /*
      * write result to output buffer.
      */
    obuffdata << ret;
    
	
    hdr.length = sizeof(hdr) + obuffdata.getUsedSize();

    obuffer << hdr;
    obuffer << obuffdata;

    /*
      * send result back to client.
      */
    n = write(fd, obuffer.data(), obuffer.getUsedSize());
      
    if (n <= 0) 
    {
       OVS_LOG_WARN(NULL, 
                          OVS_LOG_TYPE_RSRCMGR, 
                          "write() in rsrcMgrApiServer::processReadUserUnsubscribe() failed.");
    }
}

void
rsrcMgrApiServer::processCliCmd(ovsPktHdr & hdr,ovsINetworkBuffer & ibuffer,int fd)
{
    int vtyfd, nodetype, cmdtype;
    //int ret;

    if (hdr.type != OVS_MSG_CLI_CMD)
    {
        OVS_LOG_ERR(NULL, OVS_LOG_TYPE_CLISERVER, "incorrect CLI message type. Expect %d received %d", 
			        OVS_MSG_CLI_CMD, (int)hdr.type);
        return;
    }

    /*
      * read parameters from input buffer.
      */
    readCmdInfo(ibuffer, vtyfd, nodetype, cmdtype);

    /*
      * Process packet. Send response back to client.
      */
    switch (cmdtype)
    {
        case CLI_CMD_SHOW_RESOURCES:
        processShowResources(hdr, ibuffer, fd, vtyfd, nodetype, cmdtype);
        break;

        default:
        break;
    }

}

int
rsrcMgrApiServer::processShowResources(ovsPktHdr& hdr, ovsINetworkBuffer& ibuffer, int fd, int vtyfd, int nodetype, int cmdtype)
{
    int n;
    ovsONetworkBuffer obuffer, obuffdata;
	
    /*
      * Call resource manager
      */
    ovsRsrcMgr *rmgr = ovsRsrcMgr::instance();
    if (rmgr == NULL)
    {
       OVS_LOG_ERR(NULL, 
                          OVS_LOG_TYPE_RSRCMGR, 
                          "rsrcMgrApiServer::processShowResources(): unable to get resource manager instance.");
	   
        return(OVS_RET_ERROR);
    }

    writeCmdInfo(obuffdata, vtyfd, nodetype, cmdtype);

    rmgr->print(obuffdata);

    hdr.length = sizeof(hdr) + obuffdata.getUsedSize();
	
    obuffer << hdr;
    obuffer << obuffdata;

    /*
      * send result back to client.
      */
    n = write(fd, obuffer.data(), obuffer.getUsedSize());
      
    if (n <= 0) 
    {
       OVS_LOG_WARN(NULL, 
                          OVS_LOG_TYPE_RSRCMGR, 
                          "rsrcMgrApiServer::write() failed.");
    }
    
    return(OVS_RET_OK);
	
}


void 
rsrcMgrApiServer::cmdResponse(int seq, const char* str)
{
    int n;
    ovsONetworkBuffer obuffer, obufdata;
    rsrcMgrApiResponse_t tmp, *ptr;
    ovsPktHdr hdr;
	
    if (str == NULL || seq <= 0)
    {
         OVS_LOG_ERR(NULL, 
                            OVS_LOG_TYPE_RSRCMGR, 
                            "rsrcMgrApiServer::cmdResponse(): invalid response string");
  	   
          return;
    }

    tmp.seq = seq;
    if ((ptr = _apiRespList.find(tmp)) == NULL)
    {
         OVS_LOG_ERR(NULL, 
                            OVS_LOG_TYPE_RSRCMGR, 
                            "rsrcMgrApiServer::cmdResponse(): unable to find response sequence %d."
                            "Reponse ignored.",
                            seq);
  	   
          return;
    }

    writeCmdInfo(obufdata, ptr->vtyfd, ptr->nodetype, ptr->cmdtype);
    obufdata << str;

    hdr = ptr->hdr;
    hdr.length = sizeof(hdr) + obufdata.getUsedSize();
	
    obuffer << hdr;
    obuffer << obufdata;

    /*
      * send result back to client.
      */
    n = write(ptr->fd, obuffer.data(), obuffer.getUsedSize());
      
    if (n <= 0) 
    {
       OVS_LOG_WARN(NULL, 
                          OVS_LOG_TYPE_RSRCMGR, 
                          "rsrcMgrApiServer write() failed.");
    }

    _apiRespList.remove(tmp);
    
}

void 
rsrcMgrApiServer::readCmdInfo(ovsINetworkBuffer& ibuffer, int& vtyfd, int& nodetype, int& cmdtype)
{
    ibuffer >> vtyfd;
    ibuffer >> nodetype;
    ibuffer >> cmdtype;
}

void 
rsrcMgrApiServer::writeCmdInfo(ovsONetworkBuffer& obuffer, int vtyfd, int nodetype, int cmdtype)
{
    obuffer << vtyfd;
    obuffer << nodetype;
    obuffer << cmdtype;
}

void
rsrcMgrApiServer::WriteResponse(ovsPktHdr& hdr, int ret, int vtyfd, int nodetype, int cmdtype, int fd)
{
    int n;
    ovsONetworkBuffer obuffer, obufdata;

    if (vtyfd == 0)
        return;
	
    writeCmdInfo(obufdata, vtyfd, nodetype, cmdtype);
    obufdata << ret;
	
    hdr.length = sizeof(hdr) + obufdata.getUsedSize();
	
    obuffer << hdr;
    obuffer << obufdata;

    /*
      * send result back to client.
      */
    n = write(fd, obuffer.data(), obuffer.getUsedSize());
      
    if (n <= 0) 
    {
       OVS_LOG_WARN(NULL, 
                          OVS_LOG_TYPE_RSRCMGR, 
                          "rsrcMgrApiServer write() failed.");
    }

}

int 
rsrcMgrApiServer::rsrcMgrUserSubscribe(rsrcMgrUser_t& userConfig, uint16 callbacks, uint32& userHandle)
{
     /* call rsrcMgrUser.function() */
	 
     return(OVS_RET_OK);
}


