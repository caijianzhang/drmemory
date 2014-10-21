/* **********************************************************
 * Copyright (c) 2011-2014 Google, Inc.  All rights reserved.
 * **********************************************************/

/* Dr. Memory: the memory debugger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License, and no later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "dr_api.h"
#include "drsyscall.h"
#include "drsyscall_os.h"
#include "drsyscall_windows.h"

#include "../wininc/wdm.h"
#include "../wininc/ndk_extypes.h"
#include "../wininc/ndk_iotypes.h"
#include "../wininc/wdm.h"
#include "../wininc/ntifs.h"
#include "table_defines.h"

extern drsys_sysnum_t sysnum_SetInformationFile;

/* i#1549 We use the following approach here:
 * 1) We use macros below to describe secondary syscall entries.
 * 2) We add all syscalls with secondary components in the separate
 *    hashtable using drsys_sysnum_t.
 */

#define ENTRY_QueryKey(classname, typename)\
     {\
         {0, sizeof(HANDLE), SYSARG_INLINED, DRSYS_TYPE_HANDLE},\
         {1, sizeof(KEY_INFORMATION_CLASS), SYSARG_INLINED, DRSYS_TYPE_SIGNED_INT, classname},\
         {2, -3, W, 0, typename},\
         {2, -4, WI},\
         {3, sizeof(ULONG), SYSARG_INLINED, DRSYS_TYPE_UNSIGNED_INT},\
         {4, sizeof(ULONG), W|HT, DRSYS_TYPE_UNSIGNED_INT},\
    }

/* Since _ version of structure names stored in PDBs, we use the same names here. */
syscall_info_t syscall_QueryKey_info[] = {
   {{0,0},"NtQueryKey.KeyBasicInformation", OK|SYSINFO_RET_SMALL_WRITE_LAST, RNTST, 5,
         ENTRY_QueryKey("KeyBasicInformation", "_KEY_BASIC_INFORMATION")
   },
   {{0,0},"NtQueryKey.KeyNodeInformation", OK|SYSINFO_RET_SMALL_WRITE_LAST, RNTST, 5,
         ENTRY_QueryKey("KeyNodeInformation", "_KEY_NODE_INFORMATION")
   },
   {{0,0},"NtQueryKey.KeyFullInformation", OK|SYSINFO_RET_SMALL_WRITE_LAST, RNTST, 5,
         ENTRY_QueryKey("KeyFullInformation", "_KEY_FULL_INFORMATION")
   },
   {{0,0},"NtQueryKey.KeyNameInformation", OK|SYSINFO_RET_SMALL_WRITE_LAST, RNTST, 5,
         ENTRY_QueryKey("KeyNameInformation", "_KEY_NAME_INFORMATION")
   },
   {{0,0},"NtQueryKey.KeyCachedInformation", OK|SYSINFO_RET_SMALL_WRITE_LAST, RNTST, 5,
         ENTRY_QueryKey("KeyCachedInformation", "_KEY_CACHED_INFORMATION")
   },
   {{0,0},"NtQueryKey.KeyFlagsInformation", OK|SYSINFO_RET_SMALL_WRITE_LAST, RNTST, 5,
         /* Reserved */
         ENTRY_QueryKey("KeyFlagsInformation", "_KEY_FLAGS_INFORMATION")
   },
   {{0,0},"NtQueryKey.KeyVirtualizationInformation", OK|SYSINFO_RET_SMALL_WRITE_LAST, RNTST, 5,
         ENTRY_QueryKey("KeyVirtualizationInformation",
                        "_KEY_VIRTUALIZATION_INFORMATION")
   },
   {{0,0},"NtQueryKey.KeyHandleTagsInformation", OK|SYSINFO_RET_SMALL_WRITE_LAST, RNTST, 5,
         ENTRY_QueryKey("KeyHandleTagsInformation", "Reserved")
   },
   {SECONDARY_TABLE_ENTRY_MAX_NUMBER},
   {{0,0},"NtQueryKey.UNKNOWN", OK, RNTST, 6,
         ENTRY_QueryKey(NULL, NULL)
   },
};

#define ENTRY_EnumerateKey(classname, typename)\
     {\
         {0, sizeof(HANDLE), SYSARG_INLINED, DRSYS_TYPE_HANDLE},\
         {1, sizeof(ULONG), SYSARG_INLINED, DRSYS_TYPE_UNSIGNED_INT},\
         {2, sizeof(KEY_INFORMATION_CLASS), SYSARG_INLINED, DRSYS_TYPE_SIGNED_INT, classname},\
         {3, -4, W, 0, typename},\
         {3, -5, WI},\
         {4, sizeof(ULONG), SYSARG_INLINED, DRSYS_TYPE_UNSIGNED_INT},\
         {5, sizeof(ULONG), W|HT, DRSYS_TYPE_UNSIGNED_INT},\
     }

syscall_info_t syscall_EnumerateKey_info[] = {
    {{0,0},"NtEnumerateKey.KeyBasicInformation", OK, RNTST, 6,
         ENTRY_EnumerateKey("KeyBasicInformation", "_KEY_BASIC_INFORMATION")
    },
    {{0,0},"NtEnumerateKey.KeyNodeInformation", OK, RNTST, 6,
         ENTRY_EnumerateKey("KeyNodeInformation", "_KEY_NODE_INFORMATION")
    },
    {{0,0},"NtEnumerateKey.KeyFullInformation", OK, RNTST, 6,
         ENTRY_EnumerateKey("KeyFullInformation", "_KEY_FULL_INFORMATION")
    },
    {{0,0},"NtEnumerateKey.KeyNameInformation", OK, RNTST, 6,
         ENTRY_EnumerateKey("KeyNameInformation", "_KEY_NAME_INFORMATION")
    },
    {{0,0},"NtEnumerateKey.KeyCachedInformation", OK, RNTST, 6,
         ENTRY_EnumerateKey("KeyCachedInformation", "_KEY_CACHED_INFORMATION")
    },
    {{0,0},"NtEnumerateKey.KeyFlagsInformation", OK, RNTST, 6,
        /* Reserved */
         ENTRY_EnumerateKey("KeyFlagsInformation", "_KEY_FLAGS_INFORMATION")
    },
    {{0,0},"NtEnumerateKey.KeyVirtualizationInformation", OK, RNTST, 6,
         ENTRY_EnumerateKey("KeyVirtualizationInformation",
                            "_KEY_VIRTUALIZATION_INFORMATION")
    },
    {{0,0},"NtEnumerateKey.KeyHandleTagsInformation", OK, RNTST, 6,
         ENTRY_EnumerateKey("KeyHandleTagsInformation", "_KEY_HANDLE_TAGS_INFORMATION")
    },
    {SECONDARY_TABLE_ENTRY_MAX_NUMBER},
    {{0,0},"NtEnumerateKey.UNKNOWN", OK, RNTST, 6,
         ENTRY_EnumerateKey(NULL, NULL)
    },
};

#define ENTRY_EnumerateValueKey(classname, typename)\
     {\
         {0, sizeof(HANDLE), SYSARG_INLINED, DRSYS_TYPE_HANDLE},\
         {1, sizeof(ULONG), SYSARG_INLINED, DRSYS_TYPE_UNSIGNED_INT},\
         {2, sizeof(KEY_VALUE_INFORMATION_CLASS), SYSARG_INLINED, DRSYS_TYPE_SIGNED_INT, classname},\
         {3, -4, W, 0, typename},\
         {3, -5, WI},\
         {4, sizeof(ULONG), SYSARG_INLINED, DRSYS_TYPE_UNSIGNED_INT},\
         {5, sizeof(ULONG), W|HT, DRSYS_TYPE_UNSIGNED_INT},\
     }

syscall_info_t syscall_EnumerateValueKey_info[] = {
    {{0,0},"NtEnumerateValueKey.KeyValueBasicInformation", OK, RNTST, 6,
         ENTRY_EnumerateValueKey("KeyValueBasicInformation",
                                 "_KEY_VALUE_BASIC_INFORMATION")
    },
    {{0,0},"NtEnumerateValueKey.KeyValueFullInformation", OK, RNTST, 6,
         ENTRY_EnumerateValueKey("KeyValueFullInformation",
                                 "_KEY_VALUE_FULL_INFORMATION")
    },
    {{0,0},"NtEnumerateValueKey.KeyValuePartialInformation", OK, RNTST, 6,
         ENTRY_EnumerateValueKey("KeyValuePartialInformation",
                                 "_KEY_VALUE_PARTIAL_INFORMATION")
    },
    {{0,0},"NtEnumerateValueKey.KeyValueFullInformationAlign64", OK, RNTST, 6,
         ENTRY_EnumerateValueKey("KeyValueFullInformationAlign64",
                                 "_KEY_VALUE_FULL_INFORMATION")
    },
    {{0,0},"NtEnumerateValueKey.KeyValuePartialInformationAlign64", OK, RNTST, 6,
         ENTRY_EnumerateValueKey("KeyValuePartialInformationAlign64",
                                 "_KEY_VALUE_PARTIAL_INFORMATION")
    },
    {SECONDARY_TABLE_ENTRY_MAX_NUMBER},
    {{0,0},"NtEnumerateValueKey.UNKNOWN", OK, RNTST, 6,
         ENTRY_EnumerateValueKey(NULL, NULL)
    },
};

#define ENTRY_QueryDirectoryFile(classname, typename)\
     {\
         {0, sizeof(HANDLE), SYSARG_INLINED, DRSYS_TYPE_HANDLE},\
         {1, sizeof(HANDLE), SYSARG_INLINED, DRSYS_TYPE_HANDLE},\
         {2, sizeof(PIO_APC_ROUTINE), SYSARG_INLINED, DRSYS_TYPE_FUNCTION},\
         {3, sizeof(PVOID), SYSARG_INLINED, DRSYS_TYPE_VOID},\
         {4, sizeof(IO_STATUS_BLOCK), W|HT, DRSYS_TYPE_IO_STATUS_BLOCK},\
         {5, -6, W, 0, typename},\
         {6, sizeof(ULONG), SYSARG_INLINED, DRSYS_TYPE_UNSIGNED_INT},\
         {7, sizeof(FILE_INFORMATION_CLASS), SYSARG_INLINED, DRSYS_TYPE_SIGNED_INT, classname},\
         {8, sizeof(BOOLEAN), SYSARG_INLINED, DRSYS_TYPE_BOOL},\
         {9, sizeof(UNICODE_STRING), R|CT, SYSARG_TYPE_UNICODE_STRING},\
         {10, sizeof(BOOLEAN), SYSARG_INLINED, DRSYS_TYPE_BOOL},\
     }

syscall_info_t syscall_QueryDirectoryFile_info[] = {
    {SECONDARY_TABLE_SKIP_ENTRY},
    {{0,0},"NtQueryDirectoryFile.FileDirectoryInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileDirectoryInformation",
                                  "_FILE_DIRECTORY_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileFullDirectoryInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileFullDirectoryInformation",
                                  "_FILE_FULL_DIR_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileBothDirectoryInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileBothDirectoryInformation",
                                  "_FILE_BOTH_DIR_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileBasicInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileBasicInformation",
                                  "_FILE_BASIC_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileStandardInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileStandardInformation",
                                  "_FILE_STANDARD_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileInternalInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileInternalInformation",
                                  "_FILE_INTERNAL_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileEaInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileEaInformation",
                                  "_FILE_EA_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileAccessInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileAccessInformation",
                                  "_FILE_ACCESS_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileNameInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileNameInformation",
                                  "_FILE_NAME_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileRenameInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileRenameInformation",
                                  "_FILE_RENAME_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileLinkInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileLinkInformation",
                                  "_FILE_LINK_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileNamesInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileNamesInformation",
                                  "_FILE_NAMES_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileDispositionInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileDispositionInformation",
                                  "_FILE_DISPOSITION_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FilePositionInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FilePositionInformation",
                                  "_FILE_POSITION_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileFullEaInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileFullEaInformation",
                                  "_FILE_FULL_EA_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileModeInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileModeInformation",
                                  "_FILE_MODE_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileAlignmentInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileAlignmentInformation",
                                  "_FILE_ALIGNMENT_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileAllInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileAllInformation",
                                  "_FILE_ALL_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileAllocationInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileAllocationInformation",
                                  "_FILE_ALLOCATION_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileEndOfFileInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileEndOfFileInformation",
                                  "_FILE_END_OF_FILE_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileAlternateNameInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileAlternateNameInformation",
                                  "_FILE_NAME_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileStreamInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileStreamInformation",
                                  "_FILE_STREAM_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FilePipeInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FilePipeInformation",
                                  "_FILE_PIPE_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FilePipeLocalInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FilePipeLocalInformation",
                                  "_FILE_PIPE_LOCAL_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FilePipeRemoteInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FilePipeRemoteInformation",
                                  "_FILE_PIPE_REMOTE_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileMailslotQueryInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileMailslotQueryInformation",
                                  "_FILE_MAILSLOT_QUERY_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileMailslotSetInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileMailslotSetInformation",
                                  "_FILE_MAILSLOT_SET_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileCompressionInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileCompressionInformation",
                                  "_FILE_COMPRESSION_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileObjectIdInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileObjectIdInformation",
                                  "_FILE_OBJECTID_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileCompletionInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileCompletionInformation",
                                  "_FILE_COMPLETION_INFORMATION") /* Reserved */
    },
    {{0,0},"NtQueryDirectoryFile.FileMoveClusterInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileMoveClusterInformation",
                                  "_FILE_MOVE_CLUSTER_INFORMATION") /* Reserved */
    },
    {{0,0},"NtQueryDirectoryFile.FileQuotaInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileQuotaInformation", "_FILE_QUOTA_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileReparsePointInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileReparsePointInformation",
                                  "_FILE_REPARSE_POINT_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileNetworkOpenInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileNetworkOpenInformation",
                                  "_FILE_NETWORK_OPEN_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileAttributeTagInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileAttributeTagInformation",
                                  "_FILE_ATTRIBUTE_TAG_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileTrackingInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileTrackingInformation",
                                  "_FILE_TRACKING_INFORMATION") /* Reserved */
    },
    {{0,0},"NtQueryDirectoryFile.FileIdBothDirectoryInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileIdBothDirectoryInformation",
                                  "_FILE_ID_BOTH_DIR_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileIdFullDirectoryInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileIdFullDirectoryInformation",
                                  "_FILE_ID_FULL_DIR_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileValidDataLengthInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileValidDataLengthInformation",
                                  "_FILE_VALID_DATA_LENGTH_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileShortNameInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileShortNameInformation",
                                  "_FILE_NAME_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileIoCompletionNotificationInformation", OK, RNTST, 11,
         /* Reserved */
         ENTRY_QueryDirectoryFile("FileIoCompletionNotificationInformation",
                                  "_FILE_IO_COMPLETION_NOTIFICATION_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileIoStatusBlockRangeInformation", OK, RNTST, 11,
         /* Reserved */
         ENTRY_QueryDirectoryFile("FileIoStatusBlockRangeInformation",
                                  "_FILE_IO_STATUS_BLOCK_RANGE_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileIoPriorityHintInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileIoPriorityHintInformation",
                                  "_FILE_IO_PRIORITY_HINT_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileSfioReserveInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileSfioReserveInformation",
                                  "_FILE_SFIO_RESERVE_INFORMATION") /* Reserved */
    },
    {{0,0},"NtQueryDirectoryFile.FileSfioVolumeInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileSfioVolumeInformation",
                                  "_FILE_SFIO_VOLUME_INFORMATION") /* Reserved */
    },
    {{0,0},"NtQueryDirectoryFile.FileHardLinkInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileHardLinkInformation",
                                  "_FILE_LINKS_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileProcessIdsUsingFileInformation", OK, RNTST, 11,
        /* Reserved */
         ENTRY_QueryDirectoryFile("FileProcessIdsUsingFileInformation",
                                  "_FILE_PROCESS_IDS_USING_FILE_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileNormalizedNameInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileNormalizedNameInformation",
                                  "_FILE_NAME_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileNetworkPhysicalNameInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileNetworkPhysicalNameInformation",
                                  "_FILE_NETWORK_PHYSICAL_NAME_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileIdGlobalTxDirectoryInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileIdGlobalTxDirectoryInformation",
                                  "_FILE_ID_GLOBAL_TX_DIR_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileIsRemoteDeviceInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileIsRemoteDeviceInformation",
                                  "_FILE_IS_REMOTE_DEVICE_INFORMATION") /* Reserved */
    },
    {{0,0},"NtQueryDirectoryFile.FileAttributeCacheInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileAttributeCacheInformation",
                                  "_FILE_ATTRIBUTE_CACHE_INFORMATION") /* Reserved */
    },
    {{0,0},"NtQueryDirectoryFile.FileNumaNodeInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileNumaNodeInformation",
                                  "_FILE_NUMA_NODE_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileStandardLinkInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileStandardLinkInformation",
                                  "_FILE_STANDARD_LINK_INFORMATION") /* Reserved */
    },
    {{0,0},"NtQueryDirectoryFile.FileRemoteProtocolInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileRemoteProtocolInformation",
                                  "_FILE_REMOTE_PROTOCOL_INFORMATION") /* Reserved */
    },
    {{0,0},"NtQueryDirectoryFile.FileReplaceCompletionInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileReplaceCompletionInformation",
                                  "_FILE_COMPLETION_INFORMATION")
    },
    {{0,0},"NtQueryDirectoryFile.FileMaximumInformation", OK, RNTST, 11,
         ENTRY_QueryDirectoryFile("FileMaximumInformation",
                                  "_FILE_MAXIMUM_INFORMATION") /* Reserved */
    },
    {SECONDARY_TABLE_ENTRY_MAX_NUMBER},
    {{0,0},"NtQueryDirectoryFile.UNKNOWN", OK, RNTST, 6,
         ENTRY_QueryDirectoryFile(NULL, NULL)
    },
};

#define ENTRY_QueryEvent(classname, typename)\
     {\
         {0, sizeof(HANDLE), SYSARG_INLINED, DRSYS_TYPE_HANDLE},\
         {1, sizeof(EVENT_INFORMATION_CLASS), SYSARG_INLINED, DRSYS_TYPE_SIGNED_INT, classname},\
         {2, -3, W, 0, typename},\
         {2, -4, WI},\
         {3, sizeof(ULONG), SYSARG_INLINED, DRSYS_TYPE_UNSIGNED_INT},\
         {4, sizeof(ULONG), W|HT, DRSYS_TYPE_UNSIGNED_INT},\
     }

syscall_info_t syscall_QueryEvent_info[] = {
    {{0,0},"NtQueryEvent.EventBasicInformation", OK|SYSINFO_RET_SMALL_WRITE_LAST, RNTST, 5,
         ENTRY_QueryEvent("EventBasicInformation", "_EVENT_BASIC_INFORMATION")
    },
    {SECONDARY_TABLE_ENTRY_MAX_NUMBER},
    {{0,0},"NtQueryEvent.UNKNOWN", OK, RNTST, 6,
         ENTRY_QueryEvent(NULL, NULL)
    },
};

#define ENTRY_QueryVolumeInformationFile(classname, typename)\
     {\
         {0, sizeof(HANDLE), SYSARG_INLINED, DRSYS_TYPE_HANDLE},\
         {1, sizeof(IO_STATUS_BLOCK), W|HT, DRSYS_TYPE_IO_STATUS_BLOCK},\
         {2, -3, W, 0, typename},\
         {3, sizeof(ULONG), SYSARG_INLINED, DRSYS_TYPE_UNSIGNED_INT},\
         {4, sizeof(FS_INFORMATION_CLASS), SYSARG_INLINED, DRSYS_TYPE_SIGNED_INT, classname},\
     }

syscall_info_t syscall_QueryVolumeInformationFile_info[] = {
    {SECONDARY_TABLE_SKIP_ENTRY},
    {{0,0},"NtQueryVolumeInformationFile.FileFsVolumeInformation", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile("FileFsVolumeInformation", "_FILE_FS_VOLUME_INFORMATION")
    },
    {{0,0},"NtQueryVolumeInformationFile.FileFsLabelInformation", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile("FileFsLabelInformation", "_FILE_FS_LABEL_INFORMATION")
    },
    {{0,0},"NtQueryVolumeInformationFile.FileFsSizeInformation", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile("FileFsSizeInformation", "_FILE_FS_SIZE_INFORMATION")
    },
    {{0,0},"NtQueryVolumeInformationFile.FileFsDeviceInformation", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile("FileFsDeviceInformation", "_FILE_FS_DEVICE_INFORMATION")
    },
    {{0,0},"NtQueryVolumeInformationFile.FileFsAttributeInformation", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile("FileFsAttributeInformation", "_FILE_FS_ATTRIBUTE_INFORMATION")
    },
    {{0,0},"NtQueryVolumeInformationFile.FileFsControlInformation", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile("FileFsControlInformation", "_FILE_FS_CONTROL_INFORMATION")
    },
    {{0,0},"NtQueryVolumeInformationFile.FileFsFullSizeInformation", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile("FileFsFullSizeInformation", "_FILE_FS_FULL_SIZE_INFORMATION")
    },
    {{0,0},"NtQueryVolumeInformationFile.FileFsObjectIdInformation", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile("FileFsObjectIdInformation", "_FILE_FS_OBJECTID_INFORMATION")
    },
    {{0,0},"NtQueryVolumeInformationFile.FileFsDriverPathInformation", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile("FileFsDriverPathInformation", "_FILE_FS_DRIVER_PATH_INFORMATION")
    },
    {{0,0},"NtQueryVolumeInformationFile.FileFsVolumeFlagsInformation", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile("FileFsVolumeFlagsInformation", "_FILE_FS_VOLUME_FLAGS_INFORMATION")
    },
    {{0,0},"NtQueryVolumeInformationFile.FileFsSectorSizeInformation", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile("FileFsSectorSizeInformation", "_FILE_FS_SECTOR_SIZE_INFORMATION")
    },
    {SECONDARY_TABLE_ENTRY_MAX_NUMBER},
    {{0,0},"NtQueryVolumeInformationFile.UNKNOWN", OK, RNTST, 5,
         ENTRY_QueryVolumeInformationFile(NULL, NULL)
    },
};
#define ENTRY_SetInformationFile(classname, typename)\
     {\
         {0, sizeof(HANDLE), SYSARG_INLINED, DRSYS_TYPE_HANDLE},\
         {1, sizeof(IO_STATUS_BLOCK), W|HT, DRSYS_TYPE_IO_STATUS_BLOCK},\
         {2, -3, SYSARG_NON_MEMARG, 0, typename},\
         {3, sizeof(ULONG), SYSARG_INLINED, DRSYS_TYPE_UNSIGNED_INT},\
         {4, sizeof(FILE_INFORMATION_CLASS), SYSARG_INLINED, DRSYS_TYPE_SIGNED_INT, classname},\
     }

syscall_info_t syscall_SetInformationFile_info[] = {
    {SECONDARY_TABLE_SKIP_ENTRY},
    {{0,0},"NtSetInformationFile.FileDirectoryInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileDirectoryInformation", "_FILE_DIRECTORY_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileFullDirectoryInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileFullDirectoryInformation", "_FILE_FULL_DIR_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileBothDirectoryInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileBothDirectoryInformation", "_FILE_BOTH_DIR_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileBasicInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileBasicInformation", "_FILE_BASIC_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileStandardInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileStandardInformation", "_FILE_STANDARD_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileInternalInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileInternalInformation", "_FILE_INTERNAL_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileEaInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileEaInformation", "_FILE_EA_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileAccessInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileAccessInformation", "_FILE_ACCESS_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileNameInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileNameInformation", "_FILE_NAME_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileRenameInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileRenameInformation", "_FILE_RENAME_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileLinkInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileLinkInformation", "_FILE_LINK_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileNamesInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileNamesInformation", "_FILE_NAMES_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileDispositionInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileDispositionInformation", "_FILE_DISPOSITION_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FilePositionInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FilePositionInformation", "_FILE_POSITION_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileFullEaInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileFullEaInformation", "_FILE_FULL_EA_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileModeInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileModeInformation", "_FILE_MODE_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileAlignmentInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileAlignmentInformation", "_FILE_ALIGNMENT_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileAllInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileAllInformation", "_FILE_ALL_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileAllocationInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileAllocationInformation", "_FILE_ALLOCATION_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileEndOfFileInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileEndOfFileInformation", "_FILE_END_OF_FILE_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileAlternateNameInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileAlternateNameInformation", "_FILE_NAME_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileStreamInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileStreamInformation", "_FILE_STREAM_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FilePipeInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FilePipeInformation", "_FILE_PIPE_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FilePipeLocalInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FilePipeLocalInformation", "_FILE_PIPE_LOCAL_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FilePipeRemoteInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FilePipeRemoteInformation", "_FILE_PIPE_REMOTE_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileMailslotQueryInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileMailslotQueryInformation", "_FILE_MAILSLOT_QUERY_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileMailslotSetInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileMailslotSetInformation", "_FILE_MAILSLOT_SET_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileCompressionInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileCompressionInformation", "_FILE_COMPRESSION_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileObjectIdInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileObjectIdInformation", "_FILE_OBJECTID_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileCompletionInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileCompletionInformation", "_FILE_COMPLETION_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileMoveClusterInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileMoveClusterInformation", "_FILE_MOVE_CLUSTER_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileQuotaInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileQuotaInformation", "_FILE_QUOTA_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileReparsePointInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileReparsePointInformation", "_FILE_REPARSE_POINT_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileNetworkOpenInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileNetworkOpenInformation", "_FILE_NETWORK_OPEN_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileAttributeTagInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileAttributeTagInformation", "_FILE_ATTRIBUTE_TAG_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileTrackingInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileTrackingInformation", "_FILE_TRACKING_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileIdBothDirectoryInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileIdBothDirectoryInformation", "_FILE_ID_BOTH_DIR_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileIdFullDirectoryInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileIdFullDirectoryInformation", "_FILE_ID_FULL_DIR_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileValidDataLengthInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileValidDataLengthInformation", "_FILE_VALID_DATA_LENGTH_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileShortNameInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileShortNameInformation", "_FILE_NAME_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileIoCompletionNotificationInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileIoCompletionNotificationInformation", "_FILE_IO_COMPLETION_NOTIFICATION_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileIoStatusBlockRangeInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileIoStatusBlockRangeInformation", "_FILE_IO_STATUS_BLOCK_RANGE_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileIoPriorityHintInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileIoPriorityHintInformation", "_FILE_IO_PRIORITY_HINT_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileSfioReserveInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileSfioReserveInformation", "_FILE_SFIO_RESERVE_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileSfioVolumeInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileSfioVolumeInformation", "_FILE_SFIO_VOLUME_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileHardLinkInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileHardLinkInformation", "_FILE_LINKS_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileProcessIdsUsingFileInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileProcessIdsUsingFileInformation", "_FILE_PROCESS_IDS_USING_FILE_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileNormalizedNameInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileNormalizedNameInformation", "_FILE_NAME_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileNetworkPhysicalNameInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileNetworkPhysicalNameInformation", "_FILE_NETWORK_PHYSICAL_NAME_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileIdGlobalTxDirectoryInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileIdGlobalTxDirectoryInformation", "_FILE_ID_GLOBAL_TX_DIR_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileIsRemoteDeviceInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileIsRemoteDeviceInformation", "_FILE_IS_REMOTE_DEVICE_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileAttributeCacheInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileAttributeCacheInformation", "_FILE_ATTRIBUTE_CACHE_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileNumaNodeInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileNumaNodeInformation", "_FILE_NUMA_NODE_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileStandardLinkInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileStandardLinkInformation", "_FILE_STANDARD_LINK_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileRemoteProtocolInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileRemoteProtocolInformation", "_FILE_REMOTE_PROTOCOL_INFORMATION")
    },
    {{0,0},"NtSetInformationFile.FileReplaceCompletionInformation", OK, RNTST, 5,
         ENTRY_SetInformationFile("FileReplaceCompletionInformation", "_FILE_COMPLETION_INFORMATION")
    },
    {SECONDARY_TABLE_ENTRY_MAX_NUMBER},
    {{0,0},"NtSetInformationFile.UNKNOWN", OK, RNTST, 5,
         ENTRY_SetInformationFile(NULL, NULL), &sysnum_SetInformationFile
    },
};
#define ENTRY_SetInformationKey(classname, typename)\
     {\
         {0, sizeof(HANDLE), SYSARG_INLINED, DRSYS_TYPE_HANDLE},\
         {1, sizeof(KEY_SET_INFORMATION_CLASS), SYSARG_INLINED, DRSYS_TYPE_SIGNED_INT, classname},\
         {2, -3, R, 0, typename},\
         {3, sizeof(ULONG), SYSARG_INLINED, DRSYS_TYPE_UNSIGNED_INT},\
     }

syscall_info_t syscall_SetInformationKey_info[] = {
    {{0,0},"NtSetInformationKey.KeyWriteTimeInformation", OK, RNTST, 4,
         ENTRY_SetInformationKey("KeyWriteTimeInformation", "_KEY_WRITE_TIME_INFORMATION")
    },
    {{0,0},"NtSetInformationKey.KeyWow64FlagsInformation", OK, RNTST, 4,
         ENTRY_SetInformationKey("KeyWow64FlagsInformation", "_KEY_WOW64_FLAGS_INFORMATION")
    },
    {{0,0},"NtSetInformationKey.KeyControlFlagsInformation", OK, RNTST, 4,
         ENTRY_SetInformationKey("KeyControlFlagsInformation", "KEY_CONTROL_FLAGS_INFORMATION")
    },
    {{0,0},"NtSetInformationKey.KeySetVirtualizationInformation", OK, RNTST, 4,
         ENTRY_SetInformationKey("KeySetVirtualizationInformation", "_KEY_SET_VIRTUALIZATION_INFORMATION")
    },
    {{0,0},"NtSetInformationKey.KeySetDebugInformation", OK, RNTST, 4,
         ENTRY_SetInformationKey("KeySetDebugInformation", NULL)
    },
    {{0,0},"NtSetInformationKey.KeySetHandleTagsInformation", OK, RNTST, 4,
         ENTRY_SetInformationKey("KeySetHandleTagsInformation", NULL)
    },
    {SECONDARY_TABLE_ENTRY_MAX_NUMBER},
    {{0,0},"NtSetInformationKey.UNKNOWN", OK, RNTST, 4,
         ENTRY_SetInformationKey(NULL, NULL)
    },
};

#define ENTRY_SetInformationObject(classname, typename)\
     {\
         {0, sizeof(HANDLE), SYSARG_INLINED, DRSYS_TYPE_HANDLE},\
         {1, sizeof(OBJECT_INFORMATION_CLASS), SYSARG_INLINED, DRSYS_TYPE_SIGNED_INT, classname},\
         {2, -3, R, 0, typename},\
         {3, sizeof(ULONG), SYSARG_INLINED, DRSYS_TYPE_UNSIGNED_INT},\
     }

syscall_info_t syscall_SetInformationObject_info[] = {
    {{0,0},"NtSetInformationObject.ObjectBasicInformation", OK, RNTST, 4,
         ENTRY_SetInformationObject("ObjectBasicInformation", "_PUBLIC_OBJECT_BASIC_INFORMATION")
    },
    {SECONDARY_TABLE_SKIP_ENTRY},
    {{0,0},"NtSetInformationObject.ObjectTypeInformation", OK, RNTST, 4,
         ENTRY_SetInformationObject("ObjectTypeInformation", "_PUBLIC_OBJECT_TYPE_INFORMATION")
    },
    {SECONDARY_TABLE_ENTRY_MAX_NUMBER},
    {{0,0},"NtSetInformationObject.UNKNOWN", OK, RNTST, 4,
         ENTRY_SetInformationObject(NULL, NULL)
    },
};
