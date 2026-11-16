/*!
 * \file        ToolpathTypes.h
 * \brief       Type definitions of the EOSPRINT Toolpath API
 *
 * \author      EOS GmbH, Robert-Stirling-Ring 1, 82152 Krailling, Germany
 * \copyright   (c) EOS GmbH 2022
 *              All rights reserved
 */

#ifndef EOS_TOOLPATH_API_TYPES_H
#define EOS_TOOLPATH_API_TYPES_H

#include "Types.h"

#ifdef __cplusplus
extern "C"
{
#endif
    /*! Specifies in which order exposure commands should be read from a toolpath file */
    typedef enum
    {
        /*! Reading order where all exposure commands of one laser are read before the any exposure command of another laser are read. */
        EOS_TRO_BY_LASER_THEN_BY_TIME_INDEX = 0,

        /*! Reading order where all exposure commands are read in chronological order of their execution during the building process */
        EOS_TRO_BY_TIME_INDEX = 1

    } EOS_TOOLPATH_READING_ORDER;

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetBeginLayerCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   z_um                                        z position of the layer in micrometers.
     */
    typedef void(*EOS_BEGINLAYER_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROMETER z_um);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetEndLayerCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   timestamp_us                                the duration of the ending layer in microseconds.
     * \note        This callback does not match an equivalent exposure command. Its sole purpose is to indicate the timestamp at the end of a layer immediately before the next recoating cycle begins.
     */
    typedef void(*EOS_ENDLAYER_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetBeginPartCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   partId                                      the part id.
     */
    typedef void(*EOS_BEGINPART_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_INT partId);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetSelectLaserScannerCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   laserScannerIndex                           the index of the laser/scanner unit
     */
    typedef void(*EOS_SELECTLASERSCANNERUNIT_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_BYTE laserScannerIndex);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetSetLaserPowerCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   power_W                                     laser power in Watts
     * \param[in]   delay_us                                    delay after which the laser power will be applied.
     */
    typedef void(*EOS_SETLASERPOWER_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_DOUBLE power_W, EOS_MICROSECS delay_us);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetSetScanSpeedCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   speed_mm_per_s                              the speed in millimeters per seconds
     */
    typedef void(*EOS_SETSCANSPEED_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_DOUBLE speed_mm_per_s);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetSetFocusCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   focus                                       the focus setting
     */
    typedef void(*EOS_SETFOCUS_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_DOUBLE focus);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetSetSkywritingCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   skywritingEnabled                           whether or not to enable skywriting
     */
    typedef void(*EOS_SETSKYWRITING_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_INT skywritingEnabled);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetCustomDataCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customData                                  custom data
     */
    typedef void(*EOS_CUSTOMDATA_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_CWSTR customData);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetHatchCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   timestamp_us                                the timestamp of the hatch command
     * \param[in]   start                                       start coordinate of the hatch
     * \param[in]   end                                         end coordinate of the hatch
     */
    typedef void(*EOS_HATCH_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_POINT2 start, EOS_POINT2 end);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetPolyStartCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   timestamp_us                                the timestamp of the polygon start command
     * \param[in]   startCoord                                  start coordinate of the polygon/polyline
     */
    typedef void(*EOS_POLYSTART_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_POINT2 startCoord);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetPolyCoordCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   timestamp_us                                the timestamp of the polygon coord command
     * \param[in]   intermediateCoord                           intermediate coordinate of the polygon/polyline
     */
    typedef void(*EOS_POLYCOORD_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_POINT2 intermediateCoord);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetPolyEndCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   endCoord                                    end coordinate of the polygon/polyline
     */
    typedef void(*EOS_POLYEND_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_POINT2 endCoord);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetDelayCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   timestamp_us                                the timestamp of the delay command
     * \param[in]   delay_us                                    delay in microseconds
     */
    typedef void(*EOS_DELAY_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS timestamp_us, EOS_MICROSECS delay_us);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetSetExposureTypeCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   exposureType                                exposure type
     */
    typedef void(*EOS_SETEXPOSURETYPE_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_EXPOSURE_TYPE exposureType);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetSetExposureStepCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   exposureSetIndex                            exposure set index
     * \param[in]   exposureStepIndex                           exposure step index
     */
    typedef void(*EOS_SETEXPOSURESTEP_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_UINT exposureSetIndex, EOS_UINT exposureStepIndex);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetSetDepthCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   exposedDepthInMicrometers                   exposed depth in Micrometers
     */
    typedef void(*EOS_SETDEPTH_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROMETER exposedDepthInMicrometers);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetStartStripeSegmentCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   stripeBegin                                 position of the stripe begin
     * \param[in]   hatchDistVector                             hatch distance vector (length equals distance of hatch vectors within stripe)
     */
    typedef void(*EOS_STARTSTRIPESEGMENT_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_POINT2 stripeBegin, EOS_POINT2 hatchDistVector);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetStartGeneralSegmentCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   hatchDistVector                             hatch distance vector (length equals distance of hatch vectors within stripe)
     */
    typedef void(*EOS_STARTGENERALSEGMENT_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_POINT2 hatchDistVector);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetEndSegmentCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     */
    typedef void(*EOS_ENDSEGMENT_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetEndLineCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     */
    typedef void(*EOS_ENDLINE_CALLBACK) (EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetSetLaserPulseCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   pulsedWavePeriodTime_us                     the laser pulse period time in microseconds
     * \param[in]   pulsedWavePowerOnTime_us                    the laser pulse power on time in microseconds
     */
    typedef void (*EOS_SETLASERPULSE_CALLBACK)(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_MICROSECS pulsedWavePeriodTime_us, EOS_MICROSECS pulsedWavePowerOnTime_us);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetSetBeamShapeCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   beamShapeType                               the beam shape type
     * \param[in]   profileId                                   the profile id
     */
    typedef void (*EOS_SETBEAMSHAPE_CALLBACK)(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_BEAMSHAPE beamShapeType, EOS_INT profileId);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetVSegmentStartCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   vSegmentIndex                               0-based index referring to the order in which volume segments where originally specified in the OpenJob
     */
    typedef void (*EOS_VSEGMENTSTART_CALLBACK)(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_UINT vSegmentIndex);

    /*!
     * \brief       Callback function type to be used in EosToolpath_SetVSegmentEndCallback
     * \param[in]   customCallbackDataHandle                    the custom data handle that was set when registering the callback function or EOS_NO_DATA if no custom data handle was set
     * \param[in]   customReadDataHandle                        the custom data handle that was set when invoking EosToolpath_ReadLayer or EOS_NO_DATA if no custom data handle was set
     * \param[in]   vSegmentIndex                               0-based index referring to the order in which volume segments where originally specified in the OpenJob
     */
    typedef void (*EOS_VSEGMENTEND_CALLBACK)(EOS_DATA_HANDLE customCallbackDataHandle, EOS_DATA_HANDLE customReadDataHandle, EOS_UINT vSegmentIndex);

#ifdef __cplusplus
} // end of extern "C"
#endif

#endif // EOS_TOOLPATH_API_TYPES_H
