//////////////////////////////////////////////////////////////
// EyeTracker.cpp
// Simple class that takes care of the connection with the 
// eye tracker (SR Research EyeLink1000)
// 
//   2011, Alex Reichenbach 
//   a.reichenbach@ucl.ac.uk
//
//   2014, Peter Zatka-Haas: Added pupil size measurement
//	 p.zatka@ucl.ac.uk
//////////////////////////////////////////////////////////////

#include "EyeTracker.h"

////////////////////////////////////////////////////////////////////////////
/// The constructor opens the connection to the EyeLink PC and sets offline mode,
/// i.e. the eye data is tracked on the EyeLink server and can be fetched from there.
/// param: connection=0 -> real connection; connection=1 -> simulation mode for debugging 
///  
////////////////////////////////////////////////////////////////////////////
EyeTracker::EyeTracker(int connection) { 
	trackerip = "100.1.1.1"; 
	eye_used = LEFT_EYE;
	//trackerip = NULL; // set correct IP address
	isValid = true;
	isTracking = false;
	
	if(trackerip)
		set_eyelink_address(trackerip);
	// open_eyelink_connection(0) => real connection
	// open_eyelink_connection(1) => simulation mode for debugging purpose
	if(open_eyelink_connection(connection)) {
	  isValid = false;	// invalid EyeTracker object if we can't get the connection
	}
	set_offline_mode();
	timeStamp = -99;
	xPos = MISSING_DATA;
	yPos = MISSING_DATA;

}

////////////////////////////////////////////////////////////////////////////
/// The destructor closes the connection to the EyeLink PC. 
///  
////////////////////////////////////////////////////////////////////////////
EyeTracker::~EyeTracker() { 
	close_eyelink_connection();

}

////////////////////////////////////////////////////////////////////////////
/// Checks whether there is a valid and open connection. 
/// \return true if connection is ok.
///  
////////////////////////////////////////////////////////////////////////////
bool EyeTracker::connectionOk() {
	return isValid;
}

////////////////////////////////////////////////////////////////////////////
/// Starts the tracking on the EyeLink server.
/// Needs to be called before data can be fetched.
///  
////////////////////////////////////////////////////////////////////////////
void EyeTracker::startTracking() {
	if (eyelink_is_connected()) {
		int error = start_recording(0,0,1,1); // params!!!!
		if (error!=0) {
			isValid = false;
		} else {
			isTracking = true;
		}
	} else {
		isValid = false;
	}
}

////////////////////////////////////////////////////////////////////////////
/// Stops the tracking on the EyeLink server.
///  
////////////////////////////////////////////////////////////////////////////
void EyeTracker::stopTracking() {
	stop_recording();
	isTracking=false;
}

////////////////////////////////////////////////////////////////////////////
/// Checks whether the EyeLink server is currently tracking. 
/// \return true if server is tracking.
///  
////////////////////////////////////////////////////////////////////////////
bool EyeTracker::currTracking() {
	return isTracking;
}

////////////////////////////////////////////////////////////////////////////
/// Gets the latest data set from the EyeLink server, and extracts rax x & y position of the eye and the timestamp. 
/// Server needs to be in tracking mode.
/// \return true if valid data is obtained.
///  
////////////////////////////////////////////////////////////////////////////
bool EyeTracker::getData() {
	 ALLF_DATA event;

	 if (eyelink_is_connected()) {
		if (isValid && isTracking && eyelink_newest_float_sample(NULL)>0) {
			eyelink_newest_float_sample(&event);
			xPos      = event.fs.gx[eye_used];
			yPos      = event.fs.gy[eye_used];
			xPosH     = event.fs.hx[eye_used];
			yPosH     = event.fs.hy[eye_used];
			xPosP     = event.fs.px[eye_used];
			yPosP     = event.fs.py[eye_used];
			PPDx      = event.fs.rx;       /*!< screen pixels per degree */
			PPDy      = event.fs.ry;       /*!< screen pixels per degree */
			PupilDiam = event.fs.pa[eye_used];
            eyestatus = event.fs.status;
			xStart    = event.fe.gstx;
			xEnd      = event.fe.genx;
			//if (x!=MISSING_DATA && y!=MISSING_DATA && event.fs.pa[eye_used]) {
				timeStamp = event.fs.time;
			//} else {

			//}
				//if (xPos!=MISSING_DATA && yPos!=MISSING_DATA) {
				//	cout << xPos << "\t" << yPos << "\t" << timeStamp << endl;}
			return true;
		} else {
			return false;
		}
	 } else {
		isValid = false;
		return false;
	 }
}

////////////////////////////////////////////////////////////////////////////
/// Gets the timestamp [ms] of the last data set obtained with getData.
/// \return timestamp of the data set (integer value)
///  
////////////////////////////////////////////////////////////////////////////
int EyeTracker::getTimestamp() {
	return timeStamp;
}

////////////////////////////////////////////////////////////////////////////
/// Gets the raw eye x position of the last data set obtained with getData.
/// \return x position (float value)
///  
////////////////////////////////////////////////////////////////////////////
float EyeTracker::getXPos() {
	return xPos;
}

////////////////////////////////////////////////////////////////////////////
/// Gets the raw eye y position of the last data set obtained with getData.
/// \return y position (float value)
///  
////////////////////////////////////////////////////////////////////////////
float EyeTracker::getYPos() {
	return yPos;
}

////////////////////////////////////////////////////////////////////////////
/// Gets the raw eye x position of the last data set obtained with getData.
/// \return x position (float value)
///  
////////////////////////////////////////////////////////////////////////////
float EyeTracker::getXPosH() {
	return xPosH;
}

////////////////////////////////////////////////////////////////////////////
/// Gets the raw eye y position of the last data set obtained with getData.
/// \return y position (float value)
///  
////////////////////////////////////////////////////////////////////////////
float EyeTracker::getYPosH() {
	return yPosH;
}

////////////////////////////////////////////////////////////////////////////
/// Gets the raw eye x position of the last data set obtained with getData.
/// \return x position (float value)
///  
////////////////////////////////////////////////////////////////////////////
float EyeTracker::getXPosP() {
	return xPosP;
}

////////////////////////////////////////////////////////////////////////////
/// Gets the raw eye y position of the last data set obtained with getData.
/// return y position (float value)
///  
////////////////////////////////////////////////////////////////////////////
float EyeTracker::getYPosP() {
	return yPosP;
}

////////////////////////////////////////////////////////////////////////////
/// return pupil size/area in arbitrary units  
////////////////////////////////////////////////////////////////////////////
float EyeTracker::getPupilDiam() {
	return PupilDiam;
}

////////////////////////////////////////////////////////////////////////////
/// returns pixels per degree in the x direction 
///  
////////////////////////////////////////////////////////////////////////////
float EyeTracker::getPixPerDegx() {
	return PPDx;
}

////////////////////////////////////////////////////////////////////////////
/// returns pixels per degree in the y direction 
///  
////////////////////////////////////////////////////////////////////////////
float EyeTracker::getPixPerDegy() {
	return PPDy;
}

////////////////////////////////////////////////////////////////////////////
/// returns eye status 
///  
////////////////////////////////////////////////////////////////////////////
float EyeTracker::getEyeStatus() {
	return eyestatus;
}

////////////////////////////////////////////////////////////////////////////
/// returns x start 
///  
////////////////////////////////////////////////////////////////////////////

float EyeTracker::getxStart(){
	return xStart;
}

////////////////////////////////////////////////////////////////////////////
/// returns x End 
///  
////////////////////////////////////////////////////////////////////////////

float EyeTracker::getxEnd(){
	return xEnd;
}


////////////////////////////////////////////////////////////////////////////
/// returns x start 
///  
////////////////////////////////////////////////////////////////////////////

float EyeTracker::getEventTime(){
	return eventTime;
}

////////////////////////////////////////////////////////////////////////////
/// returns x End 
///  
////////////////////////////////////////////////////////////////////////////

float EyeTracker::getEventType(){
	return eventType;
}