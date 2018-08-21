#include "sim/useraircraft.h"
#include "tools/simple_file_parser.h"

namespace xffs2play
{
    ///
    /// Default Constructor for User Aircraft
    /// This class give us data access on the player Aircraft
    ///
    CUserAircraft::CUserAircraft()
    {
        m_Log = CLogger::Instance();
        GroundSpeed = 0;
        TASSpeed = 0;
        m_XPDescription = XPLMFindDataRef("sim/aircraft/view/acf_descrip");
        m_XPModel = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
        m_XPType = XPLMFindDataRef("sim/aircraft/view/acf_author");
        m_XPAltitude = XPLMFindDataRef("sim/flightmodel/position/elevation");
        m_XPGroundAltitude = XPLMFindDataRef("sim/flightmodel/position/y_agl");
        m_XPHeading = XPLMFindDataRef("sim/flightmodel/position/psi");
        m_XPLongitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
        m_XPLatitude= XPLMFindDataRef("sim/flightmodel/position/latitude");
        m_XPIASSpeed = XPLMFindDataRef("sim/flightmodel/position/indicated_airspeed2");
        m_XPGNDSpeed = XPLMFindDataRef("sim/flightmodel/position/groundspeed");
        m_XPTASSpeed = XPLMFindDataRef("sim/flightmodel/position/true_airspeed");
        m_XPPitch = XPLMFindDataRef("sim/flightmodel/position/true_theta");
        m_XPBank = XPLMFindDataRef("sim/flightmodel/position/true_phi");
        m_XPOnGround = XPLMFindDataRef("sim/flightmodel/failures/onground_any");
        m_XPSmoke = XPLMFindDataRef("sim/flightmodel/failures/smoking");
        m_XPThrottleEng = XPLMFindDataRef("sim/flightmodel/engine/ENGN_thro");
        m_XPStateEng = XPLMFindDataRef("sim/flightmodel/engine/ENGN_running");
        m_XPLandingLight = XPLMFindDataRef("sim/cockpit/electrical/landing_lights_on");
        m_XPBeaconLight = XPLMFindDataRef("sim/cockpit/electrical/beacon_lights_on");
        m_XPStrobeLight = XPLMFindDataRef("sim/cockpit/electrical/strobe_lights_on");
        m_XPNavLight = XPLMFindDataRef("sim/cockpit/electrical/nav_lights_on");
        m_XPRecoLight = XPLMFindDataRef("sim/cockpit/electrical/taxi_light_on");
        m_XPGearPos = XPLMFindDataRef("sim/aircraft/parts/acf_gear_deploy");
        //xpFlapsIndex = sim/flightmodel/controls/flaprqst
        m_XPElevatorPos = XPLMFindDataRef("sim/cockpit2/controls/yoke_pitch_ratio");
        m_XPAileronPos = XPLMFindDataRef("sim/cockpit2/controls/yoke_roll_ratio");
        m_XPRudderPos = XPLMFindDataRef("sim/cockpit2/controls/yoke_heading_ratio");
        m_XPSquawk = XPLMFindDataRef("sim/cockpit/radios/transponder_code");
        m_XPParkingBrake = XPLMFindDataRef("sim/cockpit2/controls/parking_brake_ratio");
        m_XPVariometer = XPLMFindDataRef("sim/cockpit2/gauges/indicators/vvi_fpm_pilot");
    }

    ///
    /// Desfault Destructor
    ///
    CUserAircraft::~CUserAircraft()
    {
    }

    ///
    /// Add Specific player data for protobuf datagram
    ///
    void CUserAircraft::ToAirData(AirData& pData) const
    {
        CAircraftState::ToAirData(pData);
        pData.set_tasspeed(TASSpeed);
        pData.set_gndspeed(GroundSpeed);
    }

    ///
    /// Get Specific data from protobuf for player aircraft
    ///
    void CUserAircraft::FromAirData(const AirData& pData)
    {
        CAircraftState::FromAirData(pData);
        TASSpeed = pData.tasspeed();
        GroundSpeed = pData.gndspeed();
    }

    ///
    /// Read Player Aicraft data from X-Plane
    ///
    void CUserAircraft::Update()
    {
        TimeStamp = Now();
        char description[261] ;
        std::string Description;
        XPLMGetDatab(m_XPDescription, description,0,260);
        Description=description;
        // If Descirption change
        if (m_Description.compare(Description) != 0)
        {
            ReadDataFromACF();
            m_Description = Description;
        }

        char model[41];
        XPLMGetDatab(m_XPModel, model,0,40);
        Model=model;
        char type[501];
        XPLMGetDatab(m_XPType, type,0,500);
        Type=type;
        Altitude = XPLMGetDatad(m_XPAltitude) * M_2_FEET;//Convert to feet
        GroundAltitude = Altitude - (XPLMGetDataf(m_XPGroundAltitude) * M_2_FEET);
        Heading = XPLMGetDataf(m_XPHeading);
        Longitude = XPLMGetDatad(m_XPLongitude);
        Latitude = XPLMGetDatad(m_XPLatitude);
        IASSpeed = XPLMGetDataf(m_XPIASSpeed);
        TASSpeed = XPLMGetDataf(m_XPTASSpeed)*1.94384;
        GroundSpeed = XPLMGetDataf(m_XPGNDSpeed)*1.94384;
        Pitch = -XPLMGetDataf(m_XPPitch);
        Bank = -XPLMGetDataf(m_XPBank);
        OnGround = (bool)XPLMGetDatai(m_XPOnGround);
        float ParkingBrakeRatio = XPLMGetDataf(m_XPParkingBrake);
        ParkingBrakePos = ParkingBrakeRatio;
        if (ParkingBrakeRatio > 0.5) ParkingBrake =true;
        else ParkingBrake = false;
        Smoke =(bool)XPLMGetDatai(m_XPSmoke);
        float _throttle[4];
        XPLMGetDatavf(m_XPThrottleEng, _throttle, 0, 4);
        ThrottleEng1=(uint32_t)(_throttle[0]*100);
        ThrottleEng2 = (uint32_t)(_throttle[1] * 100);
        ThrottleEng3 = (uint32_t)(_throttle[2] * 100);
        ThrottleEng4 = (uint32_t)(_throttle[3] * 100);
        int _stateEngine[4];
        XPLMGetDatavi(m_XPStateEng, _stateEngine, 0, 4);
        StateEng1 = (bool)_stateEngine[0];
        StateEng2 = (bool)_stateEngine[1];
        StateEng3 = (bool)_stateEngine[2];
        StateEng4 = (bool)_stateEngine[3];
        LandingLight = (bool)XPLMGetDatai(m_XPLandingLight);
        BeaconLight = (bool)XPLMGetDatai(m_XPBeaconLight);
        StrobeLight = (bool)XPLMGetDatai(m_XPStrobeLight);
        NavLight = (bool)XPLMGetDatai(m_XPNavLight);
        RecoLight = (bool)XPLMGetDatai(m_XPRecoLight);
        float plane_gear_deploy[5];
        XPLMGetDatavf(m_XPGearPos, plane_gear_deploy, 0, 5);
        GearPos = (bool)plane_gear_deploy[0];
        ElevatorPos = XPLMGetDataf(m_XPElevatorPos);
        AileronPos = XPLMGetDataf(m_XPAileronPos);
        RudderPos = XPLMGetDataf(m_XPRudderPos);
        Squawk = XPLMGetDatai(m_XPSquawk);
        Vario = XPLMGetDataf(m_XPVariometer);
    }

    ///
    /// read Missing data directly on the model Aircraft file (ACF) used
    /// by the player
    ///
    void CUserAircraft::ReadDataFromACF()
    {
        int PlaneCount=0;
        char FileName[256], AircraftPath[256];
        XPLMCountAircraft(&PlaneCount, 0, 0);
        if (PlaneCount > 1)
        {
            XPLMGetNthAircraftModel(0, FileName, AircraftPath);
#ifdef XFFS_DEBUG
            m_Log->Log(_("AircraftPath = ") + std::string(AircraftPath), LEVEL_DEBUG);
#endif
            simple_file_parser parser;
            parser.set_single_char_tokens(" ");
            if (parser.open(AircraftPath))
            {
#ifdef XFFS_DEBUG
                m_Log->Log(_("Loading ACF OK"), LEVEL_DEBUG);
#endif
                while (!parser.eof())
                {
                    if (parser.get_next_line())
                    {
                        if (parser.get_num_tokens()>=3)
                        {
                            if (parser.get_token(1)=="acf/_name")
                            {
                                Title = parser.get_token(2);
                                for (int i=3; i<parser.get_num_tokens();i++)
                                {
                                    Title += _(" ") + parser.get_token(i);
                                }
#ifdef XFFS_DEBUG
                                m_Log->Log(_("Title = ") + Title, LEVEL_DEBUG);
#endif
                            }
                            if (parser.get_token(1)=="acf/_is_helicopter")
                            {
                                if (parser.get_token_uint(2) >0)
                                {
                                    Category = "Helicopter";
                                }
                                else
                                {
                                    Category = "Airplane";
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
