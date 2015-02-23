#include "rsrcMgrUser.h"
#include "ovsRsrcMgr.h"
#include "ovsLists.h"

rsrcMgrUser::rsrcMgrUser(ovsRsrcMgr *rmgr, 
                        rsrcMgrUser_t* pUserCfg, 
             		uint16 callbacks)
{
    init(rmgr, pUserCfg, callbacks);
}

void
rsrcMgrUser::init(ovsRsrcMgr* rmgr,
                        rsrcMgrUser_t* pUserCfg, 
             		uint16 callbacks)
{
    _rmgr = rmgr;
	
    strcpy(_config.name, pUserCfg->name);
    _config.flags = pUserCfg->flags;
    _config.traceLevel = pUserCfg->traceLevel;

    _callbacks = callbacks;
}


void
rsrcMgrUser::modifyConfig(rsrcMgrUser_t& pUserCfg)
{
}

ovsRet_t 
rsrcMgrUser::allocRsrc(rsrcInfo_t& rsrcInfo)
{
    return(OVS_RET_OK);
}

ovsRsrc*
rsrcMgrUser::findRsrc(rmptr_t rh)
{
    return(NULL);
}

ovsRet_t 
rsrcMgrUser::freeRsrc(rmptr_t rh)
{
    return(OVS_RET_OK);
}

ovsRet_t 
rsrcMgrUser::modifyRsrc(rmptr_t rh, rsrcInfo_t& rsrcInfo)
{
    return(OVS_RET_OK);
}

ovsRet_t 
rsrcMgrUser::assocRsrc(rmptr_t rh, rmptr_t xc)
{
    return(OVS_RET_OK);
}

ovsRet_t 
rsrcMgrUser::unassocRsrc(rmptr_t rh, rmptr_t xc)
{
    return(OVS_RET_OK);
}

void 
rsrcMgrUser::notifyRsrcConfig(ovsRsrc& rsrc, rsrcCfgCmd_t cmdType, ovsRet_t retCode)
{
}

void 
rsrcMgrUser::notifyRsrcInfo(uint16 rsrcNum, rsrcInfo_t *pRsrcInfo)
{
}

void 
rsrcMgrUser::notifyRsrcStatus(rsrcStatus_t *pRsrcStatus)
{
}

void* 
rsrcMgrUser::operator new(unsigned int size)
{
    return(OVS_MEM_MALLOC(size, OVS_MEM_TYPE_RSRC));

}

void  
rsrcMgrUser::operator delete(void *ptr, unsigned int size)
{
    OVS_MEM_FREE(ptr, OVS_MEM_TYPE_RSRC);
}

bool
rsrcMgrUser::operator==(rsrcMgrUser& user)
{
    return (reinterpret_cast<int>(this) == 
		  reinterpret_cast<int>(&user)); 
}

bool
rsrcMgrUser::operator!=(rsrcMgrUser& user)
{
    return (reinterpret_cast<int>(this) != 
		  reinterpret_cast<int>(&user)); 
}

void 
rsrcMgrUser::dump(bool detail)
{
    OVS_LOG_INFO(NULL, 
    				 OVS_LOG_TYPE_RSRCMGR, 
    				 "User name: %s Callbacks: %s-%s-%s. Trace level %s. Flags 0x%x",
    				 _config.name,
    				 callbackToChar(_callbacks & RSRC_MGR_CBK_INFO),
                                 callbackToChar(_callbacks & RSRC_MGR_CBK_STATUS),
                                 callbackToChar(_callbacks & RSRC_MGR_CBK_CONFIG),
                                 ovsLogMgr::logLevelToChar(_config.traceLevel),
                                 _config.flags);
    				 
}

