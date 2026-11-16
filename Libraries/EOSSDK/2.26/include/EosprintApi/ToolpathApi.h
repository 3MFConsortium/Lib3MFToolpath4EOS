/*!
 * \file        ToolpathApi.h
 * \brief       Definition of the EOSPRINT Toolpath API
 *
 * \author      EOS GmbH, Robert-Stirling-Ring 1, 82152 Krailling, Germany
 * \copyright   (c) EOS GmbH 2021
 *              All rights reserved
 */

#ifndef EOS_TOOLPATH_API_H
#define EOS_TOOLPATH_API_H

#ifdef EOS_TOOLPATH_API_IMPLEMENTATION
#   define EOS_TOOLPATH_API __declspec(dllexport)
#else
#   define EOS_TOOLPATH_API __declspec(dllimport)
#endif

#include "Types.h"
#include "ToolpathTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

    /*!
     * \brief           Requests a challenge byte sequence for challenge/response authentication of the client,
     *                  automatically resetting the previous authentication state to 'not authenticated' and invalidating any previously allocated challenge data.
     * \param[out]      challenge                           the challenge byte sequence. Valid until another call to \p EosToolpath_GetAuthChallenge has been committed.
     * \param[out]      challengeLength                     the length of the challenge generated
     * \return          EOS_ERR_NO_ERROR                    if the challenge was generated successfully
     * \return          EOS_ERR_INTERNAL_ERROR              if an error occurred generating the challenge byte sequence.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_GetAuthChallenge(EOS_BYTE ** challenge, EOS_ULONG * challengeLength);

    /*!
     * \brief           Attempts to authenticate the current client with the Toolpath API using a byte sequence encrypted by the client
     * \param[in]       client                              the ID of the client application which is to be authenticated
     * \param[in]       response                            the encrypted response byte sequence
     * \param[in]       responseLength                      the length of \p response
     * \return          EOS_ERR_NO_ERROR                    if the authentication attempt has succeeded and the client is now authenticated
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the authentication attempt has failed
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetAuthResponse(EOS_UINT client, EOS_BYTE const * response, EOS_ULONG responseLength);

    /*!
     * \brief       Creates a regular task in the specified directory by translating the exposure toolpath from the specified *.rawtask file.
     * \param[in]   fileHandle                              an open input file handle to a toolpath (as obtained via EosToolpath_OpenInputFile)
     * \param[in]   metaData                                metadata to embed in the task file
     * \param[in]   targetDirectory                         file path to a local directory to which the task & data files will be written
     * \return      EOS_NO_ERROR                            if task creation was started successfully
     * \return      EOS_ERR_MISSING_LICENSE                 if a license for task creation is missing on the current system
     * \return      EOS_ERR_NOT_IDLE                        if task creation is already running
     * \return      EOS_ERR_ENCODING_CONVERSION_ERROR       if a directory path was passed that cannot be converted to the narrow file api codepage
     * \return      EOS_ERR_NOT_A_DIRECTORY                 if the passed path is not a directory (e.g., it does not exist, or it is a regular file)
     * \return      EOS_ERR_DIRECTORY_NOT_EMPTY             if the target directory is not empty
     * \return      EOR_ERR_ARGUMENT_INVALID                if NULL was passed as an argument
     *                                                      or if metaData->numberOfAllowedBuilds exceeds the bound of INT_MAX (2147483647)
     * \return      EOS_ERR_LOADING_DLL_FAILED              if a DLL could not be loaded because the file is missing
     *                                                      or a necessary license is missing
     * \return      EOS_ERR_FILE_HANDLE_INVALID             if the file handle is invalid or refers to an output file
     * \return      EOS_ERR_NOT_ALLOWED                     if the format of the output task is EOS_TF_RAW, but raw task export is not allowed, e.g.,
     *                                                      because a closed exposure set is assigned in the currently loaded job
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED        if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note        The triggered task creation runs in a different thread. Register a callback to get notifications about its
     *              progress and/or call EosTaskGen_WaitForTaskCreation() to wait for its completion.
     * \note        This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosTaskGen_BeginTaskCreationFromToolpath(EOS_FILE_HANDLE fileHandle, EOS_TASK_METADATA const * metaData, EOS_CWSTR targetDirectory);

    /*!
     * \brief       Opens an output file at the specified file path for writing a *.rawtask file.
     *              Process parameters and metadata will be taken from the currently loaded job and machine configuration.
     *              Neither job nor machine configuration can be unloaded before all output files/directories are closed.
     * \param[in]   filePath                            target file path for the *.rawtask file. Will be overwritten if it exists. (must not be null)
     * \param[out]  fileHandle                          if successful then the file handle will be written to the variable pointed to by this (must not be null)
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if filePath or fileHandle are null
     * \return      EOS_ERR_ILLEGAL_FILE_EXTENSION      if the file path does not end with .rawtask
     * \return      EOS_ERR_FILESYSTEM_IO_ERROR         if the file cannot be created for any file system related reason such as, e.g., missing rights,
     *                                                  the file exists and is locked by another process, the directory does not exist etc.
     * \return      EOS_ERR_CONFIG_NOT_LOADED           if no machine configuration is loaded
     * \return      EOS_ERR_CONFIG_INVALID              if the machine configuration is invalid
     * \return      EOS_ERR_JOB_NOT_LOADED              if no job is loaded
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note        While toolpath commands (EosToolpath_WriteXXX) are called on the handle returned by this function, the corresponding exposure commands
     *              will be written to the specified *.rawtask file and a number of *.data files in the same directory. Use EosTaskGen_LoadTaskInfo on the
     *              root file to determine the full set of files. It is recommended to create the files in an empty directory for clarity.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_OpenOutputFile(EOS_CWSTR filePath, EOS_FILE_HANDLE * fileHandle);

     /*!
     * \brief       Opens an output file at the specified file path for writing a *.task or a *.rawtask file.
     *              Process parameters and metadata will be taken from the currently loaded job and machine configuration.
     *              Neither job nor machine configuration can be unloaded before all output files/directories are closed.
     * \param[in]   metaData                            metadata to embed in the task file
     * \param[in]   filePath                            target file path for the *.(raw)task file. Will be overwritten if it exists. (must not be null)
     * \param[out]  fileHandle                          if successful then the file handle will be written to the variable pointed to by this (must not be null)
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if filePath or fileHandle are null
     * \return      EOS_ERR_ILLEGAL_FILE_EXTENSION      if the file path does not end with .task or .rawtask
     * \return      EOS_ERR_FILESYSTEM_IO_ERROR         if the file cannot be created for any file system related reason such as, e.g.,
     *                                                  missing rights, the file exists and is locked by another process, the directory does not exist etc.
     * \return      EOS_ERR_CONFIG_NOT_LOADED           if no machine configuration is loaded
     * \return      EOS_ERR_CONFIG_INVALID              if the machine configuration is invalid
     * \return      EOS_ERR_JOB_NOT_LOADED              if no job is loaded
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note        While toolpath commands (EosToolpath_WriteXXX) are called on the handle returned by this function, the corresponding exposure commands
     *              will be written to the specified *.(raw)task file and a number of *.data files in the same directory. Use EosTaskGen_LoadTaskInfo on the
     *              root file to determine the full set of files. It is recommended to create the files in an empty directory for clarity.
     * \note        Calling EosToolpath_CloseFile on a .task handle will check if all expected layers have been written before finalizing
     *              the task files. The resulting *.task from this command may then be directly transferred to the machine with
     *              EosMachineSession_BeginTaskUpload (no intermediate call to EosTaskGen_BeginTaskCreationFromToolpath() is necessary).
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_OpenOutputFileWithMetaData(EOS_TASK_METADATA const * metaData, EOS_CWSTR filePath, EOS_FILE_HANDLE * fileHandle);

    /*!
     * \brief       Closes all currently opened *.data files that belong to the specified file handle and opens new such .data files.
     *              All upcoming exposure commands will then be written to the new files until either EosToolpath_ChopOutput or EosToolpath_CloseFile is called.
     *              Chopping output is a means to keeping file sizes reasonably small. However, apart from filesystem limitations no technical necessity exists for chopping output.
     * \param[in]   fileHandle                              file handle to the toolpath task that should be chopped
     * \return      EOS_ERR_NO_ERROR                        if successful
     * \return      EOS_ERR_FILESYSTEM_IO_ERROR             if the file cannot be created for any file system related reason such as, e.g., missing rights,
     *                                                      the file exists and is locked by another process, the directory does not exist etc.
     * \return      EOS_ERR_FILE_NOT_FOUND                  if the file could not be found e.g. because the file handle is in valid
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED        if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note        This will only affect .rawtask output files. Generation of .task output files is handled by the machine configuration.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_ChopOutput(EOS_FILE_HANDLE fileHandle);

    /*!
     * \brief       Opens a *.task or *.rawtask file at the specified file path for reading.
     * \param[in]   filePath                            file path (must not be null, maybe *.task or *.rawtask)
     * \param[out]  fileHandle                          if successful then the file handle will be written to the variable pointed to by this (must not be null)
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if filePath or fileHandle are null
     * \return      EOS_ERR_ILLEGAL_FILE_EXTENSION      if the file path does not end with either .toolpath or .task
     * \return      EOS_ERR_FILE_NOT_FOUND              if the file does not exist
     * \return      EOS_ERR_FILESYSTEM_IO_ERROR         if the file does exist but cannot be opened for any file system related reason such as, e.g., missing rights
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \return      EOS_ERR_DATA_FORMAT_UNSUPPORTED     if the task does not contain suitable input data for the Toolpath API; this may happen if
     *                                                  a) the file was not specifically created for use with the ToolpathApi as *.rawtask and
     *                                                  b) the machine software version for which the task was created is < 2.24 (Edition 11/25)
     *                                                     => *.task files for older machine setups will not have usable toolpath data in most cases
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_OpenInputFile(EOS_CWSTR filePath, EOS_FILE_HANDLE * fileHandle);

    /*!
     * \brief       Close all files that belong to the specified file handle.
     * \param[in]   fileHandle                          file handle to close
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \return      EOS_ERR_FILECREATION_ABORTED        if the file handle points to a .task output file that was
     *                                                  not written completely, i.e. closed before all expected layers
     *                                                  were written. Ignore if this happened intentionally.
     * \note        This will act on handles returned by EosToolpath_OpenInputFile and EosToolpath_OpenOutputFile.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_CloseFile(EOS_FILE_HANDLE fileHandle);

    /*!
     * \brief       Selects the layer specified by layer index from the specified file and reads all exposure commands that apply to the specified laser/scanner unit. For each
     *              such command the corresponding callback function will be called if registered. The function will return only after all commands have been read and thus all
     *              callbacks will be invoked before this function has returned. Callback invocation will occur sequentially, i.e. the next command callback will only be invoked
     *              after the previous callback function has returned.
     * \param[in]   fileHandle                              input file handle
     * \param[in]   customDataHandle                        a custom data handle that will be passed on to the registered callback functions
     * \param[in]   layerIndex                              the one based layer index of the layer to read
     * \param[in]   readingOrder                            order in which exposure commands should be read from the toolpath files
     * \return      EOS_ERR_NO_ERROR                        if successful
     * \return      EOS_ERR_INVALID_LAYER                   if the specified layer does not exist
     * \return      EOS_ERR_FILE_HANDLE_INVALID             if the file handle is invalid
     * \return      EOS_ERR_NOT_IMPLEMENTED                 if the reading order is set to order by time only
     * \return      EOS_ERR_MISSING_LICENSE                 if a necessary license is missing
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED        if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_ReadLayer(EOS_FILE_HANDLE fileHandle, EOS_DATA_HANDLE customDataHandle, EOS_TOOLPATH_READING_ORDER readingOrder, EOS_LAYERINDEX layerIndex);

    /*!
     * \brief       Determines the number of layers in the specified task and the index of the first layer.
     * \param[in]   fileHandle                              input file handle
     * \param[out]  layerCount                              if successful, this will hold the number of layers in the specified task
     * \param[out]  firstLayer                              if successful, this will hold the index of the first layer
     * \return      EOS_ERR_NO_ERROR                        if successful
     * \return      EOS_ERR_FILE_HANDLE_INVALID             if the file handle is invalid
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED        if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_GetNumberOfLayers(EOS_FILE_HANDLE fileHandle, EOS_ULONG* layerCount, EOS_LAYERINDEX* firstLayer);

    /*!
     * \brief       Starts a new layer at the specified z position.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   z_um                                z position of the new layer in micrometers. Must be greater than 0.
     *                                                  Must be greater than the z position of the previous layer
     *                                                  Must be a valid increment to the previous z position with regard to the currently loaded material set
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle or if the specified z position is invalid
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_LAYER                       if this command was called inside a layer
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteBeginLayer(EOS_FILE_HANDLE fileHandle, EOS_MICROMETER z_um);

    /*!
     * \brief       Indicates the beginning of a new part on the current layer. All subsequent exposure commands will be mapped to the specified part until either
     *              EosToolpath_WriteBeginPart is called again with a different partId or until EosToolpath_WriteBeginLayer is called.
     *              Applies to all laser/scanner units.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   partId                              the part id links the upcoming exposure commands to a specific part in the job
     *                                                  the id must correspond to /openjob/parts/part/@id of the currently loaded job
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle or if the specified part id does not occur in the currently loaded job.
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_NO_LAYER                    if this command was called outside a layer
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note        A part may need to be "begun" multiple times, e.g., once per laser if it should be exposed by several lasers or if exposure by one laser should be interrupted in favor of another part
     *              and then continued at a later time.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteBeginPart(EOS_FILE_HANDLE fileHandle, EOS_INT partId);

    /*!
     * \brief       Switches the current laser/scanner selection to the specified laser/scanner.
     *              All following commands are executed by this laser
     * \param[in]   fileHandle                          output file handle
     * \param[in]   laserScannerIndex                   the index of the laser/scanner unit to which subsequent exposure commands apply
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_LSU_INVALID                 if the specified laser/scanner index is invalid
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteSelectLaserScanner(EOS_FILE_HANDLE fileHandle, EOS_BYTE laserScannerIndex);

    /*!
     * \brief       Sets the laser power for upcoming exposure vectors.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   power_W                             the laser power in Watts. Must be between 0 and the maximum laser power of the specified laser/scanner unit.
     *                                                  use EosTaskGen_GetMachineInfo to determine the maximum laser power.
     * \param[in]   delay_us                            a delay in microseconds after which the specified laser power should be applied. Set this to 0 to set the laser power immediately.
     *                                                  the specifed delay will be rounded to the nearest multiple of 10 us.
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if laserScannerIndex does not refer to an existing laser/scanner unit or if the power_W is not a valid power setting for that laser/scanner unit or if something is wrong with the delay_us.
     *                                                  if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_NO_PART                     if this command was called outside a part
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteSetLaserPower(EOS_FILE_HANDLE fileHandle, EOS_DOUBLE power_W, EOS_MICROSECS delay_us);

    /*!
     * \brief       Sets the scan speed for upcoming exposure vectors.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   speed_mm_per_s                      the scan speed in mm/s. Must be greater than 0 and less than maximum scan speed.
     *                                                  use EosTaskGen_GetMachineInfo to determine the maximum scan speed.
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if laserScannerIndex does not refer to an existing laser/scanner unit or if the power_W is not a valid power setting for that laser/scanner unit
     *                                                  if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_NO_PART                     if this command was called outside a part
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteSetScanSpeed(EOS_FILE_HANDLE fileHandle, EOS_DOUBLE speed_mm_per_s);

    /*!
     * \brief       Sets the focus for the specified laser. The laser/scanner unit must support this.
     *              For laser/scanner units with FOCUS_DEFOCUS cabability the specified focus must be either 1.0 (=focused) or 0.0 (=defocused).
     *              For laser/scanner units with VARIABLE_FOCUS capability the specified focus can be anything between 1.0 and 0.0.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   focus                               the focus. 1.0 for maximum focus, 0.0 for minimum.
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if laserScannerIndex does not refer to an existing laser/scanner unit or if focus is not in [0.0, 1.0]
     *                                                  if NULL was passed as fileHandle
     * \return      EOS_ERR_UNSUPPORTED                 if the specified laser/scanner unit has neither FOCUS_DEFOCUS nor VARIABLE_FOCUS capability or
     *                                                  if it has only FOCUS_DEFOCUS capability, but the specified focus is neither 1.0 nor 0.0.
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_NO_PART                     if this command was called outside a part
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteSetFocus(EOS_FILE_HANDLE fileHandle, EOS_DOUBLE focus);

    /*!
     * \brief       Enables/disables skywriting.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   skywritingEnabled                   EOS_TRUE to enable skywriting, otherwise EOS_FALSE
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_UNSUPPORTED                 if the specified laser/scanner unit has neither FOCUS_DEFOCUS nor VARIABLE_FOCUS capability or
     *                                                  if it has only FOCUS_DEFOCUS capability, but the specified focus is neither 1.0 nor 0.0.
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteSetSkywriting(EOS_FILE_HANDLE fileHandle, EOS_BOOL skywritingEnabled);

    /*!
     * \brief       Writes custom data to the toolpath stream.
     *              Writing custom data only makes sense if the toolpath is read again before converting it to a regular task.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   customData                          the custom data
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note        The customData written here has nothing to do with the customDataHandle that is used in conjunction with callback functions.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteCustomData(EOS_FILE_HANDLE fileHandle, EOS_CWSTR customData);

    /*!
     * \brief       Writes a single hatch vector for the specified laser/scanner unit with the specified start/end coordinates applying the
     *              current exposure settings such as e.g., laser power and scan speed.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   start                               start coordinate of the exposure vector (X/Y).
     * \param[in]   end                                 end coordinate of the exposure vector (X/Y).
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_BAD_COORDINATE              if x/y coordinates are outside the scanner area.
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_NO_PART                     if this command was called outside a part
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteHatch(EOS_FILE_HANDLE fileHandle, EOS_POINT2 start, EOS_POINT2 end);

    /*!
     * \brief       Jumps to the specified start coordinate of an upcoming exposure polygon or polyline.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   startCoord                          start coordinate of the polygon/polyline (X/Y).
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_BAD_COORDINATE              if one or both x/y coordinates are outside the building area.
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_NO_PART                     if this command was called outside a part
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note        Must always be followed by a sequence of zero or more PolyCoord commands followed by exactly one PolyEnd command:
     *              PolyStart , [PolyCoord]* , PolyEnd
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WritePolyStart(EOS_FILE_HANDLE fileHandle, EOS_POINT2 startCoord);

    /*!
     * \brief       Writes an exposure vector that begins at the coordinate of the preceding PolyStart/PolyCoord command and that ends
     *              at the coordinate specified by intermediateCoord, leaving the associated polygon/polyline open. Current exposure settings
     *              such as e.g., laser power and scan speed will be applied.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   intermediateCoord                   intermediate coordinate of the polygon/polyline (X/Y).
     * \param[in]   trackingErrorCompensation           tracking error is the time interval between the set and actual positions when traversing a vector.
     *                                                  Especially when scanning a polygon with acute angles this may lead to unwanted inaccuracies.
     *                                                  The tracking error compensation is a short delay at the end of the vector that allows the
     *                                                  tracking error to be reduced before continuing with the exposure of the next exposure vector.
     *                                                  During this delay the laser remains on.
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_BAD_COORDINATE              if one or both x/y coordinates are outside the building area.
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_NO_PART                     if this command was called outside a part
     * \return      EOS_ERR_COMMAND_NOT_POSSIBLE        if this command must not be written at the current position of the task for syntactic reasons.
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WritePolyCoord(EOS_FILE_HANDLE fileHandle, EOS_POINT2 intermediateCoord, EOS_MICROSECS trackingErrorCompensation);

    /*!
     * \brief       Writes an exposure vector that begins at the coordinate of the preceeding PolyStart/PolyCoord command and that ends
     *              at the coordinate specified by endCoord. Current exposure settings such as e.g., laser power and scan speed will be applied.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   endCoord                            end coordinate of the exposure vector (X/Y).
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_BAD_COORDINATE              if one or both x/y coordinates are outside the building area.
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_NO_PART                     if this command was called outside a part
     * \return      EOS_ERR_COMMAND_NOT_POSSIBLE        if this command must not be written at the current position of the task for syntactic reasons.
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WritePolyEnd(EOS_FILE_HANDLE fileHandle, EOS_POINT2 endCoord);

    /*!
     * \brief       Writes a delay for the specified laser/scanner unit.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   delay_us                            the delay in microseconds
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteDelay(EOS_FILE_HANDLE fileHandle, EOS_MICROSECS delay_us);

    /*!
     * \brief       Flags the exposure type of upcoming exposure vectors of the specified laser/scanner unit.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   exposureType                        the exposure type
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note        Setting the exposure type is not mandatory, but highly recommended in order to enable process simulation and process monitoring which may require this information.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteSetExposureType(EOS_FILE_HANDLE fileHandle, EOS_EXPOSURE_TYPE exposureType);

    /*!
     * \brief       Writes the step of the exposure. Exposure is usually subdivided into several exposure steps, e.g., as in:
     *                  0. Downskin
     *                  1. Infill
     *                  2. Upskin
     *                  3. Inner contour
     *                  4. Outer contour
     *                  5. Edges
     *              The numbers 0-5 are the step.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   exposureSetIndex                    the exposure set index
     * \param[in]   exposureStepIndex                   the step index
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle or if exposureSetIndex or exposureStepIndex do not correspond to a valid exposure step
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_NO_PART                     if this command was called outside a part
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note        Setting the step is not mandatory, but highly recommended in order to enable process simulation and process monitoring which may require this information.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteSetExposureStep(EOS_FILE_HANDLE fileHandle, EOS_UINT exposureSetIndex, EOS_UINT exposureStepIndex);

    /*!
     * \brief       Specifies the exposure depth of the upcoming exposure vectors. Essentially, this is the layer thickness of the
     *              corresponding exposure set. The information is required for calculating the correct material dosage amount when
     *              recoating.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   exposedDepthInMicrometers           the exposure depth
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle or if exposedDepthInMicrometers is not greater than zero.
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteSetDepth(EOS_FILE_HANDLE fileHandle, EOS_MICROMETER exposedDepthInMicrometers);

    /*!
     * \brief       Indicates start of a new hatch stripe
     * \param[in]   fileHandle                          output file handle
     * \param[in]   stripeBegin                         stripe start position located at the center of the stripe on the first hatch vector
     * \param[in]   hatchDistVector                     vector denoting the direction in which hatches are placed within the stripe as well as the distance between two consecutive exposure vectors
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteStartStripeSegment(EOS_FILE_HANDLE fileHandle, EOS_POINT2 stripeBegin, EOS_POINT2 hatchDistVector);

    /*!
     * \brief       Indicates start of a new hatch segment
     * \param[in]   fileHandle                          output file handle
     * \param[in]   hatchDistVector                     vector denoting the direction and distance in which hatches are placed within the stripe
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteStartGeneralSegment(EOS_FILE_HANDLE fileHandle, EOS_POINT2 hatchDistVector);

    /*!
     * \brief       Indicates the end of the current hatch segment
     * \param[in]   fileHandle                          output file handle
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteEndSegment(EOS_FILE_HANDLE fileHandle);

    /*!
     * \brief       Indicates the end of a hatch line
     * \param[in]   fileHandle                          output file handle
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteEndLine(EOS_FILE_HANDLE fileHandle);

    /*!
     * \brief       Sets new laser pulse parameters for the upcoming exposure vectors.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   pulsedWavePeriodTime_us             The duration of a pulse period
     * \param[in]   pulsedWavePowerOnTime_us            The duration during which the laser is on
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteSetLaserPulse(EOS_FILE_HANDLE fileHandle, EOS_MICROSECS pulsedWavePeriodTime_us, EOS_MICROSECS pulsedWavePowerOnTime_us);

    /*!
     * \brief       Sets beam shape parameters for the upcoming exposure vectors.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   beamShapeType                       The beam shape type
     * \param[in]   profileId                           The profile id
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteSetBeamShape(EOS_FILE_HANDLE fileHandle, EOS_BEAMSHAPE beamShapeType, EOS_INT profileId);

    /*!
     * \brief       Marks the start of a specific volume segment (may overlap other volume segments).
     *              All subsequent exposure commands will be mapped to the specified volume segment
     *              until a matching EosToolpath_WriteVSegmentEnd command is called.
     *
     * \param[in]   fileHandle                          output file handle
     * \param[in]   vSegmentIndex                       0-based index referring to the order in which volume segments where orginially specified in the OpenJob
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_NO_PART                     if this command was called outside a part
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteVSegmentStart(EOS_FILE_HANDLE fileHandle, EOS_UINT vSegmentIndex);

    /*!
     * \brief       Marks the end of a specific volume segment.
     * \param[in]   fileHandle                          output file handle
     * \param[in]   vSegmentIndex                       0-based index referring to the order in which volume segments where orginially specified in the OpenJob
     * \return      EOS_ERR_NO_ERROR                    if successful
     * \return      EOS_ERR_ARGUMENT_INVALID            if NULL was passed as fileHandle
     * \return      EOS_ERR_FILE_HANDLE_INVALID         if the file handle is invalid
     * \return      EOS_ERR_NO_MATCH                    if the file handle is valid, but refers to an input file
     * \return      EOS_ERR_NO_PART                     if this command was called outside a part
     * \return      EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                  EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_WriteVSegmentEnd(EOS_FILE_HANDLE fileHandle, EOS_UINT vSegmentIndex);

    /*!
     * \brief           Registers a custom function that will be called whenever a BeginLayer command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetBeginLayerCallback(EOS_BEGINLAYER_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called after the last exposure command of a layer has been read via EosToolpath_ReadLayer.
     *                  This callback does not match an equivalent exposure command. Its sole purpose is to indicate the total exposure time elapsed before
     *                  beginning the next recoating cycle.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetEndLayerCallback(EOS_ENDLAYER_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a BeginPart command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetBeginPartCallback(EOS_BEGINPART_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a different laser/scanner unit is selected
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetSelectLaserScannerCallback(EOS_SELECTLASERSCANNERUNIT_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever laser power or power delay parameters are set
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetSetLaserPowerCallback(EOS_SETLASERPOWER_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a SetScanSpeed command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetSetScanSpeedCallback(EOS_SETSCANSPEED_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a SetFocus command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetSetFocusCallback(EOS_SETFOCUS_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a SetSkywriting command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetSetSkywritingCallback(EOS_SETSKYWRITING_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a CustomData command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetCustomDataCallback(EOS_CUSTOMDATA_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a Hatch command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetHatchCallback(EOS_HATCH_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a PolyStart command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetPolyStartCallback(EOS_POLYSTART_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a PolyCoord command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetPolyCoordCallback(EOS_POLYCOORD_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a PolyEnd command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetPolyEndCallback(EOS_POLYEND_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a Delay command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetDelayCallback(EOS_DELAY_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a SetExposureType command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetSetExposureTypeCallback(EOS_SETEXPOSURETYPE_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a SetExposureStep command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetSetExposureStepCallback(EOS_SETEXPOSURESTEP_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a SetDepth command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetSetDepthCallback(EOS_SETDEPTH_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a StartStripeSegment command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetStartStripeSegmentCallback(EOS_STARTSTRIPESEGMENT_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a StartGeneralSegment command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetStartGeneralSegmentCallback(EOS_STARTGENERALSEGMENT_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a EndSegment command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetEndSegmentCallback(EOS_ENDSEGMENT_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever a EndLine command is encountered
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetEndLineCallback(EOS_ENDLINE_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever laser pulse parameters are set/unset
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetSetLaserPulseCallback(EOS_SETLASERPULSE_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever beam shape parameters are set/unset
     *                  while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetSetBeamShapeCallback(EOS_SETBEAMSHAPE_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever VSegmentStart is encountered while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetVSegmentStartCallback(EOS_VSEGMENTSTART_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

    /*!
     * \brief           Registers a custom function that will be called whenever VSegmentEnd is encountered while reading a layer via EosToolpath_ReadLayer.
     * \param[in]       callback                            custom function to be called
     * \param[in]       customCallbackDataHandle            handle to custom data or custom pointer which is passed through
     *                                                      as argument to the callback; pass EOS_NO_DATA if no custom data is needed
     * \return          EOS_ERR_NO_ERROR                    if the callback was registered successfully
     * \return          EOS_ERR_NOT_IDLE                    if a read operation is currently in progress and thus the currently
     *                                                      registered callback function cannot be changed at this time.
     * \return          EOS_ERR_CLIENT_NOT_AUTHENTICATED    if the caller has not yet successfully authenticated using
     *                                                      EosToolpath_GetAuthChallenge and EosToolpath_SetAuthResponse
     * \note            Only a single callback function can be registered. Calling the function twice will unregister
     *                  the callback from the first call.
     * \note            Call with NULL as callback to reset.
     * \note            This function is thread safe.
     */
    EOS_TOOLPATH_API EOS_ERROR EosToolpath_SetVSegmentEndCallback(EOS_VSEGMENTEND_CALLBACK callback, EOS_DATA_HANDLE customCallbackDataHandle);

#ifdef __cplusplus
} // end of extern "C"
#endif

#endif
