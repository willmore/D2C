
/*
 * esx_vi.h: client for the VMware VI API 2.5 to manage ESX hosts
 *
 * Copyright (C) 2009-2010 Matthias Bolte <matthias.bolte@googlemail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

#ifndef __ESX_VI_H__
# define __ESX_VI_H__

# include <libxml/tree.h>
# include <libxml/xpath.h>
# include <curl/curl.h>

# include "internal.h"
# include "virterror_internal.h"
# include "datatypes.h"
# include "esx_vi_types.h"
# include "esx_util.h"



# define ESX_VI_ERROR(code, ...)                                              \
    virReportErrorHelper(NULL, VIR_FROM_ESX, code, __FILE__, __FUNCTION__,    \
                         __LINE__, __VA_ARGS__)



# define ESX_VI__SOAP__REQUEST_HEADER                                         \
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"                            \
    "<soapenv:Envelope\n"                                                     \
    " xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\"\n"          \
    " xmlns:soapenc=\"http://schemas.xmlsoap.org/soap/encoding/\"\n"          \
    " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"              \
    " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\n"                      \
    "<soapenv:Body>\n"



# define ESX_VI__SOAP__REQUEST_FOOTER                                         \
    "</soapenv:Body>\n"                                                       \
    "</soapenv:Envelope>"



# define ESV_VI__XML_TAG__OPEN(_buffer, _element, _type)                      \
    do {                                                                      \
        virBufferAddLit(_buffer, "<");                                        \
        virBufferAdd(_buffer, _element, -1);                                  \
        virBufferAddLit(_buffer, " xmlns=\"urn:vim25\" xsi:type=\"");         \
        virBufferAdd(_buffer, _type, -1);                                     \
        virBufferAddLit(_buffer, "\">");                                      \
    } while (0)



# define ESV_VI__XML_TAG__CLOSE(_buffer, _element)                            \
    do {                                                                      \
        virBufferAddLit(_buffer, "</");                                       \
        virBufferAdd(_buffer, _element, -1);                                  \
        virBufferAddLit(_buffer, ">");                                        \
    } while (0)



typedef enum _esxVI_APIVersion esxVI_APIVersion;
typedef enum _esxVI_ProductVersion esxVI_ProductVersion;
typedef enum _esxVI_Occurrence esxVI_Occurrence;
typedef struct _esxVI_ParsedHostCpuIdInfo esxVI_ParsedHostCpuIdInfo;
typedef struct _esxVI_Context esxVI_Context;
typedef struct _esxVI_Response esxVI_Response;
typedef struct _esxVI_Enumeration esxVI_Enumeration;
typedef struct _esxVI_EnumerationValue esxVI_EnumerationValue;
typedef struct _esxVI_List esxVI_List;



enum _esxVI_APIVersion {
    esxVI_APIVersion_Undefined = 0,
    esxVI_APIVersion_Unknown,
    esxVI_APIVersion_25,
    esxVI_APIVersion_40,
    esxVI_APIVersion_41,
    esxVI_APIVersion_4x /* > 4.1 */
};

/*
 * AAAABBBB: where AAAA0000 is the product and BBBB the version. this format
 * allows simple bitmask testing for a product independent of the version
 */
enum _esxVI_ProductVersion {
    esxVI_ProductVersion_Undefined = 0,

    esxVI_ProductVersion_GSX   = (1 << 0) << 16,
    esxVI_ProductVersion_GSX20 = esxVI_ProductVersion_GSX | 1,

    esxVI_ProductVersion_ESX   = (1 << 1) << 16,
    esxVI_ProductVersion_ESX35 = esxVI_ProductVersion_ESX | 1,
    esxVI_ProductVersion_ESX40 = esxVI_ProductVersion_ESX | 2,
    esxVI_ProductVersion_ESX41 = esxVI_ProductVersion_ESX | 3,
    esxVI_ProductVersion_ESX4x = esxVI_ProductVersion_ESX | 4, /* > 4.1 */

    esxVI_ProductVersion_VPX   = (1 << 2) << 16,
    esxVI_ProductVersion_VPX25 = esxVI_ProductVersion_VPX | 1,
    esxVI_ProductVersion_VPX40 = esxVI_ProductVersion_VPX | 2,
    esxVI_ProductVersion_VPX41 = esxVI_ProductVersion_VPX | 3,
    esxVI_ProductVersion_VPX4x = esxVI_ProductVersion_VPX | 4  /* > 4.1 */
};

enum _esxVI_Occurrence {
    esxVI_Occurrence_Undefined = 0,
    esxVI_Occurrence_RequiredItem,
    esxVI_Occurrence_RequiredList,
    esxVI_Occurrence_OptionalItem,
    esxVI_Occurrence_OptionalList,
    esxVI_Occurrence_None
};

struct _esxVI_ParsedHostCpuIdInfo {
    int level;
    char eax[32];
    char ebx[32];
    char ecx[32];
    char edx[32];
};



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Context
 */

struct _esxVI_Context {
    char *url;
    char *ipAddress;
    CURL *curl_handle;
    struct curl_slist *curl_headers;
    char curl_error[CURL_ERROR_SIZE];
    virMutex curl_lock;
    char *username;
    char *password;
    esxVI_ServiceContent *service;
    esxVI_APIVersion apiVersion;
    esxVI_ProductVersion productVersion;
    esxVI_UserSession *session;
    esxVI_Datacenter *datacenter;
    esxVI_ComputeResource *computeResource;
    esxVI_HostSystem *hostSystem;
    esxVI_SelectionSpec *selectSet_folderToChildEntity;
    esxVI_SelectionSpec *selectSet_hostSystemToParent;
    esxVI_SelectionSpec *selectSet_hostSystemToVm;
    esxVI_SelectionSpec *selectSet_hostSystemToDatastore;
    esxVI_SelectionSpec *selectSet_computeResourceToHost;
    esxVI_SelectionSpec *selectSet_computeResourceToParentToParent;
    bool hasQueryVirtualDiskUuid;
    bool hasSessionIsActive;
};

int esxVI_Context_Alloc(esxVI_Context **ctx);
void esxVI_Context_Free(esxVI_Context **ctx);
int esxVI_Context_Connect(esxVI_Context *ctx, const char *ipAddress,
                          const char *url, const char *username,
                          const char *password, esxUtil_ParsedUri *parsedUri);
int esxVI_Context_LookupObjectsByPath(esxVI_Context *ctx,
                                      esxUtil_ParsedUri *parsedUri);
int esxVI_Context_LookupObjectsByHostSystemIp(esxVI_Context *ctx,
                                              const char *hostSystemIpAddress);
int esxVI_Context_DownloadFile(esxVI_Context *ctx, const char *url,
                               char **content);
int esxVI_Context_UploadFile(esxVI_Context *ctx, const char *url,
                             const char *content);
int esxVI_Context_Execute(esxVI_Context *ctx, const char *methodName,
                          const char *request, esxVI_Response **response,
                          esxVI_Occurrence occurrence);



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Response
 */

struct _esxVI_Response {
    int responseCode;                                 /* required */
    char *content;                                    /* required */
    xmlDocPtr document;                               /* optional */
    xmlNodePtr node;                                  /* optional, list */
};

int esxVI_Response_Alloc(esxVI_Response **response);
void esxVI_Response_Free(esxVI_Response **response);



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Enumeration
 */

struct _esxVI_EnumerationValue {
    const char *name;
    int value;
};

struct _esxVI_Enumeration {
    esxVI_Type type;
    esxVI_EnumerationValue values[10];
};

int esxVI_Enumeration_CastFromAnyType(const esxVI_Enumeration *enumeration,
                                      esxVI_AnyType *anyType, int *value);
int esxVI_Enumeration_Serialize(const esxVI_Enumeration *enumeration,
                                int value, const char *element,
                                virBufferPtr output);
int esxVI_Enumeration_Deserialize(const esxVI_Enumeration *enumeration,
                                  xmlNodePtr node, int *value);



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * List
 */

struct _esxVI_List {
    esxVI_List *_next;
};

typedef int (*esxVI_List_FreeFunc) (esxVI_List **item);
typedef int (*esxVI_List_DeepCopyFunc) (esxVI_List **dest, esxVI_List *src);
typedef int (*esxVI_List_CastFromAnyTypeFunc) (esxVI_AnyType *anyType,
                                               esxVI_List **item);
typedef int (*esxVI_List_SerializeFunc) (esxVI_List *item, const char *element,
                                         virBufferPtr output);
typedef int (*esxVI_List_DeserializeFunc) (xmlNodePtr node, esxVI_List **item);

int esxVI_List_Append(esxVI_List **list, esxVI_List *item);
int esxVI_List_DeepCopy(esxVI_List **destList, esxVI_List *srcList,
                        esxVI_List_DeepCopyFunc deepCopyFunc,
                        esxVI_List_FreeFunc freeFunc);
int esxVI_List_CastFromAnyType(esxVI_AnyType *anyType, esxVI_List **list,
                               esxVI_List_CastFromAnyTypeFunc castFromAnyTypeFunc,
                               esxVI_List_FreeFunc freeFunc);
int esxVI_List_Serialize(esxVI_List *list, const char *element,
                         virBufferPtr output,
                         esxVI_List_SerializeFunc serializeFunc);
int esxVI_List_Deserialize(xmlNodePtr node, esxVI_List **list,
                           esxVI_List_DeserializeFunc deserializeFunc,
                           esxVI_List_FreeFunc freeFunc);



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Utility and Convenience Functions
 *
 * Function naming scheme:
 *  - 'lookup' functions query the ESX or vCenter for information
 *  - 'get' functions get information from a local object
 */

int esxVI_Alloc(void **ptrptr, size_t size);

int esxVI_BuildSelectSet
      (esxVI_SelectionSpec **selectSet, const char *name,
       const char *type, const char *path, const char *selectSetNames);

int esxVI_BuildSelectSetCollection(esxVI_Context *ctx);

int esxVI_EnsureSession(esxVI_Context *ctx);

int esxVI_LookupObjectContentByType(esxVI_Context *ctx,
                                    esxVI_ManagedObjectReference *root,
                                    const char *type,
                                    esxVI_String *propertyNameList,
                                    esxVI_ObjectContent **objectContentList,
                                    esxVI_Occurrence occurrence);

int esxVI_GetManagedEntityStatus
      (esxVI_ObjectContent *objectContent, const char *propertyName,
       esxVI_ManagedEntityStatus *managedEntityStatus);

int esxVI_GetVirtualMachinePowerState
      (esxVI_ObjectContent *virtualMachine,
       esxVI_VirtualMachinePowerState *powerState);

int esxVI_GetVirtualMachineQuestionInfo
      (esxVI_ObjectContent *virtualMachine,
       esxVI_VirtualMachineQuestionInfo **questionInfo);

int esxVI_GetBoolean(esxVI_ObjectContent *objectContent,
                     const char *propertyName,
                     esxVI_Boolean *value, esxVI_Occurrence occurrence);

int esxVI_GetLong(esxVI_ObjectContent *objectContent, const char *propertyName,
                  esxVI_Long **value, esxVI_Occurrence occurrence);

int esxVI_GetStringValue(esxVI_ObjectContent *objectContent,
                         const char *propertyName,
                         char **value, esxVI_Occurrence occurrence);

int esxVI_GetManagedObjectReference(esxVI_ObjectContent *objectContent,
                                    const char *propertyName,
                                    esxVI_ManagedObjectReference **value,
                                    esxVI_Occurrence occurrence);

int esxVI_LookupNumberOfDomainsByPowerState
      (esxVI_Context *ctx, esxVI_VirtualMachinePowerState powerState,
       esxVI_Boolean inverse);

int esxVI_GetVirtualMachineIdentity(esxVI_ObjectContent *virtualMachine,
                                    int *id, char **name, unsigned char *uuid);

int esxVI_GetNumberOfSnapshotTrees
      (esxVI_VirtualMachineSnapshotTree *snapshotTreeList);

int esxVI_GetSnapshotTreeNames
      (esxVI_VirtualMachineSnapshotTree *snapshotTreeList, char **names,
       int nameslen);

int esxVI_GetSnapshotTreeByName
      (esxVI_VirtualMachineSnapshotTree *snapshotTreeList, const char *name,
       esxVI_VirtualMachineSnapshotTree **snapshotTree,
       esxVI_VirtualMachineSnapshotTree **snapshotTreeParent,
       esxVI_Occurrence occurrence);

int esxVI_GetSnapshotTreeBySnapshot
      (esxVI_VirtualMachineSnapshotTree *snapshotTreeList,
       esxVI_ManagedObjectReference *snapshot,
       esxVI_VirtualMachineSnapshotTree **snapshotTree);

int esxVI_LookupHostSystemProperties(esxVI_Context *ctx,
                                     esxVI_String *propertyNameList,
                                     esxVI_ObjectContent **hostSystem);

int esxVI_LookupVirtualMachineList(esxVI_Context *ctx,
                                   esxVI_String *propertyNameList,
                                   esxVI_ObjectContent **virtualMachineList);

int esxVI_LookupVirtualMachineByUuid(esxVI_Context *ctx,
                                     const unsigned char *uuid,
                                     esxVI_String *propertyNameList,
                                     esxVI_ObjectContent **virtualMachine,
                                     esxVI_Occurrence occurrence);

int esxVI_LookupVirtualMachineByName(esxVI_Context *ctx, const char *name,
                                     esxVI_String *propertyNameList,
                                     esxVI_ObjectContent **virtualMachine,
                                     esxVI_Occurrence occurrence);

int esxVI_LookupVirtualMachineByUuidAndPrepareForTask
      (esxVI_Context *ctx, const unsigned char *uuid,
       esxVI_String *propertyNameList, esxVI_ObjectContent **virtualMachine,
       esxVI_Boolean autoAnswer);

int esxVI_LookupDatastoreList(esxVI_Context *ctx, esxVI_String *propertyNameList,
                              esxVI_ObjectContent **datastoreList);

int esxVI_LookupDatastoreByName(esxVI_Context *ctx, const char *name,
                                esxVI_String *propertyNameList,
                                esxVI_ObjectContent **datastore,
                                esxVI_Occurrence occurrence);

int esxVI_LookupDatastoreByAbsolutePath(esxVI_Context *ctx,
                                        const char *absolutePath,
                                        esxVI_String *propertyNameList,
                                        esxVI_ObjectContent **datastore,
                                        esxVI_Occurrence occurrence);

int esxVI_LookupDatastoreHostMount(esxVI_Context *ctx,
                                   esxVI_ManagedObjectReference *datastore,
                                   esxVI_DatastoreHostMount **hostMount);

int esxVI_LookupTaskInfoByTask(esxVI_Context *ctx,
                               esxVI_ManagedObjectReference *task,
                               esxVI_TaskInfo **taskInfo);

int esxVI_LookupPendingTaskInfoListByVirtualMachine
      (esxVI_Context *ctx, esxVI_ObjectContent *virtualMachine,
       esxVI_TaskInfo **pendingTaskInfoList);

int esxVI_LookupAndHandleVirtualMachineQuestion(esxVI_Context *ctx,
                                                const unsigned char *uuid,
                                                esxVI_Occurrence occurrence,
                                                esxVI_Boolean autoAnswer,
                                                esxVI_Boolean *blocked);

int esxVI_LookupRootSnapshotTreeList
      (esxVI_Context *ctx, const unsigned char *virtualMachineUuid,
       esxVI_VirtualMachineSnapshotTree **rootSnapshotTreeList);

int esxVI_LookupCurrentSnapshotTree
      (esxVI_Context *ctx, const unsigned char *virtualMachineUuid,
       esxVI_VirtualMachineSnapshotTree **currentSnapshotTree,
       esxVI_Occurrence occurrence);

int esxVI_LookupFileInfoByDatastorePath(esxVI_Context *ctx,
                                        const char *datastorePath,
                                        bool lookupFolder,
                                        esxVI_FileInfo **fileInfo,
                                        esxVI_Occurrence occurrence);

int esxVI_LookupDatastoreContentByDatastoreName
      (esxVI_Context *ctx, const char *datastoreName,
       esxVI_HostDatastoreBrowserSearchResults **searchResultsList);

int esxVI_LookupStorageVolumeKeyByDatastorePath(esxVI_Context *ctx,
                                                const char *datastorePath,
                                                char **key);

int esxVI_LookupAutoStartDefaults(esxVI_Context *ctx,
                                  esxVI_AutoStartDefaults **defaults);

int esxVI_LookupAutoStartPowerInfoList(esxVI_Context *ctx,
                                       esxVI_AutoStartPowerInfo **powerInfoList);

int esxVI_HandleVirtualMachineQuestion
      (esxVI_Context *ctx,
       esxVI_ManagedObjectReference *virtualMachine,
       esxVI_VirtualMachineQuestionInfo *questionInfo,
       esxVI_Boolean autoAnswer, esxVI_Boolean *blocked);

int esxVI_WaitForTaskCompletion(esxVI_Context *ctx,
                                esxVI_ManagedObjectReference *task,
                                const unsigned char *virtualMachineUuid,
                                esxVI_Occurrence virtualMachineOccurrence,
                                esxVI_Boolean autoAnswer,
                                esxVI_TaskInfoState *finalState,
                                char **errorMessage);

int esxVI_ParseHostCpuIdInfo(esxVI_ParsedHostCpuIdInfo *parsedHostCpuIdInfo,
                             esxVI_HostCpuIdInfo *hostCpuIdInfo);

int esxVI_ProductVersionToDefaultVirtualHWVersion(esxVI_ProductVersion productVersion);

#endif /* __ESX_VI_H__ */
