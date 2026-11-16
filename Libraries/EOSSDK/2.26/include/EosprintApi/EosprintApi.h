/*!
 * \file        EosprintApi.h
 * \brief       Definition of the EOSPRINT 2 API
 *
 * \author      EOS GmbH, Robert-Stirling-Ring 1, 82152 Krailling, Germany
 * \copyright   (c) EOS GmbH 2019
 *              All rights reserved
 */

#ifndef EOS_EOSPRINT_API_H
#define EOS_EOSPRINT_API_H

#ifdef EOS_EOSPRINT_API_IMPLEMENTATION
#   define EOS_EOSPRINT_API __declspec(dllexport)
#else
#   define EOS_EOSPRINT_API __declspec(dllimport)
#endif

#include "Constants.h"
#include "EventIds.h"
#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

    /* BEGIN OF GENERAL API FUNCTIONS */

    /*!
     * \brief           Initializes the API
     * \param[in]       pathToLoggingProperties                 File path to a log4 properties file specifying loggers and log levels.
     *                                                          Pass NULL to disable logging. For recognized loggers & log levels see
     *                                                          the EOSPRINT 2 SDK documentation.
     * \return          EOS_ERR_NO_ERROR                        if initialization was successful
     * \return          EOS_ERR_FILE_NOT_FOUND                  if logger could not start, e.g. logging properties file was not found
     * \return          EOS_ERR_LOADING_DLL_FAILED              if a DLL could not be loaded because the file is missing
     *                                                          or a necessary license is missing
     * \return          EOS_ERR_MISSING_LICENSE                 if the EOSPRINT2 license is missing
     * \return          EOS_ERR_INTERNAL_ERROR                  if initialization fails due to an unexpected error
     * \attention       You must initialize the API before using any other function except Eos_GetApiVersion().
     * \warning         You need to call Eos_DeinitializeApi() before unloading the API DLL, otherwise deadlocks may occur.
     * \note            Additional calls to this function (or to Eos_InitializeApiWithOptions()) will have no effect.
     * \note            This function is thread safe.
     * \note            All calls to this function, to Eos_InitializeApiWithOptions(), and Eos_DeinitializeApi() are synchronized with a mutex,
     *                  the execution order of concurrent calls is undefined.
     */
    EOS_EOSPRINT_API EOS_ERROR Eos_InitializeApi(EOS_CWSTR pathToLoggingProperties);

    /*!
     * \brief           Initializes the API allowing for a fine-grained control of what to initialize and how.
     * \param[in]       options                                 EOS_API_OPTIONS struct. See the documentation for more information
     * \return          EOS_ERR_NO_ERROR                        if initialization was successful
     * \return          EOS_ERR_FILE_NOT_FOUND                  if a logging properties file was specified but not found
     * \return          EOS_ERR_LOADING_DLL_FAILED              if a DLL could not be loaded because the file is missing
     *                                                          or a necessary license is missing
     * \return          EOS_ERR_ARGUMENT_INVALID                if value in options->codemeterLicensing was unknown or out of bounds.
     * \return          EOS_ERR_MISSING_LICENSE                 if the EOSPRINT2 license is missing
     * \return          EOS_ERR_INTERNAL_ERROR                  if initialization fails due to an unexpected error
     * \attention       You must initialize the API before using any other function except Eos_GetApiVersion().
     * \warning         You need to call Eos_DeinitializeApi() before unloading the API DLL, otherwise deadlocks may occur.
     * \warning         Depending on the selected options, this function may alter the environment of the running process (see EOS_API_OPTIONS).
     *                  Any changes to the environment will be undone on deinitialization.
     * \note            Additional calls to this function (or to Eos_InitializeApi()) will have no effect.
     * \note            The caller must zero initialize EOS_API_OPTIONS::reserved to ensure forward compatibility with future extensions.
     * \note            This function is thread safe.
     * \note            All calls to this function, to Eos_InitializeApi(), and Eos_DeinitializeApi() are synchronized with a mutex,
     *                  the execution order of concurrent calls is undefined.
     */
    EOS_EOSPRINT_API EOS_ERROR Eos_InitializeApiWithOptions(EOS_API_OPTIONS const * options);

    /*!
     * \brief           Deinitializes the interface
     * \return          EOS_ERR_NO_ERROR                        if deinitialization was successful
     * \return          EOS_ERR_INTERNAL_ERROR                  if deinitialization fails
     * \attention       Call this before unloading the API DLL
     * \note            Additional calls to this function will not have any effect.
     * \note            This call will free all task info and material set info structs
     *                  that have not been freed yet
     * \note            This function is thread safe.
     *                  All calls to this function and Eos_InitializeApi() will be queued, the last call wins.
     * \warning         Depending on loaded jobs and cached geometries this call may take some time.
     *                  Do not deinitialize in situations with hard timeout (e.g. the .NET GC),
     *                  otherwise deinitialization may be interrupted which will lead to deadlocked threads
     *                  and blocked resources.
     */
    EOS_EOSPRINT_API EOS_ERROR Eos_DeinitializeApi();

    /*!
     * \brief           Returns the current API version of the EOSPRINT 2 API.
     * \param[out]      apiVersion                              output variable for the EOSPRINT 2 API version
     * \return          EOS_ERR_NO_ERROR                        if successful
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \note            The API version is incremented with every release of the EOSPRINT 2 API that
     *                  introduces breaking changes. However it is not incremented, if only non-breaking
     *                  changes are introduced. E.g., if EOSPRINT 2.x only contains additional functions,
     *                  but does not remove or change the signature of existing functions then the API version
     *                  is not incremented.
     * \note            This function is thread safe.
     * \note            For more information on API compatibility see the documentation.
     */
    EOS_EOSPRINT_API EOS_ERROR Eos_GetApiVersion(EOS_UINT * apiVersion);

    /*!
     * \brief           Returns the current product version of EOSPRINT.
     * \param[out]      productVersion                          output variable for the EOSPRINT product version
     * \return          EOS_ERR_NO_ERROR                        if successful
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \note            The product version reflects the version of EOSPRINT and is read
     *                  from the DLL version information. This function works without prior initialization.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR Eos_GetProductVersion(EOS_VERSION * productVersion);

    /*!
     * \brief           Registers a custom function that will be called whenever an error or warning occurs.
     * \param[in]       callback                                function that is to be called to notify about errors and warnings.
     * \param[in]       customDataHandle                        handle to custom data or custom pointer which is passed through
     *                                                          as argument to the callback; EOS_NO_DATA if none
     * \return          EOS_ERR_NO_ERROR                        if the callback was successfully set
     * \return          EOS_ERR_NOT_IDLE                        if a task creation or preview creation or a build time estimation
     *                                                          is currently running
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to remove an existing callback.
     * \note            Errors and warnings may occur within most of the TaskGen functions, not only during task creation.
     *                  Therefore, it is advisable to register an event callback early - ideally immediately after calling
     *                  Eos_InitializeApi().
     * \note            All members of the EOS_EVENT event argument passed to the callback, will be freed right afterward
     *                  the callback invocation automatically. Do not store nor try to delete manually.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR Eos_SetEventCallback(EOS_EVENT_CALLBACK callback, EOS_DATA_HANDLE customDataHandle);

    /*!
     * \brief           Resets all callbacks, that is, all callback functions registered through Set*Callback() functions will no longer be called.
     * \return          EOS_ERR_NO_ERROR                        if all callbacks were successfully removed
     * \return          EOS_ERR_TASK_CREATION_RUNNING           if callbacks could not be removed due to a running task creation
     * \return          EOS_ERR_BUILDTIME_ESTIMATION_RUNNING    if callbacks could not be removed due to a running build time estimation
     * \return          EOS_ERR_FILETRANSFER_RUNNING            if callbacks could not be removed due to a running file transfer
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR Eos_ResetAllCallbacks();

    /* END OF GENERAL API FUNCTIONS */

    /* BEGIN OF FILE PROPERTIES FUNCTIONS */

    /*!
     * \brief           Retrieves information about a given Material Set
     * \param[in]       pathToEospar                            path to the material set file (.eospar)
     * \param[out]      materialSetInfo                         output structure for material set info
     * \return          EOS_ERR_NO_ERROR                        if the material set info was successfully retrieved
     * \return          EOS_ERR_ARGUMENT_INVALID                if either of the argument pointers is NULL
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if the file could not be accessed due to a file IO error
     * \return          EOS_ERR_FILE_NOT_FOUND                  if the file was not found
     * \return          EOS_ERR_EOSPAR_CORRUPT                  if the file is corrupt, i.e. does not contain a valid material set
     * \return          EOS_ERR_EOSPAR_INVALID                  if the file contains invalid or empty values
     * \return          EOS_ERR_OUT_OF_MEMORY                   if allocating memory failed
     * \return          EOS_ERR_ENCODING_CONVERSION_ERROR       if path to eospar argument could not be converted to the filesystem code page
     * \return          EOS_ERR_UNEXPECTED_ERROR                if an unexpected error occurred
     * \note            If the result is not EOS_ERR_NO_ERROR, then the output parameter materialSetInfo remains untouched.
     * \note            The function dynamically allocates memory which should be freed using function EosFileProperties_FreeMaterialSetInfo().
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosFileProperties_LoadMaterialSetInfo(EOS_CWSTR pathToEospar, EOS_MATERIALSET_INFO const ** materialSetInfo);

    /*!
     * \brief           Frees memory that was allocated by EosFileProperties_LoadMaterialSetInfo().
     * \param[in]       materialSetInfo                         material set info structure
     * \return          EOS_ERR_NO_ERROR                        if the material set info was successfully freed
     * \return          EOS_ERR_ARGUMENT_INVALID                if the material set info argument pointer is NULL or was not created
     *                                                          with EosFileProperties_LoadMaterialSet()
     * \return          EOS_ERR_UNEXPECTED_ERROR                if an unexpected error occurred
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosFileProperties_FreeMaterialSetInfo(EOS_MATERIALSET_INFO const * materialSetInfo);

    /*!
     * \brief           Retrieves cached files that are not needed by an OpenJob
     * \param[in]       pathToOpenJob                           path to the OpenJob file (.openjob)
     * \param[in]       pathToCacheDirectory                    path to the cache directory
     * \param[out]      filepathList                            output structure for filepath list; on success, this contains a list
     *                                                          of all files in the cache directory that are guaranteed to not be used
     *                                                          by the given OpenJob; the paths are given relative to the cache directory
     * \return          EOS_ERR_NO_ERROR                        if the filepath list was successfully retrieved
     * \return          EOS_ERR_ARGUMENT_INVALID                if the filepath list argument pointer is NULL
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if the OpenJob file and/or cache directory could not be accessed due to a file IO error
     * \return          EOS_ERR_OPENJOB_NOT_FOUND               if the OpenJob file was not found
     * \return          EOS_ERR_OPENJOB_CORRUPT                 if the OpenJob file cannot be loaded
     * \return          EOS_ERR_OPENJOB_INVALID                 if the OpenJob contains invalid data
     * \return          EOS_ERR_OPENJOB_NOT_SUPPORTED           if the OpenJob references an unsupported OpenJob version
     * \return          EOS_ERR_PART_FILE_NOT_FOUND             if the OpenJob references a part file that was not found
     * \return          EOS_ERR_NOT_A_DIRECTORY                 if the cache directory is not an existing directory
     * \return          EOS_ERR_OUT_OF_MEMORY                   if allocating memory failed
     * \return          EOS_ERR_UNEXPECTED_ERROR                if an unexpected error occurred
     * \note            If the result is not EOS_ERR_NO_ERROR, then the output parameter filepathList remains untouched.
     * \note            The function dynamically allocates memory which should be freed using function EosFileProperties_FreeFilepathList().
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosFileProperties_LoadFilepathListForCachePruning(
        EOS_CWSTR pathToOpenJob,
        EOS_CWSTR pathToCacheDirectory,
        EOS_FILEPATH_LIST const ** filepathList);

    /*!
     * \brief           Frees memory that was allocated by EosFileProperties_LoadFilePathList...().
     * \param[in]       filepathList                            filepath list structure.
     * \return          EOS_ERR_NO_ERROR                        if the filepath list was successfully freed
     * \return          EOS_ERR_ARGUMENT_INVALID                if the filepath list argument pointer is NULL or was not created
     *                                                          with EosFileProperties_LoadFilepathList...()
     * \return          EOS_ERR_UNEXPECTED_ERROR                if an unexpected error occurred
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosFileProperties_FreeFilepathList(EOS_FILEPATH_LIST const * filepathList);

    /*!
     * \brief           Retrieves information about a given task file.
     * \param[in]       pathToTaskFile                          file path to the task file (.task)
     * \param[out]      taskInfo                                output variable for task info structure; if successful,
     *                                                          this will hold information about the task file - otherwise
     *                                                          remains untouched
     * \return          EOS_ERR_NO_ERROR                        if the task info was successfully retrieved
     * \return          EOS_ERR_ARGUMENT_INVALID                if the either of the arguments is NULL
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if the file could not be accessed due to a file IO error
     * \return          EOS_ERR_FILE_NOT_FOUND                  if the file was not found
     * \return          EOS_ERR_TASK_CORRUPT                    if the file is corrupt, i.e. does not contain a valid task
     * \return          EOS_ERR_TASK_INVALID                    if the file contains invalid or empty values
     * \return          EOS_ERR_OUT_OF_MEMORY                   if allocating memory failed
     * \return          EOS_ERR_ENCODING_CONVERSION_ERROR       if the networkAddress could not be converted to ANSI codepage
     * \return          EOS_ERR_UNEXPECTED_ERROR                if an unexpected error occurred
     * \note            If the result is not EOS_ERR_NO_ERROR, then the output parameter taskInfo remains untouched.
     * \note            The function dynamically allocates memory which should be freed using function EosFileInfo_FreeTaskInfo().
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosFileProperties_LoadTaskInfo(EOS_CWSTR pathToTaskFile, EOS_TASK_INFO const ** taskInfo);

    /*!
     * \brief           Frees memory that was allocated by EosFileProperties_LoadTaskInfo().
     * \param[in]       taskInfo                                task info structure
     * \return          EOS_ERR_NO_ERROR                        if the task info was successfully freed
     * \return          EOS_ERR_ARGUMENT_INVALID                if the task info pointer argument is NULL or was not created
     *                                                          with EosFileProperties_LoadTaskInfo()
     * \return          EOS_ERR_UNEXPECTED_ERROR                if an unexpected error occurred
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosFileProperties_FreeTaskInfo(EOS_TASK_INFO const * taskInfo);

    /*!
     * \brief           Checks if the given task was created from the given machine configuration. If not, it means that either the local machine configuration or the
     *                  machine configuration incorporated by the given task file is out-dated. Note that the machine configuration may contain adjustment values from
     *                  a previous calibration which may have an effect on the exposure line calculation. Therefore, use this function to check a local task file against
     *                  an up-to-date machine configuration. If differences are detected, we recommend to re-create the task using the updated machine configuration.
     * \param[in]       pathToTaskFile                          file path to the task file (.task)
     * \param[in]       pathToConfigsDirectory                  directory path to local directory containing the (downloaded) machine configurations
     * \param[out]      matches                                 output variable for result (must not be null):
     *                                                           - EOS_TRUE: The configurations used to create the task match the configurations in the given machine
     *                                                             configurations directory
     *                                                           - EOS_FALSE: One or more configurations used to create the task differ from the configurations in the
     *                                                             given machine configurations directory
     * \return          EOS_ERR_NO_ERROR                        if the check was successful. Refer to match for result.
     * \return          EOS_ERR_ARGUMENT_INVALID                if match is null
     * \return          EOS_ERR_FILE_NOT_FOUND                  if the path does not reference a regular file
     * \return          EOS_ERR_TASK_CORRUPT                    if the referenced file cannot be loaded or if associated files are missing
     * \return          EOS_ERR_TASK_INVALID                    if the referenced file contains invalid or empty values
     * \return          EOS_ERR_TASK_UNSUPPORTED_VERSION        if the task was created with an older version of this Api and is not supported by this procedure
     * \return          EOS_ERR_ILLEGAL_CONFIG_DIR              if the provided config directory does not exist or is not a directory
     * \return          EOS_ERR_CONFIG_INVALID                  if the provided config directory does not contain the necessary configuration to perform this check
     *                                                          (i.e., one or more configuration files are missing)
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosFileProperties_DoesTaskConfigMatchLocalConfig(EOS_CWSTR pathToTaskFile,
        EOS_CWSTR pathToConfigsDirectory,
        EOS_BOOL * matches);

    /*!
     * \brief Reads meta data about a machine from the given machine configuration.
     * \param[in]       pathToConfigsDirectory                  directory path to local directory containing the (downloaded) machine configurations
     * \param[out]      machineInfo                             output variable for machine info structure; remains untouched in case of error
     * \return          EOS_ERR_NO_ERROR                        if successful
     * \return          EOS_ERR_NOT_A_DIRECTORY                 if /p pathToConfigsDirectory does not exist or is not a directory
     * \return          EOS_ERR_CONFIG_CORRUPT                  if the machine configuration in the directory could not be read (e.g., because it is missing)
     * \return          EOS_ERR_CONFIG_INVALID                  if the machine configuration contains invalid data
     * \return          EOS_ERR_MACHINE_NOT_SUPPORTED           if the machine type is not supported
     * \return          EOS_ERR_OUT_OF_MEMORY                   if allocating memory failed
     * \return          EOS_ERR_UNEXPECTED_ERROR                if an unexpected error occurred
     * \note            This function is thread safe.
     * \note            The function dynamically allocates memory which should be freed using function EosFileProperties_FreeMachineInfo().
     * \note            If the machine configuration was downloaded with EosprintApi v1 then the variables machineSoftwareVersion
     *                  and machineInterfaceVersion will be all zeroes.
     */
    EOS_EOSPRINT_API EOS_ERROR EosFileProperties_LoadMachineInfo(EOS_CWSTR pathToConfigsDirectory, EOS_MACHINE_INFO const ** machineInfo);

    /*!
     * \brief           Frees memory that was allocated by EosFileProperties_LoadMachineInfo().
     * \param[in]       machineInfo                             machine info structure
     * \return          EOS_ERR_NO_ERROR                        if the task info was successfully freed
     * \return          EOS_ERR_ARGUMENT_INVALID                if the task info pointer argument is NULL or was not created
     *                                                          with EosFileProperties_LoadMachineInfo()
     * \return          EOS_ERR_UNEXPECTED_ERROR                if an unexpected error occurred
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosFileProperties_FreeMachineInfo(EOS_MACHINE_INFO const * machineInfo);

    /*!
     * \brief           Retrieves information about a given simulation model
     * \param[in]       pathToEossim                            path to the EOS simulation file (.eossim)
     * \param[out]      simuInfo                                output structure for simulation info
     * \return          EOS_ERR_NO_ERROR                        if the simulation info was successfully retrieved
     * \return          EOS_ERR_ARGUMENT_INVALID                if either of the argument pointers is NULL
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if the file could not be accessed due to a file IO error
     * \return          EOS_ERR_FILE_NOT_FOUND                  if the file was not found
     * \return          EOS_ERR_EOSSIM_CORRUPT                  if the file is corrupt, i.e. does not contain a valid simulation model
     * \return          EOS_ERR_EOSSIM_INVALID                  if the file contains invalid or empty values
     * \return          EOS_ERR_OUT_OF_MEMORY                   if allocating memory failed
     * \return          EOS_ERR_ENCODING_CONVERSION_ERROR       if path to eossim argument could not be converted to the filesystem code page
     * \return          EOS_ERR_UNEXPECTED_ERROR                if an unexpected error occurred
     * \note            If the result is not EOS_ERR_NO_ERROR, then the output parameter simuInfo remains untouched.
     * \note            The function dynamically allocates memory which should be freed using function EosFileProperties_FreeSimulationInfo().
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosFileProperties_LoadSimulationInfo(EOS_CWSTR pathToEossim, EOS_SIMULATION_INFO const ** simuInfo);

    /*!
     * \brief           Frees memory for a simulation model info.
     * \param[in]       simuInfo                                simulation info structure
     * \return          EOS_ERR_NO_ERROR                        if the simulation info was successfully freed
     * \return          EOS_ERR_ARGUMENT_INVALID                if the simulation info pointer argument is NULL or was not created
     *                                                          with EosFileProperties_LoadSimulationInfo()
     * \return          EOS_ERR_UNEXPECTED_ERROR                if an unexpected error occurred
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosFileProperties_FreeSimulationInfo(EOS_SIMULATION_INFO const * simuInfo);

    /* END OF FILE PROPERTIES FUNCTIONS */

    /* BEGIN OF MACHINE SESSION HANDLING */

    /*!
     * \brief           Establishes a connection for file exchange with the machine.
     *                  Use this function if you do not know the model type of the target machine and thus need to have EOSPRINT determine the machine model type by itself.
     *                  Otherwise, please use EosMachineSession_CreateWithHint with the appropriate model type hint in order to connect as quick as possible.
     * \param[in]       networkAddress                          the IP address or host name of the machine (convertible to ANSI codepage)
     * \param[in]       pathToCertificateDirectory              the local directory where OPC certificates are stored (will be created if it does not exist)
     *                                                          Due to 3rd party restrictions, the path must only contain ASCII characters (0 - 127).
     * \param[in]       pathToTemporaryDirectory                a local directory where temporary data will be stored, existing files may be overwritten without warning
     * \param[out]      sessionHandle                           the handle for the created session will be assigned to the pointer to session handle
     * \return          EOS_ERR_NO_ERROR                        if everything went well
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_TOO_MANY_SESSIONS               if a session was already created; only one session is allowed at a time
     * \return          EOS_ERR_MACHINE_NOT_SUPPORTED           if the machine software version is not supported
     * \return          EOS_ERR_OUT_OF_MEMORY                   if allocating memory failed
     * \return          EOS_ERR_NETWORK_IO_ERROR                if a connection to the addressed machine could not be established
     * \return          EOS_ERR_ENCODING_CONVERSION_ERROR       if the networkAddress could not be converted to ANSI codepage or
     *                                                          if any of the paths could not be converted to the filesystem API codepage
     * \return          EOS_ERR_PATH_CONTAINS_ILLEGAL_CHARS     if \p pathToCertificateDirectory contains non-ASCII characters
     * \return          EOS_ERR_CONFIG_CORRUPT                  if the connection could be established but the machine configuration could not be read
     * \return          EOS_ERR_CONFIG_INVALID                  if the connection could be established but the machine configuration contains invalid data
     * \return          EOS_ERR_NOT_A_DIRECTORY                 if the temporary directory does not exist or is not a directory
     * \note            Currently the EOSPRINT 2 API supports only one session, i.e. to create a new session, the previous must be destroyed.
     * \note            All functions operating on the created session handle will be synchronized.
     * \note            The network address must be convertible to the ANSI codepage.
     * \note            After this call returns the temporary directory can safely be deleted
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_Create(
        EOS_CWSTR networkAddress,
        EOS_CWSTR pathToCertificateDirectory,
        EOS_CWSTR pathToTemporaryDirectory,
        EOS_SESSION_HANDLE * sessionHandle);

    /*!
     * \brief           Establishes a connection for file exchange with the machine.
     *                  Use this function if you know the model type of the target machine.
     *                  If the model hint is accurate, establishing a connection will be as quick as possible. If the hint is inaccurate, the function will continue
     *                  to establish the connection by falling back to automated model type identification, potentially losing any performance benefits.
     * \param[in]       networkAddress                          the IP address or host name of the machine (convertible to ANSI codepage)
     * \param[in]       pathToCertificateDirectory              the local directory where OPC certificates are stored (will be created if it does not exist)
     *                                                          Due to 3rd party restrictions, the path must only contain ASCII characters (0 - 127).
     * \param[in]       pathToTemporaryDirectory                a local directory where temporary data will be stored, existing files may be overwritten without warning
     * \param[in]       modelTypeHint                           a machine type identifier required to establish the appropriate connection processing protocol
     * \param[out]      sessionHandle                           the handle for the created session will be assigned to the pointer to session handle
     * \return          EOS_ERR_NO_ERROR                        if everything went well
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_TOO_MANY_SESSIONS               if a session was already created; only one session is allowed at a time
     * \return          EOS_ERR_MACHINE_NOT_SUPPORTED           if the machine software version is not supported
     * \return          EOS_ERR_OUT_OF_MEMORY                   if allocating memory failed
     * \return          EOS_ERR_NETWORK_IO_ERROR                if a connection to the addressed machine could not be established
     * \return          EOS_ERR_ENCODING_CONVERSION_ERROR       if the networkAddress could not be converted to ANSI codepage or
     *                                                          if any of the paths could not be converted to the filesystem API codepage
     * \return          EOS_ERR_PATH_CONTAINS_ILLEGAL_CHARS     if \p pathToCertificateDirectory contains non-ASCII characters
     * \return          EOS_ERR_CONFIG_CORRUPT                  if the connection could be established but the machine configuration could not be read
     * \return          EOS_ERR_CONFIG_INVALID                  if the connection could be established but the machine configuration contains invalid data
     * \return          EOS_ERR_NOT_A_DIRECTORY                 if the temporary directory does not exist or is not a directory
     * \note            Currently the EOSPRINT 2 API supports only one session, i.e. to create a new session, the previous must be destroyed.
     * \note            All functions operating on the created session handle will be synchronized.
     * \note            The network address must be convertible to the ANSI codepage.
     * \note            After this call returns the temporary directory can safely be deleted
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_CreateWithHint(
            EOS_CWSTR networkAddress,
            EOS_CWSTR pathToCertificateDirectory,
            EOS_CWSTR pathToTemporaryDirectory,
            EOS_MODEL_TYPE modelTypeHint,
            EOS_SESSION_HANDLE * sessionHandle);

    /*!
     * \brief           Destroys a machine session.
     * \param[in]       sessionHandle                           the handle of the machine session to destroy.
     * \return          EOS_ERR_NO_ERROR                        if the machine session was successfully destroyed.
     * \return          EOS_ERR_SESSION_INVALID                 if the passed session handle is invalid
     * \return          EOS_ERR_FILETRANSFER_RUNNING            if the machine session has running file transfers
     * \note            Destroying the session will safely release all file transfers which have not been released yet
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_Destroy(EOS_SESSION_HANDLE sessionHandle);

    /*!
     * \brief           Begins the download of configuration files from a machine.
     * \param[in]       sessionHandle                           session handle representing a connection to a machine
     * \param[in]       pathToLocalConfigDirectory              path to a local directory to which the configuration files should be downloaded
     *                                                          (must be convertible to the narrow filesystem api). Do not mix configuration files
     *                                                          of multiple machines in one directory - ideally, reserve one directory per machine.
     * \param[out]      fileTransferHandle                      output variable for file transfer handle; if the call is successful, this will hold
     *                                                          a handle representing the file transfer. You can use the handle to check the transfer status.
     * \return          EOS_ERR_NO_ERROR                        if the download was successfully queued
     * \return          EOS_ERR_SESSION_INVALID                 if the passed session handle is invalid
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_OUT_OF_MEMORY                   if memory allocation failed
     * \return          EOS_ERR_ENCODING_CONVERSION_ERROR       if a directory path was passed that cannot be converted to the narrow file api codepage
     * \note            The given directory is created if it does not exist
     * \note            If the given directory already exists, it will not be cleaned before download. Existing files may be overwritten.
     * \note            The file transfer handle is dynamically allocated and once it is no longer needed it should be freed by
     *                  calling EosMachineSession_ReleaseFileTransferHandle().
     * \note            If additional up-/downloads are started before the first has completed, they will be queued and executed one by one.
     * \note            Transfers can be aborted with EosMachineSession_AbortFileTransfer().
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_BeginConfigDownload(
        EOS_SESSION_HANDLE sessionHandle,
        EOS_CWSTR pathToLocalConfigDirectory,
        EOS_FILETRANSFER_HANDLE * fileTransferHandle);


    /*!
     * \brief           Begins upload of a local .task, .task.<index> or .data file to a machine.
     * \param[in]       sessionHandle                           session handle representing a connection to a machine
     * \param[in]       pathToLocalTaskOrDataFile               path to .task, .task.<index> or .data file
     * \param[out]      fileTransferHandle                      output variable for file transfer handle; if successful, this will hold
     *                                                          a handle representing the file transfer; this can be used to check the transfer status
     * \return          EOS_ERR_NO_ERROR                        if the upload was successful
     * \return          EOS_ERR_SESSION_INVALID                 if the passed session handle is invalid, e.g., because it was already destroyed
     * \return          EOS_ERR_ENCODING_CONVERSION_ERROR       if a path was used that cannot be converted to the narrow file api codepage
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if a filesystem error is encountered, e.g., due to missing access rights
     * \return          EOS_ERR_FILE_NOT_FOUND                  if the specified local file was not found
     * \return          EOS_ERR_ILLEGAL_FILE_EXTENSION          if the file exists, but is not recognized as .task or .data file
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_FILETRANSFER_FAILED             if a communication error occurred before or during upload
     * \note            When you stream data to the machine (i.e. upload task/data files when task generation is still in progress)
     *                  always use the task file with the highest index and make sure the referenced data file is uploaded. E.g. during
     *                  generation, a file "<filename>001.data" will create a corresponding "<filename>.task.1" file that can be used
     *                  for upload when the task generation ist still in progress.
     *                  The final ".task" index file will not have a trailing index and reference all data files written.
     * \note            If the function fails to begin the upload, the output parameter fileTransferHandle remains untouched.
     * \note            The file transfer handle is dynamically allocated and once it is no longer needed it should be freed by
     *                  calling EosMachineSession_ReleaseFileTransferHandle().
     * \note            If additional up-/downloads are started before the first has completed, they will be queued and executed one by one.
     * \note            Transfers can be aborted with EosMachineSession_AbortFileTransfer().
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_BeginTaskUpload(
        EOS_SESSION_HANDLE sessionHandle,
        EOS_CWSTR pathToLocalTaskOrDataFile,
        EOS_FILETRANSFER_HANDLE * fileTransferHandle);

    /*!
     * \brief           Aborts an active file transfer.
     * \param[in]       fileTransferHandle                      the handle of the transfer that is to be aborted
     * \return          EOS_ERR_NO_ERROR                        if the transfer was still active and its interrupt was signalled successfully
     * \return          EOS_ERR_FILETRANSFER_INVALID            if the passed file transfer handle is invalid
     * \return          EOS_ERR_FILETRANSFER_NOT_RUNNING        if the corresponding transfer had already stopped
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_AbortFileTransfer(EOS_FILETRANSFER_HANDLE fileTransferHandle);

    /*!
     * \brief           Aborts all active file transfers running on the specified session.
     * \param[in]       sessionHandle                           session handle representing a connection to a machine
     * \return          EOS_ERR_NO_ERROR                        if at least one transfer was still active and interrupt was
     *                                                          signaled successfully to all such transfers
     * \return          EOS_ERR_FILETRANSFER_NOT_RUNNING        if no file transfers were queued or running
     * \return          EOS_ERR_SESSION_INVALID                 if the passed session handle is invalid
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_AbortAllFileTransfers(EOS_SESSION_HANDLE sessionHandle);

    /*!
     * \brief           Returns the current status of a file transfer.
     * \param[in]       fileTransferHandle                      the handle of the transfer to return the status of
     * \return          the current transfer status
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_TRANSFER_STATUS EosMachineSession_GetFileTransferState(EOS_FILETRANSFER_HANDLE fileTransferHandle);

    /*!
     * \brief           Waits until an active transfer ends or a timeout occurs.
     * \param[in]       fileTransferHandle                      handle of the file transfer to wait for
     * \param[in]       timeout                                 timeout in milliseconds to wait before returning even though the transfer was still active;
     *                                                          pass EOS_INFINITE_DURATION to wait indefinitely.
     * \return          EOS_ERR_NO_ERROR                        if the specified file transfer has ended
     *                                                          (whether with a successful result or not)
     * \return          EOS_ERR_FILETRANSFER_INVALID            if the passed file transfer handle is invalid
     * \return          EOS_ERR_WAIT_TIMED_OUT                  if the timeout occurred before the file transfer ended
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_WaitForFileTransfer(EOS_FILETRANSFER_HANDLE fileTransferHandle, EOS_MILLISECS timeout);

    /*!
     * \brief           Returns the result of a finished file transfer
     * \param[in]       fileTransferHandle                      handle of the file transfer of which to get the return value
     * \return          EOS_ERR_NO_ERROR                        if the status of the file transfer is EOS_TS_COMPLETED
     * \return          EOS_ERR_FILETRANSFER_INVALID            if the passed file transfer handle is invalid
     * \return          EOS_ERR_FILETRANSFER_ABORTED            if the status of the file transfer is EOS_TS_ABORTED
     * \return          Any other return code depending on the specific file transfer if its status is EOS_TS_FAILED
     * \note            This function will block until the given file transfer has finished.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_GetFileTransferResult(EOS_FILETRANSFER_HANDLE fileTransferHandle);

    /*!
     * \brief           Waits until all active file transfers have ended or a timeout occurs.
     * \param[in]       sessionHandle                           session handle representing a connection to a machine
     * \param[in]       timeout                                 timeout in milliseconds to wait before returning even though a transfer was still active;
     *                                                          pass EOS_INFINITE_DURATION to wait indefinitely.
     * \return          EOS_ERR_NO_ERROR                        if the all file transfers have ended
     *                                                          (whether with a successful result or not)
     * \return          EOS_ERR_FILETRANSFER_NOT_RUNNING        if no file transfers were queued or running
     * \return          EOS_ERR_SESSION_INVALID                 if the passed session handle is invalid
     * \return          EOS_ERR_WAIT_TIMED_OUT                  if the timeout occurred before all file transfers ended
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_WaitForAllFileTransfers(EOS_SESSION_HANDLE sessionHandle, EOS_MILLISECS timeout);

    /*!
     * \brief           Releases a file transfer handle obtained through either EosMachineSession_BeginConfigDownload() or EosMachineSession_BeginTaskUpload().
     * \param[in]       fileTransferHandle                      file transfer handle that should be freed
     * \return          EOS_ERR_NO_ERROR                        if the handle was released
     * \return          EOS_ERR_FILETRANSFER_INVALID            if the passed file transfer handle is invalid
     * \return          EOS_ERR_FILETRANSFER_RUNNING            if the file transfer has not finished yet
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_ReleaseFileTransferHandle(EOS_FILETRANSFER_HANDLE fileTransferHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a file transfer progress has changed
     * \param[in]       sessionHandle                           session handle representing a connection to a machine
     * \param[in]       callback                                function that is to be called to notify about file transfer progress
     * \param[in]       customDataHandle                        handle to custom data or custom pointer which is passed through as argument
     *                                                          to the callback; EOS_NO_DATA if none
     * \return          EOS_ERR_NO_ERROR                        if the callback was successfully set
     * \return          EOS_ERR_FILETRANSFER_RUNNING            if the callback could not be set caused by
     *                                                          already running file transfers
     * \return          EOS_ERR_INTERNAL_ERROR                  if an internal error occurred
     * \note            Only a single callback function for each session handle can be registered.
     *                  Calling the function twice will unregister the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            Currently, the EOSPRINT 2 API is not capable of reporting the transfer progress of an individual file, i.e. you
     *                  will only be notified once the transfer of a file is completed. However, the function
     *                  EosMachineSession_BeginConfigDownload() transfers several files (yet, represented by only one file transfer handle).
     *                  Therefore, you will receive intermediate progress notifications for the download of a machine configuration.
     * \note            Progress is managed for every active file transfer separately, i.e. if you have queued several file transfers
     *                  you will receive notifications about the individual progress of each such transfer, not for the queue as a whole.
     * \note            As file transfers are processed sequentially your callback need not be thread safe
     * \note            This function is thread safe. A changed callback will have immediate effect including currently executed file transfers
     * \warning         The callback is executed synchronously from within an internal thread. Hence strive to do as little work as possible.
     *                  Do not release the file transfer handle which triggered the callback or destroy the machine session for which the transfer was initiated.
     *                  Do not wait for the file transfer which triggered the callback. This includes waiting for all file transfers of the machine
     *                  session for which the transfer was initiated.
     *
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_SetFileTransferProgressCallback(EOS_SESSION_HANDLE sessionHandle,
        EOS_FILETRANSFER_PROGRESS_CALLBACK callback,
        EOS_DATA_HANDLE customDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a file transfer status has changed
     * \param[in]       sessionHandle                           session handle representing a connection to a machine
     * \param[in]       callback                                function that is to be called to notify about file transfer status change
     * \param[in]       customDataHandle                        handle to custom data or custom pointer which is passed through as argument
     *                                                          to the callback; EOS_NO_DATA if none
     * \return          EOS_ERR_NO_ERROR                        if the callback was successfully set
     * \return          EOS_ERR_FILETRANSFER_RUNNING            if the callback could not be set caused by
     *                                                          already running file transfers
     * \return          EOS_ERR_INTERNAL_ERROR                  if an internal error occurred
     * \note            Only a single callback function for each session handle can be registered.
     *                  Calling the function twice will unregister the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            The transfer status is managed for every active file transfer separately, i.e. if you have queued several file transfers
     *                  you will receive notifications about the individual status of each such transfer, not for the queue as a whole.
     * \note            As file transfers are processed sequentially your callback need not be thread safe
     * \note            This function is thread safe. A changed callback will have immediate effect including currently executed file transfers
     * \warning         The callback is executed synchronously from within an internal thread. Hence strive to do as little work as possible.
     *                  Do not release the file transfer handle which triggered the callback or destroy the machine session for which the transfer was initiated.
     *                  Do not wait for the file transfer which triggered the callback. This includes waiting for all file transfers of the machine
     *                  session for which the transfer was initiated.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_SetFileTransferStatusCallback(EOS_SESSION_HANDLE sessionHandle,
        EOS_FILETRANSFER_STATUS_CALLBACK callback,
        EOS_DATA_HANDLE customDataHandle);

    //! \brief  Queries free disk space on the machine [MB]
    //! \param[in]   sessionHandle    The active machine session handle
    //! \param[out]  freeDiskSpaceMB  Free disk space on the machine in megabytes
    //! \returns     EOS_ERR_NO_ERROR on success, EOS_ERR_INTERNAL_ERROR if the REST query failed
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_GetFreeDiskSpaceMB(EOS_SESSION_HANDLE sessionHandle,
        double * freeDiskSpaceMB);

     /*!
     * \brief           Fetches the minimum free disk space on the machine which must always remain for the OS.
     * \param[in]       sessionHandle                           The active machine session handle
     * \param[out]      minFreeDiskSpace_Mb                     The minimum free disk spae in Megabytes.
     * \returns         EOS_ERR_NO_ERROR                        if the minimum disk space could be fetched
     * \return          EOS_ERRT_INVALID_ARGUMENT               if NULL was passed for \p minimumFreeDiskSpace_MB
     * \return          EOS_ERR_NOT_SUPPORTED_BY_MACHINE        if the machine is too old to supply this information. *
     * \note            If an upload of a task file would reduce the free disk space below this threshold, it will be blocked and fail with EOS_ERR_INSUFFICIENT_DISK_SPACE.
     *                  To avoid this, use EosMachineSession_GetFreeDiskSpace() to query the disk space currently free on the machine. Then ensure that reducing that value
     *                  by the size of the file to upload will not leave less space than the minimum returned by this function.
     */
    EOS_EOSPRINT_API EOS_ERROR EosMachineSession_GetMinimumFreeDiskSpace(EOS_SESSION_HANDLE sessionHandle,
        double * minimumFreeDiskSpace_Mb);

    /* END OF MACHINE SESSION HANDLING */

    /* BEGIN OF TASK GENERATION */

    /*!
     * \brief           Loads a machine configuration from the specified directory.
     * \param[in]       pathToConfigDirectory                   file path pointing to a directory with a complete set of machine configuration files
     *                                                          as obtained by EosMachineSession_BeginConfigDownload()
     * \return          EOS_ERR_NO_ERROR                        if the machine configuration was successfully loaded
     * \return          EOS_ERR_CONFIG_CORRUPT                  if the machine configuration could not be read
     * \return          EOS_ERR_NOT_A_DIRECTORY                 if the path to the config directory does not exist or is not a directory
     * \return          EOS_ERR_PATH_CONTAINS_ILLEGAL_CHARS     if the directory path contains illegal characters
     * \return          EOS_ERR_ILLEGAL_CONFIG_DIR              if the specified directory does not contain a valid machine configuration
     * \return          EOS_ERR_NOT_IDLE                        if a task creation or preview creation or a build time estimation is currently running
     * \return          EOS_ERR_STRING_CONTAINS_ILLEGAL_CHARS   if a string in the machine configuration contains illegal characters
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_CANNOT_UNLOAD                   if ToolpathAPI files are currently open
     * \note            Implicitly unloads a previously loaded machine configuration.
     * \note            You cannot load a new machine configuration while the TaskGen is busy creating a task, a preview or
     *                  a build time estimation. In this case, the function will fail with return code EOS_ERR_NOT_IDLE. Stop any
     *                  running task/preview creations and build time estimation before trying to load a new machine configuration.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_LoadMachineConfiguration(EOS_CWSTR pathToConfigDirectory);

    /*!
     * \brief           Returns machine info for the current machine
     * \param[in]       machineInfo                             EOS_MACHINE_INFO pointer which is to hold the current machine info pointer.
     *                                                          The lifetime of the underlying object is maintained by the EOSPRINT SDK.
     *                                                          Thus, it must not be freed by the caller and may become invalid if the function call is repeated.
     * \return          EOS_ERR_NO_ERROR                        if the information could be retrieved properly
     * \return          EOS_ERR_ARGUMENT_INVALID                if machineInfo is nullptr
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is currently loaded
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is currently loaded
     * \return          EOS_ERR_CONFIG_INVALID                  if the information contained in the configuration is invalid
     * \return          EOS_ERR_MACHINE_NOT_SUPPORTED           if the model type stored in the loaded configuration is not supported
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetMachineInfo(EOS_MACHINE_INFO const ** machineInfo);

    /*!
     * \brief           Returns simulation info for the currently loaded job
     * \param[in]       simuInfo                                EOS_SIMULATION_INFO pointer which is to hold the current simulation info pointer.
     *                                                          The lifetime of the underlying object is maintained by the EOSPRINT SDK.
     *                                                          Thus, it must not be freed by the caller and may become invalid if the function call is repeated.
     * \return          EOS_ERR_NO_ERROR                        if the information could be retrieved properly
     * \return          EOS_ERR_ARGUMENT_INVALID                if simuInfo is nullptr
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is currently loaded
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is currently loaded
     * \return          EOS_ERR_CONFIG_INVALID                  if the information contained in the configuration is invalid
     * \return          EOS_ERR_MACHINE_NOT_SUPPORTED           if the model type stored in the loaded configuration is not supported
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetSimulationInfo(EOS_SIMULATION_INFO const ** simuInfo);

    /*!
     * \brief           Unloads a currently loaded machine configuration.
     * \return          EOS_ERR_NO_ERROR                        if the machine configuration was successfully unloaded
     *                                                          or if no machine configuration was loaded.
     * \return          EOS_ERR_NOT_IDLE                        if a task or preview creation or a build time estimation is currently running
     * \note            You cannot unload the current machine configuration while the TaskGen is busy creating a task, a preview or
     *                  a build time estimation. In this case, the function will fail with return code EOS_ERR_NOT_IDLE. Stop any
     *                  running task/preview creations and build time estimation before trying to unload a new machine configuration.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_UnloadMachineConfiguration();

    /*!
     * \brief           Determines if a machine configuration is currently loaded. A loaded machine configuration and a loaded job are
     *                  prerequisites for creating tasks and previews as well as for build time estimation.
     * \return          EOS_TRUE                                if a machine configuration is loaded
     * \return          EOS_FALSE                               otherwise
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_BOOL EosTaskGen_IsMachineConfigLoaded();

    /*!
     * \brief           Sets or unsets the cache directory for files with precomputed data.
     * \param[in]       pathToCacheDirectory                    cache directory path to use for storing and restoring cached files on disk;
     *                                                          to disable caching pass NULL.
     * \return          EOS_ERR_NO_ERROR                        if the directory is valid (exists) and is set; or caching is successfully unset
     * \return          EOS_ERR_NOT_IDLE                        if a task or preview creation or a build time estimation is currently running
     * \return          EOS_ERR_NOT_A_DIRECTORY                 if the passed cache directory does not exist or is not a directory
     * \note            To disable caching pass NULL or empty path;
     * \note            To enable caching pass a valid caching directory path
     * \note            This function cannot be used while the cache directory is in use (return code EOS_ERR_NOT_IDLE).
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_SetCacheDirectory(EOS_CWSTR pathToCacheDirectory);

    /*!
     * \brief           Loads a job from an OpenJz file.
     * \param[in]       pathToOpenJz                            file path to an OpenJz file
     * \param[in]       pathToTemporaryDirectory                file path to an empty directory to which the OpenJz can be extracted
     * \return          EOS_ERR_NO_ERROR                        if the job was loaded successfully
     * \return          EOS_ERR_STRING_CONTAINS_ILLEGAL_CHARS   if a path contains illegal characters (hash/pound and non-Windows-1252 characters)
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if a filesystem error is encountered, e.g., due to missing access rights
     * \return          EOS_ERR_FILE_NOT_FOUND                  if the OpenJz file does not exist
     * \return          EOS_ERR_NOT_A_DIRECTORY                 if the temp directory does not exist or is not a directory
     * \return          EOS_ERR_DIRECTORY_NOT_EMPTY             if the temp directory is not empty
     * \return          EOS_ERR_PATH_CONTAINS_ILLEGAL_CHARS     if either the OpenJz or the temp directory path contains illegal characters
     * \return          EOS_ERR_OPENJOB_NOT_SUPPORTED           if the .openjz file exists, but contains an unsupported OpenJob version
     * \return          EOS_ERR_OPENJOB_INVALID                 if both of the specified files exist, but the OpenJob contains invalid data
     * \return          EOS_ERR_PART_FILE_NOT_FOUND             if a part referenced by the OpenJob could not be found
     * \return          EOS_ERR_EOSPAR_UNSUPPORTED              if the .openjz file exists, but contains an unsupported Eospar version
     * \return          EOS_ERR_OPENJZ_CORRUPT                  if the zip archive cannot be opened or its files cannot be extracted
     * \return          EOS_ERR_OPENJZ_INVALID                  if the zip does not contain the required files
     * \return          EOS_ERR_EOSPAR_CORRUPT                  if the Eospar file cannot be loaded
     * \return          EOS_ERR_EOSPAR_INVALID                  if the Eospar file contains invalid data
     * \return          EOS_ERR_NOT_IDLE                        if a task or preview creation or a build time estimation is currently running
     * \return          EOS_ERR_STRING_CONTAINS_ILLEGAL_CHARS   if a string in the OpenJob definition contains illegal characters
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_CANNOT_UNLOAD                   if ToolpathAPI files are currently open
     * \note            Implicitly unloads a previously loaded job as well as all geometries referenced by that job.
     * \note            You cannot load a new job while the TaskGen is busy creating a task, a preview or a build time estimation.
     *                  In this case, the function will fail with return code EOS_ERR_NOT_IDLE. Stop any running task/preview creations
     *                  and build time estimation before trying to load a new job.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_LoadOpenJz(EOS_CWSTR pathToOpenJz, EOS_CWSTR pathToTemporaryDirectory);

    /*!
     * \brief           Loads a job from an OpenJob file.
     * \param[in]       pathToOpenJob                           file path to an OpenJob file (.openjob)
     * \param[in]       pathToEospar                            file path to a Material Set file (.eospar)
     * \return          EOS_ERR_NO_ERROR                        if the job was loaded successfully
     * \return          EOS_ERR_ENCODING_CONVERSION_ERROR       if a path was used that cannot be converted to the narrow file api codepage
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if a filesystem error is encountered, e.g., due to missing access rights
     * \return          EOS_ERR_OPENJOB_NOT_FOUND               if the OpenJob file did not exist
     * \return          EOS_ERR_EOSPAR_NOT_FOUND                if the Eospar file did not exist
     * \return          EOS_ERR_PATH_CONTAINS_ILLEGAL_CHARS     if either the OpenJob or the Eospar file path contains illegal characters
     * \return          EOS_ERR_OPENJOB_CORRUPT                 if both of the specified files exist, but the OpenJob file cannot be loaded
     * \return          EOS_ERR_OPENJOB_NOT_SUPPORTED           if the OpenJob references an unsupported OpenJob version
     * \return          EOS_ERR_OPENJOB_INVALID                 if both of the specified files exist, but the OpenJob contains invalid data
     * \return          EOS_ERR_PART_FILE_NOT_FOUND             if a part referenced by the OpenJob could not be found
     * \return          EOS_ERR_EOSPAR_CORRUPT                  if both of the specified files exist, but the Eospar file cannot be loaded
     * \return          EOS_ERR_EOSPAR_INVALID                  if both of the specified files exist, but the Eospar file contains invalid data
     * \return          EOS_ERR_EOSPAR_UNSUPPORTED              if the Eospar references an unsupported Eospar version
     * \return          EOS_ERR_NOT_IDLE                        if a task or preview creation or a build time estimation is currently running
     * \return          EOS_ERR_STRING_CONTAINS_ILLEGAL_CHARS   if a string in the OpenJob definition contains illegal characters
     * \return          EOS_ERR_CANNOT_UNLOAD                   if ToolpathAPI files are currently open
     * \note            Implicitly unloads a previously loaded job as well as all geometries referenced by that job.
     * \note            You cannot load a new job while the TaskGen is busy creating a task, a preview or a build time estimation.
     *                  In this case, the function will fail with return code EOS_ERR_NOT_IDLE. Stop any running task/preview creations
     *                  and build time estimation before trying to load a new job.
     * \note            This function does not load .openjz files.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_LoadOpenJob(EOS_CWSTR pathToOpenJob, EOS_CWSTR pathToEospar);

    /*!
     * \brief           Loads a job from string.
     * \param[in]       openJob                                 a string containing a valid OpenJob XML
     * \param[in]       pathToEospar                            file path to a Material Set file (.eospar)
     * \return          EOS_ERR_NO_ERROR                        if the job was loaded successfully
     * \return          EOS_ERR_ENCODING_CONVERSION_ERROR       if a path was used that cannot be converted to the narrow file api codepage
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if a filesystem error is encountered, e.g., due to missing access rights
     * \return          EOS_ERR_EOSPAR_NOT_FOUND                if the Eospar file did not exist
     * \return          EOS_ERR_PATH_CONTAINS_ILLEGAL_CHARS     if the Eospar file path contains illegal characters
     * \return          EOS_ERR_OPENJOB_INVALID                 if the OpenJob contains invalid data
     * \return          EOS_ERR_PART_FILE_NOT_FOUND             if a part referenced by the OpenJob could not be found
     * \return          EOS_ERR_EOSPAR_CORRUPT                  if the Eospar file cannot be loaded
     * \return          EOS_ERR_EOSPAR_INVALID                  if the Eospar file contains invalid data
     * \return          EOS_ERR_OPENJOB_NOT_SUPPORTED           if the OpenJob references an unsupported OpenJob version
     * \return          EOS_ERR_EOSPAR_UNSUPPORTED              if the Eospar references an unsupported Eospar version
     * \return          EOS_ERR_NOT_IDLE                        if a task or preview creation or a build time estimation is currently running
     * \return          EOS_ERR_STRING_CONTAINS_ILLEGAL_CHARS   if a string in the OpenJob definition contains illegal characters
     * \return          EOS_ERR_CANNOT_UNLOAD                   if ToolpathAPI files are currently open
     * \note            Implicitly unloads a previously loaded job as well as all geometries referenced by that job.
     * \note            You cannot load a new job while the TaskGen is busy creating a task, a preview or a build time estimation.
     *                  In this case, the function will fail with return code EOS_ERR_NOT_IDLE. Stop any running task/preview creations
     *                  and build time estimation before trying to load a new job.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_LoadOpenJobFromString(EOS_CWSTR openJob, EOS_CWSTR pathToEospar);

    /*!
     * \brief           Replaces the currently loaded job with the one specified, but retains memory for already loaded geometries and possibly
     *                  for an already loaded Material Set.
     *                  This is particularly useful, if you want to quickly regenerate preview data after having modified only some portions of the OpenJob
     *                  (e.g. part positions or exposure set assignment etc.), because it is faster than reloading the whole job with all geometries.
     * \param[in]       pathToOpenJob                           file path to the updated OpenJob file (.openjob); must not be NULL or empty.
     * \param[in]       pathToEospar                            file path to the updated Material Set file (.eospar);
     *                                                          pass NULL to keep the currently loaded Material Set, must not be empty otherwise.
     * \return          EOS_ERR_NO_ERROR                        if the job was updated successfully
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if a filesystem error is encountered, e.g., due to missing access rights
     * \return          EOS_ERR_OPENJOB_NOT_FOUND               if the OpenJob file does not exist
     * \return          EOS_ERR_EOSPAR_NOT_FOUND                if the Eospar file does not exist
     * \return          EOS_ERR_OPENJOB_CORRUPT                 if the OpenJob file exists, but contains invalid data
     * \return          EOS_ERR_EOSPAR_CORRUPT                  if the Eospar file exists, but contains invalid data
     * \return          EOS_ERR_OPENJOB_NOT_SUPPORTED           if the OpenJob references an unsupported OpenJob version
     * \return          EOS_ERR_EOSPAR_UNSUPPORTED              if the Eospar references an unsupported Eospar version
     * \return          EOS_ERR_PATH_CONTAINS_ILLEGAL_CHARS     if either the OpenJob or the Eospar file path contains illegal characters
     * \return          EOS_ERR_OPENJOB_INVALID                 if the OpenJob is invalid
     * \return          EOS_ERR_PART_FILE_NOT_FOUND             if a part referenced by the OpenJob could not be found
     * \return          EOS_ERR_NOT_IDLE                        if a task or preview creation or a build time estimation is currently running
     * \note            Geometry files referenced in the updated job will not be reloaded, if they were already loaded from the exact same canonicalized path in the
     *                  obsolete job.
     * \note            Not yet loaded geometry files will be loaded if no part exists in the obsolete job referencing it with the exact same canonicalized path.
     * \note            Already loaded geometry files will be unloaded if no part exists in the updated job referencing it with the exact same canonicalized path.
     * \note            You cannot update the current job while the TaskGen is busy creating a task, a preview or a build time estimation.
     *                  In this case, the function will fail with return code EOS_ERR_NOT_IDLE. Stop any running task/preview creations
     *                  and build time estimation before trying to load a new job.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_UpdateJobFromFile(EOS_CWSTR pathToOpenJob, EOS_CWSTR pathToEospar);

    /*!
     * \brief           Replaces the currently loaded job with the one specified, but retains memory for already loaded geometries and possibly
     *                  for an already loaded Material Set.
     *                  This is particularly useful, if you want to quickly regenerate preview data after having modified only some portions of the OpenJob
     *                  (e.g. part positions or exposure set assignment etc.), because it is faster than reloading the whole job with all geometries.
     * \param[in]       openJob                                 string containing an OpenJob XML; must not be empty.
     * \param[in]       pathToEospar                            file path to the updated Material Set file (.eospar);
     *                                                          pass NULL to keep the currently loaded Material Set, must not be empty otherwise.
     * \return          EOS_ERR_NO_ERROR                        if the job was updated successfully
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if a filesystem error is encountered, e.g., due to missing access rights
     * \return          EOS_ERR_EOSPAR_NOT_FOUND                if the Eospar file does not exist
     * \return          EOS_ERR_OPENJOB_CORRUPT                 if the OpenJob file exists, but contains invalid data
     * \return          EOS_ERR_EOSPAR_CORRUPT                  if the Eospar file exists, but contains invalid data
     * \return          EOS_ERR_PATH_CONTAINS_ILLEGAL_CHARS     if the Eospar file path contains illegal characters
     * \return          EOS_ERR_OPENJOB_NOT_SUPPORTED           if the OpenJob references an unsupported OpenJob version
     * \return          EOS_ERR_EOSPAR_UNSUPPORTED              if the Eospar references an unsupported Eospar version
     * \return          EOS_ERR_NOT_IDLE                        if a task or preview creation or a build time estimation is currently running
     * \note            Geometry files referenced in the updated job will not be reloaded, if they were already loaded from the exact same path in the
     *                  obsolete job.
     * \note            Not yet loaded geometry files will be loaded if no part exists in the obsolete job referencing it with the exact same path.
     * \note            Already loaded geometry files will be unloaded if no part exists in the updated job referencing it with the exact same path.
     * \note            You cannot update the current job while the TaskGen is busy creating a task, a preview or a build time estimation.
     *                  In this case, the function will fail with return code EOS_ERR_NOT_IDLE. Stop any running task/preview creations
     *                  and build time estimation before trying to load a new job.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_UpdateJobFromString(EOS_CWSTR openJob, EOS_CWSTR pathToEospar);

    /*!
     * \brief           Unloads the currently loaded job.
     * \return          EOS_ERR_NO_ERROR                        if the job was unloaded successfully
     * \return          EOS_ERR_NOT_IDLE                        if a task or preview creation or a build time estimation is currently running
     * \note            Calling this function has no effect, if no job is loaded.
     * \note            You cannot unload the current job while the TaskGen is busy creating a task, a preview or a build time estimation.
     *                  In this case, the function will fail with return code EOS_ERR_NOT_IDLE. Stop any running task/preview creations and
     *                  build time estimation before trying to unload a job.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_UnloadJob();

    /*!
     * \brief           Determines if a job is currently loaded. A loaded job and a machine configuration are prerequisites for creating tasks and
     *                  previews as well as for build time estimation.
     * \return          EOS_TRUE                                if a job is loaded
     * \return          EOS_FALSE                               otherwise
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_BOOL EosTaskGen_IsJobLoaded();

    /*!
     * \brief           Begins preview generation for the layer at the specified layer index using the currently loaded job and the currently loaded
     *                  machine configuration.
     * \param[in]       layerIndex                              1-based index of the layer for which a preview should be generated
     * \param[in]       vectorTypes                             bit mask specifying which vector types to generate
     * \return          EOS_ERR_NO_ERROR                        if preview generation was started successfully
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_NOT_IDLE                        if a preview is already running - abort it first
     * \return          EOS_ERR_ARGUMENT_INVALID                if unknown vector types were specified or none at all
     * \return          EOS_ERR_PREVIEW_LAYER_INVALID           if the layer index is out of bounds, i.e., less than the first and greater than the last layer
     * \return          EOS_ERR_LOADING_DLL_FAILED              if a DLL could not be loaded because the file is missing
     *                                                          or a necessary license is missing
     * \note            Combine the EOS_VT_* constants to set the vector types as desired, e.g., EOS_VT_HATCHING|EOS_VT_PART_BOUNDARY to generate hatching and
     *                  part boundary vectors.
     * \note            Unless the function is called with vectorTypes == EOS_VT_PART_BOUNDARY exposure time details will be calculated
     *                  for later retrieval using EosTaskGen_GetExposureTimeDetail().
     * \note            Generating hatching vectors is an expensive operation. However, part boundaries always need to be calculated, so not setting EOS_VT_PART_BOUNDARY
     *                  is merely a way of filtering the result, but has no impact on complexity.
     * \note            This function is thread safe.
     * \note            Required licenses for preview creation are not checked by this function. If licenses are missing on the current system, the
     *                  EOS_EVENT_CALLBACK will report the error code EOS_ERR_MISSING_LICENSE.
     *                  Use EosFileProperties_LoadMaterialSetInfo() to find required licenses for certain material sets and exposure sets
     *                  (materialSetInfo->exposureSets->licenses).
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_BeginPreviewCreation(EOS_LAYERINDEX layerIndex, EOS_VECTORTYPES vectorTypes);

    /*!
     * \brief           Aborts an active preview creation.
     * \return          EOS_ERR_NO_ERROR                        if a preview creation was still active and its interrupt was signaled successfully
     * \return          EOS_ERR_PREVIEW_CREATION_NOT_RUNNING    if no preview was active
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_AbortPreviewCreation();

    /*!
     * \brief           Returns the current state of preview creation.
     * \return          the current state of preview creation
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_TASK_STATE EosTaskGen_GetPreviewCreationState();

    /*!
     * \brief           Waits until the currently running preview creation has finished or the specified timeout has occurred.
     * \param[in]       timeout                                 the timeout in milliseconds. If no timeout should be applied, pass EOS_INFINITE_DURATION.
     * \return          EOS_ERR_NO_ERROR                        if the preview generation has stopped (whether with a successful result or not)
     * \return          EOS_ERR_WAIT_TIMED_OUT                  if the timeout occurred before preview generation stopped, i.e., if a preview creation is still running
     * \return          EOS_ERR_PREVIEW_NOT_RUNNING             if no preview creation is running
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_WaitForPreviewCreation(EOS_MILLISECS timeout);

    /*!
     * \brief           Returns the result of the last finished preview creation request
     * \return          EOS_ERR_NO_ERROR                        if preview creation was completed successfully
     *                                                          or if no creation was started yet or if a preview creation is currently running
     * \return          EOS_ERR_PREVIEW_ABORTED                 if the status of the preview creation is EOS_TS_ABORTED
     * \return          Any other return code depending on the specific preview creation if its status is EOS_TS_FAILED
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetPreviewCreationResult();

     /*!
     * \brief           Retrieves preview data in form of exposure vectors.
     * \param[out]      previewData                             if successful *previewData will point to an exposure vector array
     * \param[out]      exposureData                            if successful *exposureData will point to an exposure data array. If no exposure-data is required,
     *                                                          NULL may be passed for this argument
     * \return          EOS_ERR_NO_ERROR                        if valid preview data exists
     * \return          EOS_ERR_NO_MATCH                        if no valid preview data exists, e.g., because a preview creation has not been started or is not
     *                                                          finished yet.
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as argument for previewdata
     * \note            In order to avoid expensive copy operations, the retrieved preview data is owned by the TaskGen instance itself.
     *                  It will become invalid when either a machine configuration or a job is loaded or updated and also when another
     *                  preview creation is triggered.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetPreviewData2(EOS_EXPOSURE_VECTOR_ARRAY const ** previewData, EOS_EXPOSURE_DATA_ARRAY const ** exposureData);

    /*!
     * \brief           Retrieves image data derived from the last calculated layer, if any.
     * \param[in]       imageSource                             selects which image to retrieve
     * \param[out]      image                                   if successful, *image will be filled with the requested image data
     * \return          EOS_ERR_NO_ERROR                        if the image was retrieved successfully
     * \return          EOS_ERR_NO_MATCH                        if no matching image is currently available, e.g., because a preview creation has not been started or is not
     *                                                          finished yet.
     * \return          EOS_ERR_PREVIEW_RUNNING                 if a preview creation is currently running
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as argument for image
     * \return          EOS_ERR_NOT_IMPLEMENTED                 if imageSource is unknown
     * \note            *image is left unchanged if this function does not return EOS_ERR_NO_ERROR.
     * \note            EOS_IMAGE::data points into a buffer owned by the TaskGen instance itself. It is only valid until either
     *                  a machine configuration or a job is loaded or updated and also when another preview creation is triggered.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetImageData(EOS_IMAGE_SOURCE imageSource, EOS_IMAGE * image);


    /*!
     * \brief           Begins the build time estimation using the currently loaded job and machine configuration.
     * \return          EOS_ERR_NO_ERROR                        if build time estimation was started successfully
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_NOT_IDLE                        if build time estimation is already running
     * \return          EOS_ERR_LOADING_DLL_FAILED              if a DLL could not be loaded because the file is missing
     *                                                          or a necessary license is missing
     * \note            The triggered build time estimation runs in a different thread. Register a callback to get notifications about its
                        progress and/or call EosTaskGen_WaitForBuildTimeEstimation() to wait for its completion.
     * \note            It is recommended not to wait for completion of the build time estimation, because its results are already very
     *                  accurate after a few seconds. Instead register callbacks and abort the build time estimation once you notice
     *                  that the deviation of subsequent estimates is small.
     * \note            This function is thread safe.
     * \note            Required licenses for build time estimation are not checked by this function. If licenses are missing on the current system, the
     *                  EOS_EVENT_CALLBACK will report the error code EOS_ERR_MISSING_LICENSE.
     *                  Use EosFileProperties_LoadMaterialSetInfo() to find required licenses for certain material sets and exposure sets
     *                  (materialSetInfo->exposureSets->licenses).
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_BeginBuildTimeEstimation();

    /*!
     * \brief           Aborts build time estimation.
     * \return          EOS_ERR_NO_ERROR                         if build time estimation was running and its interruption was signaled successfully
     * \return          EOS_ERR_BUILDTIME_ESTIMATION_NOT_RUNNING if build time estimation was not running
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_AbortBuildTimeEstimation();

    /*!
     * \brief           Returns the current state of build time estimation.
     * \return          the current state of build time estimation
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_TASK_STATE EosTaskGen_GetBuildTimeEstimationState();

    /*!
     * \brief           Waits until the currently running build time estimation has finished or the specified timeout has occurred.
     * \param[in]       timeout                                 the timeout in milliseconds ; pass EOS_INFINITE_DURATION if no timeout should be applied
     * \return          EOS_ERR_NO_ERROR                        if the build time estimation has stopped (whether with a successful result or not)
     *                                                          or if none has been started
     * \return          EOS_ERR_WAIT_TIMED_OUT                  if the timeout occurred before build time estimation stopped
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_WaitForBuildTimeEstimation(EOS_MILLISECS timeout);

    /*!
     * \brief           Returns the most recent result of the build time estimation
     * \return          estimated build time in milliseconds or 0 if no valid build time estimate exists
     * \note            It may take several seconds after triggering a build time estimation before the first valid build time estimate
     *                  exists. Until then this function returns 0.
     * \note            Result will be reset when either a machine configuration or a job is loaded or updated and also when another
     *                  build time estimation is triggered.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_MILLISECS EosTaskGen_GetBuildTimeEstimation();

    /*!
     * \brief           Registers a custom function that will be called whenever an updated build time estimate becomes available
     * \param[in]       callback                                function that is to be called to notify about updated build time estimation
     * \param[in]       customDataHandle                        handle to custom data or custom pointer which is passed through as argument to the callback; EOS_NO_DATA if none
     * \return          EOS_ERR_NO_ERROR                        if the callback was successfully set
     * \return          EOS_ERR_BUILDTIME_ESTIMATION_RUNNING    if the call back could not be set caused by already running task creation
     * \note            Only a single callback function can be registered. Calling the function twice will unregister the
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            You cannot register a callback function while a build time estimation is already running.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_SetNewBuildTimeEstimateCallback(EOS_BUILD_TIME_PROGRESS_CALLBACK callback, EOS_DATA_HANDLE customDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever the build time estimation state has changed
     * \param[in]       callback                                function that is to be called to notify about build time estimation state changes
     * \param[in]       customDataHandle                        handle to custom data or custom pointer which is passed through as argument to the callback; EOS_NO_DATA if none
     * \return          EOS_ERR_NO_ERROR                        if the callback was successfully set
     * \return          EOS_ERR_BUILDTIME_ESTIMATION_RUNNING    if the call back could not be set caused by already running task creation
     * \note            Only a single callback function can be registered. Calling the function twice will unregister the
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            You cannot register a callback function while a build time estimation is already running.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_SetBuildTimeStateChangedCallback(EOS_STATE_CHANGE_CALLBACK callback, EOS_DATA_HANDLE customDataHandle);

    /*!
     * \brief           Retrieves exposure time details for the currently loaded job and machine configuration that were generated
     *                  during the last preview data calculation.
     * \param[in]       partId                                  ID of the part for which exposure time details are requested. Can be ALL_PARTS to indicate
     *                                                          that exposure time details should be accumulated over all parts in the calculated layer.
     * \param[in]       laserScannerIndex                       Index of the laser/scanner unit for which exposure time details are requested. Can be
     *                                                          ALL_LASERS to indicate that exposure time details should be accumulated over all laser/scanner units.
     * \param[out]      exposureTimeDetail                      Pointer to an instance of struct EOS_EXPOSURETIME_DETAIL which is to receive the resulting values
     * \return          EOS_ERR_NO_ERROR                        if finished successfully
     * \return          EOS_ERR_ARGUMENT_INVALID                if an invalid argument was supplied, e.g. NULL for exposureTimeDetail
     * \return          EOS_ERR_NO_MATCH                        if no data is currently available, e.g. because a preview creation has not been started or is not
     *                                                          finished yet, or if no matching data for the given part and scanner is available
     * \return          EOS_ERR_MISSING_LICENSE                 if no valid license has been found
     * \note            Exposure time details will not be available if preview has been generated for EOS_VT_PART_BOUNDARY only.
     * \note            The function can only retrieve data that has been generated by a preceding call to EosTaskGen_BeginPreviewCreation.
     *                  Therefore, if no valid preview data exists, the function will return EOS_ERR_NO_MATCH.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetExposureTimeDetail(EOS_PARTID partId,
        EOS_BYTE laserScannerIndex,
        EOS_EXPOSURETIME_DETAIL * exposureTimeDetail);

    /*!
     * \brief           Begins the task creation using the currently loaded job and machine configuration.
     * \param[in]       metaData                                meta data to embed in the task file
     * \param[in]       targetDirectory                         file path to a local directory to which the task & data files will be written
     * \return          EOS_ERR_NO_ERROR                        if build time estimation was started successfully
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_CONFIG_INVALID                  if the machine configuration is erroneous
     *                                                          or if the job does not match the machine configuration
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_NOT_IDLE                        if task creation is already running
     * \return          EOS_ERR_ENCODING_CONVERSION_ERROR       if a directory path was passed that cannot be converted to the narrow file api codepage
     * \return          EOS_ERR_NOT_A_DIRECTORY                 if the passed path is not a directory (e.g., it does not exist or it is a regular file)
     * \return          EOS_ERR_DIRECTORY_NOT_EMPTY             if the target directory is not empty
     * \return          EOR_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     *                                                          or if metaData->numberOfAllowedBuilds exceeds the bound of INT_MAX (2147483647)
     * \return          EOS_ERR_LOADING_DLL_FAILED              if a DLL could not be loaded because the file is missing
     *                                                          or a necessary license is missing
     * \return          EOS_ERR_NOT_ALLOWED                     if the format of the output task is EOS_TF_RAW, but raw task export is not allowed, e.g.,
     *                                                          because a closed exposure set is assigned in the currently loaded job
     * \note            The triggered task creation runs in a different thread. Register a callback to get notifications about its
     *                  progress and/or call EosTaskGen_WaitForTaskCreation() to wait for its completion.
     * \note            The caller must initialize metaData.reserved as an array of zero bytes. This ensures compatibility with future extensions.
     * \note            This function is thread safe.
     * \note            Required licenses for task creation are not checked by this function. If licenses are missing on the current system, the
     *                  EOS_EVENT_CALLBACK will report the error code EOS_ERR_MISSING_LICENSE.
     *                  Use EosFileProperties_LoadMaterialSetInfo() to find required licenses for certain material sets and exposure sets
     *                  (materialSetInfo->exposureSets->licenses).
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_BeginTaskCreation(EOS_TASK_METADATA const * metaData, EOS_CWSTR targetDirectory);

    /*!
     * \brief           Aborts a running task creation.
     * \return          EOS_ERR_NO_ERROR                        if a task creation was running and its interrupt was signaled successfully
     * \return          EOS_ERR_TASK_CREATION_NOT_RUNNING       if no task creation was running
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_AbortTaskCreation();

    /*!
     * \brief           Returns the current state of task creation.
     * \return          the current state of task creation
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_TASK_STATE EosTaskGen_GetTaskCreationState();

    /*!
     * \brief           Returns the result of the last task creation.
     * \return          The result of the last run task creation
     * \note            This function is thread safe
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetTaskCreationResult();

    /*!
     * \brief           Waits until the currently running task creation has stopped or the specified timeout has occurred.
     * \param[in]       timeout                                 the timeout in milliseconds ; pass EOS_INFINITE_DURATION if no timeout should be applied
     * \return          EOS_ERR_NO_ERROR                        if task creation has stopped (whether with a successful result or not)
     * \return          EOS_ERR_TASK_CREATION_NOT_RUNNING       if no task creation was running
     * \return          EOS_ERR_WAIT_TIMED_OUT                  if the timeout occurred before task creation stopped
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_WaitForTaskCreation(EOS_MILLISECS timeout);

    /*!
     * \brief           Registers a custom function that will be called whenever task creation state has changed.
     * \param[in]       callback                                function that is to be called to notify about task creation state changes
     * \param[in]       customDataHandle                        handle to custom data or custom pointer which is passed through as argument to the callback; EOS_NO_DATA if none
     * \return          EOS_ERR_NO_ERROR                        if the callback was successfully set
     * \return          EOS_ERR_TASK_CREATION_RUNNING           if the call back could not be set caused by already running task creation
     * \note            Only a single callback function can be registered.
     *                  Calling the function twice will unregister the callback of the first call.
     * \note            Call with NULL as callback to remove an existing callback.
     * \note            You cannot register a callback function while the TaskGen is busy creating a task, a preview or a build time estimation.
     *                  In this case, the function will fail and return EOS_FALSE.
     * \note            The callback is guaranteed to be called sequentially.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_SetStateChangeCallback(EOS_STATE_CHANGE_CALLBACK callback, EOS_DATA_HANDLE customDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a new pair of data and task files was completed
     *                  during task creation.
     * \param[in]       callback                                function that is to be called for each completed data/task file pair
     * \param[in]       customDataHandle                        handle to custom data or custom pointer which is passed through as argument to the callback; EOS_NO_DATA if none
     * \return          EOS_ERR_NO_ERROR                        if the callback was successfully set
     * \return          EOS_ERR_TASK_CREATION_RUNNING           if a task creation is running, in which case the callback cannot be changed
     * \note            Only a single callback function can be registered. Subsequent calls will unregister the callback from the previous call.
     * \note            Call with NULL as callback to remove an existing callback.
     * \note            The callback is called from a separate thread, so blocking inside the callback only blocks subsequent callbacks.
     *                  However, it is still recommended to block the callback only for a short time, e.g., to copy the paths to a local structure.
     * \note            The callback is guaranteed to be called sequentially.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_SetDatafileCompletedCallback(EOS_DATAFILE_COMPLETED_CALLBACK callback, EOS_DATA_HANDLE customDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever task creation progress has changed.
     * \param[in]       callback                                function that is to be called to notify about task creation progress
     * \param[in]       customDataHandle                        handle to custom data or custom pointer which is passed through as argument to the callback; EOS_NO_DATA if none
     * \return          EOS_ERR_NO_ERROR                        if the callback was successfully set
     * \return          EOS_ERR_TASK_CREATION_RUNNING           if a task creation is running, in which case the callback cannot be changed
     * \note            Only a single callback function can be registered. Subsequent calls will unregister the callback from the previous call.
     * \note            Call with NULL as callback to remove an existing callback.
     * \note            The callback is called from a separate thread, so blocking inside the callback only blocks subsequent callbacks.
     *                  However, it is still recommended to block the callback only for a short time, e.g., to copy the paths to a local structure.
     * \note            The callback is guaranteed to be called sequentially.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_SetProgressCallback(EOS_GENERATION_PROGRESS_CALLBACK callback, EOS_DATA_HANDLE customDataHandle);

    /*!
     * \brief           Returns the index of the layer that is currently being calculated by a running task creation.
     * \param[in]       layerIndex                              output variable to which the index will be written if successful
     * \return          EOS_ERR_NO_ERROR                        if the index was written successfully
     * \return          EOS_ERR_IDLE                            if no task creation is running
     * \return          EOS_ERR_INTERNAL_ERROR                  if an internal error occurred
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \note            Layer indices start at 1.
     * \note            If task creation state is not idle, you can use this function in conjunction with
     *                  EosTaskGen_GetTargetLayerIndex() to calculate the current task creation progress as follows:
     *                  (currentLayerIndex - startLayerIndex) / (targetLayerIndex - startLayerIndex + 1)
     *                  Please note that the layers of a task may be of variable thickness. Therefore, progress calculated on
     *                  basis of the current layer index may deviate from progress calculated on basis of the current z-height (see
     *                  EosTaskGen_GetCurrentZHeight()). Choose whatever approach you deem appropriate.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetCurrentLayerIndex(EOS_LAYERINDEX * layerIndex);

    /*!
     * \brief           Returns the index of the first layer in the job.
     * \param[in]       layerIndex                              output variable to which the index will be written if successful
     * \return          EOS_ERR_NO_ERROR                        if the index was written successfully
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_INTERNAL_ERROR                  if an internal error occurred
     * \note            Layer indices start at 1.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetStartLayerIndex(EOS_LAYERINDEX * layerIndex);

    /*!
     * \brief           Returns the index of the last layer in the job.
     * \param[in]       layerIndex                              output variable to which the index will be written if successful
     * \return          EOS_ERR_NO_ERROR                        if the index was written successfully
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_EMPTY                           if the current job has no layers
     * \return          EOS_ERR_INTERNAL_ERROR                  if an internal error occurred
     * \note            Layer indices start at 1.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetTargetLayerIndex(EOS_LAYERINDEX * layerIndex);

    /*!
     * \brief           Returns the z-position in micrometers of the layer that is currently being calculated by a running task creation.
     * \param[in]       zHeight                                 output variable to which the z height will be written if successful
     * \return          EOS_ERR_NO_ERROR                        if the index was written successfully
     * \return          EOS_ERR_IDLE                            if no task creation is running
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_EMPTY                           if the current job has no layers
     * \return          EOS_ERR_INTERNAL_ERROR                  if an internal error occurred
     * \note            If task creation state is not idle, you can use this function in conjunction with
     *                  EosTaskGen_GetTargetZHeight() to calculate the current task creation progress as follows:
     *                  (currentZHeight - startZHeight) / (targetZHeight - startZHeight)
     *                  Please note that the layers of a task may be of variable thickness. Therefore, progress calculated on
     *                  basis of the current layer index may deviate from progress calculated on basis of the current z-height (see
     *                  EosTaskGen_GetCurrentLayerIndex()). Choose whatever approach you deem appropriate.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetCurrentZHeight(EOS_MICROMETER * zHeight);

    /*!
     * \brief           Returns the start z-position in micrometers of the currently loaded job.
     * \param[in]       zHeight                                 output variable to which the z height will be written if successful
     * \return          EOS_ERR_NO_ERROR                        if the z-position was written successfully
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_INTERNAL_ERROR                  if an internal error occurred
     * \note            The z position of a layer is the distance of its top edge to the job start position. The job start position is always at Z=0
     *                  and should not be mistaken with the z axis position. The job start position is not affected by the start process height
     *                  parameter (see OpenJob spec for /openjob/height). The start process height only affects how many layers of the job will
     *                  be skipped.
     * \note            There is no layer at z-position 0.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetStartZHeight(EOS_MICROMETER * zHeight);

    /*!
     * \brief           Returns the target z-position in micrometers of the currently loaded job.
     * \param[in]       zHeight                                 output variable to which the z height will be written if successful
     * \return          EOS_ERR_NO_ERROR                        if the z-position was written successfully
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_INTERNAL_ERROR                  if an internal error occurred
     * \note            The z position of a layer is the distance of its top edge to the job start position. The job start position is always at Z=0
     *                  and should not be mistaken with the z axis position. The job start position is not affected by the start process height
     *                  parameter (see OpenJob spec for /openjob/height). The start process height only affects how many layers of the job will
     *                  be skipped.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetTargetZHeight(EOS_MICROMETER * zHeight);

    /*!
     * \brief           Retrieves meta data about all layers in the currently loaded job.
     * \param[out]      layerInfos                              if successful, this will hold an array of layer infos
     * \param[out]      layerInfosSize                          if successful, this will hold the size of the layerInfos array
     * \return          EOS_ERR_NO_ERROR                        if layer infos were successfully retrieved
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \note            In all error cases the output parameters remain untouched.
     * \note            The returned pointer will become invalid when either a machine configuration or a job is loaded or updated.
     * \note            The pointer to the layer info struct array and all contained values and pointers are valid as long as the job remains unchanged
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetLayers(EOS_LAYER_INFO const ** layerInfos, EOS_SIZET * layerInfosSize);

    /*!
     * \brief           Retrieves meta data of the first layer found near the specified z position using the specified search mode.
     * \param[in]       zPos                                    the z position from which search will start
     * \param[in]       searchMode                              the search mode
     * \param[out]      layerInfo                               if search is successful, this will contain the meta data of the layer - otherwise remains untouched
     * \return          EOS_ERR_NO_ERROR                        if the search was successful and layerInfo now contains updated meta data
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_NO_MATCH                        if the specified layer was not found
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_INTERNAL_ERROR                  if an internal error occurred
     * \note            Passing EOS_SM_EXACT for searchMode will only yield a valid result if a layer is found at zPos exactly
     * \note            The values in the layer info structs are valid as long as the job remains unchanged (this includes pointers)
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_FindLayerByAbsoluteHeight(EOS_MICROMETER zPos, EOS_SEARCHMODE searchMode, EOS_LAYER_INFO * layerInfo);

    /*!
     * \brief           Retrieves meta data of the first layer with exposed areas found near the specified z position using the specified search mode.
     * \param[in]       zPos                                    the z position from which search will start
     * \param[in]       searchMode                              the search mode
     * \param[out]      layerInfo                               if search is successful, this will contain the meta data of the layer - otherwise remains untouched
     * \return          EOS_ERR_NO_ERROR                        if the search was successful and layerInfo now contains updated meta data
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_NO_MATCH                        if the specified layer was not found
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \note            Passing EOS_SM_EXACT for searchMode will only yield a valid result if an exposed layer is found at zPos exactly
     * \note            The values in the layer info structs are valid as long as the job remains unchanged (this includes pointers)
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_FindFirstExposedLayerByAbsoluteHeight(EOS_MICROMETER zPos, EOS_SEARCHMODE searchMode, EOS_LAYER_INFO * layerInfo);

    /*!
     * \brief           Retrieves meta data of the layer specified by layer index within the currently loaded job.
     * \param[in]       layerIndex                              1-based layer index
     * \param[out]      layerInfo                               if the layer exists, this will contain its meta data - otherwise remains untouched
     * \return          EOS_ERR_NO_ERROR                        if the search was successful and layerInfo now contains updated meta data
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_NO_MATCH                        if the specified layer was not found
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \note            The values in the layer info structs are valid as long as the job remains unchanged (this includes pointers)
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_FindLayerByIndex(EOS_LAYERINDEX layerIndex, EOS_LAYER_INFO * layerInfo);

    /*!
     * \brief           Retrieves meta data of the first layer with exposed areas found near the specified layer using the specified search mode.
     * \param[in]       layerIndex                              1-based layer index
     * \param[in]       searchMode                              the search mode
     * \param[out]      layerInfo                               if search is successful, this will contain the meta data of the layer - otherwise remains untouched
     * \return          EOS_ERR_NO_ERROR                        if the search was successful and layerInfo now contains updated meta data
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_NO_MATCH                        if the specified layer was not found
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \note            Passing EOS_SM_EXACT for searchMode will only yield a valid result if an exposed layer is found at layerIndex exactly
     * \note            The values in the layer info structs are valid as long as the job remains unchanged (this includes pointers)
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_FindFirstExposedLayerByIndex(EOS_LAYERINDEX layerIndex, EOS_SEARCHMODE searchMode, EOS_LAYER_INFO * layerInfo);

    /*!
     * \brief           Determines the first and last exposed layer of the given part and returns the information of these layers
     * \param[in]       partId                                  The numerical id of the part, see @id of <part> elements in the OpenJob
     * \param[in]       firstLayerInfo                          Pointer to a layer info struct to which the information of the first layer will be written
     * \param[in]       lastLayerInfo                           Pointer to a layer info struct to which the information of the last layer will be written
     * \return          EOS_ERR_NO_ERROR                        if the information was successfully written
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_NO_MATCH                        if there are no exposed layers for the given part id (possibly because there is no part with the given id)
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument or firstLayerInfo == lastLayerInfo
     * \note            The values in the layer info structs are valid as long as the job remains unchanged (this includes pointers)
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_FindFirstAndLastLayerOfPart(EOS_PARTID partId, EOS_LAYER_INFO * firstLayerInfo, EOS_LAYER_INFO * lastLayerInfo);

    /*!
     * \brief           Determines the first and last exposed layer of the given part's specified z-segment and returns the information of these layers
     * \param[in]       partId                                  The numerical id of the part, see @id of <part> elements in the OpenJob
     * \param[in]       segmentIdx                              The index of the segment (starting at 0, sorted by start layer in ascending order)
     * \param[in]       firstLayerInfo                          Pointer to a layer info struct to which the information of the first layer will be written
     * \param[in]       lastLayerInfo                           Pointer to a layer info struct to which the information of the last layer will be written
     * \return          EOS_ERR_NO_ERROR                        if the information was successfully written
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_NO_MATCH                        if there are no exposed layers for the given part id or segment index
     *                                                          (possibly because there is no part with the given id or segment with the given index)
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument or firstLayerInfo == lastLayerInfo
     * \note            The values in the layer info structs are valid as long as the job remains unchanged (this includes pointers)
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_FindFirstAndLastLayerOfSegment(EOS_PARTID partId, EOS_UINT segmentIdx, EOS_LAYER_INFO * firstLayerInfo, EOS_LAYER_INFO * lastLayerInfo);

    /*!
     * \brief           Finds the next valid process height near the specified z position using the specified search mode.
     *
     *                  This function is very similar to EosTaskGen_FindLayerByAbsoluteHeight. The major difference is that
     *                  it will ignore both, process start height and process end height, if either was set for the currently loaded job.
     *                  Another difference is that it finds a z position only, not a layer info.
     *
     *                  This function is particularly useful, if you need to set a new process start height or process end height value
     *                  on a job that already has such settings. The reasons herefore are that the EOSPRINT API expects the process heights
     *                  in OpenJob documents to match a valid layer position exactly (the build process cannot stop in-between two layers)
     *                  and that it is impossible to retrieve a layer info for a z height that has already been removed due to
     *                  process height settings.
     *
     *                  For clarification consider the following scenario:
     *                  The currently loaded job has only one part which ranges from 0 mm to 6 mm in Z. That part is assigned an Exposure Set
     *                  of layer thickness 0.03 mm. Also, the job has a process start height of 3 mm, i.e. all layers below that threshold
     *                  are removed. If the user now tried to lower the process start height from 3 mm to 2 mm, then this would fail, because
     *                  the z position 2 mm cannot be reached by stacking layers of 0.03 mm thickness. If all layers have the same thickness
     *                  it's easy to find a valid z position, but if multiple layer thickness are used within the same job this becomes a
     *                  difficult problem. Yet, it's not possible to use e.g. EosTaskGen_FindLayerByAbsoluteHeight for this, because
     *                  no layers exist outside the z range [process start height, process end height].
     *
     *                  That's where this function becomes useful, because it allows finding a z position that qualifies as a new
     *                  process start height / process end height for the currently loaded job.
     *
     * \param[in]       zPos                                    the z position from which search will start
     * \param[in]       searchMode                              the search mode.
     * \param[out]      validProcessHeight                      if search is successful, this will contain the next valid process height near zPos
     * \return          EOS_ERR_NO_ERROR                        if the search was successful and validProcessHeight now contains a result
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_NO_MATCH                        if the specified layer was not found
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \note            Passing EOS_SM_EXACT for searchMode will basically validate the given zPos by returning EOS_ERR_NO_ERROR if
     *                  zPos is a valid process height or EOS_ERR_NO_MATCH otherwise.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_FindValidProcessHeight(EOS_MICROMETER zPos, EOS_SEARCHMODE searchMode, EOS_MICROMETER * validProcessHeight);

    /*!
     * \brief           Retrieves split line information about the layer with the given layer index
     * \param[in]       layerIndex                              1-based layer index
     * \param[out]      splitLinePoints                         array of points. the memory is owned by task gen api. the array is only guaranteed to be valid as long as the job is not changed.
     * \param[out]      splitLinePointsCount                    number of points in the array
     * \return          EOS_ERR_NO_ERROR                        if the split line information for the given layer index was successfully retrieved
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_NO_MATCH                        if the specified layer was not found
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \note            This function is thread safe
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetSplitLinesForLayer(EOS_LAYERINDEX layerIndex, EOS_POINT2 const ** splitLinePoints, EOS_SIZET * splitLinePointsCount);

    /*!
     * \brief           Retrieves the current process hash.
     *                  Designed to identify individual processes, the process hash is calculated based on all part, machine and exposure
     *                  data and settings that have a physical impact on the print process.
     *                  Data and settings that lack any physical impact on the print process, e.g. inactive exposure settings, are not included in the process hash.
     *
     *                  Thus, among others, the following operations change the process hash:
     *                  - Modifying an exposure set (e.g. laser power, scan speed) that is assigned to at least one part
     *                  - Modifying process settings, e.g. recoater speed, platform temperature, flow
     *                  - Changing part geometries
     *                  - Moving/rotating parts
     *                  - Changing the scanfield overlap settings
     *                  - Changing laser assignment
     *                  - Changing beam offset or shrinkage compensation
     *                  - etc.
     *
     *                  In contrast, the following operations do not have physical impact and thus do not have any impact on the process hash either:
     *                  - Changing an exposure set that is not assigned to a part.
     *                  - Changing the job description
     *                  - Changing names or identifiers of resources, parts, segments etc.
     *                  - Adding custom data to the job

     * \param[out]      processHash                             output variable for result (must not be null)
     * \return          EOS_ERR_NO_ERROR                        if the check was successful. Refer to match for result.
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine config is loaded
     * \return          EOS_ERR_INVALID_ARGUMENT                if NULL was passed as pointer to the process hash
     *                                                          (i.e. empty directory or one or more configuration files are missing)
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetProcessHash(EOS_ULONG * processHash);

    /*!
     * \brief           Retrieves the job- or machine specific adjustment of an adjustable process parameter as defined by the currently loaded job
     *                  or machine configuration, respectively.
     * \param[in]       id                                      the id of the adjustable process parameter for which the adjustment shall be retrieved
     * \param[in]       type                                    the type of adjustment that shall be retrieved, i.e., machine specific or job specific
     * \param[out]      adjustment                              output variable for adjustment structure; remains untouched in case of error
     * \return          EOS_ERR_NO_ERROR                        if the adjustment was retrieved successfully
     * \return          EOS_ERR_INVALID_ARGUMENT                if NULL was passed as id or as output parameter
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \return          EOS_ERR_ADJUSTMENT_ID_INVALID           if the currently loaded material set does not contain an adjustable process parameter with the specified id
     * \return          EOS_ERR_ADJUSTMENT_TYPE_NOT_ENABLED     if the requested adjustment type is not enabled for the adjustable process parameter referenced by id.
     *                                                          E.g., when trying to get a job specific adjustment for an adjustable process parameter that has only
     *                                                          machine specific adjustments enabled.
     * \note            Use EosFileProperties_LoadMaterialSetInfo() to get a list of all adjustable process parameters for a certain material set.
     * \note            If successful, the function dynamically allocates memory which should be freed using function EosTaskGen_FreeProcessParameterAdjustment().
     * \note            This function is thread safe
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetProcessParameterAdjustment(EOS_CWSTR id, EOS_ADJUSTMENT_TYPE type, EOS_PROCESS_PARAMETER_ADJUSTMENT const ** adjustment);

    /*!
     * \brief           Frees a process parameter adjustment which was allocated with EosTaskGen_GetProcessParameterAdjustment
     * \param[in]       adjustment                              adjustment structure
     * \returns         EOS_ERR_NO_ERROR                        if the given struct was freed successfully
     * \return          EOS_ERR_ARGUMENT_INVALID                if the adjustment pointer is NULL or was not created with EosTaskGen_GetProcessParameterAdjustment()
     * \note            This function is thread safe
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_FreeProcessParameterAdjustment(EOS_PROCESS_PARAMETER_ADJUSTMENT const * adjustment);

    /*!
     * \brief           Retrieves the job and machine specific adjustments of all adjustable process parameters as defined by the currently loaded job
     *                  and machine configuration.
     * \param[out]      adjustments                             output variable for adjustment list; remains untouched in case of error
     * \return          EOS_ERR_NO_ERROR                        if the adjustments were retrieved successfully
     * \return          EOS_ERR_INVALID_ARGUMENT                if NULL was passed for one of the output parameters
     * \return          EOS_ERR_CONFIG_NOT_LOADED               if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED                  if no job is loaded
     * \note            If successful, the function dynamically allocates memory which should be freed using function EosTaskGen_FreeCompleteProcessParameterAdjustmentList().
     * \note            The adjustments will appear in lexicographical order of their id. If both, job specific and machine specific adjustment is enabled for
     *                  an adjustable parameter,
     * \note            The size of the adjustments array depends solely on the number of adjustable process parameters and whether each of these parameters
     *                  has job specific or machine specific adjustments enabled. I.e., it is independent of how many adjustments were actually specified.
     *                  Example:
     *                  The underlying material set defines 3 adjustable parameters: one with only job specific adjustments enabled, one with only machine specific
     *                  adjustments enabled and one with both enabled. Therefore, EosTaskGen_GetCompleteProcessParameterAdjustmentList will always return 4 entries,
     *                  irrespective of which adjustments were actually specified by the user. Entries for unspecified adjustments will simply have their
     *                  hasValue flag set to EOS_FALSE.
     * \note            This function is thread safe
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetCompleteProcessParameterAdjustmentList(EOS_PROCESS_PARAMETER_ADJUSTMENT_LIST const ** adjustments);

    /*!
     * \brief           Frees an array of process parameter adjustments which was allocated with EosTaskGen_GetCompleteProcessParameterAdjustmentList
     * \param[in]       adjustments                             adjustment list
     * \returns         EOS_ERR_NO_ERROR                        if the given list was freed successfully
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as parameter or if the list was not created with
     *                                                          EosTaskGen_GetCompleteProcessParameterAdjustmentList
     * \note            This function is thread safe
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_FreeCompleteProcessParameterAdjustmentList(EOS_PROCESS_PARAMETER_ADJUSTMENT_LIST const * adjustments);

    /*!
     * \brief           Returns the currently effective value of an adjustable process parameter
     * \param[in]       id                                      the id of the adjustable process parameter for which the effective value shall be returned
     * \param[out]      effectiveValue                          if successful the effective value will be stored here
     * \param[out]      incorporatesMachineSpecificAdjustment   if successful and not NULL, this will flag whether or not a machine specific adjustment was
     *                                                          applied and is currently in effect. A machine specific adjustment may not be in effect,
     *                                                          even though the corresponding adjustable process parameter has machine specific adjustments enabled
     *                                                          and such a machine specific adjustment was specified. This is the case if the operation type
     *                                                          of the adjustable process parameter is EOS_AO_OVERRIDE and a job specific adjustment was specified.
     *                                                          Job specific adjustments are always applied after machine specific adjustments, therefore, if
     *                                                          the operation type is EOS_AO_OVERRIDE, the job specific adjustment will override any previous
     *                                                          machine specific adjustment. In that case the flag will be set to EOS_FALSE.
     * \param[out]      incorporatesJobSpecificAdjustment       If successful and not NULL, this will flag whether or not a job specific adjustment was applied.
     * \returns         EOS_ERR_NO_ERROR                        if the given array was freed successfully
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed for any of the parameters
     * \note            This function is thread safe
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetEffectiveProcessParameterValue(EOS_CWSTR id,
        EOS_DOUBLE * effectiveValue,
        EOS_BOOL * incorporatesMachineSpecificAdjustment,
        EOS_BOOL * incorporatesJobSpecificAdjustment);

    /*!
     * \brief           Determines the indices of all laser/scanner units that can be assigned to the part denoted by partId
     * \param[in]       partId                                  id of the part to examine
     * \param[out]      laserScannerIndices                     pointer to an array where the indices of the assignable laser/scanner units will be written
     * \param[in]       sizeOfLaserScannerIndices               size of the output array
     * \return          EOS_ERR_NO_ERROR                        if successful
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_INSUFFICIENT_SIZE               if the array is not big enough to hold the indices of all assignable laser/scanner units
     * \return          EOS_ERR_INVALID_PART_ID                 if there are no exposed layers for the given part id or segment index
     *                                                          (possibly because there is no part with the given id or segment with the given index)
     * \note            If the array has more elements than there are assignable laser/scanner units, the surplus entries will be set to EOS_NO_LASER.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_DetermineAssignableLasers(EOS_PARTID partId,
        EOS_BYTE * laserScannerIndices,
        EOS_SIZET sizeOfLaserScannerIndices);

    /*!
     * \brief           Retrieves the process heights of the currently loaded job
     *                  and machine configuration. Use this to verify that the
     *                  job can be built on the target machine
     *                  (i.e., processHeights.totalZTravel <= processHeights.maxZTravel).
     * \param[out]      processHeights              if successful the process heights will
     *                                              be written to the output structure.
     * \return          EOS_ERR_NO_ERROR            if the process heights were retrieved
     *                                              successfully
     * \return          EOS_ERR_INVALID_ARGUMENT    if NULL was passed
     * \return          EOS_ERR_CONFIG_NOT_LOADED   if no machine configuration is loaded
     * \return          EOS_ERR_JOB_NOT_LOADED      if no job is loaded
     * \note            This function is thread safe
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetProcessHeights(EOS_PROCESS_HEIGHTS * processHeights);

    /* END OF TASK GENERATION */

    /* BEGIN OF GEOMETRY FUNCTIONS */

    /*!
     * \brief           Begins to apply linear scaling to a given STL file and saves it as a new STL file.
     * \param[in]       pathToInputStlFile                      path to an STL file
     * \param[in]       pathToOutputStlFile                     path for the result STL file
     * \param[in]       scalingOptions                          contains all linear scaling parameters.
     * \param[in]       pathToEospar                            file path to a Material Set file (.eospar)
     * \param[out]      fileCreationHandle                      if the call is successful, this will hold a handle representing the file creation.
     *                                                          You can use the handle to check the creation status.
     * \return          EOS_ERR_NO_ERROR                        if the creation was successfully queued
     * \return          EOS_ERR_FILE_NOT_FOUND                  if the input STL file was not found
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if the output STL cannot be written to the path provided
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_EOSPAR_NOT_FOUND                if the eospar was not found
     * \return          EOS_ERR_EOSPAR_CORRUPT                  if the eospar has an unrecognized format
     * \return          EOS_ERR_EOSPAR_INVALID                  if the eospar has invalid or missing data
     * \return          EOS_ERR_OUT_OF_MEMORY                   if memory allocation failed
     * \return          EOS_ERR_LOADING_DLL_FAILED              if a DLL could not be loaded because the file is missing
     *                                                          or a necessary license is missing
     * \note            All scaling percentages given as parameters specify a scaling amount that is added on top of the current size.
     *                  The resulting scaling factor is calculated by (100.0 + <scalingPercent>) / 100.0, i.e. a parameter value of 0 results
     *                  in no change in size and a value of 10 results in scaling by 1.1
     * \note            The scaling in z direction is interpolated linearly between scalingPercentZBottom (at the bottom) and
     *                  scalingPercentZTop (at the top).
     * \note            The triggered scaling operation runs in a different thread.
     *                  Use EosGeometry_GetFileCreationStatus() to check if it has finished or call
     *                  EosGeometry_WaitForFileCreation() to wait for its completion.
     * \note            File creation can be aborted with EosGeometry_AbortFileCreation().
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosGeometry_BeginScalingLinearWithOptions(
        EOS_CWSTR pathToInputStlFile,
        EOS_CWSTR pathToOutputStlFile,
        EOS_SCALING_OPTIONS const * scalingOptions,
        EOS_CWSTR pathToEospar,
        EOS_FILECREATION_HANDLE * fileCreationHandle);

    /*!
     * \brief           Begins to apply smart scaling to a given STL file and saves it as a new STL file.
     * \param[in]       pathToInputStlFile                      path to an STL file
     * \param[in]       pathToOutputStlFile                     path for the result STL file
     * \param[in]       scalingOptions                          contains all smart scaling parameters.
     * \param[in]       pathToEospar                            file path to a Material Set file (.eospar)
     * \param[out]      fileCreationHandle                      if the call is successful, this will hold a handle representing the file creation.
     *                                                          You can use the handle to check the creation status.
     * \return          EOS_ERR_NO_ERROR                        if the creation was successfully queued
     * \return          EOS_ERR_FILE_NOT_FOUND                  if the input STL file was not found
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if the output STL cannot be written to the path provided
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_EOSPAR_NOT_FOUND                if the eospar was not found
     * \return          EOS_ERR_EOSPAR_CORRUPT                  if the eospar has an unrecognized format
     * \return          EOS_ERR_EOSPAR_INVALID                  if the eospar has invalid or missing data
     * \return          EOS_ERR_DOMAIN_ERROR                    if jobHeightInMm is negative or 0
     * \return          EOS_ERR_STL_ABOVE_JOB_HEIGHT            if the input stl file is (partially) above the given job height
     * \return          EOS_ERR_STL_BELOW_PLATFORM              if the input stl file is (partially) below the platform, i.e. it contains negative z coordinates
     * \return          EOS_ERR_STL_OUTSIDE_BUILD_VOLUME        if the input stl file is (partially) outside the build volume of the machine
     * \return          EOS_ERR_OUT_OF_MEMORY                   if memory allocation failed
     * \return          EOS_ERR_LOADING_DLL_FAILED              if a DLL could not be loaded because the file is missing
     *                                                          or a necessary license is missing
     * \note            Both parameters scalingPercentX and scalingPercentY specify a scaling amount that is added on top of the current size.
     *                  The resulting scaling factor is calculated by (100.0 + <scalingPercent>) / 100.0, i.e. a parameter value of 0 results
     *                  in no change in size and a value of 10 results in scaling by 1.1
     * \note            The scaling in z direction is done in a 'smart' way: The inverse of typical deformations on produced parts is applied,
     *                  so that the resulting parts closely match the initial geometry. The smart scaling is performed after the scaling in
     *                  x and y direction.
     * \note            The triggered scaling operation runs in a different thread.
     *                  Use EosGeometry_GetFileCreationStatus() to check if it has finished or call
     *                  EosGeometry_WaitForFileCreation() to wait for its completion.
     * \note            The caller must initialize scalingOptions.reserved as an array of zero bytes. This ensures compatibility with future extensions.
     * \note            File creation can be aborted with EosGeometry_AbortFileCreation().
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosGeometry_BeginScalingSmartWithOptions(
        EOS_CWSTR pathToInputStlFile,
        EOS_CWSTR pathToOutputStlFile,
        EOS_SCALING_OPTIONS const * scalingOptions,
        EOS_CWSTR pathToEospar,
        EOS_FILECREATION_HANDLE * fileCreationHandle);

    /*!
     * \brief           Begins to slice a given STL file and saves it as an SLI file.
     * \param[in]       pathToInputStlFile                          path to an STL file
     * \param[in]       pathToOutputSliFile                         path for the result SLI file
     * \param[in]       slicingMode                                 the slicing mode to be used
     * \param[in]       sliceThickness                              the thickness in micrometer to be used for slicing
     * \param[out]      fileCreationHandle                          if the call is successful, this will hold a handle representing the file creation.
     *                                                              You can use the handle to check the creation status.
     * \return          EOS_ERR_NO_ERROR                            if the creation was successfully queued
     * \return          EOS_ERR_FILE_NOT_FOUND                      if the input STL file was not found
     * \return          EOS_ERR_STL_INVALID                         if the input STL could not be converted into a mesh (empty stl files are permitted)
     * \return          EOS_ERR_STL_CONTAINS_NEGATIVE_COORDINATES   if the input STL includes points with negative coordinates
     * \return          EOS_ERR_OVERFLOW_ERROR                      if the slicing generated too many points to store in an SLI file
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR                 if the output SLI cannot be written to the path provided
     * \return          EOS_ERR_ARGUMENT_INVALID                    if NULL was passed as an argument (paths, file CreationHandle)
     *                                                              if a negative sliceThickness was specified
     * \return          EOS_ERR_OUT_OF_MEMORY                       if memory allocation failed
     * \return          EOS_ERR_LOADING_DLL_FAILED                  if a DLL could not be loaded because the file is missing
     *                                                              or a necessary license is missing
     * \note            The triggered slicing operation runs in a different thread.
     *                  Use EosGeometry_GetFileCreationStatus() to check if it has finished or call
     *                  EosGeometry_WaitForFileCreation() to wait for its completion.
     * \note            File creation can be aborted with EosGeometry_AbortFileCreation().
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosGeometry_BeginSlicing(
        EOS_CWSTR pathToInputStlFile,
        EOS_CWSTR pathToOutputSliFile,
        EOS_SLICING_MODE slicingMode,
        EOS_MICROMETER sliceThickness,
        EOS_FILECREATION_HANDLE * fileCreationHandle);

    /*!
     * \brief           Aborts an active file creation.
     * \param[in]       fileCreationHandle                      the handle of the file creation that should be aborted
     * \return          EOS_ERR_NO_ERROR                        if the file creation was still active and its interrupt was signalled successfully
     * \return          EOS_ERR_FILECREATION_INVALID            if the passed file creation handle is invalid
     * \return          EOS_ERR_FILECREATION_NOT_RUNNING        if the corresponding file creation had already stopped
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosGeometry_AbortFileCreation(EOS_FILECREATION_HANDLE fileCreationHandle);

    /*!
     * \brief           Returns the current status of a file creation.
     * \param[in]       fileCreationHandle                      handle of the file creation to get the status for
     * \return          the current creation status
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_FILECREATION_STATUS EosGeometry_GetFileCreationStatus(EOS_FILECREATION_HANDLE fileCreationHandle);

    /*!
     * \brief           Waits until an active file creation ends or a timeout occurs.
     * \param[in]       fileCreationHandle                      handle of the file creation to wait for
     * \param[in]       timeout                                 timeout in milliseconds to wait before returning even though the creation was still active;
     *                                                          pass EOS_INFINITE_DURATION to wait indefinitely.
     * \return          EOS_ERR_NO_ERROR                        if the specified file creation has ended
     *                                                          (with whatever result, not necessarily successful)
     * \return          EOS_ERR_FILECREATION_INVALID            if the passed file creation handle is invalid
     * \return          EOS_ERR_WAIT_TIMED_OUT                  if the timeout occurred before the file creation ended
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosGeometry_WaitForFileCreation(EOS_FILECREATION_HANDLE fileCreationHandle, EOS_MILLISECS timeout);

    /*!
     * \brief           Returns the result of a finished file transfer
     * \param[in]       fileCreationHandle                      handle of the file creation to get the result for
     * \return          EOS_ERR_NO_ERROR                        if the status of the file creation is EOS_TS_COMPLETED
     * \return          EOS_ERR_FILECREATION_INVALID            if the passed file creation handle is invalid
     * \return          EOS_ERR_FILECREATION_ABORTED            if the status of the file creation is EOS_TS_ABORTED
     *                  Any other return code depending on the specific file creation if its status is EOS_TS_FAILED
     * \note            This function will block until the given file creation has finished.
     */
    EOS_EOSPRINT_API EOS_ERROR EosGeometry_GetFileCreationResult(EOS_FILECREATION_HANDLE fileCreationHandle);

    /*!
    * \brief           Releases a file creation handle obtained through either EosGeometry_BeginScalingLinearWithOptions(),
    *                  EosGeometry_BeginScalingSmartWithOptions() or EosGeometry_BeginSlicing().
    * \param[in]       fileCreationHandle                       file creation handle that should be freed
    * \return          EOS_ERR_NO_ERROR                         if the handle was released
    * \return          EOS_ERR_FILECREATION_INVALID             if the passed file creation handle is invalid
    * \return          EOS_ERR_FILECREATION_RUNNING             if the handle could not be released caused by
    *                                                           running file creations
    * \note            This function is thread safe.
    */
    EOS_EOSPRINT_API EOS_ERROR EosGeometry_ReleaseFileCreationHandle(EOS_FILECREATION_HANDLE fileCreationHandle);

    /*!
     * \brief           Queries the number of threads that will be used for task creation
     * \param[in]       numberOfThreads                         output argument, will be set the the number of threads if successful
     * \return          EOS_ERR_NO_ERROR                        if the number of threads was written successfully
     * \return          EOS_ERR_NOT_IDLE                        if a task generation is currently running
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \note            The number can only be queried if no task export is running
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_GetNumberOfTaskCreationThreads(EOS_SIZET * numberOfThreads);

    /*!
     * \brief           Changes the number of threads that will be used for task creation
     * \param[in]       numberOfThreads                         number of threads that will be used for the next task creation
     * \return          EOS_ERR_NO_ERROR                        if the number of threads was changed successfully
     * \return          EOS_ERR_NOT_IDLE                        if a task generation is currently running
     * \return          EOS_ERR_ARGUMENT_INVALID                if numberOfThreads is greater than the system supports or if it is 0
     * \note            The number can only be changed if no task export is running
     * \note            Use EosThreads_GetNumberOfAvailableThreads() to query the number of available threads for your system
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosTaskGen_SetNumberOfTaskCreationThreads(EOS_SIZET numberOfThreads);

    /*!
     * \brief           Queries the number of available threads of the current system
     * \param[in]       numberOfThreads                         output argument, will be set the number of threads if successful
     * \return          EOS_ERR_NO_ERROR                        if the number of threads was retrieved successfully
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosThreads_GetNumberOfAvailableThreads(EOS_SIZET * numberOfThreads);

    /* END OF GEOMETRY FUNCTIONS */

    /* BEGIN OF PARAMETERS FUNCTIONS */

    /*!
     * \brief           Opens a material set (.eospar) file for read & write access.
     *                  The file will be locked until it is closed via EosParameters_CloseMaterialSet or the EOSPRINT API is deinitialized.
     * \param[in]       filePath                                file path to an .eospar file
     * \param[out]      fileHandle                              if successful, this will hold the file handle that can be used with other functions
     *                                                          such as EosParameters_GetProcessParameters or EosParameters_SetProcessParameters
     * \return          EOS_ERR_NO_ERROR                        if opening the file was successful
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_FILE_NOT_FOUND                  if the file was not found
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if the file was found, but cannot be opened for read & write access
     * \return          EOS_ERR_EOSPAR_CORRUPT                  if the file was found, but cannot be read
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_OpenMaterialSet(EOS_CWSTR filePath, EOS_FILE_HANDLE * fileHandle);

    /*!
     * \brief           Saves all changes of the specified material set (.eospar) to disk, but keeps the file open for further changes.
     * \param[in]       fileHandle                              handle to the file that should be closed
     * \return          EOS_ERR_NO_ERROR                        if opening the file was successful
     * \return          EOS_ERR_ARGUMENT_INVALID                if fileHandle is unknown
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if the file could not be saved
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_SaveMaterialSet(EOS_FILE_HANDLE fileHandle);

    /*!
     * \brief           Closes the specified material set (.eospar) file handle.
     *                  If EosParameters_SaveMaterialSet is not called before, the file is not changed.
     * \param[in]       fileHandle                              handle to the file that should be closed
     * \return          EOS_ERR_NO_ERROR                        if opening the file was successful
     * \return          EOS_ERR_ARGUMENT_INVALID                if fileHandle is unknown
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_CloseMaterialSet(EOS_FILE_HANDLE fileHandle);

    /*!
     * \brief           Reads all accessible process parameters from a material set (.eospar).
     * \param[in]       fileHandle                              The handle to the material set (.eospar) file
     * \param[in]       parameters                              If successful, this will hold a list of process parameters
     * \return          EOS_ERR_NO_ERROR                        if successful
     * \return          EOS_ERR_ARGUMENT_INVALID                if fileHandle is unknown
     * \note            Allocates memory for parameters that must be freed via EosParameters_FreeProcessParameterList
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_ReadProcessParameters(EOS_FILE_HANDLE fileHandle, EOS_PROCESS_PARAMETER_LIST const ** parameters);

    /*!
     * \brief           Frees memory that was allocated by EosParameters_ReadProcessParameters().
     * \param[in]       fileHandle                              The handle to the material set (.eospar) file
     * \param[in]       parameters                              process parameter list
     * \return          EOS_ERR_NO_ERROR                        if memory was successfully freed
     * \return          EOS_ERR_ARGUMENT_INVALID                if fileHandle is unknown
     * \return          EOS_ERR_ARGUMENT_INVALID                if the parameters pointer is NULL or was not created
     *                                                          with EosParameters_ReadProcessParameters()
     * \return          EOS_ERR_UNEXPECTED_ERROR                if an unexpected error occurred
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_FreeProcessParameterList(EOS_FILE_HANDLE fileHandle, EOS_PROCESS_PARAMETER_LIST const * parameters);

    /*!
     * \brief           Writes the specified process parameter to the material set (.eospar) file handle.
     *                  It is not possible to change the displayName or unit of a parameter via this operation.
     *                  Only parameters with accessMode EOS_PPAM_READWRITE can be changed.
     * \param[in]       fileHandle                              The handle to the material set (.eospar) file
     * \param[in]       path                                    Path of the process parameter.
     * \param[in]       type                                    Type of the process parameter.
     * \param[in]       value                                   Value of the process parameter.
     * \return          EOS_ERR_NO_ERROR                        if the parameter was set successfully
     * \return          EOS_ERR_PARAMETER_DOES_NOT_EXIST        if the parameter does not already exist within the material
     *                                                          set. This function cannot be used to create new parameters.
     * \return          EOS_ERR_PARAMETER_INACCESSIBLE          if the parameter does not have accessMode EOS_PPAM_READWRITE
     *                                                          (see EOS_PROCESS_PARAMETER_ACCESS_MODE) in the material set
     * \return          EOS_ERR_INVALID_PARAMETER_VALUE         if the value is invalid, e.g. because it's of wrong type or
     *                                                          does not fall into the parameter's min/max value range.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_WriteProcessParameter(EOS_FILE_HANDLE fileHandle, EOS_CWSTR path, EOS_VARIANT_TYPE type, EOS_VARIANT value);

    /*!
     * \brief           Clones an exposure set
     * \param[in]       fileHandle                              The handle to the material set (.eospar) file
     * \param[in]       originalExposureSetName                 The name of the exposure set to clone (must exist and be editable)
     * \param[in]       clonedExposureSetName                   The name of the cloned exposure set (must not exist)
                                                                The name may only contain characters of the codepage Windows-1252 except the hash character (0x23)
     * \return          EOS_ERR_NO_ERROR                        if successful
     * \return          EOS_ERR_NO_MATCH                        if the material set does not contain an exposure set with the name specified by originalExposureSetName
     * \return          EOS_ERR_NOT_ALLOWED                     if the exposure set is protected
     * \return          EOS_ERR_ID_CONFLICT                     if a material set with the name specified by clonedExposureSetName already exists
     * \return          EOS_ERR_STRING_CONTAINS_ILLEGAL_CHARS   if clonedExposureSetName contains a hash character or a character which is not representable with the codepage Windows-1252
     * \return          EOS_ERR_ARGUMENT_INVALID                if fileHandle, originalExposureSetName or clonedExposureSetName is NULL
     * \note            Only editable exposure sets may be cloned because immutable clones are of no use.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_CloneExposureSet(EOS_FILE_HANDLE fileHandle, EOS_CWSTR originalExposureSetName, EOS_CWSTR clonedExposureSetName);

    /*!
     * \brief           Deletes an exposure set
     * \param[in]       fileHandle                              The handle to the material set (.eospar) file
     * \param[in]       exposureSetName                         The name of the exposure set to delete
     * \return          EOS_ERR_NO_ERROR                        if successful
     * \return          EOS_ERR_NO_MATCH                        if the materual set does not contain an exposure set with the name specified by originalExposureSetName
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_DeleteExposureSet(EOS_FILE_HANDLE fileHandle, EOS_CWSTR exposureSetName);

    /*!
     * \brief           Commits changes
     * \param[in]       fileHandle                             The handle to the material set (.eospar) file
     * \param[in]       client                                 The name of the client application which commits this change,
     *                                                         may not contain the '#' character.
     * \param[in]       comment                                A comment to store with this commit, may be NULL,
     *                                                         may not contain the '#' character.
     * \return          EOS_ERR_NO_ERROR                       if changes were successfully committed
     * \return          EOS_ERR_ARGUMENT_INVALID               if fileHandle is unknown
     * \return          EOS_ERR_ARGUMENT_INVALID               if client is NULL
     * \return          EOS_ERR_STRING_CONTAINS_ILLEGAL_CHARS  if client or comment contain the '#' character
     * \return          EOS_ERR_INCORRECT_ENCODING             if client or comment are not valid UTF-16 strings
     * \return          EOS_ERR_NOT_ALLOWED                    if there are no changes to commit
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_CommitChanges(EOS_FILE_HANDLE fileHandle, EOS_CWSTR client, EOS_CWSTR comment);

    /*!
     * \brief           Loads exposure type templates from the specified exposureTypes.eoscft
     * \param[in]       filePath                                The path to an exposureTypes.eoscft for the specified machine type
     * \param[in]       exposureTypeTemplates                   if successful this will hold an struct describing the exposure type
     *                                                          templates of the loaded file
     * \return          EOS_ERR_NO_ERROR                        if successful
     * \return          EOS_ERR_FILE_NOT_FOUND                  if the specified file was not found
     * \return          EOS_ERR_FILESYSTEM_IO_ERROR             if the file cannot be accessed
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     * \return          EOS_ERR_FILE_CORRUPT                    if the exposureTypes.eoscft file cannot be loaded
     * \note            The pointer to the exposure type templates remains valid until EosParameters_FreeExposureTypeTemplates() was called
     * \note            In all error cases the output parameter exposureSetTemplates remains untouched.
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_LoadExposureTypeTemplates(EOS_CWSTR filePath,
        EOS_EXPOSURETYPE_TEMPLATES const ** exposureTypeTemplates);

    /*!
     * \brief           Frees memory that was allocated by EosParameters_LoadExposureTypeTemplates().
     * \param[in]       exposureTypeTemplates                   Pointer to an exposure type templates struct, obtained by
     *                                                          EosParameters_LoadExposureTypeTemplates()
     * \return          EOS_ERR_NO_ERROR                        if the struct was successfully freed
     * \return          EOS_ERR_ARGUMENT_INVALID                if the pointer argument is NULL or if the pointed-to struct was not created
     *                                                          with EosParameters_LoadExposureTypeTemplates()
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_FreeExposureTypeTemplates(EOS_EXPOSURETYPE_TEMPLATES const * exposureTypeTemplates);

    /*!
     * \brief           Inserts a new exposure step into an exposure set, based on the specified exposure type template.
     * \param[in]       fileHandle                              The handle to the material set (.eospar) file
     * \param[in]       exposureSetName                         The name of the exposure set to modify
     * \param[in]       exposureTypeTemplates                   The pointer to the exposure type templates, obtained by
     *                                                          EosParameters_LoadExposureTypeTemplates()
     * \param[in]       exposureTypeTemplateIndex               The index of the exposure type template to insert
     * \param[in]       exposureStepInsertAtIndex               The index at which the new exposure step should be inserted,
     *                                                          existing steps at or after the insertion point will be shifted to higher indexes.
     * \return          EOS_ERR_NO_ERROR                        if successful
     * \return          EOS_ERR_CONFIG_INVALID                  if the model type or the version of the exposure type templates does not match
     *                                                          that of the exposure set
     * \return          EOS_NO_MATCH                            if the material set does not contain an exposure set with the name specified
     *                                                          by exposureSetName
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed for fileHandle, exposureSetName or exposureTypeTemplates
     *                                                          or if exposureTypeTemplates was not obtained via
     *                                                          EosParameters_LoadExposureTypeTemplates()
     *                                                          or if exposureTypeTemplatesIndex is not less than
     *                                                          exposureTypeTemplates.exposureTypeTemplatesSize
     *                                                          or if exposureStepInsertAtIndex is greater than exposureStepsSize of the
     *                                                          given exposure set
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_InsertExposureStep(EOS_FILE_HANDLE fileHandle,
        EOS_CWSTR exposureSetName,
        EOS_EXPOSURETYPE_TEMPLATES const * exposureTypeTemplates,
        EOS_UINT exposureTypeTemplateIndex,
        EOS_UINT exposureStepInsertAtIndex);

    /*!
     * \brief           Deletes an existing exposure step from an exposure set.
     * \param[in]       fileHandle                              The handle to the material set (.eospar) file
     * \param[in]       exposureSetName                         The name of the exposure set to modify
     * \param[in]       exposureStepIndex                       The index at which the exposure step should be deleted,
     *                                                          existing steps after the deletion point will be shifted to lower indexes.
     * \return          EOS_ERR_NO_ERROR                        if successful
     * \return          EOS_ERR_NO_MATCH                        if the material set does not contain an exposure set with the name specified
     *                                                          by exposureSetName
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed for fileHandle or exposureSetName
     *                                                          or if exposureStepIndex is not less than the number of exposure steps in the
     *                                                          given exposure set
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_DeleteExposureStep(EOS_FILE_HANDLE fileHandle,
        EOS_CWSTR exposureSetName,
        EOS_UINT exposureStepIndex);

    /*!
     * \brief           Changes the exposure order of exposure steps of an exposure set.
     * \param[in]       fileHandle                              The handle to the material set (.eospar) file
     * \param[in]       exposureSetName                         The name of the exposure set to modify
     * \param[in]       oldExposureStepIndex                    The current index of the exposure step to move
     * \param[in]       newExposureStepIndex                    The index which the exposure step should have after the move,
     *                                                          existing steps between both indexes will be shifted towards
     *                                                          oldExposureStepIndex
     * \return          EOS_ERR_NO_ERROR                        if successful
     * \return          EOS_ERR_NO_MATCH                        if the material set does not contain an exposure set with the name specified
     *                                                          by exposureSetName
     * \return          EOS_ERR_ARGUMENT_INVALID                if NULL was passed for fileHandle or exposureSetName
     *                                                          or if oldExposureStepIndex is not less than the number of exposure steps in
     *                                                          the given exposure set
     *                                                          or if newExposureStepIndex is not less than the number of exposure steps in
     *                                                          the given exposure set
     * \note            This function is thread safe.
     */
    EOS_EOSPRINT_API EOS_ERROR EosParameters_MoveExposureStep(EOS_FILE_HANDLE fileHandle,
        EOS_CWSTR exposureSetName,
        EOS_UINT oldExposureStepIndex,
        EOS_UINT newExposureStepIndex);

    /* END OF PARAMETERS FUNCTIONS */

#ifdef __cplusplus
} // end of extern "C"
#endif

#endif // EOS_EOSPRINT_API_H
