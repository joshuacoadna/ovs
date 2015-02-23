#include "ovsRsrcMgrApi.h"

/* Resource Manager API */
ovsRet_t rsrcMgrUserSubscribe(
                 /*INPUT*/ rsrcMgrUser_t* pUserCfg,         /* user configuration parameters */
                 /*INPUT*/ OVS_RSRC_CONFIG_CB rsrcCfgCb,          /* resource configuration callback */
                 /*INPUT*/ OVS_RSRC_INFO_CB rsrcInfoCb,         /* resource information callback */
                 /*INPUT*/ OVS_RSRC_STATUS_CB rsrcStatusCb,     /* resource status callback */
                 /*OUTPUT*/ rmptr_t*  ppUserHandle)         /* returned user handle */
{
    uint16 callbacks = 0;
    callbacks |= (rsrcCfgCb ? RSRC_MGR_CBK_CONFIG : 0);
    callbacks |= (rsrcInfoCb ? RSRC_MGR_CBK_INFO : 0);
    callbacks |= (rsrcStatusCb ? RSRC_MGR_CBK_STATUS: 0);

    
	
    return(OVS_RET_OK);
}

ovsRet_t rsrcMgrUserUnsubscribe(
                 /*INPUT*/ rmptr_t  pUserHandle)         /* user handle */
{
    return(OVS_RET_OK);
}

ovsRet_t rsrcMgrUserModify(
                 /*INPUT*/ rmptr_t  pUserHandle,         /* user handle */
                 /*INPUT*/ rsrcMgrUser_t pUserCfg)   /* user configuration parameters */
{
    return(OVS_RET_OK);
}

ovsRet_t rsrcAlloc(
	         /*INPUT*/ uint32 portId,                     /* OVS port identifier */
                 /*INPUT*/ rmptr_t pUserHandle,         /* user handle */
                 /*INPUT*/ rsrcInfo_t *pRsrcInfo)          /* requested resource configuration */
{
    return(OVS_RET_OK);
}

ovsRet_t rsrcFree(
                 /*INPUT*/ rmptr_t pUserHandle,         /* user handle */
                 /*INPUT*/ rmptr_t pRsrcHandle)          /* resource handle */
{
    return(OVS_RET_OK);
}

ovsRet_t  rsrcModify(
                 /*INPUT*/ rmptr_t   pUserHandle,          /*user handle */
                 /*INPUT*/ rmptr_t   pRsrcHandle,        /* resource handle returned by rsrcAlloc */
                 /*INPUT*/ rsrcInfo_t *rsrcInfo)            /* modified resource parameters */
{
    return(OVS_RET_OK);
}

ovsRet_t rsrcAssociate(
                 /*INPUT*/ rmptr_t pUserHandle,         /* user handle */
                 /*INPUT*/ rmptr_t pSrcRsrcHandle,          /* resource handle */
                 /*INPUT*/ rmptr_t pDstRsrcHandle)      /* resource handle (e.g. a xc) which is associated with the previous resource (e.g. a port) */
{
    return(OVS_RET_OK);
}

ovsRet_t rsrcUnassociate(
                 /*INPUT*/ rmptr_t pUserHandle,         /* user handle */
                 /*INPUT*/ rmptr_t pSrcRsrcHandle,          /* resource handle */
                 /*INPUT*/ rmptr_t pDstRsrcHandle)      /* resource handle (e.g. a xc) which is unassociated with the previous resource (e.g. a port) */
{
    return(OVS_RET_OK);
}


