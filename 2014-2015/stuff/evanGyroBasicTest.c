#pragma config(Sensor, S1,     turnSensor,     sensorI2CHiTechnicGyro)
#pragma config(Motor,  motorB,          rightMotor,    tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorC,          leftMotor,     tmotorNXT, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

task main()
{
	// take initial readings from gyro
	int degreesToTurn = 90;
	float degreesSoFar = 0;
	int initialTurnReading = SensorValue[turnSensor];
	//start turning
	nSyncedMotors = synchBC;
	nSyncedTurnRatio = -100; //b & c go oposite directions
	motor[motorB] = 30;
	//check if we turned enough
	while(degreesSoFar < degreesToTurn)
	{
		wait1Msec(1);//wait 10 milisecounds which is equal to 1/100
		int currentGyroReading = SensorValue[turnSensor] - initialTurnReading;
		degreesSoFar = degreesSoFar + currentGyroReading * .01;
	}
	//stop turning and stuff
	motor[motorB] = 0;

}
