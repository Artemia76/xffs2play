#include "sim/useraircraft.h"

namespace xffs2play
{
	CUserAircraft::CUserAircraft()
	{
		xpbTitle = XPLMFindDataRef("sim/aircraft/view/acf_descrip");
		xpbModel = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
		xpbType = XPLMFindDataRef("sim/aircraft/view/acf_author");
		xpdAltitude = XPLMFindDataRef("sim/flightmodel/position/elevation");
		xpfGroundAltitude = XPLMFindDataRef("sim/flightmodel/position/y_agl");
		xpfHeading = XPLMFindDataRef("sim/flightmodel/position/psi");
		xpdLongitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
		xpdLatitude= XPLMFindDataRef("sim/flightmodel/position/latitude");
		xpfIASSpeed = XPLMFindDataRef("sim/flightmodel/position/indicated_airspeed2");
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
	}

	CUserAircraft::~CUserAircraft()
	{
		//dtor
	}

	void CUserAircraft::Update()
	{
		TimeStamp = Now();
		char title[261] ;
		XPLMGetDatab(xpbTitle, title,0,260);
		Title=title;
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
		Pitch = -XPLMGetDataf(xpfPitch);
		Bank = -XPLMGetDataf(xpfBank);
		OnGround = (bool)XPLMGetDatai(xpiOnGround);
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
		Squawk = XPLMGetDataf(xpSquawk);
	}
}
