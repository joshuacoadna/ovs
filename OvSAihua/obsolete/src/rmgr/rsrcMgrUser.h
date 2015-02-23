
#ifndef _RSRC_MGR_USER_H_
#define _RSRC_MGR_USER_H_

#include "ovsCommDef.h"
#include "ovsRsrcMgrApi.h"
#include "ovsRsrcXc.h"

class ovsRsrc;
class ovsRsrcMgr;

class rsrcMgrUser
{
public:
    rsrcMgrUser(ovsRsrcMgr* rmgr, rsrcMgrUser_t* pUserCfg, 
             		uint16 callbacks);

    virtual ~rsrcMgrUser() {}

    void init(ovsRsrcMgr* rmgr, rsrcMgrUser_t* pUserCfg, 
             		uint16 callbacks);

    rmptr_t getUserHandle(void) { return(reinterpret_cast<rmptr_t>(this)); }

    void modifyConfig(rsrcMgrUser_t& pUserCfg);
    rsrcMgrUser_t& getConfig(void) { return(_config); }

    ovsRet_t allocRsrc(rsrcInfo_t& rsrcInfo);
    ovsRsrc* findRsrc(rmptr_t rh);
    ovsRet_t freeRsrc(rmptr_t rh);
    ovsRet_t modifyRsrc(rmptr_t rh, rsrcInfo_t& rsrcInfo);
    ovsRet_t assocRsrc(rmptr_t rh, rmptr_t xc); /* associate a resource to a xc */
    ovsRet_t unassocRsrc(rmptr_t rh, rmptr_t xc);
	
    void notifyRsrcConfig(ovsRsrc& rsrc, rsrcCfgCmd_t cmdType, ovsRet_t retCode);

    void notifyRsrcInfo(uint16 rsrcNum, rsrcInfo_t *pRsrcInfo);

    void notifyRsrcStatus(rsrcStatus_t *pRsrcStatus);

    void dump(bool detail);

    static void* operator new(unsigned int size);
    static void  operator delete(void *ptr, unsigned int size);

    bool operator==(rsrcMgrUser& user);
    bool operator!=(rsrcMgrUser& user);

    const char* getName(void) { return((const char*)_config.name); }

private:

    ovsRsrcMgr           *_rmgr;
    rsrcMgrUser_t _config;
    uint16 _callbacks;

    ovsRsrcList _rsrcList[RSRC_TYPE_NUM];

    rsrcMgrUser() {}
};

typedef ovsPtrList<rsrcMgrUser> rsrcMgrUserList;

#endif /* _RSRC_MGR_USER_H_ */

