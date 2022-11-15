/////////////////////////////////////////////////////////////////////////////
// ManipulandumRed_API3D.h 
//
// written for the 3DoF variant of the 3DoM pantograph robot under a s626 io board 
//
// Derived from Manipulandum.h originally by Joern Diedrichsen, 2009
// j.diedrichsen@ucl.ac.uk
// 
// Modified by Nick Roach and Julius Klein, 2011
// n.roach@imperial.ac.uk, j.klein@imperial.ac.uk
//
/////////////////////////////////////////////////////////////////////////////
// revisions
// incom2 - 
//			addition of FF friction compensation for shoulder motors
//			aditional writs tuining functionality
//
//	
// incom3 - 
//			addition of FF friction compensation to wrist PID
//			fixed double "updateoutput" bug
//			addition of gain term to dynamics compensation


#ifndef MANIPULANDUM_H_
#define MANIPULANDUM_H_

#define TUNINGMODE  //makes loop tuning vars public for adjustment

#include "Vector2d.h"
#include "Vector3d.h"
#include "Vector7d.h"
#include "Matrix2d.h" 
#include "Matrix7x2d.h" 
#include "KalmanFilter.h"
#include "ATI_DAQ.h"  
#include <string> 

#define WRISTTORQUE 1 //flag for selection of wrist torque control
#define WRISTANGLE 2 //flag for selection of wrist angle control

#define RECORD_LIMIT 300000 //max size of trajectory file
#define OMEGA_BUFF 5000	//buffer for omega graph

using namespace std;

/////////////////////////////////////////////////////////////////////////////
/// \brief Class for control of the threedom robot (located in QS33, lab1) under a s626 io board 
///
/// Function init is called with a parameter file that contains the data from 
/// the Calibration. Then you only have to call update() in regular intervals 
/// in the updateHaptics() loop, and you have access to estimates of position and velocity. 
/// Manipulandum uses a KalmanFilter for smoothing. 
/// \author Nick Roach, Joern Diedrichsen, Julius Klein 2011
/// 
/////////////////////////////////////////////////////////////////////////////

class Manipulandum {
public:
	Manipulandum(void);						  ///< Constructor 
	~Manipulandum();						  ///< Destructor 
	void init(string paramfile);			  ///< initialize the manipulandum with parameter file 

	void update(double dt);					  ///< Updates the sensory state of the robot for time dt
	void recenter();						  ///< Put (0,0) of world at current position
	void recenter(const Vector2D &);	      ///< Put (0,0) of global coordinates at the specified position 

	void getCounter(long &sh,long &el,long &wr);	///< Get the raw counter values 
	void getPosition(Vector2D &thePos);				///< Get Positions 
	Vector2D getPosition() {return position;}		///< Get Position 
	void getVelocity(Vector2D &theVel);				///< Get Velocities
	Vector2D getVelocity() {return velocity;}		///< Get Velocity
	void getAngles(Vector3D &theAngles);			///< Get angles of the Shoulders and wrist
	Vector3D getAngles(){return theta;};			///< Get the Shoulders and wrist angle in radians

	void setForce(Vector2D theForce);				///< Sets endpoint forces in Euclidean cooridinates #
	void setTorque(Vector3D theTorque);				///< Sets motor torques 
	void setVolts(double v0,double v1,double v2);	      ///< sets voltages to shoulder and elbow directly 
	
	void getLocalFTForce(Vector3D &theFTForce);   // Getforce measured by ft in sensor frame
	Vector3D getLocalFTForce();   // Getforce measured by ft in sensor frame
	void getGlobalFTForce(Vector3D &theFTForce);;  // Get force measured by ft in global frame
	Vector3D getGlobalFTForce();  // Get force measured by ft in global frame
	void getGlobalFTTorque(Vector3D &theFTForce);;  // Get torque measured by ft in global frame 
	Vector3D getGlobalFTTorque();  // Get torque measured by ft in global frame

	//Wrist control
	void enableWristControl();		///> Activates PID contol of wrist motor to align ft sensor (Will cause brief drop in force output when called for safety)
 	void disableWristControl();		///> Deactivates PID contol of wrist motor to align ft sensor (Will cause brief drop in force output when called for safety)
	
	void selectWristTorqueControl();		///> Sets wrist motor control to torque mode
	void selectWristTorqueControlUnsafe(); ///> Selects wrist mode control with NO DEAD PERIOD, could produce unpredictable results!
 	void selectWristAngleControl();		///> Sets wrist motor to position contol mode

	void enableDynamicsComp();			///> enable compensation of dynamics (Will cause brief drop in force output when called for safety)
	void disableDynamicsComp();			///> disable compensation of dynamics(Will cause brief drop in force output when called for safety			
	void setDynamicsCompGain(double gain);			///> sets level of dynamics compensation (0 = no compensation, 1 = full compensation)			

	void enableOutput();			///> Master enable for Robot force output (Will cause brief drop in force output when called for safety)
	void disableOutput();			///> Master disable for Robot force output (Will cause brief drop in force output when called for safety)
	void updateOutput();			///>  Sends calculated force and torque commands to DAQ / motors  

	void setWristAngle(double theAngle); ///> Sets desired wrist angle in Radians. Only fuctional when wrist is in constant angle mode
	void setWristTorque(double theTorque); ///> Sets desired wrist torque in Nm. Only fuctional when wrist is in constant torque mode
	void setWristControlGain(double gain); ///> Sets final gain of wrist controller. Limited to range 0>Gain>1

	int getWristMode();	///>checks current mode of wrist contoller

	void printState(int col=0);	 ///< prints the state of a robot on the text display 

	void resetWDT();		///< pulses wdt reset line 

private: 

	void checkVoltLimits(void);				///< sets forces in Euclidean cooridinates 
	void checkPosLimits(void);				///< check position is in operating range
	Vector2D ati2World(Vector2D atiforce);    ///< computes the force in world coordinate based on ATI force readings 
	
	//Forward kinematics and Jacobian
	Vector2D forwardKinePos(Vector3D shoulderAngle); ///>calculate robot local end effector position given elbow angles in rads	
	Matrix2D Jacobian(Vector2D shoulderAngles);		///>calculates static motor torques for given motor angles and desired forces
	Matrix2D J_dot(Vector2D q,Vector2D qd); //calculates the time derivative of the Jacobian
	Matrix7x2D CalcPsi(Vector2D q,Vector2D qd,Vector2D qdd); //calculates the Psi matrix (mass, inertia, friction)

	//adaptive FB fuctions
	void update_pd_torque(double dt);
	void open_fb_file(char fin[]);
	void write_fb_file(double dt);
	void load_trajectory_data(char fin[]); //trajectory data loader for feedforward testing

	//Output and control functions
	void updateWristControl(double dt); ///< updates wrist angle PID controller  
	void updateWristAngleControl(double dt); ///< executes single cycle of angle PID
	void updateWristTorqueControl(double dt); ///< executes single cycle of angle PID 

	//dynamics compensation
	Vector2D updateDynamicsComp(double dt); ///<Updates Dynamics compensation

	void pollWDT();			//< change wdt line state. Called each servo loop to prevent timeout

	void checkLimitSwitch(); //< check hard limits, disable robot if encountered 
	void checkMotorHardEnable(); //< checks the status of motor hard enable line. ie whether WDT is active, emg stop is off and enable switch down.

	double sign(double v) //sign function
	{
		return v > 0 ? 1 : (v < 0 ? -1 : 0);
	}

	public:

	///robot values that can be read 
	Vector2D position;		///< global position of wrist (3d) or handle (2d), following recentering etc. 
	Vector2D velocity;		///< Current velocity of wrist (3d) or handle (2d), unfiltered in cm/s [CHECK?]
	Vector2D positionFilt;  ///< Position after Kalmanfiltering 
	Vector2D velocityFilt;  ///< Velocity estimate after Kalman filtering 
	Vector2D force;			///< Current Force Goal in global cooridinates  
	Vector2D forceProd;		///< Current estimate of force produced in global coordinates 
	Vector2D positionEP;	///< Position of endpoint in 3d
	Vector2D velocityEP;	///< 
	Vector2D positionFiltEP; ///< 
	Vector2D velocityFiltEP; ///< 
	double wristLength;		 ///< Length of the distance between wrist joint and finger tip 
	double WristAngle;

	Vector2D AngularAccFilt;  ///< Angular velocity estimate for shoudler motors after Kalman filtering 
	Vector2D AngularVelocityFilt;  ///< Angular velocity estimate for shoudler motors after Kalman filtering 
	Vector2D LastAngularVelocityFilt;  ///< Last Angular velocity estimate for shoudler motors after Kalman filtering 
	Vector2D AngleFilt;  ///< Angle estimate for shoulder motors after Kalman filtering 

	double currentRate;		///< Current servo cycle rate 

	//Position of various parts of the robot in global cord (unaffected by recenter)
	Vector2D motorPos[2];	///< Position of motors  (local coordinates) 
	Vector2D elbowPos[2];	///< Position of elbows (local coordinates) 
	Vector2D elbow_angles[2];	///< Angle of elbows (local coordinates) 

	//Copies of safety gain and torque output variables (effectively read only)
	double safeVoltGainOut;		///< public copy of safety gain ramp on final output voltage 
	Vector3D torqueDemandedOut;	///< public copy of current output torque  
	int outputEnableOut;		///< public copy of Master output enable for all motors (1=enabled, 0= disabled)
	int hardEnableOut;			///< public copy of status of hard enable flag for motors (1=enabled, 0= disabled)
	int hardLimitOut;			///< public copy of status of hard limti switches flag for motors (1=at limit, 0= in range)

private: 

	///robot parameters determined from calibration file 
	string paramfilename;	///< name of current parameter file 
	int board_cnt[3];		///< Board for encoder channels 
	int channel[3];			///< Channels for counters 
	int board_da;			///< Board number used for force output 
	int da_offset;			///< Channel number of da offset 
	double ang_ati2arm;		///< Angle from ATI force tranducer to arm coordinates 

	double jointLengthUpper[2];    ///< Length of robot upper joint 
	double jointLengthLower[2];    ///< Length of robot upper joint 
	double robotDistance;		   ///< distance between the two robots 

	Vector3D offset;		///< Offset for 0 degrees on Shoulder/ Elbow encoder 
	Vector3D dir_cnt;		///< Counting direction of Encoders 
	Vector3D torque2volts;    ///< Torque to volts transversion at motors 

	Vector3D torqueDemanded; ///< Master motor torque demand variable
	Vector2D dymCompTorqueDemanded; ///< Master motor torque demand variable

	///robot values that are updated by update step
	bool isFirstUpdate;						///< is first update? 
	KalmanFilter<2>	filter;					///< Kalman filter for optimal velocity and position estimation of the wrist (3d) or endpoint (2d)
	KalmanFilter<2>	filterEP;				///< Kalman filter for the finger tip (3d) 
	KalmanFilter<2>	shoulderEncoderFilter;	///< Kalman filter for optimal velocity and position estimation 
	KalmanFilter<2>	wristAngleFilter;		///< Kalman filter for wrist velocity error estimation 
	KalmanFilter<2>	wristTorqueFilter;		///< Kalman filter for wrist torque error derivative
	KalmanFilter<2>	wristFilter;			///< Kalman filter for wrist velcity estimation

	long rShCnt;				///< Shoulder encoder count 
	long lShCnt;				///< Elbow encoder count 
	long wrCnt;				///< wrist encoder count 
	Vector3D theta;				///< Angles of shoulder joint [0] and elbow joint [1]
	Vector2D rawPosition;	///< raw global position, unfiltered in m 
	Vector3D torque;			///< Current Torques of shoulder [0] and elbow [1] 
	Vector2D lastPosition;		///< Last position 
	Matrix2D JdxdTheta;		///< Jacobian dx/theta 
	Matrix2D curJdTheta;    ///< Temp variable for current Jacobian result

	Vector3D volts;					///< variable for voltage output to motors 
	Vector2D positionOffset;		///< position offset for recentering. 

	Vector3D globalFTForce;		///< FT force in sensor co-ordinate system
	Vector3D localFTForce;      ///< FT force in global robot co-ordinate system

	//safety cutouts
	Vector2D wristLimitAngle;		///< limit of safe wrist motor angle (rad)
	Vector2D rShouldLimitAngle;		///< limit of safe right shoudler angle (rad)
	Vector2D lShouldLimitAngle;		///< limit of safe left shoudler angle (rad)

	double safeVoltRiseTime;				///< Post Cutout voltage gain recovery duration in seconds
	double safeVoltGain;				///< Final gain of motor drive volatages (allow for safe "ramping" of output)

	float shouldVoltLimit;		///< Maximum safe voltage level for shoulder motors
    float wristVoltLimit;		///< Maximum safe voltage level for wrist motors

	int outputEnable; ///< Master output enable for all motors (0 = Disable, 1 = Enable). Controlled by enableOutput/disableOutput rather than directly 

	int wdt_pulse_state; ///< state of watchdog control line
	int first_cycle;


	//wrist angle loop vars
	int wristFBOn; ///<Enable pid for wrist angle control (0=Inactive, 1=Active)
	int wristControlMode;

	#ifdef TUNINGMODE
		public: 
	#endif

	double wristASetAngle; ///< Wrist angle set point 
	double wristTSetTorque; ///< Wrist angle set point 

	double wristAPTerm; ///< proportional term for wrist angle control
	double wristAITerm; ///< integral term for wrist angle control
	double wristADTerm; ///< integral term for wrist angle control
	double wristAError; ///< error term for wrist angle control
	double wristAErrLast; ///< integral term for wrist angle control

	double wristAKp; ///< Proportional gain for wrist angle control
	double wristAKi; ///< Integral gain for wrist angle control
	double wristAKd; ///< Deriavtive for wrist angle control
	double wristADemandTorque; ///< Intermediate demand torque signal to wrist motor

	double WristControlGain; ///< Final gain for both wrist control loop, should stay in range 0-1

	//private
	double wristTPTerm; ///< proportional term for wrist angle control
	double wristTITerm; ///< integral term for wrist angle control
	double wristTDTerm; ///< integral term for wrist angle control
	double wristTError; ///< error term for wrist angle control
	double wristTErrLast; ///< integral term for wrist angle control

	double wristTKp; ///< Proportional gain for wrist angle control
	double wristTKi; ///< Integral gain for wrist angle control
	double wristTKd; ///< Deriavtive for wrist angle control
	double wristTDemandTorque; ///< Intermediate demand torque signal to wrist motor

	double wristFFDeadZone; //feedforward dead zone for friction 
	double wristFFSlopeZone; //feedforward slope for friction 

	double wristKpStep; ///< Proportional gain step for wrist angle control
	double wristKiStep; ///< Integral gain step for wrist angle control
	double wristKdStep; ///< Deriavtive step for wrist angle control

	double wristTSetTorqueLimit; ///< Maximum torque target for control loop (Nm)

	double dt;	///< holder for servo sample period

	//vars for pd elbow torque control
	int traj_index;
	int en_traj; //enable trajectory control
	Vector2D FB; //feedback drive command
	Vector2D FF; //feedforward drive command
	Vector7D Omega; //System parameter matrix 
	Vector7D DeltaOmega; //diff in sys para matrix
	Matrix7x2D Psi; // system matrix 

	Vector7D OmegaReal; //System parameter matrix from prior estimation
	Matrix7x2D PsiReal; // system matrix from real data
	Vector2D Tau; //final torque

	Vector2D qe; //angular error for PD controller 
	double shoul_a_pk; //shoulder pk proportional gain
	double shoul_a_dk; //shoulder pd derivative gain
	double l_step;
	double l_g_step;

	int compEnabled; //enable trajectory control
	double compGain; //master gain for dynamics compensation 

	int ff_on; //enables disables feedforward for model 

	KalmanFilter<2>	filter_q;		///< Kalman filter for anglular motor velocity and position estimation 

	double Lmat[7]; //matrix of preset Learning factors
	double Lgain; //gain for Leranign factors
	double omega_hist[OMEGA_BUFF][7];
	int omega_hi; //index into array

	Vector2D q_last; //current measured motor angle (right motor, left motor)
	Vector2D q; //current measured angle
	Vector2D qd; //current anglular velocity as derivative
				
	Vector2D q_des; //target angle
	Vector2D qd_des; //target anglular velocity
	Vector2D qdd_des; //target anglular acceleration
	Vector2D q_vel_filt;
	
	//trajectory data for pd
	double trajectory_list[RECORD_LIMIT][6]; //Trajectory data for inertial comp test. Ordered: left angle, right angle, left av, right av, left aa, right aa
	FILE *FB_file_out; //output file pointer for test data
	int traj_length; //length of trajectory data
	double sample_index;
	double time_elapsed;
	
	

}; 

#endif


