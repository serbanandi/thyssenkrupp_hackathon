#include "CDriver.h"
#include <math.h>

#define PI 3.141592654
/* Gear Changing Constants*/
const int gearUp[6]=
    {
        5000,6000,6000,6500,7000,0
    };
const int gearDown[6]=
    {
        0,2500,3000,3000,3500,3500
    };

/* Stuck constants*/
const int stuckTime = 25;
const float stuckAngle = .523598775; //PI/6

/* Accel and Brake Constants*/
const float maxSpeedDist=70;
const float maxSpeed=350;
const float sin5 = 0.08716;
const float cos5 = 0.99619;

/* Steering constants*/
const float steerLock=0.785398;
const float steerSensitivityOffset=80.0;
const float wheelSensitivityCoeff=1;

/* ABS Filter Constants */
const float wheelRadius[4]={0.3179,0.3179,0.3276,0.3276};
const float absSlip=2.0;
const float absRange=3.0;
const float absMinSpeed=3.0;

/* Clutch constants */
const float clutchMax=0.5;
const float clutchDelta=0.05;
const float clutchRange=0.82;
const float clutchDeltaTime=0.02;
const float clutchDeltaRaced=10;
const float clutchDec=0.01;
const float clutchMaxModifier=1.3;
const float clutchMaxTime=1.5;

int stuck;
float clutch;

const float sensorAngle[19] = { -PI/2, -(PI/12)*5, -PI/3, -PI/4, -PI/6, -PI/9, -PI/12, -PI/18, -PI/36, 0, PI/36, PI/18, PI/12, PI/9, PI/6, PI/4, PI/3, (PI/12)*5, PI/2};
const float cols[19] = {0, 0.258819045, 0.5, 0.707106781, 0.866025404, 0.939692621, 0.965925826, 0.984807753, 0.996194698, 1.0, 0.996194698, 0.984807753, 0.965925826, 0.939692621 , 0.866025404 , 0.707106781 , 0.5, 0.258819045, 0};
const float sinj[19] = {1.0, 0.965925826, 0.866025404, 0.707106781, 0.5, 0.342020143, 0.258819045, 0.173648178, 0.087155743, 0.0, 0.087155743, 0.173648178, 0.258819045, 0.342020143, 0.5, 0.707106781, 0.866025404 ,0.965925826, 1.0};

typedef enum Direction {
    LEFT,
    RIGHT
}Direction;

//kisz�molja, hogy h�nyadik sebess�gbe kell rakni
int getGear(structCarState *cs)
{
    int gear = cs->gear;
    int rpm  = cs->rpm;

    // if gear is 0 (N) or -1 (R) just return 1 
    if (gear<1)
        return 1;
    // check if the RPM value of car is greater than the one suggested 
    // to shift up the gear from the current one     
    if (gear <6 && rpm >= gearUp[gear-1])
        return gear + 1;
    else
    	// check if the RPM value of car is lower than the one suggested 
    	// to shift down the gear from the current one
        if (gear > 1 && rpm <= gearDown[gear-1])
            return gear - 1;
        else // otherwhise keep current gear
            return gear;
}

typedef struct Coordinate {
    double x;
    double y;
}Coordinate;


Coordinate trackCoordinate(int i, structCarState* cs) {
    Coordinate coordinate;
    coordinate.x = sin(sensorAngle[i] + cs->angle) * cs->track[i];
    coordinate.y = cos(sensorAngle[i] + cs->angle) * cs->track[i];
    return coordinate;
}

double distance(Coordinate A, Coordinate B) {
    return sqrt((A.x - B.x)*(A.x - B.x) + (A.y - B.y)*(A.y - B.y));
}

double area(Coordinate A, Coordinate B, Coordinate C) {
    return abs((B.x - A.x)*(C.y - B.y) - (B.y - A.y)*(C.x - B.x))/2;
}

double curvature(Coordinate A, Coordinate B, Coordinate C) {
    return ((distance(A, B)*distance(B, C)*distance(A, C)) / (4*area(A, B, C)));
}
//Lehetne egy konstans t�mb
double trackCurvature(Direction direction, structCarState* cs) {
    if (direction == RIGHT) {
        return (curvature(trackCoordinate(0, cs), trackCoordinate(1, cs), trackCoordinate(2, cs)) + curvature(trackCoordinate(3, cs), trackCoordinate(4, cs), trackCoordinate(5, cs)) + curvature(trackCoordinate(6, cs), trackCoordinate(7, cs), trackCoordinate(8, cs)) + curvature(trackCoordinate(0, cs), trackCoordinate(5, cs), trackCoordinate(9, cs))+ curvature(trackCoordinate(1, cs), trackCoordinate(6, cs), trackCoordinate(8, cs))) / 5;
    }
    else if (direction == LEFT) {
        return (curvature(trackCoordinate(18, cs), trackCoordinate(17, cs), trackCoordinate(16, cs)) + curvature(trackCoordinate(15, cs), trackCoordinate(14, cs), trackCoordinate(13, cs)) + curvature(trackCoordinate(12, cs), trackCoordinate(11, cs), trackCoordinate(10, cs)) + curvature(trackCoordinate(18, cs), trackCoordinate(13, cs), trackCoordinate(9, cs)) + curvature(trackCoordinate(17, cs), trackCoordinate(12, cs), trackCoordinate(10, cs))) / 5;
        }
}

Direction direction(structCarState* cs) {
    Direction direction;
    if (cs->track[8] > cs->track[10]) {
        direction = LEFT;
    }
    else {
        direction = RIGHT;
    }
}

/*Coordinate center(Coordinate A, Coordinate B, Coordinate C) {
    Coordinate centre;
    centre.x = A.x*;
}*/

int chevkbBend(structCarState* cs) {
    if (cs->track[9] < 199) {
        return 1;
    }
    return 0;
}

int isBend(structCarState* cs) {
    for (int i = 0; i < 19; i++) {
        if (cs->track[i] > 199) {
            return 0;
        }
    }
    return 1;
}


int maxDistIdx(structCarState* cs) {
    int maxIdx = 0;
    float maxDist = 0;
    int avgIdx = 0;
    int sameCtr = 0;



    for (int i = 0; i < 19; i++) {
        if (cs->track[i] > maxDist) {
            maxDist = cs->track[i];
            maxIdx = i;
        }
    }

    if (cs->track[9] == maxDist)
        return 9;

    for (int i = 0; i < 19; i++) {
        if (cs->track[i] == maxDist) {
            sameCtr++;
            avgIdx += i;
        }
    }
    if (sameCtr)
        return avgIdx / sameCtr;

    return maxIdx;
}
void bendSharpness(Direction direction, structCarState* cs, float* maxSpeedDist) {
    int countBlindSensor = 0;
    float sharpness = 0;
    int maxDIdx = maxDistIdx(cs);
    for (int i = 0; i < 19; ++i) {
        if (cs->track[i] > 199.99) {
            countBlindSensor++;
        }
    }
    if (!countBlindSensor) {
        if ((18 - maxDIdx) < 3 || (18 - maxDIdx) > 15) {
            //Mit csin�ljunk ilyenkor
        } 
        else {
            if (direction == RIGHT) {
                
                for (int i = maxDIdx+1; i < 17; ++i) {
                        sharpness += curvature(trackCoordinate(i, cs), trackCoordinate(i + 1, cs), trackCoordinate(i + 2, cs));
                }
                sharpness /= (16 - maxDIdx);
            }
            else if (direction == LEFT) {
                for (int i = 0; i < maxDIdx-1; ++i) {
                    sharpness += curvature(trackCoordinate(i, cs), trackCoordinate(i + 1, cs), trackCoordinate(i + 2, cs));
                }
                sharpness /= (maxDIdx-2);
            }
        }
        
    }
    *maxSpeedDist = sharpness * maxDIdx;
}

/*void aside(Direction direction, structCarState* cs) {
    bendSharpness(direction, cs);
}
*/

float getSteer(structCarState* cs)
{
    int maxDIdx = maxDistIdx(cs);
    // steering angle is compute by correcting the actual car angle w.r.t. to track
    // axis [cs->angle] and to adjust car position w.r.t to middle of track [cs->trackPos*0.5]
    float targetAngle = cs->angle - (abs(cs->trackPos) * (-1) * cs->angle + (1 - abs(cs->trackPos)) * sensorAngle[maxDIdx]) - cs->trackPos * 0.5;          ///* abs(cs->trackPos) */ 1 - (abs(cs->trackPos)) * sensorAngle[maxDIdx];  //sensorAngle[maxDIdx]; (cs->angle - cs->trackPos * 0.5) + 1 -
        // at high speed reduce the steering command to avoid loosing the control
        if (cs->speedX > steerSensitivityOffset)
            return targetAngle / (steerLock * (cs->speedX - steerSensitivityOffset) * wheelSensitivityCoeff);
        else
            return (targetAngle) / steerLock;

}

/*float getSteer(structCarState* cs)
{
    // steering angle is compute by correcting the actual car angle w.r.t. to track 
    // axis [cs->angle] and to adjust car position w.r.t to middle of track [cs->trackPos*0.5]
    int R;
    R = trackCurvature(cs);
    float targetAngle = (cs->angle - cs->trackPos * 0.5) - 
    // at high speed reduce the steering command to avoid loosing the control
    if (isBend(cs)) {
       // targetAngle = sensorAngle[maxDistIdx(cs)];
    }
    if (cs->speedX > steerSensitivityOffset)
        return targetAngle / (steerLock * (cs->speedX - steerSensitivityOffset) * wheelSensitivityCoeff);
    else
        return (targetAngle) / steerLock;

}*/

//gyorsul�st adja meg (mennyire l�pj�l r� a g�zra)
float getAccel(structCarState* cs)
{
    // checks if car is out of track
    if (cs->trackPos < 1 && cs->trackPos > -1)
    {
        // reading of sensor at +5 degree w.r.t. car axis
        float rxSensor = cs->track[10];
        // reading of sensor parallel to car axis
        float cSensor = cs->track[9];
        // reading of sensor at -5 degree w.r.t. car axis
        float sxSensor = cs->track[8];

        float targetSpeed;

        // track is straight and enough far from a turn so goes to max speed
        if (cSensor > maxSpeedDist || (cSensor >= rxSensor && cSensor >= sxSensor))
            targetSpeed = maxSpeed;
        else
        {
            // computing approximately the "angle" of turn
            int maxDIdx = maxDistIdx(cs);
            float sinAngle = abs(sinj[maxDIdx]);
            // estimate the target speed depending on turn and on how close it is
            if (sinAngle == 0) {
                sinAngle = 1;
            }
            targetSpeed = maxSpeed * (cSensor * sinAngle / maxSpeedDist);

        }

        // accel/brake command is expontially scaled w.r.t. the difference between target speed and current one
        return 2 / (1 + exp(cs->speedX - targetSpeed)) - 1;
    }
    else
        return 0.3; // when out of track returns a moderate acceleration command
}
    structCarControl CDrive(structCarState cs)
    {
        if (cs.stage != cs.prevStage)
        {
            cs.prevStage = cs.stage;
        }
        // check if car is currently stuck
        if (fabs(cs.angle) > stuckAngle)
        {
            // update stuck counter
            stuck++;
        }
        else
        {
            // if not stuck reset stuck counter
            stuck = 0;
        }

        // after car is stuck for a while apply recovering policy
        if (stuck > stuckTime)
        {
            /* set gear and sterring command assuming car is
             * pointing in a direction out of track */

             // to bring car parallel to track axis
            float steer = -cs.angle / steerLock;
            int gear = -1; // gear R

            // if car is pointing in the correct direction revert gear and steer  
            if (cs.angle * cs.trackPos > 0)
            {
                gear = 1;
                steer = -steer;
            }

            // Calculate clutching
            clutching(&cs, &clutch);

            // build a CarControl variable and return it
            structCarControl cc = { 1.0f,0.0f,gear,steer,clutch };
            return cc;
        }

        else // car is not stuck
        {
            // compute accel/brake command
            float accel_and_brake = getAccel(&cs);
            // compute gear 
            int gear = getGear(&cs);
            // compute steering
            float steer = getSteer(&cs);


            // normalize steering
            if (steer < -1)
                steer = -1;
            if (steer > 1)
                steer = 1;

            // set accel and brake from the joint accel/brake command 
            float accel, brake;
            if (accel_and_brake > 0)
            {
                accel = accel_and_brake;
                brake = 0;
            }
            else
            {
                accel = 0;
                // apply ABS to brake
                brake = filterABS(&cs, -accel_and_brake);
            }

            // Calculate clutching
            clutching(&cs, &clutch);

            // build a CarControl variable and return it
            structCarControl cc = { accel,brake,gear,steer,clutch };
            return cc;
        }
    }

float filterABS(structCarState *cs,float brake)
{
	// convert speed to m/s
	float speed = cs->speedX / 3.6;
	// when spedd lower than min speed for abs do nothing
    if (speed < absMinSpeed)
        return brake;
    
    // compute the speed of wheels in m/s
    float slip = 0.0f;
    for (int i = 0; i < 4; i++)
    {
        slip += cs->wheelSpinVel[i] * wheelRadius[i];
    }
    // slip is the difference between actual speed of car and average speed of wheels
    slip = speed - slip/4.0f;
    // when slip too high applu ABS
    if (slip > absSlip)
    {
        brake = brake - (slip - absSlip)/absRange;
    }
    
    // check brake is not negative, otherwise set it to zero
    if (brake<0)
    	return 0;
    else
    	return brake;
}

void ConShutdown()
{
    printf("Bye bye!");
}

void ConRestart()
{
    printf("Restarting the race!");
}

void clutching(structCarState *cs, float *clutch)
{
  float maxClutch = clutchMax;

  // Check if the current situation is the race start
  if (cs->curLapTime<clutchDeltaTime  && cs->stage == RACE && cs->distRaced < clutchDeltaRaced)
    *clutch = maxClutch;

  // Adjust the current value of the clutch
  if(clutch > 0)
  {
    float delta = clutchDelta;
    if (cs->gear < 2)
	{
      // Apply a stronger clutch output when the gear is one and the race is just started
	  delta /= 2;
      maxClutch *= clutchMaxModifier;
      if (cs->curLapTime < clutchMaxTime)
        *clutch = maxClutch;
	}

    // check clutch is not bigger than maximum values
	*clutch = fmin(maxClutch,*clutch);
    
	// if clutch is not at max value decrease it quite quickly
	if (*clutch!=maxClutch)
	{
	  *clutch -= delta;
	  *clutch = fmax(0.0,*clutch);
	}
	// if clutch is at max value decrease it very slowly
	else
		*clutch -= clutchDec;
  }
}

//gives 19 angles for the distance sensors
void Cinit(float *angles)
{

	// set angles as {-90,-75,-60,-45,-30,20,15,10,5,0,5,10,15,20,30,45,60,75,90}

	for (int i=0; i<5; i++)
	{
		angles[i]=-90+i*15;
		angles[18-i]=90-i*15;
	}

	for (int i=5; i<9; i++)
	{
			angles[i]=-20+(i-5)*5;
			angles[18-i]=20-(i-5)*5;
	}
	angles[9]=0;
}