/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. */  

#include <jsw.h>	/* libjsw */

#include "utils/pd_send.h"
#include "remote/ltports.h"
#include "player-master/default/playlist_parse.h"

#include <sys/stat.h>

// pour le test de synchro de frame()
#include <sys/time.h>
#include <time.h>

int initJoystick( js_data_struct* jsd, const char* device, const char* calib ) {

  //printf( "JSInit( &jsd=%d, '%s',  '%s', ... )\n", (unsigned long)jsd, device, calib );
	int status = JSInit( jsd, device, calib, JSFlagNonBlocking );
  //printf( "... done\n" );
	/* Error occured opening joystick? */
	if(status == JSSuccess) return 1;
	else {
	    /* There was an error opening the joystick, print by the
	     * recieved error code from JSInit().
	     */
	    switch(status)
	    {
	      case JSBadValue:
          fprintf( stderr, "%s: Invalid value encountered while initializing joystick.\n", device	);
          break;

	      case JSNoAccess:
          fprintf( stderr, "%s: Cannot access resources, check permissions and file locations.\n", device );
          break;

	      case JSNoBuffers:
          fprintf( stderr, "%s: Insufficient memory to initialize.\n", device );
          break;

	      default:	/* JSError */
          fprintf( stderr, "%s: Error occured while initializing.\n", device );
          break;
	    }

	    /* Print some helpful advice. */
	    fprintf( stderr,
               "Make sure that:\n\
1. Your joystick is connected (and turned on as needed).\n\
2. Your joystick modules are loaded (`modprobe <driver>').\n\
3. Your joystick needs to be calibrated (use `jscalibrator').\n\
\n" );
  }
   
  return 0;
}

double pitch_sum;
double yaw_sum;
double dY_sum;

int update_camera_position( camera *cam, double dt, double pitch, double yaw, double throttle, double sidestep, double dY )
{
  vector4 axis,throttle_axis,sidestep_axis;
  vector3 t,w,t2;
  matrix4 T;
  matrix4 Rx,Ry,Rz,Rtmp,R;
  matrix4 Ryinv;
  matrix4 M,M_new;

  pitch_sum-=pitch;
  yaw_sum-=yaw;
  dY_sum+=dY;

  mat4Identity(Rz);
  mat4XRotation(pitch_sum*180.0/M_PI,Rx);
  mat4YRotation(yaw_sum*180.0/M_PI,Ry);
  mat4Inverse(Ry,Ryinv);

  mat4Multiply(Rx,Ryinv,Rtmp);
  mat4Multiply(Ry,Rtmp,R);

  throttle_axis[0]=0.0;
  throttle_axis[1]=0.0;
  throttle_axis[2]=1.0;
  throttle_axis[3]=1.0;
  mat4MultiplyVector(Ry,throttle_axis,axis);
  vectxScale(axis,throttle,throttle_axis,3);

  sidestep_axis[0]=1.0;
  sidestep_axis[1]=0.0;
  sidestep_axis[2]=0.0;
  sidestep_axis[3]=1.0;
  mat4MultiplyVector(Ry,sidestep_axis,axis);
  vectxScale(axis,sidestep,sidestep_axis,3);

  vectxAdd(cam->position,throttle_axis,t,3);
  vectxAdd(t,sidestep_axis,t2,3);
  t2[1]=dY_sum;
  mat4RotationToAxis(R,w);

  camSetExternalParams(cam,t2,w);

  //vect3Print(cam->position);
  //printf("%f %f %f\n",roll_sum,pitch_sum,yaw_sum);
  //vect3Print(cam->orientation);
  //mat4Print(cam->M);

  return 0;
}

int main( int argc, char **argv )
{
    int i,k;
    char msg[1000];
    char calibFilename[255];
    int usleep_time;
    struct timeval tv;
    int refresh_rate,frame_time;
	//bimulticast bmc;
    tcpcast *pd_send;
    camera *cam;
    matrix4 M;

    pd_send=NULL;
    cam=NULL;
    camAllocate(&cam,NULL);

    strcpy(calibFilename,"joystick_calib");
    for(i=1;i<argc;i++) {
        if( strcmp("-calib",argv[i])==0 && i+1<argc ) {
            strcpy(calibFilename,argv[i+1]);
            i++;continue;
        }
    }

    js_data_struct jsd;
    //const char *device = JSDefaultDevice;
    const char *device = "/dev/input/js0";     

    struct stat buf;
    // ************************ INIT JOYSTICK!!
    i = stat ( device, &buf );
    if ( i != 0 ) {
      printf( "joystick not found... using mouse instead.\n" );
      exit(0);
    }
    i = stat ( calibFilename, &buf );
    if ( i != 0 ) {
      printf( "calibration file not found...\n" );
      printf( "please put the calibration file joystick_calib on this directory\n" );
      exit(0);
    }

    /* Joystick data structure. */
      
    printf( "initializing joystick TEST\n" );
    int status = JSInit( &jsd, device, calibFilename, JSFlagNonBlocking );
    printf( "initializing joystick TEST done status=%d\n",status );

    printf( "initializing joystick\n" );
    if ( !initJoystick(&jsd, device, calibFilename) ) {
      // do stuff?  quit?
      exit(0);
    }

    double pitch=0;
    double yaw=0;
    double throttle=0;
    double sidestep=0;
    double dY=0;
    vector3 t,w;
    unsigned char pitch_on=0;
    unsigned char trigger[2];
    long int tick_trigger[2],last_trigger[2];

    for (k=0;k<2;k++)
    {
      trigger[k]=0;
      last_trigger[k]=gettimeofday(&tv,NULL);
    }

    //camera translation,rotation
    t[0]=0;
    t[1]=0;
    t[2]=0;
    w[0]=0;
    w[1]=0;
    w[2]=0;

    refresh_rate=30;

    pitch_sum=0;
    yaw_sum=0;

    while( 1 )
    {
        for (k=0;k<2;k++) trigger[k]=0;
        // wait for all cull and draw threads to complete.

        if(JSUpdate(&jsd) == JSGotEvent) // THE JOYSTICK MOVED! WOW!!!!
        {
          for (k=0;k<2;k++)
          {
            if ( JSGetButtonState(&jsd, k) == 1 ) // trigger
            {
              gettimeofday(&tv,NULL);
              tick_trigger[k]=tv.tv_sec; 
              if (abs(tick_trigger[k]-last_trigger[k])>=1)
              {
                trigger[k]=1;
                last_trigger[k]=tick_trigger[k];
              }
            }
          }
          if (trigger[0]==1)
          {
              double rd=matRandNumber();

              /*if (rd<1.0/6)
              {
                t[0]=148.98;
                t[1]=4.14;
                t[2]=285.54;
              }
              else if (rd<1.0/6*2)
              {
                t[0]=323.43;
                t[1]=15.75;
                t[2]=226.72;
              }
              else if (rd<1.0/6*3)
              {
                t[0]=88.86;
                t[1]=2.90;
                t[2]=208.67;
              }
              else if (rd<1.0/6*4)
              {
                t[0]=76.93;
                t[1]=2.08;
                t[2]=136.63;
              }
              else if (rd<1.0/6*5)
              {
                t[0]=17.21;
                t[1]=5.18;
                t[2]=-26.39;
              }
              else
              {
                t[0]=-71.10;
                t[1]=27.77;
                t[2]=148.98;
              }*/
              pitch_sum=0;
              yaw_sum=0;
              dY_sum=0;
            
              camSetExternalParams(cam,t,w);
          }
          if (trigger[1]==1)
          {
              if (pitch_on==0) pitch_on=1;
              else pitch_on=0;
          }
             
          // axis 0 = roll
          // axis 1 = pitch
          // axis 2 = yaw
          // axis 3 = throttle
          // axis 4 = hat switch horizontal
          // axis 5 = hat switch vertical

          pitch    = -JSGetAxisCoeffNZ( &jsd, 3 ) / 200.0;
          yaw      = JSGetAxisCoeff( &jsd, 2 ) / 50.0;
          sidestep     = JSGetAxisCoeff( &jsd, 0 ) * 20.0;
          throttle       = -JSGetAxisCoeff( &jsd, 1 ) * 20.0;
          dY       = JSGetAxisCoeff( &jsd, 5 ) * 20.0;

          //printf( "(r, p, y) = (%2f, %2f, %2f); throttle = %2f; hat = (%f)\n",roll, pitch, yaw, throttle, dY );
        }
        if (pitch_on==0)
        {
          pitch=0;
        }
        update_camera_position(cam, 1.0, pitch, yaw, throttle, sidestep, dY );

        // prepare information to send
        mat4Transpose(cam->M,M);
        sprintf(msg,"camera viewmatrix %d_%d_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f_%f",0,0,yaw_sum,
          M[0],M[1],M[2],M[3],M[4],M[5],M[6],M[7],M[8],M[9],M[10],M[11],M[12],M[13],M[14],M[15]); 

        //printf("sending '%s'\n",msg);
        //send_string_unsafe(&bmc,(unsigned char *)msg);
        pdSend(&pd_send,"localhost",PD_PLAYLIST_PORT,msg);

        frame_time=1000000/refresh_rate; // en usec
        gettimeofday(&tv,NULL);
        usleep_time=frame_time-tv.tv_usec%frame_time;
        if (usleep_time>100 ) usleep(usleep_time);
        //printf("time=%d.%d sleep=%d\n",tv.tv_sec,tv.tv_usec,usleep_time);
    }


    // close the joystick... 
    //printf("closing joystick...\n");
    JSClose(&jsd);

    //printf("closing bmc...\n");
    //uninit_bimulticast(&bmc);
    pdClose(&pd_send);

    camFree(&cam);

    //printf("bye!\n");
    return 0;
}



