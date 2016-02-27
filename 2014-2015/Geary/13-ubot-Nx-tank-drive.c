#pragma config(Hubs,  S1, HTServo,  none,     none,     none)
#pragma config(Hubs,  S2, HTMotor,  none,     none,     none)
#pragma config(Sensor, S1,     ,               sensorI2CMuxController)
#pragma config(Sensor, S2,     ,               sensorI2CMuxController)
#pragma config(Motor,  motorA,           ,             tmotorNXT, openLoop)
#pragma config(Motor,  motorB,           ,             tmotorNXT, openLoop)
#pragma config(Motor,  motorC,           ,             tmotorNXT, openLoop)
#pragma config(Motor,  mtr_S2_C1_1,     left,          tmotorTetrix, openLoop)
#pragma config(Motor,  mtr_S2_C1_2,     right,         tmotorTetrix, openLoop, reversed)
#pragma config(Servo,  srvo_S1_C1_1,    turntable,            tServoContinuousRotation)
#pragma config(Servo,  srvo_S1_C1_2,    arm,                  tServoContinuousRotation)
#pragma config(Servo,  srvo_S1_C1_3,    claw,                 tServoStandard)
#pragma config(Servo,  srvo_S1_C1_4,    debris,               tServoContinuousRotation)
#pragma config(Servo,  srvo_S1_C1_5,    servo5,               tServoNone)
#pragma config(Servo,  srvo_S1_C1_6,    servo6,               tServoNone)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

// based on PSP-Nx-motor-control.c
// http://www.mindsensors.com/index.php?module=documents&JAS_DocumentManager_op=viewDocument&JAS_Document_id=13

#include "PSP-Nx-lib.h"
// #include "geary-nomux-stuff.c"

//////////////////////////////////////////////////////////////////////////////
//
//      Globals
//
/////////////////////////////////////////////////////////////////////////////

// Servo value of 0 = Full Power/Speed Reverse
// Servo value of 127 = Stop
// Servo value of 256 = Full Power/Speed Forward

#define SERVO_BACKWARD 100
#define SERVO_STOP 127
#define SERVO_FORWARD 150
#define CLAW_OPEN 160
#define CLAW_CLOSED_1 140
#define CLAW_CLOSED_2 100

const ubyte Addr = 0x02;
const tSensors SensorPort = S4;        // Connect PSPNX sensorto this port!!
const int DeadZone = 50;

void TestPSPAnalog () {
  psp currState;
	PSP_ReadButtonState(SensorPort, Addr, currState);
	if ((int)currState.l_j_y == 99 && (int)currState.r_j_y == 99) {
	  	PlaySound(soundDownwardTones);
      wait1Msec(500);
      StopAllTasks();
    }
  }

//////////////////////////////////////////////////////////////////////////////
//
//      Main
//
/////////////////////////////////////////////////////////////////////////////


task
main ()
{
  int powerLeft = 0;
  int powerRight = 0;
  int d_left_X, d_left_Y;
  int d_right_X, d_right_Y;
  bool arm_up, arm_down, arm_left, arm_right;
  int left_joystick_click, right_joystick_click;
  bool claw_close_1, claw_close_2;
  bool hopper_forward, hopper_backward;
  int claw_position, hopper_position;
  int tank_drive_ok = false;
  int steer_drive_ok = false;
  int turbo_speed = false;
  psp currState;

  // Note: program cannot be terminated if we hijack the 'exit' button. So there has to be an escape sequence
  //       that will return buttons to system control! We'll use a triple click
  //
  nNxtExitClicks = 3;                // Triple clicking EXIT button will terminate program
  nI2CBytesReady[SensorPort] = 0;
  SensorType[SensorPort] = sensorI2CMuxController;
  wait10Msec (100);
  TestPSPAnalog();

  while ( true )
    {
      wait1Msec (100);

      PSP_ReadButtonState(SensorPort, Addr, currState);

      // joysticks
      d_left_X = (int)currState.l_j_x;
      d_left_Y = (int)currState.l_j_y;
      d_right_X = (int)currState.r_j_x;
      d_right_Y = (int)currState.r_j_y;

      // trigger buttons control claw, if connected
      claw_close_1 = (!(bool)currState.l1) || (!(bool)currState.l2);
      claw_close_2 = (!(bool)currState.r1) || (!(bool)currState.r2);

      // trigger buttons control ball hopper, if connected
      hopper_forward = (!(bool)currState.r1);
      hopper_backward = (!(bool)currState.r2);

      // for controlling tank or steer
      left_joystick_click = !(bool)currState.l_j_b ;
      right_joystick_click = !(bool)currState.r_j_b;

      // by default, do not enable driving, to avoid driving off a table
      if (left_joystick_click) { tank_drive_ok = true; steer_drive_ok = false; }
      if (right_joystick_click) { tank_drive_ok = false; steer_drive_ok = true; }

      // drive slowly unless turbo
      turbo_speed = false ; // trigger_pressed;

      if (!tank_drive_ok) { // if not tank drive, then left joystick controls arm and turntable
	      // left y controls arm
	      arm_up = (d_left_Y > DeadZone);
	      arm_down = (d_left_Y < -DeadZone);
	      // left x controls turntable
	      arm_left = (d_left_X > DeadZone);
	      arm_right = (d_left_X < -DeadZone);
	    }

      // tank drive
      if (tank_drive_ok) {
	      powerLeft = abs(d_left_Y) > DeadZone ? abs(d_left_Y)*d_left_Y/100 : 0;
	      powerRight = abs(d_right_Y) > DeadZone ? abs(d_right_Y)*d_right_Y/100 : 0;
	      if (!turbo_speed) {	powerLeft /= 2; powerRight /= 2; }
	  	  nxtDisplayTextLine(1,"Left: %d", powerLeft);
  		  nxtDisplayTextLine(2,"Right: %d", powerRight);

  		// steering
  	  } else if (steer_drive_ok) {
  	    // distribute power proportionally based on x value
  	    int differential = abs(d_right_X) > DeadZone ? d_right_X : 0;
	      int totalPower = abs(d_right_Y) > DeadZone ? abs(d_right_Y)*d_right_Y/100 : 0;
	      if (!turbo_speed) {	totalPower /= 2; }
	      float proportion = abs(differential) / 100.0;  // extreme steer -> 1
	      powerLeft = differential > 0 ? totalPower * proportion : totalPower * (1.0 - proportion);
	      powerRight = differential < 0 ? totalPower * proportion : totalPower * (1.0 - proportion);
	  	  nxtDisplayTextLine(1,"Left: %d", powerLeft);
  		  nxtDisplayTextLine(2,"Right: %d", powerRight);

  		  // driving not enabled yet
	    } else {
		    nxtDisplayTextLine(1,"Click a joystick");
		    nxtDisplayTextLine(2,"to drive");
		  }

		  // motor[left] = powerLeft;
		  // motor[right] = powerRight;

		  // arm
		  if (arm_up) {
		  	servo[arm] = SERVO_FORWARD;
		  	nxtDisplayTextLine(3,"Arm up");
			} else if (arm_down) {
		  	servo[arm] = SERVO_BACKWARD;
		  	nxtDisplayTextLine(3,"Arm down");
		  } else {
		  	servo[arm] = SERVO_STOP;
		  	nxtDisplayTextLine(3,"Arm stop");
		  }

		  // turntable
		  if (arm_left) {
		  	servo[turntable] = SERVO_FORWARD;
		  	nxtDisplayTextLine(4,"Arm left");
			} else if (arm_right) {
		  	servo[turntable] = SERVO_BACKWARD;
		  	nxtDisplayTextLine(4,"Arm right");
		  } else {
		  	servo[turntable] = SERVO_STOP;
		  	nxtDisplayTextLine(4,"Arm not turning");
		  }

		  // claw, if connected
		  if (claw_close_1) {
		  	claw_position = CLAW_CLOSED_1;
			} else if (claw_close_2) {
		  	claw_position = CLAW_CLOSED_2;
		  } else {
		  	claw_position = CLAW_OPEN;
		  }
		  servo[claw] = claw_position;
		  nxtDisplayTextLine(4,"Claw position %d", claw_position);

		  // debris ball hopper, if connected
		  if (hopper_forward) {
		  	hopper_position = SERVO_FORWARD;
		  } else if (hopper_backward) {
		  	hopper_position = SERVO_BACKWARD;
		  } else {
		  	hopper_position = SERVO_STOP;
		  }
		  servo[debris] = hopper_position;
		  nxtDisplayTextLine(4,"Hopper position %d", hopper_position);

		  // drive
		  motor[left] = powerLeft;
		  motor[right] = powerRight;

    }

  StopAllTasks ();
}
