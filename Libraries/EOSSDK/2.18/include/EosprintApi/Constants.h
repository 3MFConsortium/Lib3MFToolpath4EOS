/*!
 * \file        Constants.h
 * \brief       Constants for the EOSPRINT 2 API
 *
 * \author      EOS GmbH, Robert-Stirling-Ring 1, 82152 Krailling, Germany
 * \copyright   (c) EOS GmbH 2018
 *              All rights reserved
 */

#ifndef EOS_EOSPRINT_API_CONSTANTS_H
#define EOS_EOSPRINT_API_CONSTANTS_H

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

    /*! \brief All hatching vectors such as infill, upskin, downskin, edge. */
    static EOS_VECTORTYPES const EOS_VT_HATCHING = 1;
    /*! \brief Part boundary vectors */
    static EOS_VECTORTYPES const EOS_VT_PART_BOUNDARY = 2;
    /*! \brief Jump vectors */
    static EOS_VECTORTYPES const EOS_VT_JUMPS = 4;

    /*! Licensable feature: Build on machine */
    static EOS_LICENSABLE_FEATURES const EOS_LF_BUILD = 1;
    /*! Licensable feature: Edit */
    static EOS_LICENSABLE_FEATURES const EOS_LF_EDIT = 2;

    /*! Eternity */
    static EOS_MILLISECS const EOS_INFINITE_DURATION = 0;

    /*! An infinte number of times */
    static EOS_UINT const EOS_INFINITE_TIMES = 0;

    /*! Value representing not-a-machine-session */
    static EOS_SESSION_HANDLE const EOS_NO_SESSION = 0;
    /*! Value representing not-a-filetransfer */
    static EOS_FILETRANSFER_HANDLE const EOS_NO_FILETRANSFER = 0;
    /*! Value representing not-a-filecreation */
    static EOS_FILECREATION_HANDLE const EOS_NO_FILECREATION = 0;
    /*! Value representing not-a-data */
    static EOS_DATA_HANDLE const EOS_NO_DATA = 0;
    /*! Value representing not-a-file */
    static EOS_FILE_HANDLE const EOS_NO_FILE = 0;

    /*! Default laser scanner infos for EOS M 290 */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_M290[1] = {
        {
            /* laserPowerNominal_W */
            390.0,
            /* laserPowerMax_W */
            390.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 250.0, 250.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS M 290 */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_M290 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        250.0, 250.0, 325.0,  0.0,
        /* model type */
        EOS_MT_M290,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        1,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_M290,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! Default laser scanner infos for EOS M 290-2 */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_M293[2] = {
        {
            /* laserPowerNominal_W */
            390.0,
            /* laserPowerMax_W */
            390.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 250.0, 250.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            390.0,
            /* laserPowerMax_W */
            390.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 250.0, 250.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS M 290-2 */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_M293 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        250.0, 250.0, 325.0,  0.0,
        /* model type */
        EOS_MT_M293,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        2,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_M293,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! Default laser scanner infos for EOS M 290 1kW */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_M294[1] = {
        {
            /* laserPowerNominal_W */
            999.0,
            /* laserPowerMax_W */
            999.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 250.0, 250.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS M 290 1kW */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_M294 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        250.0, 250.0, 325.0,  0.0,
        /* model type */
        EOS_MT_M294,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        1,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_M293,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! Default laser scanner infos for EOS M 300-4 */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_M304[4] = {
        {
            /* laserPowerNominal_W */
            390.0,
            /* laserPowerMax_W */
            390.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 300.0, 300.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            390.0,
            /* laserPowerMax_W */
            390.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 300.0, 300.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            390.0,
            /* laserPowerMax_W */
            390.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 300.0, 300.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            390.0,
            /* laserPowerMax_W */
            390.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 300.0, 300.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS M 300-4 */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_M304 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        300.0, 300.0, 400.0,  0.0,
        /* model type */
        EOS_MT_M304,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        4,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_M304,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! Default laser scanner infos for EOS M 300-4 1kW */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_M314[4] = {
        {
            /* laserPowerNominal_W */
            999.0,
            /* laserPowerMax_W */
            999.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 300.0, 300.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            999.0,
            /* laserPowerMax_W */
            999.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 300.0, 300.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            999.0,
            /* laserPowerMax_W */
            999.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 300.0, 300.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            999.0,
            /* laserPowerMax_W */
            999.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 300.0, 300.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS M 300-4 1kW */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_M314 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        300.0, 300.0, 400.0,  0.0,
        /* model type */
        EOS_MT_M314,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        4,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_M314,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! Default laser scanner infos for EOS M 400 */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_M400[1] = {
        {
            /* laserPowerNominal_W */
            999.0,
            /* laserPowerMax_W */
            999.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 400.0, 400.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS M 400 */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_M400 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        400.0, 400.0, 400.0, 0.0,
        /* model type */
        EOS_MT_M400,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        2,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_M400,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! Default laser scanner infos for EOS M 400-4 */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_M404[4] = {
        {
            /* laserPowerNominal_W */
            390.0,
            /* laserPowerMax_W */
            390.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 400.0, 400.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            390.0,
            /* laserPowerMax_W */
            390.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 400.0, 400.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            390.0,
            /* laserPowerMax_W */
            390.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 400.0, 400.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            390.0,
            /* laserPowerMax_W */
            390.0,
            /* maxScanSpeed_mm_per_s */
            7000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 400.0, 400.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS M 400-4 */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_M404 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        400.0, 400.0, 400.0, 0.0,
        /* model type */
        EOS_MT_M404,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        2,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_M404,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! Default laser scanner infos for EOS P3 NEXT */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_P397[1] = {
        {
            /* laserPowerNominal_W */
            85.0,
            /* laserPowerMax_W */
            85.0,
            /* maxScanSpeed_mm_per_s */
            12000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 350, 350.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS P3 NEXT */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_P397 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        350.0, 350.0, 620.0,  0.0,
        /* model type */
        EOS_MT_P397,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        1,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_P397,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! Default laser scanner infos for EOS P 500 DR01 */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_P500[2] = {
        {
            /* laserPowerNominal_W */
            65.0,
            /* laserPowerMax_W */
            65.0,
            /* maxScanSpeed_mm_per_s */
            10000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 500.0, 330.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            65.0,
            /* laserPowerMax_W */
            65.0,
            /* maxScanSpeed_mm_per_s */
            10000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 500.0, 330.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS P500 DR01 */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_P500 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        500.0, 330.0, 400.0,  0.0,
        /* model type */
        EOS_MT_P500,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        2,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_P500,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! Default laser scanner infos for EOS P 500 DR02 */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_P501[2] = {
        {
            /* laserPowerNominal_W */
            65.0,
            /* laserPowerMax_W */
            65.0,
            /* maxScanSpeed_mm_per_s */
            10000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 500.0, 330.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            65.0,
            /* laserPowerMax_W */
            65.0,
            /* maxScanSpeed_mm_per_s */
            10000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 500.0, 330.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS P500 DR02 */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_P501 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        500.0, 330.0, 400.0,  0.0,
        /* model type */
        EOS_MT_P501,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        2,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_P501,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! Default laser scanner infos for EOS P 500 FDR DR01 */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_P520[2] = {
        {
            /* laserPowerNominal_W */
            65.0,
            /* laserPowerMax_W */
            65.0,
            /* maxScanSpeed_mm_per_s */
            10000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 500.0, 330.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            65.0,
            /* laserPowerMax_W */
            65.0,
            /* maxScanSpeed_mm_per_s */
            10000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 500.0, 330.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS P500 FDR DR01 */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_P520 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        500.0, 330.0, 400.0,  0.0,
        /* model type */
        EOS_MT_P520,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        2,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_P520,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! Default laser scanner infos for EOS P 500 FDR DR02 */
    static EOS_LASER_SCANNER_INFO const EOS_DEFAULT_LASER_SCANNER_INFO_P521[2] = {
        {
            /* laserPowerNominal_W */
            65.0,
            /* laserPowerMax_W */
            65.0,
            /* maxScanSpeed_mm_per_s */
            10000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 500.0, 330.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        },
        {
            /* laserPowerNominal_W */
            65.0,
            /* laserPowerMax_W */
            65.0,
            /* maxScanSpeed_mm_per_s */
            10000.0,
            /* minPoint2D */
            { 0.0, 0.0 },
            /* maxPoint2D */
            { 500.0, 330.0 },
            /* capabilities */
            0,
            /* reserved */
            { 0 }
        }
    };

    /*! Default machine information for the EOS P500 FDR DR02 */
    static EOS_MACHINE_INFO const EOS_DEFAULT_MACHINE_P521 = {
        /* volume type */
        EOS_VOL_CUBOID,
        /* dimX dimY  dimZ   dimR */
        500.0, 330.0, 400.0,  0.0,
        /* model type */
        EOS_MT_P521,
        /* software version (unknown for default) */
        { -1, -1, -1, -1 },
        /* interface version (unknown for default) */
        { -1, -1, -1, -1 },
        /* serial number (empty for default) */
        L"",
        /* number of laser scanner units */
        2,
        /* print domain (none for default) */
        NULL,
        /* laser scanner infos */
        EOS_DEFAULT_LASER_SCANNER_INFO_P521,
        /* EOSTATE Exposure OT version (unknown for default) */
        { -1, -1, -1, -1 },
        /* Smart Fusion License Infos */
        0uLL,
        /* Smart Fusion Compatibility */
        EOS_SFC_EXPOSURE_OT_UNKNOWN,
        /* reserved */
        { 0 }
    };

    /*! The adjustable would affect the exposure stored in the task */
    static EOS_AFFECTED_UNITS const EOS_AU_EXPOSURE = 1;

    /*! The adjustable would affect the process environment on the machine */
    static EOS_AFFECTED_UNITS const EOS_AU_PROCESS_ENVIRONMENT = 2;

    /*! Indeterminate part selector */
    static EOS_PARTID const EOS_PID_ALL_PARTS = 0;

    /*! Indeterminate part selector
     * ALL_PARTS was deprecated with EOSPRINT 2.18
     * and was superseded with EOS_PID_ALL_PARTS */
    static EOS_PARTID const ALL_PARTS = 0;

    /*! Indeterminate laser/scanner selector */
    static EOS_BYTE const EOS_LSI_ALL_LASERS  = 0xFF;

    /*! Indeterminate laser/scanner selector
     * ALL_LASERS was deprecated with EOSPRINT 2.18
     * and was superseded with EOS_LSI_ALL_LASERS */
    static EOS_BYTE const ALL_LASERS = 0xFF;

    /*! Machine supports binary defocussing of the laser beam */
    static EOS_LS_CAPABILITIES const EOS_LSC_FOCUS_DEFOCUS  = 0x1;

    /*! Machine supports (de-)focus
     * FOCUS_DEFOCUS was deprecated with EOSPRINT 2.18
     * and was superseded with EOS_LSC_FOCUS_DEFOCUS */
    static EOS_LS_CAPABILITIES const FOCUS_DEFOCUS = 0x1;

    /*! Machine supports variable defocusing of the laser beam */
    static EOS_LS_CAPABILITIES const EOS_LSC_VARIABLE_FOCUS = 0x2;

    /*! Machine supports variable focus
     * VARIABLE_FOCUS was deprecated with EOSPRINT 2.18
     * and was superseded with EOS_LSC_VARIABLE_FOCUS */
    static EOS_LS_CAPABILITIES const VARIABLE_FOCUS = 0x2;

    /*! Constant to denote that no laser is being referenced */
    static EOS_BYTE const EOS_LSI_NO_LASER = 0xFE;

    /*! This bit encodes if the license information is known for the machine */
    static EOS_MACHINE_LICENSE_INFOS const EOS_MLI_IS_KNOWN = 0x0001u;

    /*! This bit encodes if there is a license present.
     *  The license can still be invalid, also check EOS_MLI_LICENSE_VALID
     *  This bit will only be set if the EOS_MLI_IS_KNOWN bit is set */
    static EOS_MACHINE_LICENSE_INFOS const EOS_MLI_LICENSE_PRESENT = 0x0002u;

    /*! This bit encodes if there is a valid license present.
     *  For more detailed information the bits EOS_MLI_LICENSE_EXPIRED
     *  and EOS_MLI_LICENSE_INSUFFICIENT can be queried.
     *  This bit will only be set if the EOS_MLI_LICENSE_PRESENT bit is set */
    static EOS_MACHINE_LICENSE_INFOS const EOS_MLI_LICENSE_VALID = 0x0004u;

    /*! This bit encodes if a present license is also available.
     *  A license is available if there is sufficient quantity.
     *  This bit will only be set if the EOS_MLI_LICENSE_VALID bit is set */
    static EOS_MACHINE_LICENSE_INFOS const EOS_MLI_LICENSE_AVAILABLE = 0x0010u;

    /*! This bit encodes if the license has an expiration date.
     *  This bit will only be set if the EOS_MLI_LICENSE_PRESENT bit is set */
    static EOS_MACHINE_LICENSE_INFOS const EOS_MLI_LICENSE_CAN_EXPIRE = 0x0020u;

    /*! This bit encodes if the license is not yet expired.
     *  This bit will only be set if the EOS_MLI_LICENSE_VALID bit is set
     *  and if the EOS_MLI_LICENSE_CAN_EXPIRE bit is set */
    static EOS_MACHINE_LICENSE_INFOS const EOS_MLI_LICENSE_NOT_EXPIRED = 0x0040u;

    /*! This constant contains the highest available compatibility value. */
    static EOS_SMART_FUSION_COMPATIBILITY const EOS_MAX_SMART_FUSION_COMPATIBILITY = EOS_SFC_EXPOSURE_OT_1_15;

#ifdef __cplusplus
} // end of extern "C"
#endif

#endif // EOS_EOSPRINT_API_CONSTANTS_H
