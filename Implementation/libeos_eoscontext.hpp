/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the 3MF Consortium nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE 3MF CONSORTIUM BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Abstract: This is the class declaration of CEOSContext

*/


#ifndef __LIBEOS_EOSCONTEXT
#define __LIBEOS_EOSCONTEXT

#include "lib3mftoolpath4eos_interfaces.hpp"

// Parent classes
#include "libeos_base.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

// Include custom headers here.
#include "libeos_eossdk.hpp"
#include "libeos_powerramp.hpp"
#include "libeos_eosconversionlog.hpp"
#include "libeos_toolpathlayers.hpp"
#include "libeos_namespaceregistry.hpp"
#include "../Common/common_workingdirectory.hpp"

namespace Lib3MF4EOS {
namespace Impl {


/*************************************************************************************************************************
 Class declaration of CEOSContext 
**************************************************************************************************************************/




class CEOSContext : public virtual IEOSContext, public virtual CBase {
private:

protected:
    PEOSSDK m_pSDK;
    
    std::string m_sDefaultSDKDirectory;
    std::string m_sDefaultLogFileName;

    uint32_t m_nSDKVersionOverrideMajor;
    uint32_t m_nSDKVersionOverrideMinor;
    bool m_bSDKAPIIsInitialized;

    std::string m_sSDKDirectory;
    std::string m_sLoggingConfigFile;
    std::string m_sLoggingLogFile;

    AMCCommon::PWorkingDirectory m_pOpenJZTempPath;

    // The namespaces this context reads and writes. Copied into the futures and loaders it creates.
    CNamespaceRegistry m_NamespaceRegistry;

    void UnloadSDK();
    void LoadSDKIfNotExisting(eSimulationMode simulationMode);
    std::string findSDKDirectory(const std::string& sEOSPrintAPIDLLName);
    std::string getDetectedVersionString();

    static Lib3MF::PWrapper getLib3MFWrapper();

    struct S3MFJobWriteOptions {
        // If set, parts without eos3mf:partparameter get their exposure set by the rules of the settings,
        // and the choice is logged. Otherwise they get the _Default_ parameters.
        const SConversionSettings* m_pSettings = nullptr;
        PConversionLogData m_pLog;
        // Written as the job's height end if not negative.
        double m_dHeightEndInMM = -1.0;
        // If set, receives the job's layer thickness in micrometers, or 0 if it is not known.
        uint32_t* m_pLayerThicknessOut = nullptr;
        // Layer thickness of every part in micrometers, used to choose its exposure set. Parts that are not in it use the job's layer thickness.
        const std::map<uint32_t, uint32_t>* m_pPartLayerThickness = nullptr;
        // If set, receives the exposure set chosen for every part without eos3mf:partparameter.
        std::map<uint32_t, std::string>* m_pChosenExposureSetsOut = nullptr;
        // Gives the machine target if the model has no eos3mf:machinetarget.
        IMachineConfigFuture* m_pMachineConfiguration = nullptr;
    };

    // The material thickness of the eos3mf metadata, else the base layer thickness of the embedded material set; 0 if unknown.
    uint32_t readJobLayerThickness(Lib3MF::PModel pModel);

    // A 3MF toolpath job after its job and machine configuration are loaded.
    struct S3MFToolpathJob {
        double m_dModelUnitInMM = 1.0;
        double m_dToolpathUnitInMM = 1.0;
        std::vector<SEOS3MFPart> m_Parts;
        std::map<std::string, uint32_t> m_BuildItemPartIDs;
        SToolpathLayerScan m_Scan;
        SToolpathZLayout m_Layout;
        uint32_t m_nJobLayerThickness = 0;
        double m_dMachineHeightInMM = 0.0;
        // Laser scanners of the machine; 0 if the machine configuration does not give them.
        uint32_t m_nLaserScannerCount = 0;
        // Capability bits (EOS_LSC_*) of every laser scanner, if the machine configuration gives them.
        bool m_bHasLaserScannerCapabilities = false;
        std::vector<uint64_t> m_LaserScannerCapabilities;
    };

    // Analyses the toolpath layers, loads the job with an exposure set per part and activates the machine configuration.
    void prepare3MFToolpathJob(Lib3MF::PModel pModel, Lib3MF::PToolpath pToolpath, IMachineConfigFuture* pMachineConfiguration, const SConversionSettings& conversionSettings,
        PConversionLogData pLog, S3MFToolpathJob& job);

    // The layer stack of the loaded job, simulated in the full SDK simulation.
    // Returns false, with a warning, if the SDK cannot give it.
    bool retrieveLayerStack(const S3MFToolpathJob& job, SEOSLayerStack& stack, PConversionLogData pLog);

    // Message of the exception being handled, with the LibEOS error code.
    static std::string describeCurrentException();

    // The toolpath that the build selects with tp:toolpathid, or the only toolpath of the model.
    static Lib3MF::PToolpath getBuildToolpath(Lib3MF::PModel pModel);

    // Writes the meshes of a 3MF model as OpenJZ. Job settings come from the eos3mf metadata,
    // missing values are taken from the embedded material set. Returns the extracted material set path.
    std::string write3MFAsOpenJZ(Lib3MF::PModel pModel, const std::string& sOpenJZFileName, AMCCommon::PWorkingDirectory pWorkingDir,
        const S3MFJobWriteOptions& options);
    std::string write3MFAsOpenJZ(Lib3MF::PModel pModel, const std::string& sOpenJZFileName, AMCCommon::PWorkingDirectory pWorkingDir) { return write3MFAsOpenJZ(pModel, sOpenJZFileName, pWorkingDir, S3MFJobWriteOptions()); }

    void loadJobFrom3MF(Lib3MF::PModel pModel, const S3MFJobWriteOptions& options);

    IToolpathComputationFuture* startToolpathComputation(Lib3MF::PModel pInputModel, IMachineConfigFuture* pMachineConfiguration, const std::string& sTaskComment);

    // Copies and validates the settings of an API object.
    static SConversionSettings getConversionSettings(IEOSToolpathConversionSettings* pSettings);

    static PConversionLogData createConversionLog(const SConversionSettings& settings);

    // Adds the SDK version and adapter of the loaded SDK to the log.
    void addSDKInfoToLog(PConversionLogData pLog);

    // What a toolpath conversion has seen so far, kept for the log if it fails.
    struct S3MFToolpathConversionState;

    static void addConversionStateToLog(const S3MFToolpathConversionState& state, PConversionLogData pLog);

    ITaskCreationFuture* convert3MFToolpath(Lib3MF::PModel pModel, Lib3MF::PToolpath pToolpath, IMachineConfigFuture* pMachineConfiguration, const SConversionSettings& conversionSettings,
        PConversionLogData pLog, S3MFToolpathConversionState& state, const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion);


public:

    CEOSContext(const std::string& sSDKDirectory, const std::string& sLogFileName);

    virtual ~CEOSContext();

    IEOSSDKSimulator* EnableSDKSimulation(const Lib3MF4EOS::eEOSSimulationMode eSimulationMode) override;

    std::string LoadSDK() override;

    std::string GetSDKFilePath() override;

    std::string GetDetectedAPIVersion() override;

    void SetSDKVersionOverride(const Lib3MF4EOS_uint32 nMajor, const Lib3MF4EOS_uint32 nMinor) override;

    void GetSDKProductVersion(Lib3MF4EOS_uint32& nMajor, Lib3MF4EOS_uint32& nMinor, Lib3MF4EOS_uint32& nBuild, Lib3MF4EOS_uint32& nRevision) override;

    void GetSDKAdapterVersion(Lib3MF4EOS_uint32& nMajor, Lib3MF4EOS_uint32& nMinor) override;

    std::string GetSupportedSDKVersions() override;

    std::string GetSDKVersionMessages() override;

    IEOSMaterialSet* ImportMaterialSet(const std::string& sFileName) override;

	void LoadOpenJZFromFile(const std::string& sFileName) override;

	void LoadJobFrom3MF(Lib3MF::PModel pModel) override;

	void LoadOpenJob(const std::string& sOpenJobFileName, const std::string& sMaterialSetFileName) override;

	IEOSConversionLog* CheckLayerCoverage(Lib3MF::PModel pModel, IMachineConfigFuture* pMachineConfiguration, IEOSToolpathConversionSettings* pSettings) override;

	IEOSConversionLog* CheckPartToolpathAlignment(Lib3MF::PModel pModel, const Lib3MF4EOS_double dTolerance) override;

	void UnloadJob() override;

    void SetCacheDirectory(const std::string& sDirectory) override;

	bool JobIsLoaded() override;

	IBuildTimeEstimationFuture* EstimateBuildTime() override;

	ITaskCreationFuture* CreateTask(const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion) override;

	ITaskCreationFuture* CreateRawTask(const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion) override;

	IMachineSessionFuture* ConnectToMachine(const std::string& sNetworkAddress) override;

	IMachineConfigFuture* LoadMachinePackage(const Lib3MF4EOS_uint64 nStreamDataBufferSize, const Lib3MF4EOS_uint8* pStreamDataBuffer) override;

	IMachineConfigFuture* LoadMachinePackageFromFile(const std::string& sFileName) override;

    IEOSMachineInfo* ActivateMachineConfigurationFromPath(const std::string& sFilePath) override;

    IEOSMachineInfo* ActivateMachineConfiguration(IMachineConfigFuture* pMachineConfiguration) override;

    IEOSMachineInfo* RetrieveCurrentMachineInformation() override;

    void AuthenticateToolpathAPI(const Lib3MF4EOS_uint32 nClientID, const std::string& sClientKey) override;

    void AuthenticateToolpathAPIFromCredentialStore(const Lib3MF4EOS_uint32 nClientID, const std::string& sCredentialName) override;

    IEOSToolpathConversionSettings* CreateToolpathConversionSettings() override;

    IEOSRawTaskWriter* CreateRawTaskWriter(IEOSToolpathConversionSettings* pSettings) override;

    IEOSRawTaskLayer* CreateEOSRawTaskLayer(const Lib3MF4EOS_int64 nZHeightInMicrometers) override;

    IToolpathComputationFuture* ComputeToolpathFrom3MF(Lib3MF::PModel pInputModel, IMachineConfigFuture* pMachineConfiguration) override;

    IMachineConfigFuture* LoadMachineConfigFromDirectory(const std::string& sPath) override;

    void Convert3MFToOpenJZ(Lib3MF::PModel pModel, const std::string& sOpenJZFileName) override;

    void ConvertOpenJZTo3MF(const std::string& sOpenJZFileName, Lib3MF::PModel pModel) override;

    void AttachMaterialSetTo3MF(Lib3MF::PModel pModel, const std::string& sEOSParFileName) override;

    std::string GetMaterialSetRelationshipType() override;

    std::string GetNamespace(const Lib3MF4EOS::eEOSNamespaceKind eKind) override;

    void RegisterLegacyNamespace(const Lib3MF4EOS::eEOSNamespaceKind eKind, const std::string& sNamespaceURI) override;

    void ClearLegacyNamespaces() override;

    IToolpathComputationFuture* ComputeToolpathFromOpenJZ(const std::string& sOpenJZFileName, IMachineConfigFuture* pMachineConfiguration) override;

    ITaskCreationFuture* CreateTaskFrom3MFToolpath(Lib3MF::PModel pModel, IMachineConfigFuture* pMachineConfiguration, IEOSToolpathConversionSettings* pSettings, const std::string& sName, const std::string& sComment, const std::string& sGeneratorName, const std::string& sGeneratorVersion) override;

};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_EOSCONTEXT
