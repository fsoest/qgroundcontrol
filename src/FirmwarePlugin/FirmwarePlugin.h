/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once
/// @file

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariantList>
#include <QtPositioning/QGeoCoordinate>

#include "QGCMAVLink.h"
#include "FollowMe.h"
#include "FactMetaData.h"

class VehicleComponent;
class AutoPilotPlugin;
class Vehicle;
class MavlinkCameraControl;
class QGCCameraManager;
class Autotune;
class LinkInterface;
class FactGroup;


struct FirmwareFlightMode
{
    QString     mode_name       = "Unknown";
    uint8_t     standard_mode   = UINT8_MAX;
    uint32_t    custom_mode     = UINT32_MAX;
    bool        canBeSet        = false;
    bool        advanced        = false;
    bool        fixedWing       = false;
    bool        multiRotor      = true;

    FirmwareFlightMode(QString mName, uint32_t cMode, bool cbs = false, bool adv = false)
        : mode_name     (mName)
        , standard_mode (0)
        , custom_mode   (cMode)
        , canBeSet      (cbs)
        , advanced      (adv)
        , fixedWing     (false)
        , multiRotor    (true)
    {
    }

    FirmwareFlightMode(QString mName, uint8_t sMode, uint32_t cMode,
                       bool cbs = false, bool adv = false,
                       bool fWing = false, bool mRotor = true
    )
        : mode_name     (mName)
        , standard_mode (sMode)
        , custom_mode   (cMode)
        , canBeSet      (cbs)
        , advanced      (adv)
        , fixedWing     (fWing)
        , multiRotor    (mRotor)
    {
    }
};

typedef QList<FirmwareFlightMode>         FlightModeList;
typedef QMap<uint32_t,QString>            FlightModeCustomModeMap;
/// This is the base class for Firmware specific plugins
///
/// The FirmwarePlugin class represents the methods and objects which are specific to a certain Firmware flight stack.
/// This is the only place where flight stack specific code should reside in QGroundControl. The remainder of the
/// QGroundControl source is generic to a common mavlink implementation. The implementation in the base class supports
/// mavlink generic firmware. Override the base clase virtuals to create your own firmware specific plugin.

class FirmwarePlugin : public QObject
{
    Q_OBJECT

public:
    FirmwarePlugin ();

    /// Set of optional capabilites which firmware may support
    typedef enum {
        SetFlightModeCapability =           1 << 0, ///< FirmwarePlugin::setFlightMode method is supported
        PauseVehicleCapability =            1 << 1, ///< Vehicle supports pausing at current location
        GuidedModeCapability =              1 << 2, ///< Vehicle supports guided mode commands
        OrbitModeCapability =               1 << 3, ///< Vehicle supports orbit mode
        TakeoffVehicleCapability =          1 << 4, ///< Vehicle supports guided takeoff
        ROIModeCapability =                 1 << 5, ///< Vehicle supports ROI (both in Fly guided mode and from Plan creation)
        ChangeHeadingCapability =           1 << 6, ///< Vehicle supports changing heading at current location
    } FirmwareCapabilities;

    /// Maps from on parameter name to another
    ///     key:    parameter name to translate from
    ///     value:  mapped parameter name
    typedef QMap<QString, QString> remapParamNameMap_t;

    /// Maps from firmware minor version to remapParamNameMap_t entry
    ///     key:    firmware minor version
    ///     value:  remapParamNameMap_t entry
    typedef QMap<int, remapParamNameMap_t> remapParamNameMinorVersionRemapMap_t;

    /// Maps from firmware major version number to remapParamNameMinorVersionRemapMap_t entry
    ///     key:    firmware major version
    ///     value:  remapParamNameMinorVersionRemapMap_t entry
    typedef QMap<int, remapParamNameMinorVersionRemapMap_t> remapParamNameMajorVersionMap_t;

    /// @return The AutoPilotPlugin associated with this firmware plugin. Must be overridden.
    virtual AutoPilotPlugin* autopilotPlugin(Vehicle* vehicle);

    /// Called when Vehicle is first created to perform any firmware specific setup.
    virtual void initializeVehicle(Vehicle* vehicle);

    /// @return true: Firmware supports all specified capabilites
    virtual bool isCapable(const Vehicle *vehicle, FirmwareCapabilities capabilities);

    /// Returns ComponentInformation for specified Vehicle
    ///     @param vehicle Vehicle  to associate with components
    /// @return List of ComponentInformation for the specified vehicle. Caller owns returned objects and must
    ///         free when no longer needed.
    virtual QList<VehicleComponent*> componentsForVehicle(AutoPilotPlugin* vehicle);

    /// Returns the list of available flight modes for the Fly View dropdown. This may or may not be the full
    /// list available from the firmware. Call will be made again if advanced mode changes.
    virtual QStringList flightModes(Vehicle* /*vehicle*/) { return QStringList(); }

    /// Returns the name for this flight mode. Flight mode names must be human readable as well as audio speakable.
    ///     @param base_mode Base mode from mavlink HEARTBEAT message
    ///     @param custom_mode Custom mode from mavlink HEARTBEAT message
    virtual QString flightMode(uint8_t base_mode, uint32_t custom_mode) const;

    /// Sets base_mode and custom_mode to specified flight mode.
    ///     @param[out] base_mode Base mode for SET_MODE mavlink message
    ///     @param[out] custom_mode Custom mode for SET_MODE mavlink message
    virtual bool setFlightMode(const QString& flightMode, uint8_t* base_mode, uint32_t* custom_mode);

    /// returns true if this flight stack supports MAV_CMD_DO_SET_MODE
    virtual bool MAV_CMD_DO_SET_MODE_is_supported() const { return false; }

    /// Returns The flight mode which indicates the vehicle is paused
    virtual QString pauseFlightMode(void) const { return QString(); }

    /// Returns the flight mode for running missions
    virtual QString missionFlightMode(void) const { return QString(); }

    /// Returns the flight mode for RTL
    virtual QString rtlFlightMode(void) const { return QString(); }

    /// Returns the flight mode for Smart RTL
    virtual QString smartRTLFlightMode(void) const { return QString(); }

    virtual bool supportsSmartRTL(void) const { return false; }

    /// Returns the flight mode for Land
    virtual QString landFlightMode(void) const { return QString(); }

    /// Returns the flight mode for TakeOff
    virtual QString takeOffFlightMode(void) const { return QString(); }

    /// Returns the flight mode for Motor Detection
    virtual QString motorDetectionFlightMode(void) const { return QString(); }

    /// Returns the flight mode for Stabilized
    virtual QString stabilizedFlightMode(void) const { return QString(); }

    /// Returns the flight mode to use when the operator wants to take back control from autonomouse flight.
    virtual QString takeControlFlightMode(void) const { return QString(); }

    /// Returns whether the vehicle is in guided mode or not.
    virtual bool isGuidedMode(const Vehicle* vehicle) const;

    /// Returns the flight mode which the vehicle will be in if it is performing a goto location
    virtual QString gotoFlightMode(void) const;

    /// Returns the flight mode which the vehicle will be for follow me
    virtual QString followFlightMode(void) const { return QString(); }

    /// Set guided flight mode
    virtual void setGuidedMode(Vehicle* vehicle, bool guidedMode);

    /// Causes the vehicle to stop at current position. If guide mode is supported, vehicle will be let in guide mode.
    /// If not, vehicle will be left in Loiter.
    virtual void pauseVehicle(Vehicle* vehicle);

    /// Command vehicle to return to launch
    virtual void guidedModeRTL(Vehicle* vehicle, bool smartRTL);

    /// Command vehicle to land at current location
    virtual void guidedModeLand(Vehicle* vehicle);

    /// Command vehicle to takeoff from current location to a firmware specific height.
    virtual void guidedModeTakeoff(Vehicle* vehicle, double takeoffAltRel);

    /// Command vehicle to rotate towards specified location.
    virtual void guidedModeChangeHeading(Vehicle *vehicle, const QGeoCoordinate &headingCoord);

    /// @return The minimum takeoff altitude (relative) for guided takeoff.
    virtual double minimumTakeoffAltitudeMeters(Vehicle* /*vehicle*/) { return 3.048; }

    /// @return The maximum horizontal groundspeed for a multirotor.
    virtual double maximumHorizontalSpeedMultirotor(Vehicle* /*vehicle*/) { return NAN; }

    /// @return The maximum equivalent airspeed setpoint.
    virtual double maximumEquivalentAirspeed(Vehicle* /*vehicle*/) { return NAN; }

    /// @return The minimum equivalent airspeed setpoint
    virtual double minimumEquivalentAirspeed(Vehicle* /*vehicle*/) { return NAN; }

    /// @return Return true if the GCS has enabled Grip_enable option
    virtual bool hasGripper(const Vehicle* /*vehicle*/) const { return false; } 
    
    /// @return Return true if we have received the ground speed limits for the mulirotor.
    virtual bool mulirotorSpeedLimitsAvailable(Vehicle* /*vehicle*/) { return false; }

    /// @return Return true if we have received the airspeed limits for fixed wing.
    virtual bool fixedWingAirSpeedLimitsAvailable(Vehicle* /*vehicle*/) { return false; }

    /// Command the vehicle to start the mission
    virtual void startMission(Vehicle* vehicle);

    /// Command vehicle to move to specified location (altitude is included and relative)
    virtual void guidedModeGotoLocation(Vehicle* vehicle, const QGeoCoordinate& gotoCoord);

    /// Command vehicle to change altitude
    ///     @param altitudeChange If > 0, go up by amount specified, if < 0, go down by amount specified
    ///     @param pauseVehicle true: pause vehicle prior to altitude change
    virtual void guidedModeChangeAltitude(Vehicle* vehicle, double altitudeChange, bool pauseVehicle);

        /// Command vehicle to change groundspeed
    ///     @param groundspeed Groundspeed in m/s
    virtual void guidedModeChangeGroundSpeedMetersSecond(Vehicle* vehicle, double groundspeed);

    /// Command vehicle to change equivalent airspeed
    ///     @param airspeed_equiv Equivalent airspeed in m/s
    virtual void guidedModeChangeEquivalentAirspeedMetersSecond(Vehicle* vehicle, double airspeed_equiv);

    /// Default tx mode to apply to joystick axes
    /// TX modes are as outlined here: http://www.rc-airplane-world.com/rc-transmitter-modes.html
    virtual int defaultJoystickTXMode(void);

    /// Returns true if the vehicle and firmware supports the use of a throttle joystick that
    /// is zero when centered. Typically not supported on vehicles that have bidirectional
    /// throttle.
    virtual bool supportsThrottleModeCenterZero(void);

    /// Returns true if the vehicle and firmware supports the use of negative thrust
    /// Typically supported rover.
    virtual bool supportsNegativeThrust(Vehicle *);

    /// Returns true if the firmware supports the use of the RC radio and requires the RC radio
    /// setup page. Returns true by default.
    virtual bool supportsRadio(void);

    /// Returns true if the firmware supports the AP_JSButton library, which allows joystick buttons
    /// to be assigned via parameters in firmware. Default is false.
    virtual bool supportsJSButton(void);

    /// Returns true if the firmware supports calibrating motor interference offsets for the compass
    /// (CompassMot). Default is true.
    virtual bool supportsMotorInterference(void);

    /// Called before any mavlink message is processed by Vehicle such that the firmwre plugin
    /// can adjust any message characteristics. This is handy to adjust or differences in mavlink
    /// spec implementations such that the base code can remain mavlink generic.
    ///     @param vehicle Vehicle message came from
    ///     @param message[in,out] Mavlink message to adjust if needed.
    /// @return false: skip message, true: process message
    virtual bool adjustIncomingMavlinkMessage(Vehicle* vehicle, mavlink_message_t* message);

    /// Called before any mavlink message is sent to the Vehicle so plugin can adjust any message characteristics.
    /// This is handy to adjust or differences in mavlink spec implementations such that the base code can remain
    /// mavlink generic.
    ///
    /// This method must be thread safe.
    ///
    ///     @param vehicle Vehicle message came from
    ///     @param outgoingLink Link that messae is going out on
    ///     @param message[in,out] Mavlink message to adjust if needed.
    virtual void adjustOutgoingMavlinkMessageThreadSafe(Vehicle* vehicle, LinkInterface* outgoingLink, mavlink_message_t* message);

    /// Determines how to handle the first item of the mission item list. Internally to QGC the first item
    /// is always the home position.
    /// @return
    ///     true: Send first mission item as home position to vehicle. When vehicle has no mission items on
    ///             it, it may or may not return a home position back in position 0.
    ///     false: Do not send first item to vehicle, sequence numbers must be adjusted
    virtual bool sendHomePositionToVehicle(void);

    /// Returns the parameter set version info pulled from inside the meta data file. -1 if not found.
    /// Note: The implementation for this must not vary by vehicle type.
    /// Important: Only CompInfoParam code should use this method
    virtual void _getParameterMetaDataVersionInfo(const QString& metaDataFile, int& majorVersion, int& minorVersion);

    /// Returns the internal resource parameter meta date file.
    /// Important: Only CompInfoParam code should use this method
    virtual QString _internalParameterMetaDataFile(const Vehicle* /*vehicle*/) const { return QString(); }

    /// Loads the specified parameter meta data file.
    /// @return Opaque parameter meta data information which must be stored with Vehicle. Vehicle is responsible to
    ///         call deleteParameterMetaData when no longer needed.
    /// Important: Only CompInfoParam code should use this method
    virtual QObject* _loadParameterMetaData(const QString& /*metaDataFile*/) { return nullptr; }

    /// Returns the FactMetaData associated with the parameter name
    ///     @param opaqueParameterMetaData Opaque pointer returned from loadParameterMetaData
    /// Important: Only CompInfoParam code should use this method
    virtual FactMetaData* _getMetaDataForFact(QObject* /*parameterMetaData*/, const QString& /*name*/, FactMetaData::ValueType_t /* type */, MAV_TYPE /*vehicleType*/) { return nullptr; }

    /// List of supported mission commands. Empty list for all commands supported.
    virtual QList<MAV_CMD> supportedMissionCommands(QGCMAVLink::VehicleClass_t vehicleClass) const;

    /// Returns the name of the mission command json override file for the specified vehicle type.
    ///     @param vehicleClass Vehicle class to return file for, VehicleClassGeneric is a request for overrides for all vehicle types
    virtual QString missionCommandOverrides(QGCMAVLink::VehicleClass_t vehicleClass) const;

    /// Returns the mapping structure which is used to map from one parameter name to another based on firmware version.
    virtual const remapParamNameMajorVersionMap_t& paramNameRemapMajorVersionMap(void) const;

    /// Returns the highest major version number that is known to the remap for this specified major version.
    virtual int remapParamNameHigestMinorVersionNumber(int majorVersionNumber) const;

    /// @return true: Motors are coaxial like an X8 config, false: Quadcopter for example
    virtual bool multiRotorCoaxialMotors(Vehicle* /*vehicle*/) { return false; }

    /// @return true: X confiuration, false: Plus configuration
    virtual bool multiRotorXConfig(Vehicle* /*vehicle*/) { return false; }

    /// Return the resource file which contains the set of params loaded for offline editing.
    virtual QString offlineEditingParamFile(Vehicle* /*vehicle*/) { return QString(); }

    /// Return the resource file which contains the brand image for the vehicle for Indoor theme.
    virtual QString brandImageIndoor(const Vehicle* /*vehicle*/) const { return QString(); }

    /// Return the resource file which contains the brand image for the vehicle for Outdoor theme.
    virtual QString brandImageOutdoor(const Vehicle* /*vehicle*/) const { return QString(); }

    /// Return the resource file which contains the vehicle icon used in the flight view when the view is dark (Satellite for instance)
    virtual QString vehicleImageOpaque(const Vehicle* vehicle) const;

    /// Return the resource file which contains the vehicle icon used in the flight view when the view is light (Map for instance)
    virtual QString vehicleImageOutline(const Vehicle* vehicle) const;

    // This is the content item for the expanded portion of the main status indicator
    virtual QVariant mainStatusIndicatorContentItem(const Vehicle* vehicle) const;

    /// Returns the list of toolbar tool indicators associated with a vehicle
    ///     signals toolIndicatorsChanged
    /// @return A list of QUrl with the indicators
    virtual const QVariantList& toolIndicators(const Vehicle* vehicle);

    /// Returns the list of toolbar mode indicators associated with a vehicle
    ///     signals modeIndicatorsChanged
    /// @return A list of QUrl with the indicators
    virtual const QVariantList& modeIndicators(const Vehicle* vehicle);

    /// Creates vehicle camera manager.
    virtual QGCCameraManager* createCameraManager(Vehicle *vehicle);

    /// Camera control.
    virtual MavlinkCameraControl* createCameraControl(const mavlink_camera_information_t* info, Vehicle* vehicle, int compID, QObject* parent = nullptr);

    /// Returns a pointer to a dictionary of firmware-specific FactGroups
    virtual QMap<QString, FactGroup*>* factGroups(void);

    /// Returns the data needed to do battery consumption calculations
    ///     @param[out] mAhBattery Battery milliamp-hours rating (0 for no battery data available)
    ///     @param[out] hoverAmps Current draw in amps during hover
    ///     @param[out] cruiseAmps Current draw in amps during cruise
    virtual void batteryConsumptionData(Vehicle* vehicle, int& mAhBattery, double& hoverAmps, double& cruiseAmps) const;

    // Returns the parameter which control auto-disarm. Assume == 0 means no auto disarm
    virtual QString autoDisarmParameter(Vehicle* vehicle);

    /// Used to determine whether a vehicle has a gimbal.
    ///     @param[out] rollSupported Gimbal supports roll
    ///     @param[out] pitchSupported Gimbal supports pitch
    ///     @param[out] yawSupported Gimbal supports yaw
    /// @return true: vehicle has gimbal, false: gimbal support unknown
    virtual bool hasGimbal(Vehicle* vehicle, bool& rollSupported, bool& pitchSupported, bool& yawSupported);

    /// Convert from HIGH_LATENCY2.custom_mode value to correct 32 bit value.
    virtual uint32_t highLatencyCustomModeTo32Bits(uint16_t hlCustomMode);

    /// Used to check if running firmware is latest stable version.
    virtual void checkIfIsLatestStable(Vehicle* vehicle);

    /// Used to check if running current version is equal or higher than the one being compared.
    /// returns 1 if current > compare, 0 if current == compare, -1 if current < compare
    int versionCompare(const Vehicle* vehicle, QString& compare) const;
    int versionCompare(const Vehicle* vehicle, int major, int minor, int patch) const;

    /// Allows the Firmware plugin to override the facts meta data.
    ///     @param vehicleType - Type of current vehicle
    ///     @param metaData - MetaData for fact
    virtual void adjustMetaData(MAV_TYPE /*vehicleType*/, FactMetaData* /*metaData*/) {}

    /// Sends the appropriate mavlink message for follow me support
    virtual void sendGCSMotionReport(Vehicle *vehicle, FollowMe::GCSMotionReport &motionReport, uint8_t estimationCapabilities);

    // gets hobbs meter from autopilot. This should be reimplmeented for each firmware
    virtual QString getHobbsMeter(Vehicle* vehicle) { Q_UNUSED(vehicle); return "Not Supported"; }

    /// Creates Autotune object.
    virtual Autotune* createAutotune(Vehicle *vehicle);

    /// Update Available flight modes recieved from vehicle
    virtual void updateAvailableFlightModes(FlightModeList modeList);

signals:
    void toolIndicatorsChanged(void);
    void modeIndicatorsChanged(void);

protected:
    // Arms the vehicle with validation and retries
    // @return: true - vehicle armed, false - vehicle failed to arm
    bool _armVehicleAndValidate(Vehicle* vehicle);

    // Sets the vehicle to the specified flight mode with validation and retries
    // @return: true - vehicle in specified flight mode, false - flight mode change failed
    bool _setFlightModeAndValidate(Vehicle* vehicle, const QString& flightMode);

    // returns url with latest firmware release information.
    virtual QString _getLatestVersionFileUrl(Vehicle* /*vehicle*/) { return QString(); }

    // Callback to process file with latest release information
    virtual void _versionFileDownloadFinished(QString& remoteFile, QString& localFile, Vehicle* vehicle);

    // Returns regex QString to extract version information from text
    virtual QString _versionRegex() { return QString(); }

    // Set Custom Mode mapping to Flight Mode String
    void _setModeEnumToModeStringMapping(FlightModeCustomModeMap enumToString);

    // Convert Base enum to Derived class Enums
    virtual uint32_t _convertToCustomFlightModeEnum(uint32_t val) const { return val; }

    // Update internal mappings for a list of flight modes
    void _updateFlightModeList(FlightModeList &flightModeList);
    void _addNewFlightMode  (FirmwareFlightMode &flightMode);

    FlightModeList          _flightModeList;
    FlightModeCustomModeMap _modeEnumToString;

protected:
    QVariantList _toolIndicatorList;
    QVariantList _modeIndicatorList;
};

class FirmwarePluginInstanceData : public QObject
{
    Q_OBJECT

public:

    using QObject::QObject;

    // support for detecting whether the firmware a vehicle is running
    // supports a given MAV_CMD:
    enum class CommandSupportedResult : uint8_t {
        SUPPORTED = 23,
        UNSUPPORTED = 24,
        UNKNOWN = 25,
    };
    // anyVersionSupportsCommand returns true if any version of the
    // firmware has supported cmd.  Used so that extra round-trips to
    // the autopilot to probe for command support are not required.
    virtual CommandSupportedResult anyVersionSupportsCommand(MAV_CMD cmd) const {
        return CommandSupportedResult::UNKNOWN;
    }

    // support for detecting whether the firmware a vehicle is running
    // supports a given MAV_CMD:
    void setCommandSupported(MAV_CMD cmd, CommandSupportedResult status) {
        MAV_CMD_supported[cmd] = status;
    }
    CommandSupportedResult getCommandSupported(MAV_CMD cmd) const {
        if (anyVersionSupportsCommand(cmd) == CommandSupportedResult::UNSUPPORTED) {
            return CommandSupportedResult::UNSUPPORTED;
        }
        return MAV_CMD_supported.value(cmd, CommandSupportedResult::UNKNOWN);
    }

private:
    QMap<MAV_CMD, CommandSupportedResult> MAV_CMD_supported;
};
