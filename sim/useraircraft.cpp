#include "sim/useraircraft.h"
#include "tools/simple_file_parser.h"

namespace xffs2play
{
	CUserAircraft::CUserAircraft()
	{
        Log = CLogger::instance();
		GroundSpeed = 0;
		TASSpeed = 0;
		xpbDescription = XPLMFindDataRef("sim/aircraft/view/acf_descrip");
		xpbModel = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
		xpbType = XPLMFindDataRef("sim/aircraft/view/acf_author");
		xpdAltitude = XPLMFindDataRef("sim/flightmodel/position/elevation");
		xpfGroundAltitude = XPLMFindDataRef("sim/flightmodel/position/y_agl");
		xpfHeading = XPLMFindDataRef("sim/flightmodel/position/psi");
		xpdLongitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
		xpdLatitude= XPLMFindDataRef("sim/flightmodel/position/latitude");
		xpfIASSpeed = XPLMFindDataRef("sim/flightmodel/position/indicated_airspeed2");
		xpfGNDSpeed = XPLMFindDataRef("sim/flightmodel/position/groundspeed");
		xpfTASSpeed = XPLMFindDataRef("sim/flightmodel/position/true_airspeed");
		xpfPitch = XPLMFindDataRef("sim/flightmodel/position/true_theta");
		xpfBank = XPLMFindDataRef("sim/flightmodel/position/true_phi");
		xpiOnGround = XPLMFindDataRef("sim/flightmodel/failures/onground_any");
		xpiSmoke = XPLMFindDataRef("sim/flightmodel/failures/smoking");
		xpThrottleEng = XPLMFindDataRef("sim/flightmodel/engine/ENGN_thro");
		xpStateEng = XPLMFindDataRef("sim/flightmodel/engine/ENGN_running");
		xpLandingLight = XPLMFindDataRef("sim/cockpit/electrical/landing_lights_on");
		xpBeaconLight = XPLMFindDataRef("sim/cockpit/electrical/beacon_lights_on");
		xpStrobeLight = XPLMFindDataRef("sim/cockpit/electrical/strobe_lights_on");
		xpNavLight = XPLMFindDataRef("sim/cockpit/electrical/nav_lights_on");
		xpRecoLight = XPLMFindDataRef("sim/cockpit/electrical/taxi_light_on");
		xpGearPos = XPLMFindDataRef("sim/aircraft/parts/acf_gear_deploy");
		//xpFlapsIndex = sim/flightmodel/controls/flaprqst
		xpElevatorPos = XPLMFindDataRef("sim/cockpit2/controls/yoke_pitch_ratio");
		xpAileronPos = XPLMFindDataRef("sim/cockpit2/controls/yoke_roll_ratio");
		xpRudderPos = XPLMFindDataRef("sim/cockpit2/controls/yoke_heading_ratio");
		xpSquawk = XPLMFindDataRef("sim/cockpit/radios/transponder_code");
		xpfParkingBrake = XPLMFindDataRef("sim/cockpit2/controls/parking_brake_ratio");
		xpfVariometer = XPLMFindDataRef("sim/cockpit2/gauges/indicators/vvi_fpm_pilot");
	}

	CUserAircraft::~CUserAircraft()
	{
		//dtor
	}

	void CUserAircraft::toAirData(AirData& data) const
	{
		CAircraftState::toAirData(data);
		data.set_tasspeed(TASSpeed);
		data.set_gndspeed(GroundSpeed);
	}

	void CUserAircraft::fromAirData(const AirData& data)
	{
		CAircraftState::fromAirData(data);
		TASSpeed = data.tasspeed();
		GroundSpeed = data.gndspeed();
	}

	void CUserAircraft::Update()
	{
		TimeStamp = Now();
		char description[261] ;
		std::string Description;
		XPLMGetDatab(xpbDescription, description,0,260);
		Description=description;
		// If Descirption change
		if (m_description.compare(Description) != 0)
		{
            readDataFromACF();
            m_description = Description;
		}

		char model[41];
		XPLMGetDatab(xpbModel, model,0,40);
		Model=model;
		char type[501];
		XPLMGetDatab(xpbType, type,0,500);
		Type=type;
		Altitude = XPLMGetDatad(xpdAltitude) * M_2_FEET;//Convert to feet
		GroundAltitude = Altitude - (XPLMGetDataf(xpfGroundAltitude) * M_2_FEET);
		Heading = XPLMGetDataf(xpfHeading);
		Longitude = XPLMGetDatad(xpdLongitude);
		Latitude = XPLMGetDatad(xpdLatitude);
		IASSpeed = XPLMGetDataf(xpfIASSpeed);
		TASSpeed = XPLMGetDataf(xpfTASSpeed)*1.94384;
		GroundSpeed = XPLMGetDataf(xpfGNDSpeed)*1.94384;
		Pitch = -XPLMGetDataf(xpfPitch);
		Bank = -XPLMGetDataf(xpfBank);
		OnGround = (bool)XPLMGetDatai(xpiOnGround);
		float ParkingBrakeRatio = XPLMGetDataf(xpfParkingBrake);
		ParkingBrakePos = ParkingBrakeRatio;
		if (ParkingBrakeRatio > 0.5) ParkingBrake =true;
		else ParkingBrake = false;
		Smoke =(bool)XPLMGetDatai(xpiSmoke);
		float _throttle[4];
		XPLMGetDatavf(xpThrottleEng, _throttle, 0, 4);
		ThrottleEng1=(uint32_t)(_throttle[0]*100);
		ThrottleEng2 = (uint32_t)(_throttle[1] * 100);
		ThrottleEng3 = (uint32_t)(_throttle[2] * 100);
		ThrottleEng4 = (uint32_t)(_throttle[3] * 100);
		int _stateEngine[4];
		XPLMGetDatavi(xpStateEng, _stateEngine, 0, 4);
		StateEng1 = (bool)_stateEngine[0];
		StateEng2 = (bool)_stateEngine[1];
		StateEng3 = (bool)_stateEngine[2];
		StateEng4 = (bool)_stateEngine[3];
		LandingLight = (bool)XPLMGetDatai(xpLandingLight);
		BeaconLight = (bool)XPLMGetDatai(xpBeaconLight);
		StrobeLight = (bool)XPLMGetDatai(xpStrobeLight);
		NavLight = (bool)XPLMGetDatai(xpNavLight);
		RecoLight = (bool)XPLMGetDatai(xpRecoLight);
		float plane_gear_deploy[5];
		XPLMGetDatavf(xpGearPos, plane_gear_deploy, 0, 5);
		GearPos = (bool)plane_gear_deploy[0];
		ElevatorPos = XPLMGetDataf(xpElevatorPos);
		AileronPos = XPLMGetDataf(xpAileronPos);
		RudderPos = XPLMGetDataf(xpRudderPos);
		Squawk = XPLMGetDatai(xpSquawk);
		Vario = XPLMGetDataf(xpfVariometer);
	}

	void CUserAircraft::readDataFromACF()
	{
        int PlaneCount=0;
        char FileName[256], AircraftPath[256];
        XPLMCountAircraft(&PlaneCount, 0, 0);
        if (PlaneCount > 1)
		{
            XPLMGetNthAircraftModel(0, FileName, AircraftPath);
#ifdef XFFS_DEBUG
			Log->log(_("AircraftPath = ") + std::string(AircraftPath), LEVEL_DEBUG);
#endif
            simple_file_parser parser;
            parser.set_single_char_tokens(" ");
            if (parser.open(AircraftPath))
            {
#ifdef XFFS_DEBUG
                Log->log(_("Loading ACF OK"), LEVEL_DEBUG);
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
                                Log->log(_("Title = ") + Title, LEVEL_DEBUG);
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
