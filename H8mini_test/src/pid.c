/*
The MIT License (MIT)

Copyright (c) 2015 silverx

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#include <stdbool.h>
#include "pid.h"
#include "util.h"
#include "config.h"

#include "defines.h"

#define PIDGAIN 1.0
#define GYROPERCENT 0.0

// this Kp is used for a normal PID ( PI-D , really )
float pidkp[PIDNUMBER] = {3e-2*PIDGAIN*(1-GYROPERCENT) , 3e-2*PIDGAIN*(1-GYROPERCENT)  , 5e-1 }; 
//  											ROLL       PITCH     YAW
// this Kp2 is used for a I-PD controller instead of the above PID
float pidkp2[PIDNUMBER] = { 5e-2*PIDGAIN*(1-GYROPERCENT) , 5e-2*PIDGAIN*(1-GYROPERCENT) ,  0e-2 };	
// Ki
float pidki[PIDNUMBER] = { 8e-1*PIDGAIN  , 8e-1*PIDGAIN , 25e-1 };	
// Kd											ROLL       PITCH     YAW
float pidkd[PIDNUMBER] = { 1.8e-1*PIDGAIN , 1.8e-1*PIDGAIN  , 1.0e-1 };	




// output limit			
const float outlimit[PIDNUMBER] = { 1.0 , 1.0 , 0.4 };

// limit of integral term (abs)
const float integrallimit[PIDNUMBER] = { 1.0 , 1.0 , 0.2 };


float ierror[PIDNUMBER] = { 0 , 0 , 0};	
float lastrate[PIDNUMBER];
float pidoutput[PIDNUMBER];

extern float error[PIDNUMBER];
float lasterror[PIDNUMBER];

extern float looptime;
extern float gyro[3];
extern int onground;

float lasterror[PIDNUMBER];

float lastdt[3];

float dtfilt[3];

float pid(int x )
{ 

        if (onground) 
				{
           ierror[x] *= 0.8;
				}
	
				int iwindup = 0;
				if (( pidoutput[x] == outlimit[x] )&& ( error[x] > 0) )
				{
					iwindup = 1;		
				}
				if (( pidoutput[x] == -outlimit[x])&& ( error[x] < 0) )
				{
					iwindup = 1;				
				}
        if ( !iwindup)
				{
				 // trapezoidal rule instead of rectangular
         ierror[x] = ierror[x] + (error[x] + lasterror[x]) * 0.5 *  pidki[x] * looptime;
				 //ierror[x] = ierror[x] + error[x] *  pidki[x] * looptime; 					
				}
				
				lasterror[x] = error[x];
				limitf( &ierror[x] , integrallimit[x] );
		
				// P term
          pidoutput[x] = error[x] * pidkp[x] ;
									
				// P2 (direct feedback) term	
				  pidoutput[x] = pidoutput[x] -  ( gyro[x] + lastrate[x] ) * 0.5 *pidkp2[x];
				
				// I term	
					pidoutput[x] += ierror[x];
			
				// D term
			
				//lpf ( &dtfilt[x] , (gyro[x] - lastrate[x] ) , 0.8 );
				
					pidoutput[x] = pidoutput[x] - (gyro[x] - lastrate[x] ) * pidkd[x]; 
				//pidoutput[x] = pidoutput[x] - dtfilt[x]* pidkd[x];
				//pidoutput[x] = pidoutput[x] - (gyro[x] - lastrate[x] + lastdt[x])*0.5 * pidkd[x]; 
		
					lastdt[x] = gyro[x] - lastrate[x];
				
				  limitf(  &pidoutput[x] , outlimit[x]);
					
lastrate[x] = gyro[x];	
lasterror[x] = error[x];

return pidoutput[x];		 		
}

