/*!
 * \file        Types.h
 * \brief       Type definitions of the EOSPRINT 2 API
 *
 * \author      EOS GmbH, Robert-Stirling-Ring 1, 82152 Krailling, Germany
 * \copyright   (c) EOS GmbH 2018
 *              All rights reserved
 */

#ifndef EOS_EOSPRINT_API_TYPES_H
#define EOS_EOSPRINT_API_TYPES_H

#ifndef EOS_MT_UNDISCLOSED_MACHINES
#   define EOS_MT_UNDISCLOSED_MACHINES
#endif

#ifndef EOS_MT_ABANDONED_MACHINES
#    define EOS_MT_ABANDONED_MACHINES
#endif

#include <stdint.h>

/* BEGIN OF SUBSECTION: PRIMITIVE TYPES */

/*! Unsigned byte type (8 bits) */
typedef uint8_t             EOS_BYTE;

/*! Signed integer type with width of exactly 32 bits */
typedef int32_t             EOS_INT;

/*! Unsigned integer type with width of exactly 32 bits */
typedef uint32_t            EOS_UINT;

/*! Signed integer type with width of exactly 64 bits */
typedef int64_t             EOS_LONG;

/*! Unsigned integer type with width of exactly 64 bits */
typedef uint64_t            EOS_ULONG;

/*! Single precision floating point type */
typedef float               EOS_FLOAT;

/*! Double precision floating point type */
typedef double              EOS_DOUBLE;

/*! Type for UTF-16 character representation */
typedef wchar_t             EOS_WCHAR;

/*! Default immutable string type (UTF-16) */
typedef EOS_WCHAR const *   EOS_CWSTR;

/*! Default mutable string type (UTF-16) */
typedef EOS_WCHAR *         EOS_WSTR;

/*! Type for all kinds of EOSPRINT 2 API handles */
typedef uint64_t            EOS_HANDLE;

/*! Type for EOSPRINT 2 session handles */
typedef EOS_HANDLE          EOS_SESSION_HANDLE;

/*! Type for EOSPRINT 2 file transfer handles */
typedef EOS_HANDLE          EOS_FILETRANSFER_HANDLE;

/*! Type for EOSPRINT 2 file creation handles */
typedef EOS_HANDLE          EOS_FILECREATION_HANDLE;

/*! Type for EOSPRINT 2 arbitrary data handles (e.g. custom data ptr) */
typedef EOS_HANDLE          EOS_DATA_HANDLE;

/*! Type for EOSPRINT 2 file handles */
typedef EOS_HANDLE          EOS_FILE_HANDLE;

/*! Integral type to represent micrometers */
typedef EOS_INT             EOS_MICROMETER;

/*! Floating point type to represent millimeters */
typedef EOS_DOUBLE          EOS_MILLIMETER;

/*! Type to represent an index into a layer collection, first item has index 1 */
typedef EOS_ULONG           EOS_LAYERINDEX;

/*! Integral type to count milliseconds */
typedef EOS_ULONG           EOS_MILLISECS;

/*! Type to count microseconds*/
typedef EOS_DOUBLE          EOS_MICROSECS;

/*! Integral type to represent timestamps as milliseconds since epoch (1970-01-01 00:00:00.000000) */
typedef EOS_MILLISECS       EOS_TIMESTAMP; // ms since epoch

/*! Size type */
typedef EOS_ULONG           EOS_SIZET;

/*! Type for bit masks describing vector type categories such as hatching & contours
 * The following constants are defined: EOS_VT_HATCHING, EOS_VT_PART_BOUNDARY, EOS_VT_JUMPS
 */
typedef EOS_UINT            EOS_VECTORTYPES;

/*! Type for bit masks describing features that can be licensed separately for each Exposure Set */
typedef EOS_UINT            EOS_LICENSABLE_FEATURES;

/*! Type for part ids - see OpenJob format documentation on attribute /openjob/parts/part/@id */
typedef EOS_INT             EOS_PARTID;

/*! Generic flags type with 64 bits */
typedef EOS_ULONG           EOS_FLAGS64;

/*! Generic flags type with 64 bits (deprecated) */
typedef EOS_FLAGS64         EOS_FLAGS;

/*! Generic flags type with 32 bits */
typedef EOS_UINT            EOS_FLAGS32;

/*! Laser/scanner capabilities */
typedef EOS_FLAGS64         EOS_LS_CAPABILITIES;

/*! Information on licenses on the machine that control exposure features */
typedef EOS_FLAGS32         EOS_MACHINE_LICENSE_INFOS;

/*! Specifies the type of eos variant */
typedef enum
{
    /*! For signed int type; 32 bits */
    EOS_VAR_INT = 0,

    /*! For double precision floating point type; 64 bits */
    EOS_VAR_DOUBLE = 1,

    /*! For const wide char string type */
    EOS_VAR_CWSTR = 2,

} EOS_VARIANT_TYPE;

/*! Possible licensing systems */
typedef enum
{
    /*! The WIBU CodeMeter licensing system */
    EOS_LS_CODEMETER = 1

} EOS_LICENSING_SYSTEM;

/*! Possible licensings in the codemeter licensing system */
typedef enum
{
    /*! Licensing as defined by environment variable EOS_LICENSING or default licensing (CmStick) if
    environment variable is not set or invalid. */
    EOS_CL_DEFAULT = 0,

    /*! CmStick licensing (firmcode: 101328) */
    EOS_CL_CMSTICK = 1,

    /*! Universal licensing (firmcode: 6000856) */
    EOS_CL_UNIVERSAL = 2

} EOS_CODEMETER_LICENSING;

/*! Type to encode the Smart Fusion compatibility, the higher the number the more recent */
typedef enum
{
    /*! The machine does not support Smart Fusion */
    EOS_SFC_NOT_SUPPORTED = 0,

    /*! The Material Set does not request Smart Fusion */
    EOS_SFC_NOT_REQUESTED = EOS_SFC_NOT_SUPPORTED,

    /*! The machine supports at least EOS_SFC_EXPOSURE_OT_1_11, but the exact level is unknown.
     *  The compatibility of a Material Set is always known. */
    EOS_SFC_EXPOSURE_OT_UNKNOWN = 1,

    /*! Either the machine supports or the Material Set requests that
     *  Smart Fusion is available and EOSTATE Exposure OT version is at least 1.11 */
    EOS_SFC_EXPOSURE_OT_1_11 = 2,

    /*! Either the machine supports or the Material Set requests that
     *  Smart Fusion is available and EOSTATE Exposure OT version is at least 1.15 */
    EOS_SFC_EXPOSURE_OT_1_15 = 3,

    /*! Either the machine supports or the Material Set requests that
     *  Smart Fusion is available and EOSTATE Exposure OT version is at least 1.24 */
    EOS_SFC_EXPOSURE_OT_1_24 = 4

} EOS_SMART_FUSION_COMPATIBILITY;


/*! Type to represent boolean values (i.e. true or false) */
typedef enum
{
    /*! Boolean value of false */
    EOS_FALSE = 0,

    /*! Boolean value of true */
    EOS_TRUE = 1

} EOS_BOOL;

/*! Type of the return codes for many EOSPRINT 2 API functions */
typedef enum
{
    /*! Everything went well */
    EOS_ERR_NO_ERROR = 1,

    /*! An invalid argument was passed, e.g., a null pointer where not allowed */
    EOS_ERR_ARGUMENT_INVALID = 2,

    /*! A string could not be converted to another encoding */
    EOS_ERR_ENCODING_CONVERSION_ERROR = 3,

    /*! A string contains illegal characters. E.g., characters not in codepage Windows-1252 are illegal for task creation in general. In addition, non-ASCII characters are illegal for M100/P120 tasks. */
    EOS_ERR_STRING_CONTAINS_ILLEGAL_CHARS = 4,

    /*! A file path contains illegal characters.
     * Generally all characters not in codepage Windows-1252 are currently illegal in file paths.
     * For models M100, P110, and P120 all non-ASCII characters are illegal. */
    EOS_ERR_PATH_CONTAINS_ILLEGAL_CHARS = 5,

    /*! A required license is missing */
    EOS_ERR_MISSING_LICENSE = 6,

    /*! Empty, nothing to get */
    EOS_ERR_EMPTY = 7,

    /*! Nothing found, no match */
    EOS_ERR_NO_MATCH = 8,

    /*! The given path is not a directory (e.g., because it does not exist or because it's a file) */
    EOS_ERR_NOT_A_DIRECTORY = 9,

    /*! The specified file was not found */
    EOS_ERR_FILE_NOT_FOUND = 10,

    /*! The extension is invalid; e.g. extension does not fit to the requirements (not: file not found) */
    EOS_ERR_ILLEGAL_FILE_EXTENSION = 11,

    /*! A file system level error occurred, e.g., because of missing access rights (not: file not found), disk out of space  */
    EOS_ERR_FILESYSTEM_IO_ERROR = 12,

    /*! A network level error occurred, e.g., because missing network, missing server access rights, wrong network address */
    EOS_ERR_NETWORK_IO_ERROR = 13,

    /*! An operation could not be performed because memory allocating failed */
    EOS_ERR_OUT_OF_MEMORY = 14,

    /*! A machine type is not supported */
    EOS_ERR_MACHINE_NOT_SUPPORTED = 15,

    /*! An operation could not be performed, because the subject is (already) in idle state; e.g. aborting something which is not running */
    EOS_ERR_IDLE = 16,

    /*! An operation could not be performed, because a competing operation is already running */
    EOS_ERR_NOT_IDLE = 17,

    /*! An invalid file transfer handle was passed */
    EOS_ERR_FILETRANSFER_INVALID = 18,

    /*! A file transfer is running or pending (queued) */
    EOS_ERR_FILETRANSFER_RUNNING = 19,

    /*! No running or queued file transfer (nothing to abort)*/
    EOS_ERR_FILETRANSFER_NOT_RUNNING = 20,

    /*! The file transfer was aborted by the user */
    EOS_ERR_FILETRANSFER_ABORTED = 21,

    /*! The file transfer failed, e.g., caused by transmission failure, protocol failure, non-gracefully disconnection */
    EOS_ERR_FILETRANSFER_FAILED = 22,

    /*! An invalid session handle was passed */
    EOS_ERR_SESSION_INVALID = 23,

    /*! The maximum of simultaneously allowed sessions was already reached */
    EOS_ERR_TOO_MANY_SESSIONS = 24,

    /*! A build time estimation is running or pending */
    EOS_ERR_BUILDTIME_ESTIMATION_RUNNING = 25,

    /*! No running build time estimation (nothing to abort)*/
    EOS_ERR_BUILDTIME_ESTIMATION_NOT_RUNNING = 26,

    /*! The build time estimation was aborted by the user */
    EOS_ERR_BUILDTIME_ESTIMATION_ABORTED = 27,

    /*! A preview is running or pending */
    EOS_ERR_PREVIEW_RUNNING = 28,

    /*! No running preview (nothing to abort)*/
    EOS_ERR_PREVIEW_NOT_RUNNING = 29,

    /*! The preview was aborted by the user */
    EOS_ERR_PREVIEW_ABORTED = 30,

    /*! A non-existent layer index was specified */
    EOS_ERR_LAYER_INVALID = 31,

    /*! A non-existent layer index was specified.
     * EOS_ERR_PREVIEW_LAYER_INVALID was deprecated with EOSPRINT 2.13
     * and was superseded by EOS_ERR_LAYER_INVALID. */
    EOS_ERR_PREVIEW_LAYER_INVALID = EOS_ERR_LAYER_INVALID,

    /*! An invalid z-height was specified */
    EOS_ERR_PREVIEW_ZHEIGHT_INVALID = 32,

    /*! A task creation is running or pending (queued) */
    EOS_ERR_TASK_CREATION_RUNNING = 33,

    /*! No running task creation (nothing to abort)*/
    EOS_ERR_TASK_CREATION_NOT_RUNNING = 34,

    /*! The task creation was aborted by the user */
    EOS_ERR_TASK_CREATION_ABORTED = 35,

    /*! Some information could not be written to the task */
    EOS_ERR_TASK_WRITE_FAILED = 36,

    /*! The task is invalid, e.g., because it contains invalid/inconsistent data */
    EOS_ERR_TASK_INVALID = 37,

    /*! The task is corrupt, i.e. it cannot be read, because its format is completely unrecognized */
    EOS_ERR_TASK_CORRUPT = 38,

    /*! No job loaded; operations which need a loaded job could not be performed */
    EOS_ERR_JOB_NOT_LOADED = 39,

    /*! No machine configuration loaded; operations which need a loaded machine configuration could not be performed */
    EOS_ERR_CONFIG_NOT_LOADED = 40,

    /*! An operation could not be performed, e.g., because machine configuration is invalid: serial number differs from machine or outdated or contains inconsistent data */
    EOS_ERR_CONFIG_INVALID = 41,

    /*! The machine configuration is corrupt, e.g., because files cannot be opened or because their format is not recognized */
    EOS_ERR_CONFIG_CORRUPT = 42,

    /*! One of the required files in the configuration directory is missing */
    EOS_ERR_ILLEGAL_CONFIG_DIR = 43,

    /*! The OpenJob was not found */
    EOS_ERR_OPENJOB_NOT_FOUND = 44,

    /*! The OpenJob is of a not supported version */
    EOS_ERR_OPENJOB_NOT_SUPPORTED = 45,

    /*! The OpenJob contains invalid data, e.g., a reference to an undefined positioning point */
    EOS_ERR_OPENJOB_INVALID = 46,

    /*! The OpenJob is corrupt, i.e. its format is completely unrecognized (e.g., not an XML or not well-formed) */
    EOS_ERR_OPENJOB_CORRUPT = 47,

    /*! The OpenJz is invalid, e.g., because it can be opened and extracted, but its internal file structure is unrecognized */
    EOS_ERR_OPENJZ_INVALID = 48,

    /*! The OpenJz is corrupt, i.e. it cannot be extracted */
    EOS_ERR_OPENJZ_CORRUPT = 49,

    /*! The .eospar file was not found */
    EOS_ERR_EOSPAR_NOT_FOUND = 50,

    /*! The .eospar file is of a not supported version */
    EOS_ERR_EOSPAR_UNSUPPORTED = 51,

    /*! The .eospar file contains invalid/inconsistent data */
    EOS_ERR_EOSPAR_INVALID = 52,

    /*! The .eospar file cannot be read, because its format is completely unrecognized */
    EOS_ERR_EOSPAR_CORRUPT = 53,

    /*! A directory that was expected to be empty was not */
    EOS_ERR_DIRECTORY_NOT_EMPTY = 54,

    /*! Waiting timed out */
    EOS_ERR_WAIT_TIMED_OUT = 55,

    /*! The provided code page is invalid */
    EOS_ERR_INVALID_CODE_PAGE = 56,

    /*! A string was not correctly encoded as unicode */
    EOS_ERR_INCORRECT_ENCODING = 57,

    /*! A specified part file was not found; e.g., a referenced part file in an openjob */
    EOS_ERR_PART_FILE_NOT_FOUND = 58,

    /*! The machine software version does either not fit the minimum requirements for a given machine or is too high */
    EOS_ERR_MACHINE_VERSION_NOT_SUPPORTED = 59,

    /*! An invalid file transfer handle was passed */
    EOS_ERR_FILECREATION_INVALID = 60,

    /*! No running or queued file creation (nothing to abort)*/
    EOS_ERR_FILECREATION_NOT_RUNNING = 61,

    /*! The file creation was aborted by the user */
    EOS_ERR_FILECREATION_ABORTED = 62,

    /*! The given stl file cannot be loaded */
    EOS_ERR_STL_INVALID = 63,

    /*! The requested deformation operation on a mesh could not be performed */
    EOS_ERR_DEFORMATION_ERROR = 64,

    /*! A numerical input value was outside the domain of an operation */
    EOS_ERR_DOMAIN_ERROR = 65,

    /*! An overflow occured while processing the result of an operation */
    EOS_ERR_OVERFLOW_ERROR = 66,

    /*! The given stl file is (partially) below the platform, i.e. it contains negative z coordinates */
    EOS_ERR_STL_BELOW_PLATFORM = 67,

    /*! The given stl file is (partially) outside the build volume of the machine */
    EOS_ERR_STL_OUTSIDE_BUILD_VOLUME = 68,

    /*! The given stl file is (partially) above the given job height */
    EOS_ERR_STL_ABOVE_JOB_HEIGHT = 69,

    /*! The given stl file contains negative coordinates, which is not supported by the requested action */
    EOS_ERR_STL_CONTAINS_NEGATIVE_COORDINATES = 70,

    /*! An invalid part id was supplied */
    EOS_ERR_INVALID_PART_ID = 71,

    /*! An invalid Exposure Set name was supplied */
    EOS_ERR_INVALID_EXPOSURE_SET_NAME = 72,

    /*! A feature, either requested by the OpenJob or by an API call is not supported by the target machine */
    EOS_ERR_FEATURE_NOT_SUPPORTED = 73,

    /*! The version of the task or one of its dependencies is not supported for this feature */
    EOS_ERR_TASK_UNSUPPORTED_VERSION = 74,

    /*! After applying one or several adjustments (job- or machine-specific) to an adjustable building process parameter
        its value now violates the specified min/max threshold */
    EOS_ERR_ADJUSTED_VALUE_VIOLATES_THRESHOLD = 75,

    /*! The currently loaded material set does not contain an adjustable process parameter with the specified id */
    EOS_ERR_ADJUSTMENT_ID_INVALID = 76,

    /*! The specified adjustment type is not enabled for the specified adjustable process parameter.
        (e.g., when trying to get a job specific adjustment for an adjustable process parameter that only allows machine specific
       adjustments) */
    EOS_ERR_ADJUSTMENT_TYPE_NOT_ENABLED = 77,

    /*! The specified adjustment was valid, but it is not supported by the machine to which it applies, because the machine
        is running an old version of EOSYSTEM. */
    EOS_ERR_ADJUSTMENT_NOT_SUPPORTED_BY_MACHINE = 78,

    /*! The specfied adjustment cannot be applied, because the EOSPRINT API only allows adjustment of process parameters that have an
        effect on exposure. */
    EOS_ERR_ADJUSTMENT_HAS_NO_EFFECT_ON_EXPOSURE = 79,

    /*! The requested operation is not (yet) implemented */
    EOS_ERR_NOT_IMPLEMENTED = 80,

    /*! A necessary DLL could not be loaded, because it was not found or because necessary licenses are missing */
    EOS_ERR_LOADING_DLL_FAILED = 81,

    /*! A constraint has been violated */
    EOS_ERR_CONSTRAINT_VIOLATED = 82,

    /*! Defocus has been requested but is not supported by the machine */
    EOS_ERR_DEFOCUS_NOT_SUPPORTED = 83,

    /*! A protocol required to run smoothly did not complete successfully. */
    EOS_ERR_PROTOCOL_ERROR = 84,

    /*! A file creation is running or pending (queued) */
    EOS_ERR_FILECREATION_RUNNING = 85,

    /*! A file handle was supplied but was found to be invalid */
    EOS_ERR_FILE_HANDLE_INVALID = 86,

    /*! Requested layer does not exist */
    EOS_ERR_INVALID_LAYER = 87,

    /*! A faulty mesh caused an unfixable error during slicing */
    EOS_ERR_FAULTY_TRIANGLE_MESH = 88,

    /*! An input file handle is used for writing */
    EOS_ERR_CANNOT_WRITE_TO_INPUT_FILE = 89,

    /*! An API call has been issued which is only valid within LayerStart/LayerEnd */
    EOS_ERR_NO_LAYER = 90,

    /*! An API call has been issued before a laser/scanner unit was set */
    EOS_ERR_NO_SCANNER = 91,

    /*! An API call has been issued which is only valid within ObjectStart/ObjectEnd */
    EOS_ERR_NO_PART = 92,

    /*! An API call has been issued which is not valid within LayerStart/LayerEnd */
    EOS_ERR_LAYER = 93,

    /*! An API call has been issued which is not valid within ObjectStart/ObjectEnd */
    EOS_ERR_PART = 94,

    /*! The requested laser/scanner unit is not valid in the current system */
    EOS_ERR_LSU_INVALID = 95,

    /*! The requested operation is not supported by the active hardware modules */
    EOS_ERR_UNSUPPORTED = 96,

    /*! The requested coordinate is not supported by the active hardware modules */
    EOS_ERR_BAD_COORDINATE = 97,

    /*! The requested command sequence is not valid */
    EOS_ERR_COMMAND_NOT_POSSIBLE = 98,

    /*! The job or configuration cannot be unloaded as files are still in use */
    EOS_ERR_CANNOT_UNLOAD = 99,

    /*! The collection or array is of insufficient size to add all the necessary information */
    EOS_ERR_INSUFFICIENT_SIZE = 100,

    /*! The desired operation is not permitted with the current setup */
    EOS_ERR_NOT_ALLOWED = 101,

    /*! The calling application has not successfully authenticated */
    EOS_ERR_CLIENT_NOT_AUTHENTICATED = 102,

    /*! A used exposure set has invalid parameters. See log file for further information */
    EOS_ERR_INVALID_EXPOSURE_PARAMETERS = 103,

    /*! Importing a part with an external importer failed */
    EOS_ERR_RESOURCECACHE_EXTERNAL_IMPORT_ERROR = 104,

    /*! Mesh generation using an external importer failed */
    EOS_ERR_RESOURCECACHE_EXTERNAL_MESH_ERROR = 105,

    /*! Contour extraction using an external importer failed */
    EOS_ERR_RESOURCECACHE_EXTERNAL_CONTOUR_ERROR = 106,

    /*! Warning reported by external importer while loading file */
    EOS_ERR_RESOURCECACHE_EXTERNAL_IMPORT_WARNING = 107,

    /*! Warning reported by external importer during mesh generation */
    EOS_ERR_RESOURCECACHE_EXTERNAL_MESH_WARNING = 108,

    /*! Warning reported by external importer during contour generation */
    EOS_ERR_RESOURCECACHE_EXTERNAL_CONTOUR_WARNING = 109,

    /*! The file is malformed, e.g., it contains invalid XML elements */
    EOS_ERR_FILE_MALFORMED = 110,

    /*! The parameter does not already exist within the material set */
    EOS_ERR_PARAMETER_DOES_NOT_EXIST= 111,

    /*! The parameter is not of access mode read/write */
    EOS_ERR_PARAMETER_INACCESSIBLE = 112,

    /*! The value is invalid (e.g. wrong type or outside min/max value range) */
    EOS_ERR_INVALID_PARAMETER_VALUE = 113,

    /*! An instance with the same name/id already exists (e.g. an exposure set with the same name) */
    EOS_ERR_ID_CONFLICT = 114,

    /*! The file cannot be opened because its format is not recognized */
    EOS_ERR_FILE_CORRUPT = 115,

    /*! The file could not be uploaded because the available disk space on the machine would fall below the RunSpace threshold */
    EOS_ERR_INSUFFICIENT_DISK_SPACE = 116,

    /*! The requested operation cannot be performed, because the input data format is not supported. */
    EOS_ERR_DATA_FORMAT_UNSUPPORTED = 117,

    /*! The .eossim file contains invalid/inconsistent data */
    EOS_ERR_EOSSIM_INVALID = 118,

    /*! The .eossim file cannot be read, because its format is completely unrecognized */
    EOS_ERR_EOSSIM_CORRUPT = 119,

    /*! An internal error state was reached, please consider reporting this to EOS support */
    EOS_ERR_INTERNAL_ERROR = 32766,

    /*! An unknown exception was caught, please consider reporting this to EOS support */
    EOS_ERR_UNEXPECTED_ERROR = 32767

} EOS_ERROR;

/*! State of a TaskGen operation (task creation, preview creation, build time estimation) */
typedef enum
{
    /*! Operation has not been started yet */
    EOS_TG_IDLE = 0,

    /*! Operation is ongoing */
    EOS_TG_RUNNING = 1,

    /*! Operation is suspended and could be resumed (note, that currently none of the TaskGen operations supports suspend/resume) */
    EOS_TG_SUSPENDED = 2,

    /*! The operation was finished successfully */
    EOS_TG_COMPLETED = 3,

    /*! The operation terminated with an error, check the log output or use Eos_SetEventCallback() to get details. */
    EOS_TG_FAILED = 4,

    /*! The operation was aborted by the client */
    EOS_TG_ABORTED = 5

} EOS_TASK_STATE;

typedef enum
{
    /*! No inert gas atmosphere */
    EOS_IG_NONE = 0,

    /*! Gas type for builds under nitrogen atmosphere */
    EOS_IG_NITROGEN = 1,

    /*! Gas type for builds under argon atmosphere */
    EOS_IG_ARGON = 2

} EOS_INERT_GAS;

/*! State of an asynchronous operation */
typedef enum
{
    /*! This is not a state, but will be returned by respective functions, if an invalid handle is passed */
    EOS_TS_INVALID_HANDLE = 0,

    /*! Operation is running */
    EOS_TS_RUNNING = 1,

    /*! Operation is queued, but not yet running */
    EOS_TS_QUEUED = 2,

    /*! Operation is completed */
    EOS_TS_COMPLETED = 3,

    /*! Operation failed - register an event callback (see Eos_SetEventCallback()) for more details */
    EOS_TS_FAILED = 4,

    /*! Operation was aborted by client */
    EOS_TS_ABORTED = 5

} EOS_ASYNC_OPERATION_STATUS;

/*! State of a file transfer asynchronous operation */
typedef EOS_ASYNC_OPERATION_STATUS EOS_TRANSFER_STATUS;

/*! State of a file creation asynchronous operation */
typedef EOS_ASYNC_OPERATION_STATUS EOS_FILECREATION_STATUS;

/*! Search mode for finding layers */
typedef enum
{
    /*! No search - the layer must be specified exactly */
    EOS_SM_EXACT = 0,

    /*! Search for the nearest layer satisfying the search criteria */
    EOS_SM_NEAREST = 1,

    /*! Search for the next layer satisfying the search criteria with z-position equal or lower than the one specified */
    EOS_SM_DOWNWARD = 2,

    /*! Search for the next layer satisfying the search criteria with z-position equal or higher than the one specified */
    EOS_SM_UPWARD = 3

} EOS_SEARCHMODE;

/*! Mode in which the task is created */
typedef enum
{
    /*! Mode for creation of a regular task that contains several layers. */
    EOS_TM_REGULAR = 0,

    /*! Mode for creation of a single layer task (e.g. for Position Contour) which may be treated differently by the
     * machine software. E.g., EOSYSTEM makes single layer tasks available through the setup wizard. */
    EOS_TM_SINGLELAYER = 1

} EOS_TASKMODE;

/*! Defines the type of an exposure vector in the preview or an exposure step in the material set.
 *  Some exposure types in the material set might result in different types in the preview. This case will be documented below.
 */
typedef enum
{
    /*! Part boundary (not an actual exposure) */
    EOS_EXPOSURE_TYPE_PARTBOUNDARY = 0,

    /*! Downskin (infill) exposure */
    EOS_EXPOSURE_TYPE_DOWNSKIN = 1,

    /*! Infill exposure */
    EOS_EXPOSURE_TYPE_INFILL = 2,

    /*! Upskin (infill) exposure */
    EOS_EXPOSURE_TYPE_UPSKIN = 3,

    /*! Downskin contour exposure */
    EOS_EXPOSURE_TYPE_DOWNSKIN_CONTOUR = 4,

    /*! Contour exposure */
    EOS_EXPOSURE_TYPE_CONTOUR = 5,

    /*! Upskin contour exposure */
    EOS_EXPOSURE_TYPE_UPSKIN_CONTOUR = 6,

    /*! Edge exposure */
    EOS_EXPOSURE_TYPE_EDGE = 7,

    /*! Support outline (not an actual exposure) */
    EOS_EXPOSURE_TYPE_SUPPORT = 8,

    /*! Hatch gap (not an actual exposure) */
    EOS_EXPOSURE_TYPE_HATCH_GAP = 9,

    /*! Simple contour - does not distinguish up/down/in */
    EOS_EXPOSURE_TYPE_CONTOUR_SIMPLE = 10,

    /*! Jump between two points (also not an actual exposure) */
    EOS_EXPOSURE_TYPE_JUMP = 11,

    /*! Support exposure */
    EOS_EXPOSURE_TYPE_SUPPORT_EXPOSED = 12,

    /*! Material set only, will result in preview types EOS_EXPOSURE_TYPE_CONTOUR,
     *  EOS_EXPOSURE_TYPE_DOWNSKIN_CONTOUR, EOS_EXPOSURE_TYPE_UPSKIN_CONTOUR */
    EOS_EXPOSURE_TYPE_UPDOWNCONTOUR = 13,

    /*! Material set only. No exposure vector will ever be of this exposure type.
     *  Exposure step Core has a child exposure step (usually a hatch type) which defines the type of exposure vectors generated by Core. */
    EOS_EXPOSURE_TYPE_CORE = 14,

    /*! Material set only. No exposure vector will ever be of this exposure type.
     *  Exposure step Hatch has a child exposure step (usually a hatch type) which defines the type of exposure vectors generated by Hatch. */
    EOS_EXPOSURE_TYPE_HATCH = 15,

    /*! Indicates that original exposure types and vectors are not disclosed in the preview */
    EOS_EXPOSURE_TYPE_HIDDEN = 16,

} EOS_EXPOSURE_TYPE;

/*! Specifies the type of a machine's building volume */
typedef enum
{
    /*! For cuboidal building volumes */
    EOS_VOL_CUBOID = 1,

    /*! For cylindrical building volumes */
    EOS_VOL_CYLINDER = 2

} EOS_VOLUME_TYPE;

/*! Specifies the task type for export purposes. Use EOS_TF_RAW for raw task export, otherwise EOS_TF_REGULAR. */
typedef enum
{
    /*! Standard task */
    EOS_TF_REGULAR = 0,

    /*! Raw task */
    EOS_TF_RAW = 1

} EOS_TASKFORMAT;

/*! Type represents the module id (for events and event callbacks) */
typedef EOS_INT             EOS_MODULE_ID;

/*! Type represents the module id (for events and event callbacks) */
typedef EOS_INT             EOS_EVENT_ID;

/*! Specifies the severity level (for events and event callbacks) */
typedef enum
{
    /*! For warning severity */
    EOS_SEV_WARNING = 0,

    /*! For error severity */
    EOS_SEV_ERROR = 1,

    /*! For fatal severity */
    EOS_SEV_FATAL = 2

} EOS_SEVERITY;

/*! Specifies the display options (for events and event callbacks) */
typedef enum
{
    /*! For no displaying */
    EOS_DSP_NONE = 0,

    /*! For modal displaying */
    EOS_DSP_MODAL_WINDOW = 1,

    /*! For balloon displaying */
    EOS_DSP_BALLOON = 2

} EOS_DISPLAY_OPTIONS;


/*! Mode in which slicing is performed */
typedef enum
{
    /*! Standard slicing mode. */
    EOS_SM_REGULAR = 0,

    /*! Slicing mode to be used for support geometry. */
    EOS_SM_SUPPORT = 1

} EOS_SLICING_MODE;

/*! Specifies the compression level to use for task data files */
typedef enum
{
    EOS_CL_NO_COMPRESSION = 10,

    EOS_CL_BEST_SPEED = 1,

    EOS_CL_HIGH_SPEED = 2,

    EOS_CL_NORMAL = 3,

    EOS_CL_HIGH_COMPRESSION = 4,

    EOS_CL_BEST_COMPRESSION = 5,

    EOS_CL_DEFAULT_COMPRESSION = 0

} EOS_COMPRESSION_LEVEL;

/*! Specifies beam shape types */
typedef enum
{
    /*! Standard beam shape type, only one supported for now */
    EOS_BEAMSHAPE_STANDARD = 0

} EOS_BEAMSHAPE;

/*! Selector for image sources */
typedef enum
{
    /*! Prediction of an OpticalTomography integral image.
     * The resulting image has color space EOS_ICS_GREYSCALE
     * and channel format EOS_ICF_FLOAT_32. */
    EOS_IS_OT_PREDICTION

} EOS_IMAGE_SOURCE;


/*! Color space of an image,
 *  this includes channel count and channel order */
typedef enum
{
    /*! Single channel */
    EOS_ICS_GREYSCALE,

    /*! Three channels (blue, green, red) */
    EOS_ICS_BGR,

    /*! Four channels (red, green, blue, alpha) */
    EOS_ICS_RGBA

} EOS_IMAGE_COLOR_SPACE;

/*! Data type of the image pixel */
typedef enum
{
    /*! 8 bits unsigned integer */
    EOS_ICF_UNSIGNED_8,

    /*! 8 bits signed integer */
    EOS_ICF_SIGNED_8,

    /*! 16 bits unsigned integer */
    EOS_ICF_UNSIGNED_16,

    /*! 16 bits signed integer */
    EOS_ICF_SIGNED_16,

    /*! 32 bits unsigned integer */
    EOS_ICF_UNSIGNED_32,

    /*! 32 bits signed integer */
    EOS_ICF_SIGNED_32,

    /*! 16 bits half-precision floating point */
    EOS_ICF_FLOAT_16,

    /*! 32 bits single-precision floating point */
    EOS_ICF_FLOAT_32,

    /*! 64 bits double-precision floating point */
    EOS_ICF_FLOAT_64

} EOS_IMAGE_CHANNEL_FORMAT;

/* END OF SUBSECTION: PRIMITIVE TYPES */

/* BEGIN OF SUBSECTION: COMPOSITE TYPES */

/*! Specifies the api options type (for fine-grained control over settings), 32 Byte */
typedef struct
{
    /*! Path to logging properties file (set to NULL to disable logging) */
    EOS_CWSTR               pathToLoggingProperties;

    /*!
     * Maximum permitted number of simultaneously openened files for this process.
     * Will be ignored if 0u.
     * Default is 512u, maximum is platform dependent, hard maximum is 8192u.
     * For more information see documentation of _setmaxstdio().
     * In case this value exceeds the platform maximum then API initialization will fail
     * with EOS_ERR_ARGUMENT_INVALID and the errno is logged.
     */
    EOS_UINT                maxOpenFiles;

    /*! Codemeter licensing behavior (selects the firmcode to use) */
    EOS_CODEMETER_LICENSING codemeterLicensing;

    /*! Time in minutes after which an active license will be automatically be freed.
        Can be 0 to use the system default, otherwise will be the max of the given value and 6 min.
        License keepalive time is the minimum of 15 min or (cleanup time - 5 min) */
    EOS_UINT licenseCleanupTime_min;

    /*! Memory block reserved for future extensions */
    EOS_BYTE                reserved[12];

} EOS_API_OPTIONS;

/*! Specifies the variants used by EOS_INFO_ITEM (for events and event callbacks) */
typedef union
{
    EOS_INT intVariant;

    EOS_DOUBLE doubleVariant;

    EOS_CWSTR cWStrVariant;

} EOS_VARIANT;

/*! Specifies the info item type (for events and event callbacks) */
typedef struct
{
    /*! Key of the item */
    EOS_CWSTR           key;

    /*! Type of the contained value */
    EOS_VARIANT_TYPE    type;

    /*! Value container */
    EOS_VARIANT         value;

    /*! Memory block reserved for future extensions */
    EOS_BYTE            reserved[8];

} EOS_INFO_ITEM;

typedef struct
{
    /*! The ID of the module where the event is coming from */
    EOS_MODULE_ID       moduleId;

    /*! The ID of the event */
    EOS_EVENT_ID        eventId;

    /*! The error code which is reported by the event */
    EOS_ERROR           errorCode;

    /*! The severity level of the event */
    EOS_SEVERITY        severity;

    /*! The name of the event */
    EOS_CWSTR           name;

    /*! The display options meant to the event */
    EOS_DISPLAY_OPTIONS displayOptions;

    /*! The count of the info items of the event */
    EOS_UINT            infoItemCount;

    /*! Array of info items of the event.
     * This is not EOS_INFO_ITEM const * due to an oversight. Eventually, the const will be added. */
    EOS_INFO_ITEM *     infoItems;

    /*! Memory block reserved for future extensions */
    EOS_BYTE            reserved[24];

} EOS_EVENT;


/*! A structure holding a 2D floating point */
typedef struct
{
    /*! X coordinate of the point in mm, starting at the left edge of the platform */
    EOS_MILLIMETER      x_mm;

    /*! Y coordinate of the point in mm, starting at the front edge of the platform */
    EOS_MILLIMETER      y_mm;

} EOS_POINT2;

/*! A structure holding an axis aligned bounding box. */
typedef struct
{
    /*! Min X coordinate of the bounding box */
    EOS_MILLIMETER      minX_mm;

    /*! Min Y coordinate of the bounding box */
    EOS_MILLIMETER      minY_mm;

    /*! Min Z coordinate of the bounding box */
    EOS_MILLIMETER      minZ_mm;

    /*! Max X coordinate of the bounding box */
    EOS_MILLIMETER      maxX_mm;

    /*! Max Y coordinate of the bounding box */
    EOS_MILLIMETER      maxY_mm;

    /*! Max Z coordinate of the bounding box */
    EOS_MILLIMETER      maxZ_mm;

} EOS_BBOX;

/*! A structure holding information about a Z-segment. See OpenJob documentation
for element /openjob/parts/part/segments for additional info. */
typedef struct
{
    /*! Min Z coordinate of the segment */
    EOS_MICROMETER      minZ_um;

    /*! Max Z coordinate of the segment */
    EOS_MICROMETER      maxZ_um;

    /*! The name of the Exposure Set assigned to the segment */
    EOS_CWSTR           exposureSetName;

    /*! The slice thickness of the segment as defined by the assigned Exposure Set */
    EOS_MICROMETER      sliceThickness_um;

    /*! The name of the segment */
    EOS_CWSTR           name;

    /*! Memory block reserved for future extensions */
    EOS_BYTE            reserved[24];

} EOS_ZSEGMENT_INFO;

/*! A structure holding information about a part's segmentation. See OpenJob
documentation for element /openjob/parts/part/segments for additional info.*/
typedef struct
{
    /*! The original bounding box of the part respecting any transformation specified in the job */
    EOS_BBOX                    bboxOriginal;

    /*! The adjusted bounding box of the part. */
    EOS_BBOX                    bboxAdjusted;

    /*! Pointer to an array of Z-segments for this part. */
    EOS_ZSEGMENT_INFO const *   segments;

    /*! The number of segments. There's always at least one segment
    (exactly one if the part is not segmented). */
    EOS_SIZET                   segmentCount;

    /*! Memory block reserved for future extensions */
    EOS_BYTE                    reserved[16];

} EOS_PART_SEGMENTATION_INFO;

/*! A structure holding metadata about a single layer in a job. */
typedef struct
{
    /*! The index of the layer, starting with 1 and counting upwards in z-direction */
    EOS_LAYERINDEX      layerIndex;

    /*! Z distance to the previous layer in micrometers. 0 for the first layer. */
    EOS_MICROMETER      delta_um;

    /*! Distance of the layer from the job start height in micrometers */
    EOS_MICROMETER      zPosition_um;

    /*! Flag that indicates if the layer contains any exposure information or will simply add a powder layer on top of the previous one */
    EOS_BOOL            isExposed;

    /*! Ids of parts that are exposed in this layer */
    EOS_INT const *     exposedPartIds;

    /*! Number of parts that are exposed which is also the size of the exposedPartIds array*/
    EOS_SIZET           exposedPartsCount;

    /*! Memory block reserved for future extensions */
    EOS_BYTE            reserved[24];

} EOS_LAYER_INFO;

/*! The type represents metadata for the task that is exported as part of the task and controls behavior of the machine system */
typedef struct
{
    /*! The name of the task, must not be NULL.
     *  If wcslen(name) == 0u, the task name is instead derived from the job filename. */
    EOS_CWSTR           name;

    /*! An optional comment to the task.
     *  If NULL, an empty string is used. */
    EOS_CWSTR           comment;

    /*! Specifies how often the task can be built ; set to EOS_INFINITE_TIMES for no restriction */
    EOS_UINT            numberOfAllowedBuilds;

    /*! The task mode, either regular or single layer (see EOS_TASKMODE for details) */
    EOS_TASKMODE        taskMode;

    /*! Name of the application that generated the task, i.e., your application.
     *  If NULL, an empty string is used. */
    EOS_CWSTR           generatorName;

    /*! Version of the application that generated the task, i.e., your application.
     *  If NULL, an empty string is used. */
    EOS_CWSTR           generatorVersion;

    /*! All displayed part paths will be made relative to this directory if possible.
     *  Use NULL or an empty string to use the directory of the job as base. */
    EOS_CWSTR           partBaseDirectory;

    /*! The task format, either regular or raw (see EOS_TASKFORMAT for details) */
    EOS_TASKFORMAT      taskFormat;

    /*! Compression level for task export. Allows for trade off between task size and export time */
    EOS_COMPRESSION_LEVEL compression;

    /*! Memory block reserved for future extensions */
    EOS_BYTE            reserved[72];

} EOS_TASK_METADATA;

/*!
 * Meta data about a task.
 */
typedef struct
{
    /* Metadata that was specified during task creation.
     * Note, that the field partBaseDirectory cannot be restored and will be set to NULL. */
    EOS_TASK_METADATA const *   metaData;

    /*! Name of the operating system user who created the task */
    EOS_CWSTR                   creatorName;

    /*! Specifies if the task is complete */
    EOS_BOOL                    isComplete;

    /*! Array of data files the task includes. Size of the array is specified in dataFileCount */
    EOS_CWSTR const *           dataFiles;

    /*! Number of data files the task includes. At the same time the size of the dataFiles array. */
    EOS_SIZET                   dataFileCount;

    /*! The serial number of the machine for which the task was generated. The task will be rejected on other machines. */
    EOS_CWSTR                   serialNumber;

    /*! The print domain for which the task was generated, NULL in case the task does not support print domains */
    EOS_CWSTR                   printDomain;

    /*! The total building time in milliseconds. The value includes exposure and recoating times for all layers that have already been written. If the task is not complete, the value is not final.
        \note    Before EOSPRINT SDK 2.13 this member erroneously contained the total exposure time, not the total building time. */
    EOS_MILLISECS               buildingTime;

    /*! The total exposure time in milliseconds as estimated at the time of task export */
    EOS_MILLISECS               exposureTime;

    /*! The total recoating time in milliseconds as estimated at the time of task export */
    EOS_MILLISECS               recoatingTime;

    /*! Memory block reserved for future extensions */
    EOS_BYTE                    reserved[48];

} EOS_TASK_INFO;

/*!
 * \brief       Type which represents the start or end of an exposure vector
 * \note        An exposure vector is a straight connection of two consecutive coordinates.
 * \note        The following properties shall be equal for both points of a vector:
 *              \a exposureType, \a partId and \a laserScannerIndex
 * \deprecated  This structure is deprecated and might be removed in future versions of the API
 */
typedef struct
{
    /*! X coordinate of the point in mm, starting at the left border of the platform */
    EOS_MILLIMETER      x_mm;

    /*! Y coordinate of the point in mm, starting at the front border of the platform */
    EOS_MILLIMETER      y_mm;

    /*! Exposure type of the vector */
    EOS_EXPOSURE_TYPE   exposureType;

    /*! Part ID identifying the part the exposure vector belongs to.
     *  The part ID is a positive number. In error cases it is set to -1.
     */
    EOS_INT             partId;

    /*! Index of the laser/scanner unit which is used to expose at this particular exposure point. */
    EOS_BYTE            laserScannerIndex;

    /*! Memory block reserved for future extensions */
    EOS_BYTE            reserved[15];

} EOS_EXPOSUREPOINT;

/*!
 * \brief       Representation of an exposure vector. It contains start and end coordinates
 *              as well as an index into the separate exposure data array.
 *              This index will only be valid when exposure data was requested.
 *              Furthermore, it contains a timestamp. This timestamp will always be 0
 *              if no valid EOS Build license is found.
 */
typedef struct
{
    /*! Start point of this exposure vector */
    EOS_POINT2          start;

    /*! End point of this exposure vector */
    EOS_POINT2          end;

    /*! The index to the corresponding EOS_EXPOSURE_DATA element in the EOS_EXPOSURE_DATA_ARRAY */
    EOS_SIZET           exposureDataIndex;

    /*! Timestamp of exposure end. It can only be queried when the Build
     *  license is available, otherwise it is set to 0.0
     */
    EOS_MICROSECS       timestamp_us;

} EOS_EXPOSURE_VECTOR;

/*!
 * \brief       Represents exposure data for a vector.
 * \note        Some members like timestamp_us require certain licenses (i.e. Build) to be present
 */
typedef struct
{
    /*! Part ID identifying the part the exposure vector belongs to.
     *  The part ID is a positive number. In error cases it is set to -1.
     */
    EOS_INT             partId;

    /*! Exposure type of the vector */
    EOS_EXPOSURE_TYPE   exposureType;

    /*! Index of the laser/scanner unit which is used to expose at this particular exposure point. */
    EOS_BYTE            laserScannerIndex;

    /*! Laser power of the vector. Requires Build license and licensed material set, 0.0 otherwise */
    EOS_DOUBLE          laserPower_W;

    /*! Scanner speed of the vector. Requires Build license and licensed material set, 0.0 otherwise */
    EOS_DOUBLE          scannerSpeed_mm_per_s;

    /*! Duration of one laser pulse-pause period in microseconds. 1.0 for continuous wave vectors. Requires Build license and licensed material set, 0.0 otherwise */
    EOS_MICROSECS       pulsedWavePeriodTime_us;

    /*! Duration of one pulse in microseconds. Guaranteed to be less or equal to the pulsedWavePeriodTime_us. 1.0 for continuous wave vectors. Requires Build license and licensed material set, 0.0 otherwise */
    EOS_MICROSECS       pulsedWavePowerOnTime_us;

    /*! Delay before the first pulse in microseconds. */
    EOS_MICROSECS       pulsedWavePowerOnDelay_us;

    /*! The index of the exposure set to which this vector belongs.
        The list of exposure sets can be queried via EosFileProperties_LoadMaterialSetInfo. */
    EOS_UINT            exposureSetIndex;

    /*! The exposure step to which this vector belongs. Exposure sets are composed from a sequence of exposure steps.
        E.g. Contour, Upskin, Downskin, Infill, Contour, Edge. The sequence can be queried via
        EosFileProperties_LoadMaterialSetInfo. The variable exposureStepIndex references an individual exposure step from that sequence by
        its sequence position.
        Without the step it would be impossible to distinguish the first and the last Contour.
        Requires Build license and licensed material set, 0 otherwise. */
    EOS_UINT            exposureStepIndex;

    /*! The profile id of the laser beam used for this exposure vector. */
    EOS_INT             beamProfileId;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE            reserved[28];

} EOS_EXPOSURE_DATA;

/*!
 * \brief       A sequence of exposure points
 * \note        Given index i the points at array indices [2 * i] and [2 * i + 1] form the start and end point of the
 *              i-th exposure vector.
 * \note        No connection must be drawn between the points at array indices [2 * i + 1] and [2 * i + 2], because
 *              they are end and start of of two different exposure vectors.
 * \deprecated  This structure is deprecated and might be removed in future versions of the API
 */
typedef struct
{
    /*! Pointer to the start of the array, i.e. the start exposure point of the first exposure vector */
    EOS_EXPOSUREPOINT const * points;

    /*! Number of exposure points in the array, always positive and even; hence the array contains (size / 2) exposure vectors */
    EOS_SIZET           size;

} EOS_EXPOSUREPOINTARRAY;

/*!
 * \brief       A sequence of exposure vectors
 */
typedef struct
{
    /*! Pointer to the start of the array, i.e. the first exposure vector */
    EOS_EXPOSURE_VECTOR const * vectors;

    /*! Number of exposure vectors in the array */
    EOS_SIZET size;

} EOS_EXPOSURE_VECTOR_ARRAY;

/*!
 * \brief       A sequence of exposure data elements
 */
typedef struct
{
    /*! Pointer to the start of the array, i.e. the first exposure vector */
    EOS_EXPOSURE_DATA const * data;

    /*! Number of exposure data elements in the array, always positive */
    EOS_SIZET size;

} EOS_EXPOSURE_DATA_ARRAY;

/*! Supported model types */
typedef enum
{
    EOS_MT_UNDISCLOSED_MACHINES
    EOS_MT_ABANDONED_MACHINES

    /*! EOS M 290 400W */
    EOS_MT_M290 = 1,

    /*! EOS M 400 */
    EOS_MT_M400 = 2,

    /*! EOS M 400-4 */
    EOS_MT_M404 = 3,

    /*! EOS P 500 DR01 (Design Release 1) */
    EOS_MT_P500 = 4,

    /*! EOS M 300-4 */
    EOS_MT_M304 = 5,

    /*! EOS P 900 */
    /* EOS_MT_P900 = 6, the machine was abandoned */

    /*! EOS P 396 */
    /* EOS_MT_P396 = 7, the machine was discontinued */

    /*! Integra P 450 */
    /* EOS_MT_P420 = 8, the machine was abandoned */

    /*! EOS P 810 */
    /* EOS_MT_P711 = 9, the machine was discontinued */

    /*! EOS M 100 */
    /* EOS_MT_M100 = 10, the machine was discontinued */

    /*! FORMIGA P 110 FDR */
    /* EOS_MT_P120 = 11, the machine was discontinued */

    /*! EOS M 290XF */
    /* EOS_MT_M292 = 12, the machine was discontinued */

    /*! EOS P 500 FDR DR01 (Design Release 1) */
    EOS_MT_P520 = 13,

    /*! EOS M 290-2 */
    EOS_MT_M293 = 14,

    /*! FORMIGA P 110 Velocis */
    /* EOS_MT_P110 = 15, the machine was discontinued */

    /* This machine was abandoned and has never been released */
    /* EOS_MT_ABANDONDED_16 = 16, */

    /*! EOS M 290 1kW */
    EOS_MT_M294 = 17,

    /*! EOS M 300-4 1kW */
    EOS_MT_M314 = 18,

    /*! EOS P3 NEXT */
    EOS_MT_P397 = 20,

    /*! AMCM M 8K-8 1kW */
    EOS_MT_M808 = 21,

    /*! EOS P 500 (Design Release 2) */
    EOS_MT_P501 = 22,

    /*! EOS P 500 FDR (Design Release 2) */
    EOS_MT_P521 = 23,

    /*! EOS M4 ONYX */
    EOS_MT_M450 = 25,

    /*! EOS M4 ONYX CFL */
    EOS_MT_M451 = 27,

    /*! AMCM M 4K-4 FLX */
    EOS_MT_M405 = 28,

} EOS_MODEL_TYPE;

/*! Available operations for adjustable parameters */
typedef enum
{
    /*! The adjustable would override the existing value with its value */
    EOS_AO_OVERRIDE = 1,

    /*! The adjustable would offset the existing value with its value */
    EOS_AO_OFFSET = 2

} EOS_ADJUSTMENT_OPERATION;

/*! Target of an adjustable */
typedef enum
{
    /*! The adjustable would be applied to compensate for singularities of the machine */
    EOS_AT_MACHINE_ADJUSTMENT = 1,

    /*! The adjustable would be applied to compensate for singularities of the job */
    EOS_AT_JOB_ADJUSTMENT = 2

} EOS_ADJUSTMENT_TYPE;

/*!
 * \brief   Type for specifying the affected units of an adjustable process parameter.
 * \note    In earlier versions of the EOSPRINT 2 API this was a bitfield, thereby allowing
 *          for adjustable process parameters that affect more than one unit. However,
 *          there never was such a process parameter and there never will be. Any process
 *          parameter will affect either the exposure or the process environment on the
 *          machine, but not both. Therefore, EOS_AFFECTED_UNITS could as well be an
 *          enumeration. However, in order not to introduce a breaking change the type
 *          remains as is.
 * \note    See Constants.h for possible values.
 */
typedef EOS_INT EOS_AFFECTED_UNITS;

/*! A version number of the form <major>.<minor>.<build>.<revision>, e.g. 1.2.3.45 */
typedef struct
{
    /*! The major number of the version, i.e. the first section from left */
    EOS_INT major;

    /*! The minor number of the version, i.e. the second section from left  */
    EOS_INT minor;

    /*! The build number of the version, i.e. the third section from left  */
    EOS_INT build;

    /*! The revision number of the version, i.e. the fourth section from left  */
    EOS_INT revision;

} EOS_VERSION;

/*!
 * \brief       Describes an adjustable process parameter as defined by a specific Material Set.
 * \note        The actual adjustment is not part of this structure. See EOS_PROCESS_PARAMETER_ADJUSTMENT instead.
 */
typedef struct
{
    /*! Unique identifier of the adjustable parameter (per Material Set) */
    EOS_CWSTR                   id;

    /*! Pretty name to display to the user */
    EOS_CWSTR                   displayName;

    /*! Unit of the adjustable parameter */
    EOS_CWSTR                   unit;

    /*! Indicates whether the adjustable allows changing the job specific value */
    EOS_BOOL                    isJobSpecificAdjustmentEnabled;

    /*! Indicates whether the adjustable allows changing the machine specific value */
    EOS_BOOL                    isMachineSpecificAdjustmentEnabled;

    /*! The type of the operation that this adjustable performs with the material default */
    EOS_ADJUSTMENT_OPERATION    adjustmentOperation;

    /*!
     * \brief   Indicates on which units the adjustment has an effect. This can be either
     *          the exposure or the process environment.
     * \note    In earlier versions of the EOSPRINT 2 API this was a bitfield, thereby allowing
     *          for adjustable process parameters that affect more than one unit. However,
     *          there never was such a process parameter and there never will be. Any process
     *          parameter will affect either the exposure or the process environment on the
     *          machine, but not both. Therefore, EOS_AFFECTED_UNITS could as well be an
     *          enumeration. However, in order not to introduce a breaking change the type
     *          remains as is.
     */
    EOS_AFFECTED_UNITS          hasEffectOn;

    /*! EOS_TRUE if a lower bound for the effective value is present, else EOS_FALSE */
    EOS_BOOL                    hasRangeMin;

    /*! EOS_TRUE if an upper bound for the effective value is present, else EOS_FALSE */
    EOS_BOOL                    hasRangeMax;

    /*! Lower bound for the effective value (i.e., the adjusted value) */
    EOS_DOUBLE                  valueRangeMin;

    /*! Upper bound for the effective value (i.e., the adjusted value) */
    EOS_DOUBLE                  valueRangeMax;

    /*! The unadjusted value as it is defined by the material set */
    EOS_DOUBLE                  valueUnadjusted;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE                    reserved[56];

} EOS_ADJUSTABLE_PROCESS_PARAMETER;

/*!
 * \brief           Describes a single actual adjustment defined by machine or job
 */
typedef struct
{
    /*! Identifier of the adjustable parameter that is implemented with this adjustment */
    EOS_CWSTR                   id;

    /*! Specifies if the adjustment is specific to the machine or to the job */
    EOS_ADJUSTMENT_TYPE         type;

    /*! EOS_TRUE if the adjustment of the given type is used and has a value, else EOS_FALSE */
    EOS_BOOL                    hasValue;

    /*! If hasValue is EOS_TRUE then the value of the adjustment,
     *  still needs to be applied to the material default as given by type */
    EOS_DOUBLE                  value;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE                    reserved[104];

} EOS_PROCESS_PARAMETER_ADJUSTMENT;

/*!
 * \brief           Describes a list of adjustments
 */
typedef struct
{
    /*! Array of adjustments */
    EOS_PROCESS_PARAMETER_ADJUSTMENT const *    adjustments;

    /*! Size of the adjustments array */
    EOS_SIZET                                   adjustmentsSize;

} EOS_PROCESS_PARAMETER_ADJUSTMENT_LIST;

/*!
 * \brief           Defines a default exposure set assignment based on file name suffixes.
 *                  Auto assignment rules are used to encode the desired exposure set assignment
 *                  of a part in its file name. E.g., all files ending with _p could be
 *                  assigned the exposure set EOS_DirectPart.
 *                  If there are no rules with fitting suffixes, the special values of "Default"
 *                  and "Default_Support" will be used as a fall back assignment.
 *
 *                  From version 2.8 on, EOSPRINT applies these auto assignment rules automatically
 *                  upon file load and also when manually triggered by the user. Clients of the EOSPRINT 2 API
 *                  are not required to support this feature, but they should not display auto assignment rules to
 *                  users if they are not capable to apply them.
 */
typedef struct
{
    /*! \brief File name suffix for which the auto assignment rule should be applied.
     *  \note  The suffix is the end of the file name, not the file extension. */
    EOS_CWSTR    fileNameSuffix;

    /*! Name of Exposure Set to assign */
    EOS_CWSTR    exposureSetName;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE     reserved[32];

} EOS_EXPOSURESET_AUTO_ASSIGNMENT_RULE;

/*! Describes a license required for a specific feature of an Exposure Set. */
typedef struct
{
    /*! Bit mask describing the features for which this license is required */
    EOS_LICENSABLE_FEATURES     requiredFor;

    /*! Licensing system, e.g., CodeMeter. This may be necessary to correctly interpret the licensing descriptor. */
    EOS_LICENSING_SYSTEM        licensingSystem;

    /*! A string describing the license - the format is specific to the licensing system.
     *  For licensing system EOS_LS_CODEMETER the descriptor has the form <firmcode>:<productcode>.
     *  Please refer to the WIBU CodeMeter manual for an explanation of this scheme. */
    EOS_CWSTR                   licenseDescriptor;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE                    reserved[112];

} EOS_LICENSE_INFO;

/*! Describes an Exposure Step */
typedef struct
{
    /*! The name of the Exposure Step */
    EOS_CWSTR           name;

    /*! The exposure type of vectors generated by this exposure step.
     *  \note Exposure steps of type EOS_EXPOSURE_TYPE_CONTOUR can also generate vectors of types
     * EOS_EXPOSURE_TYPE_DOWNSKIN_CONTOUR and EOS_EXPOSURE_TYPE_UPSKIN_CONTOUR. */
    EOS_EXPOSURE_TYPE   exposureType;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE            reserved[20];

} EOS_EXPOSURESTEP_INFO;

/*! Describes an Exposure Step usable as a template for inserting into an Exposure Set */
typedef EOS_EXPOSURESTEP_INFO EOS_EXPOSURETYPE_TEMPLATE;

typedef struct
{
    /*! The machine model type for which these templates apply */
    EOS_MODEL_TYPE                      modelType;

    /*! An array of exposure type templates describing the types that can be added to a matching material set */
    EOS_EXPOSURETYPE_TEMPLATE const *   exposureTypeTemplates;

    /*! The size of the exposure type templates array */
    EOS_SIZET                           exposureTypeTemplatesSize;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE                            reserved[40];

} EOS_EXPOSURETYPE_TEMPLATES;

/*! Describes an Exposure Set */
typedef struct
{
    /*! The name of the Exposure Set */
    EOS_CWSTR                       name;

    /*! The layer thickness to which the Exposure Set applies */
    EOS_MICROMETER                  layerThickness;

    /*! Whether or not the Exposure Set is editable to users holding the required license(s) */
    EOS_BOOL                        isEditable;

    /*! An array of license infos describing the licenses that are required by this Exposure Set. */
    EOS_LICENSE_INFO const *        licenses;

    /*! The size of the licenses array */
    EOS_SIZET                       licensesSize;

    /*! EOS_TRUE if this exposure set is intended for support structures comprising open polygons. */
    EOS_BOOL                        isSupport;

    /*! An array of exposure step infos describing the individual exposure steps of this exposure set.
     *  Empty if the exposure set is not editable or not all required licenses are available
     *  (i.e., those listed under licenses + the EOSPRINT Build license. */
    EOS_EXPOSURESTEP_INFO const *   exposureSteps;

    /*! The size of the exposureSteps array */
    EOS_SIZET                       exposureStepsSize;

    /*! Whether or not the Exposure Set uses Smart Fusion Technology. Smart Fusion requires an EOSTATE Exposure OT system and
        a valid Smart Fusion license on the target system. */
    EOS_BOOL                        usesSmartFusion;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE                        reserved[68];

} EOS_EXPOSURESET_INFO;

/*! A revision of a Material Set. */
typedef struct
{
    /*! Identifies this revision of the Material Set. The UUID is pseudo-random generated. */
    EOS_CWSTR           uuid;

    /*! The UUID of the immediate predecessor revision */
    EOS_CWSTR           parentUuid;

    /*! The UTC timestamp when this revision was created in milliseconds since epoch (1970-01-01 00:00:00.000000) */
    EOS_TIMESTAMP       timestampUtc;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE            reserved[104];

} EOS_MATERIALSET_REVISION;

/*! \brief Type representing the information of a Material Set. */
typedef struct
{
    /*!
     * \brief   The name of the Material Set.
     * \note    This field is controlled by the author, so there is no guarantee that two different Material Sets will have unambiguous names. */
    EOS_CWSTR                               name;

    /*!
     * \brief   The version of the Material Set.
     * \note    This field is controlled by the author, so there is no guarantee that two different Material Sets will have different versions. */
    EOS_VERSION                             version;

    /*!
     * \brief   Unambiguously identifies this revision of the Material Set.
     * \note    The UUID is pseudo-random generated. */
    EOS_CWSTR                               uuid;

    /*! The UUID of this Material Sets immediate predecessor revision - Empty string, if there is no predecessor. */
    EOS_CWSTR                               parentUuid;

    /*! The machine type to which this Material Set applies */
    EOS_MODEL_TYPE                          modelType;

    /*! The name of the material to which this Material Set applies */
    EOS_CWSTR                               material;

    /*!
     * \brief   The base layer thickness of this Material Set.
     * \note    This is a common divisor of all Exposure Set specific layer thicknesses, usually the greatest common divisor.
     *          Example:
     *          The Material Set has Exposure Sets with layer thicknesses 20 um and 30 um. Thus, base layer thickness must be
     *          10 um or any other layer thickness lt, so that layerThickness / lt is a positive integer for all
     *          Exposure Sets within this Material Set. */
    EOS_MICROMETER                          baseLayerThickness;

    /*! The inert gas used in the build chamber (e.g. nitrogen or argon) */
    EOS_INERT_GAS                           inertGas;

    /*! EOS_TRUE if this is an unmodified Material Set from EOS, EOS_FALSE otherwise */
    EOS_BOOL                                isAuthoredByEos;

    /*!
     * \brief   EOS_TRUE if this revision is final, EOS_FALSE otherwise.
     * \note    It is possible to modify final Material Sets, but any such modification will yield a new history entry
     *          with the old revision and also create a new unique identifier for the new revision. The new revision will be
     *          set to non-final automatically. */
    EOS_BOOL                                isFinal;

    /*!
     * \brief   Currently the history contains all final predecessor revisions of this Material Set. In future it may
     *          be possible to manually remove some of the history entries. However, it will always be possible to determine
     *          the original revision (root). */
    EOS_MATERIALSET_REVISION const *        history;

    /*! The number of elements in the history array. */
    EOS_SIZET                               historySize;

    /*!
     * \brief   An array of EOS_EXPOSURESET_INFO elements describing all Exposure Sets specified in this Material Set
     *          that in principle are available to the user.
     * \note    Some of the Exposure Sets may require one or more licenses to be used. */
    EOS_EXPOSURESET_INFO const *            exposureSets;

    /*! The number of elements in the exposureSets array. */
    EOS_SIZET                               exposureSetsSize;

    /*!
     * \brief   An array of EOS_ADJUSTABLE_PROCESS_PARAMETER elements describing all adjustable parameters
     *          that are available for the material set. */
    EOS_ADJUSTABLE_PROCESS_PARAMETER const * adjustableParameters;

    /*! The number of items in the array \a adjustableParameters */
    EOS_SIZET                               adjustableParametersSize;

    /*!
     * \brief   An array of EOS_EXPOSURESET_AUTO_ASSIGNMENT_RULE elements defining
     *          a mapping of file name suffixes to exposure sets that may be applied
     *          to parts of a job if the user loads a new part or if he manually
     *          triggers the auto assignment of exposure sets.
     */
    EOS_EXPOSURESET_AUTO_ASSIGNMENT_RULE const * autoAssignmentRules;

    /*! The number of elements in the autoAssignmentRules array. */
    EOS_SIZET                               autoAssignmentRulesSize;

    /*! The name of the machine model to which this Material Set applies. */
    EOS_CWSTR                               modelName;

    /*! True if this Material Set targets a customized machine. */
    EOS_BOOL                                targetsCustomizedMachine;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE                                reserved[108];

} EOS_MATERIALSET_INFO;

/*! Type to represent the access mode of a single process parameter within a material set. */
typedef enum
{
    /*! Inaccessible process parameter - you will never see this or it signals an error/invalid state */
    EOS_PPAM_NONE = 0,

    /*! The process parameter is readable, but not writable. */
    EOS_PPAM_READ = 1,

    /*! The process parameter is readable and writable. */
    EOS_PPAM_READWRITE = 2

} EOS_PROCESS_PARAMETER_ACCESS_MODE;

/*! A single process parameter from a material set */
typedef struct
{
    /*! The unique path to the process parameter within the material set */
    EOS_CWSTR                         path;

    /*! Pretty name to display to the user. This can be empty if the parameter is not supposed to be displayed */
    EOS_CWSTR                         displayName;

    /*! Unit of the parameter (can be empty string) */
    EOS_CWSTR                         unit;

    /*! Data type of the parameter */
    EOS_VARIANT_TYPE                  type;

    /*! Access mode of the parameter. Boils down to either read only or read/write. */
    EOS_PROCESS_PARAMETER_ACCESS_MODE accessMode;

    /*! The value of the parameter. */
    EOS_VARIANT                       value;

    /*! EOS_TRUE if a lower bound for the value is specified, else EOS_FALSE */
    EOS_BOOL                          hasRangeMin;

    /*! EOS_TRUE if an upper bound for the value is specified, else EOS_FALSE */
    EOS_BOOL                          hasRangeMax;

    /*! EOS_TRUE if a default value is specified, else EOS_FALSE */
    EOS_BOOL                          hasDefault;

    /*! Lower bound for the parameter value. Undefined if hasRangeMin == EOS_FALSE */
    EOS_VARIANT                       valueRangeMin;

    /*! Upper bound for the parameter value. Undefined if hasRangeMax == EOS_FALSE */
    EOS_VARIANT                       valueRangeMax;

    /*! Default value or undefined if hasDefault == EOS_FALSE */
    EOS_VARIANT                       valueDefault;

    /*! Memory block reserved for future extensions */
    EOS_BYTE                          reserved[48];

} EOS_PROCESS_PARAMETER;

/*! A list of process parameters from a material set. */
typedef struct
{
    /*! Array of parameters */
    EOS_PROCESS_PARAMETER const *   processParameters;

    /*! Size of the processParameters array */
    EOS_SIZET                       processParametersSize;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE                        reserved[16];

} EOS_PROCESS_PARAMETER_LIST;

/*! \brief Meta data about a specific laser */
typedef struct
{
    /*! \brief The laser's nominal power output */
    EOS_DOUBLE laserPowerNominal_W;

    /*! \brief The laser's maximum power output */
    EOS_DOUBLE laserPowerMax_W;

    /*! \brief The maximum scanner movement speed (in mm per sec) */
    EOS_DOUBLE maxScanSpeed_mm_per_s;

    /*! \brief The point closest to the origin of the system coordinate system which the laser can expose */
    EOS_POINT2 minPoint2D;

    /*! \brief The point farthest away from the origin of the system coordinate system which the laser can expose */
    EOS_POINT2 maxPoint2D;

    /*! \brief Bit field containing the laser capabilities
     *
     *  Current options:
     *      EOS_LSC_FOCUS_DEFOCUS
     *      EOS_LSC_VARIABLE_FOCUS
     *      EOS_LSC_BEAMSHAPING
     */
    EOS_LS_CAPABILITIES capabilities;

    /*! Memory block reserved for future extensions */
    EOS_BYTE            reserved[64];

} EOS_LASER_SCANNER_INFO;

/*! \brief Metadata about a specific machine. */
typedef struct
{
    /*! The type of the building volume (e.g., cylinder or cuboid).
     *  \note This affects the validity of dimX, dimY and dimR */
    EOS_VOLUME_TYPE                 volumeType;

    /*! \brief The x dimension of the building volume, if volumeType == EOS_VOL_CUBOID, otherwise undefined */
    EOS_MILLIMETER                  dimX;

    /*! \brief The y dimension of the building volume, if volumeType == EOS_VOL_CUBOID, otherwise undefined */
    EOS_MILLIMETER                  dimY;

    /*! \brief The z dimension of the building volume */
    EOS_MILLIMETER                  dimZ;

    /*! \brief The radius of the building volume, if volumeType == EOS_VOL_CYLINDER, otherwise undefined */
    EOS_MILLIMETER                  dimR;

    /*! \brief The model type of the machine */
    EOS_MODEL_TYPE                  modelType;

    /*! \brief The number of laser/scanner units the machine employs */
    EOS_INT                         numberOfLaserScanners;

    /*! \brief The version of the machine software
     *          -1.-1.-1.-1 if this information could not be retrieved */
    EOS_VERSION                     machineSoftwareVersion;

    /*! \brief The version of the machine interface
     *          -1.-1.-1.-1 if this information could not be retrieved */
    EOS_VERSION                     machineInterfaceVersion;

    /*! \brief The serial number of the machine */
    EOS_CWSTR                       serialNumber;

    /*! \brief The print domain of the machine, NULL in case the machine does not support print domains */
    EOS_CWSTR                       printDomain;

    /*! \brief The laser/scanner info (one entry per laser/scanner unit, see numberOfLaserScanners) */
    EOS_LASER_SCANNER_INFO const *  laserScannerInfo;

    /*! \brief The version of the EOSTATE Exposure OT system.
     *          0.0.0.0 if no EOSTATE Exposure OT exists.
     *          -1.-1.-1.-1 if it is unknown whether the system has EOSTATE Exposure OT. */
    EOS_VERSION                     eostateExposureOtVersion;

    /*! \brief The version of the EOSTATE MeltPool system.
     *          0.0.0.0 if no EOSTATE MeltPool exists.
     *          -1.-1.-1.-1 if it is unknown whether the system has EOSTATE MeltPool. */
    EOS_VERSION                     eostateMeltpoolVersion;

    /*! \brief Information on the availability of the Smart Fusion license
     *         possible flags are:
     *         * EOS_MLI_IS_KNOWN               if the state is known
     *         * EOS_MLI_LICENSE_PRESENT        if a license is present
     *         * EOS_MLI_LICENSE_VALID          if a license is valid
     *         * EOS_MLI_LICENSE_AVAILABLE      if there is sufficient quantity of valid licenses
     *         * EOS_MLI_LICENSE_CAN_EXPIRE     if the present license can expire
     *         * EOS_MLI_LICENSE_NOT_EXPIRED    if the present license is not expired
     */
    EOS_MACHINE_LICENSE_INFOS       smartFusionLicenseInfos;

    /*! \brief The Smart Fusion compatibility of the machine */
    EOS_SMART_FUSION_COMPATIBILITY  smartFusionCompatibility;

    /*! \brief The name of the machine model, e.g., "EOS M4 ONYX" */
    EOS_CWSTR                       modelName;

    /*! \brief True if the machine is not a standard machine of the given modelType */
    EOS_BOOL                        isCustomizedMachine;

    /*! \brief A block of memory reserved for future extension. */
    EOS_BYTE reserved[36];

} EOS_MACHINE_INFO;

/*! Output type of a simulation model */
typedef enum
{
    /*! Simulation output Integral OT. */
    EOS_SO_INT_OT = 0x1,

    /*! Simulation output Maximum OT. */
    EOS_SO_MAX_OT = 0x2,

    /*! Simulation output LCF (Laser Correction Factor). */
    EOS_SO_LCF = 0x3

} EOS_SIMULATION_OUTPUT;

/*! (Machine) model type to which a simulation model applies */
typedef enum
{
    /*! Onnx model type. */
    EOS_ONNX_MODEL = 0x1

} EOS_SIMULATION_MODEL_TYPE;

/*!
 * \brief       Meta information about an EOS simulation model.
 */
typedef struct
{
    /*! Unique id of the simulation model */
    EOS_CWSTR                       uuid;

    /*! Name of the simulation model */
    EOS_CWSTR                       name;

    /*! Version of the simulation model */
    EOS_VERSION                     version;

    /*! Output of the model */
    EOS_SIMULATION_OUTPUT           outputType;

    /*! (Machine) model type to which the simulation model applies */
    EOS_SIMULATION_MODEL_TYPE       modelType;

    /*! Material of the predicted process */
    EOS_CWSTR                       material;

    /*! Layer thickness of the predicted process */
    EOS_MICROMETER                  baseLayerThickness;

    /*! Machine model names the model is compatible with */
    EOS_CWSTR                       machineModelNames;

    /*! Machine model types the model is compatible with */
    EOS_CWSTR                       machineModelTypes;

    /*! Human-readable description of the simulation model */
    EOS_CWSTR                       description;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE                        reserved[20];

} EOS_SIMULATION_INFO;

/*!
 * \brief Type which represents the build time info of a build time estimation
 */
typedef struct
{
    /*! \name  Accumulated results
     *  \brief Accumulated results of all layers that have already been calculated
     */
    /*!@{*/

    /*! Sum of part build times [ms] */
    EOS_DOUBLE          buildTimesMs;

    /*! Number of calculated layers [0,...] */
    EOS_SIZET           numberOfCalculatedLayers;

    /*! Number of all layers [0,...] */
    EOS_SIZET           numberOfAllLayers;

    /*! Sum of exposure times [ms] */
    EOS_DOUBLE          exposureTimesMs;

    /*! Sum of non exposure times [ms] */
    EOS_DOUBLE          nonExposureTimesMs;

    /*!@}*/

    /*! \name  Last results
     *  \brief Results of the last calculated layer
     */
    /*!@{*/

    /*! Current 1-based number of calculated layer */
    EOS_LAYERINDEX      currentLayerNumber;

    /*! Exposure time per laser [ms] */
    EOS_DOUBLE const *  exposureTimePerLaserScannerMs;

    /*! Number of entries in exposureTimePerLaserScannerMs */
    EOS_SIZET           numberOfLaserScanners;

    /*!@}*/

    /*! \name  Reserved memory
     */
    /*!@{*/

    /*! Memory block reserved for future extensions */
    EOS_BYTE            reserved[64];

    /*!@}*/

} EOS_BUILDTIMEINFO;

typedef struct
{
    //! Exposure time spent on delays
    EOS_DOUBLE delayTimeSeconds;

    //! Array of exposure times spent on each exposure type.
    //! Use the enumeration EOS_EXPOSURE_TYPE to index this array. E.g., exposureTimeSecondsPerType[EOS_EXPOSURE_TYPE_CONTOUR] for times
    //! spent on contours.
    //! \note The enumeration EOS_EXPOSURE_TYPE contains some values that do not correspond to an actual exposure - these array entries
    //!       will always be 0.
    //! \note The array has more entries than the enumeration EOS_EXPOSURE_TYPE has enumeration values - this is to account for future
    //!       extensions. The surplus entries will be set to 0.
    EOS_DOUBLE exposureTimeSecondsPerType[24];

    //! Reserved for future use
    EOS_BYTE reserved[56];

} EOS_EXPOSURETIME_DETAIL;

/*!
 * \brief       Type which represents a list of file paths
 */
typedef struct
{
    /*! array of file paths */
    EOS_CWSTR const *   filepaths;

    /*! number of entries in filepaths */
    EOS_SIZET           numberOfFilepaths;

    /*! A block of memory reserved for future extension. */
    EOS_BYTE            reserved[16];

} EOS_FILEPATH_LIST;

/*!
 * \brief       A structure holding information about the process heights that result from
 *  building a certain job on a specific target machine.
 */
typedef struct {

    /*! Height of the pre process */
    EOS_MILLIMETER preProcessHeight;

    /*! Height of the bottom offset */
    EOS_MILLIMETER bottomOffsetHeight;

    /*! Height of the payload (i.e., the range where parts lie)*/
    EOS_MILLIMETER payloadHeight;

    /*! Height of the top offset */
    EOS_MILLIMETER topOffsetHeight;

    /*! Height of the post process */
    EOS_MILLIMETER postProcessHeight;

    /*! Accumulated height for compensating Z shrinkage */
    EOS_MILLIMETER accumulatedShrinkageCompensation;

    /*! Total Z travel :=
    *                    preProcessHeight +
    *                    bottomOffsetHeight +
    *                    payloadHeight +
    *                    topOffsetHeight +
    *                    postProcessHeight +
    *                    accumulatedShrinkageCompensation
    */
    EOS_MILLIMETER totalZTravel;

    /*! Maximum Z travel of the target system */
    EOS_MILLIMETER maxZTravel;

    /*! Memory block reserved for future extensions */
    EOS_BYTE       reserved[64];

} EOS_PROCESS_HEIGHTS;

/*!
 * \brief       A structure containing all configuration parameters for linear & smart scaling.
 * The resulting scaling factors are calculated by (100.0 + <scalingPercent>) / 100.0,
 * i.e. a parameter value of 0 results in no change in size and a value of 10 results in scaling by 1.1
 *
 * Parameters that are not used must be set to zero, otherwise functions will return EOS_ERROR
 */
typedef struct {

    /*! Parameters for linear and smart scaling */

    /*  to transform mesh to the right position in the building platform if necessary*/
    EOS_FLOAT transformMatrix4x4[16];

    /*! scaling in x direction */
    EOS_DOUBLE scalingPercentX;

    /*! scaling in y direction */
    EOS_DOUBLE scalingPercentY;

    /*! Parameters for linear scaling only*/

    /*! scaling in z direction (at the bottom of the build envelope)*/
    EOS_DOUBLE scalingPercentZBottom;

     /*! scaling in z direction (at the top of the build envelope)*/
    EOS_DOUBLE scalingPercentZTop;

    /*! Parameters for smart scaling only*/

    /*! Job height in mm */
    EOS_DOUBLE jobHeightInMm;

    /*! Linear scaling percentage at z=0 */
    EOS_DOUBLE linearLoweringPercentageZBottom;

    /*! Linear scaling percentage at z-top */
    EOS_DOUBLE linearLoweringPercentageZTop;

    /*! Memory block reserved for future extensions */
    EOS_BYTE reserved[64];

} EOS_SCALING_OPTIONS;

typedef struct
{
    /*! Pointer to the image data */
    EOS_BYTE const * data;

    /*! Size of the image data in bytes.
     *  dataSize = rowSizeWithPadding * rowCount */
    EOS_SIZET dataSize;

    /*! Number of image rows.
     *  Image data is organized in row major. */
    EOS_SIZET rowCount;

    /*! Number of image columns,
     *  i.e., number of pixels per row. */
    EOS_SIZET columnCount;

    /*! Number of bytes per row, including padding bytes. */
    EOS_SIZET rowSizeWithPadding;

    /*! The size of a single pixel in bytes */
    EOS_SIZET pixelSize;

    /*! Color space of the image */
    EOS_IMAGE_COLOR_SPACE colorSpace;

    /*! Data type of the image pixel */
    EOS_IMAGE_CHANNEL_FORMAT channelFormat;

    /*! Memory block reserved for future extensions */
    EOS_BYTE reserved[72];

} EOS_IMAGE;

/* END OF SUBSECTION: COMPOSITE TYPES */

/* BEGIN OF SUBSECTION: FUNCTION TYPES */

/*!
 * \brief       Callback function type to be used in EosTaskGen_SetStateChangeCallback
 * \param[in]   customDataHandle                            A possible custom data pointer which is passed as argument to the callback
 *                                                          EOS_NO_DATA if none
 * \param[in]   state                                       The new state
 */
typedef void(*EOS_STATE_CHANGE_CALLBACK) (EOS_DATA_HANDLE customDataHandle, EOS_TASK_STATE state);

/*!
 * \brief       Callback function type to be used in EosTaskGen_SetProgressCallback
 * \param[in]   customDataHandle                            A possible custom data pointer which is passed as argument to the callback
 *                                                          EOS_NO_DATA if none
 * \param[in]   finishedIndex                               The index of the layer whose calculation has been finished
 * \param[in]   finishedHeight_um                           The absolute height above the building platform that has been finished (in um)
 */
typedef void(*EOS_GENERATION_PROGRESS_CALLBACK) (EOS_DATA_HANDLE customDataHandle, EOS_LAYERINDEX finishedIndex, EOS_MICROMETER finishedHeight_um);

/*!
 * \brief       Callback function type to be used in EosMachineSession_SetFileTransferProgressCallback
 * \param[in]   customDataHandle                            A possible custom data pointer which is passed as argument to the callback
 *                                                          EOS_NO_DATA if none
 * \param[in]   handle                                      The handle of the reporting file transfer
 * \param[in]   progress                                    The progress as a value from the closed range [0.0; 1.0]
 */
typedef void(*EOS_FILETRANSFER_PROGRESS_CALLBACK) (EOS_DATA_HANDLE customDataHandle, EOS_FILETRANSFER_HANDLE handle, EOS_DOUBLE progress);

/*!
 * \brief       Callback function type to be used in EosMachineSession_SetFileTransferStatusCallback
 * \param[in]   customDataHandle                            A possible custom data pointer which is passed as argument to the callback
 *                                                          EOS_NO_DATA if none
 * \param[in]   handle                                      The handle of the reporting file transfer
 * \param[in]   status                                      The status of the reporting file transfer
 */
typedef void (*EOS_FILETRANSFER_STATUS_CALLBACK)(EOS_DATA_HANDLE customDataHandle, EOS_FILETRANSFER_HANDLE handle, EOS_TRANSFER_STATUS status);

/*!
 * \brief       Callback function type to be used in EosTaskGen_SetNewBuildTimeEstimateCallback
 * \param[in]   customDataHandle                            A possible custom data pointer which is passed as argument to the callback
 *                                                          EOS_NO_DATA if none
 * \param[in]   info                                        Pointer to current build time estimation information struct.
 *                                                          The pointer will dangle once the callback returns, only keep deep copies.
 * \note        Shall not throw, otherwise the behavior is undefined
 */
typedef void(*EOS_BUILD_TIME_PROGRESS_CALLBACK) (EOS_DATA_HANDLE customDataHandle, EOS_BUILDTIMEINFO const * info);

/*!
 * \brief       Callback function type to be used in Eos_SetEventCallback
 * \param[in]   customDataHandle                            A possible custom data pointer which is passed as argument to the callback
 *                                                          EOS_NO_DATA if none
 * \param[in]   event                                       Pointer to the current event struct.
 *                                                          The pointer and all contained pointers will dangle once the callback returns, only keep deep copies.
 * \note        Shall not throw, otherwise the behavior is undefined
 */
typedef void(*EOS_EVENT_CALLBACK) (EOS_DATA_HANDLE customDataHandle, EOS_EVENT const * event);

/*!
 * \brief       Callback function type to be used in EosTaskGen_SetDatafileCompletedCallback
 * \param[in]   customDataHandle                            A possible custom data pointer which is passed as argument to the callback
 *                                                          EOS_NO_DATA if none
 * \param[in]   pathToDataFile                              Path to the completed *.data file (to be transferred before the task file)
 * \param[in]   pathToTaskFile                              Path to the completed *.task file (to be transferred after the data file)
 *                                                          Both path pointers will dangle once the callback returns, only keep deep copies.
 */
typedef void(*EOS_DATAFILE_COMPLETED_CALLBACK) (EOS_DATA_HANDLE customDataHandle, EOS_CWSTR pathToDataFile, EOS_CWSTR pathToTaskFile);

/* END OF SUBSECTION: FUNCTION TYPES */

#endif // EOS_EOSPRINT_API_TYPES_H
