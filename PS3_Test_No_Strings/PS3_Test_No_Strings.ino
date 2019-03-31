// =======================================================================================
//                     PS3 Test Sketch for Notre Dame Droid Class
// =======================================================================================
//                          Last Revised Date: 03/22/2018
//                             Revised By: Prof McLaughlin
// =======================================================================================
// ---------------------------------------------------------------------------------------
//                          Libraries
// ---------------------------------------------------------------------------------------
#include <PS3BT.h>
#include <usbhub.h>
#include <Servo.h>
#include <MP3Trigger.h>
#include<Sabertooth.h>

// ---------------------------------------------------------------------------------------
//                       Debug - Verbose Flags
// ---------------------------------------------------------------------------------------
#define SHADOW_DEBUG       //uncomment this for console DEBUG output

// ---------------------------------------------------------------------------------------
//                 Setup for USB, Bluetooth Dongle, & PS3 Controller
// ---------------------------------------------------------------------------------------
//MP3Trigger MP3Trigger;
USB Usb;
BTD Btd(&Usb);
PS3BT *PS3Controller=new PS3BT(&Btd);
Servo myservo;

byte domespeed = 127; // Max value is 127 (Motor controller takes input from -127 to 127)
#define SYREN_ADDR 129 // Address of Dome Controller – shared Serial Port (DIP SWITCHES!!)
Sabertooth *SyR=new Sabertooth(SYREN_ADDR, Serial2);

byte drivespeed1 = 127; //For Speed Setting (Normal): set this to whatever speeds works for you. 0-stop, 127-full speed.
byte turnspeed = 75; // Recommend beginner: 40 to 50, experienced: 50+, I like 75
byte driveDeadBandRange = 10; // Used to set the Sabertooth DeadZone for foot motors
#define SABERTOOTH_ADDR 128 // Serial Address for Foot Sabertooth (Dip Switches!)
Sabertooth *ST=new Sabertooth(SABERTOOTH_ADDR, Serial2); // SAME Serial Port as the Dome Motor





int leftX = 0;
int rightX = 0;
int leftY = 0;
int rightY = 0;
int leftRampX = 0;
int rightRampX = 0;
int leftRampY = 0;
int rightRampY = 0;

// Dome globals
byte domeRotationSpeed = 0;

// Leg globals
byte turnnum = 0;
byte footDriveSpeed = 0;

int servoAngle = 90;
// ---------------------------------------------------------------------------------------
//    Output String for Serial Monitor Output
// ---------------------------------------------------------------------------------------
char output[300];

// ---------------------------------------------------------------------------------------
//    Deadzone range for joystick to be considered at nuetral
// ---------------------------------------------------------------------------------------
byte joystickDeadZoneRange = 15;

// ---------------------------------------------------------------------------------------
//    Used for PS3 Fault Detection
// ---------------------------------------------------------------------------------------
uint32_t msgLagTime = 0;
uint32_t lastMsgTime = 0;
uint32_t currentTime = 0;
uint32_t lastLoopTime = 0;
int badPS3Data = 0;

boolean isPS3ControllerInitialized = false;
boolean mainControllerConnected = false;
boolean WaitingforReconnect = false;
boolean isFootMotorStopped = true;

// ---------------------------------------------------------------------------------------
//    Used for PS3 Controller Click Management
// ---------------------------------------------------------------------------------------
long previousMillis = millis();
long domeMillis = millis();
long legMillis = millis();
long lStickMillis = millis();
long rStickMillis = millis();

boolean extraClicks = false;

// =======================================================================================
//                                 Main Program
// =======================================================================================
// =======================================================================================
//                          Initialize - Setup Function
// =======================================================================================
void setup()
{
  
    //Debug Serial for use with USB Debugging
    Serial.begin(115200);
    while (!Serial);
    
    if (Usb.Init() == -1)
    {
        Serial.print(F("\r\nOSC did not start"));
        while (1); //halt
    }

    Serial.print(F("\r\nBluetooth Library Started"));

    strcpy(output, "");
    

    Serial2.begin(9600);
    while (!Serial2);


    // Leg stuff
    ST->autobaud();
    ST->setTimeout(10); // 100ms increments
    ST->setDeadband(driveDeadBandRange); 

    // Dome stuff
    SyR->autobaud();
    SyR->setTimeout(20); // 100ms increments
    //SyR->stop();
    
    //Setup for PS3 Controller
    PS3Controller->attachOnInit(onInitPS3Controller); // onInitPS3Controller is called upon a new connection

    // Setup MP3
    //MP3Trigger.setup(&Serial1);
    //Serial1.begin(MP3Trigger::serialRate());
    
    //myservo.attach(13);
    //myservo.write(servoAngle);  // set servo to mid-point

    Serial.println("Setup Complete");
}

// =======================================================================================
//           Main Program Loop - This is the recurring check loop for entire sketch
// =======================================================================================
void loop()
{   
    if ( !readUSB() )
    {
      //We have a fault condition that we want to ensure that we do NOT process any controller data!!!
      printOutput(output);
      return;
    }
    
    checkController();
  
   if (extraClicks)
   {
      if ((previousMillis + 500) < millis())
      {
          extraClicks = false;
      }
   }
   // Dome movement
   if((domeMillis + 500) < millis()) {
    domeRotation();
    domeMillis = millis();
   }
   
   // Leg movement
   if((legMillis + 20) < millis()) {
    //legMovement();
    legMillis = millis();

   // SyR->motor(-100);
   // Serial.println("Dome Motor: -100");

   }
   // MP3 Stuff
   
  // MP3Trigger.update();
   printOutput(output);

}

// =======================================================================================
//          Check Controller Function to show all PS3 Controller inputs are Working
// =======================================================================================
void domeRotation() {
  char sRampY[5], sY[5];
  
  if(rightRampY > rightY){
    rightRampY-=2;
  }
  if(rightRampY<rightY) {
    rightRampY+=2;
  }
  itoa(rightRampY, sRampY, 10);
  itoa(rightY, sY, 10);
  
  strcat(output, "Current Y: ");
  strcat(output, sY);
  strcat(output, "\r\n");
  
  strcat(output, "Ramp Y");
  strcat(output, sRampY);
  strcat(output, "\r\n");

  domeRotationSpeed = (map(rightRampY, 0, 255, -domespeed, domespeed));
  SyR->motor(domeRotationSpeed);
  //SyR->stop();
}

void legMovement() {
  char sLeftRampX[5], sLeftX[5], sLeftRampY[5], sLeftY[5];
  if(leftRampX < leftX)
    leftRampX+=2;
  if(leftRampX > leftX)
    leftX-=2;
  if(leftRampY > leftY)
    leftRampY-=2;
  if(leftRampY < leftY)
    leftRampY+=2;

  itoa(leftRampY, sLeftRampY, 10);
  itoa(leftY, sLeftY, 10);
  itoa(leftRampX, sLeftRampX, 10);
  itoa(leftX, sLeftX, 10);

  strcat(output, "Current Left Y: ");
  strcat(output, sLeftY);
  strcat(output, "\r\n");
  
  strcat(output, "Left Ramp Y");
  strcat(output, sLeftRampY);
  strcat(output, "\r\n");

  
  turnnum = (map(leftRampX, 0, 255, -turnspeed,turnspeed));
  footDriveSpeed = (map(leftRampY, 0, 255, -drivespeed1,drivespeed1));

  /*String turnnumString;
  String fdsString;
  itoa(turnnum, turnnumString, 10);
  itoa(*/
 // strcat(output, "turnNum =");
  ST->turn(turnnum);
  ST->drive(footDriveSpeed);
  //SyR->stop();
}
void checkController()
{
       if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(UP) && !extraClicks)
     {              
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: UP Selected.\r\n");
            #endif
            servoAngle = servoAngle + 90;
            myservo.write(servoAngle);
            
            previousMillis = millis();
            extraClicks = true;
            
     }
  
     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(DOWN) && !extraClicks)
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: DOWN Selected.\r\n");
            #endif

            servoAngle = servoAngle - 90;
            myservo.write(servoAngle);
            
            previousMillis = millis();
            extraClicks = true;
       
     }

     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(LEFT) && !extraClicks)
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: LEFT Selected.\r\n");
            #endif  
            
            previousMillis = millis();
            extraClicks = true;

     }
     
     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(RIGHT) && !extraClicks)
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: RIGHT Selected.\r\n");
            #endif       
            
            previousMillis = millis();
            extraClicks = true;
                     
     }
     
     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(CIRCLE) && !extraClicks)  // Play noise here
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: CIRCLE Selected.\r\n");
            #endif      
            
            previousMillis = millis();
            extraClicks = true;

//            MP3Trigger.trigger(8);
     }

     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(CROSS) && !extraClicks)
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: CROSS Selected.\r\n");
            #endif       
            
            previousMillis = millis();
            extraClicks = true;
              
     }
     
     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(TRIANGLE) && !extraClicks)
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: TRIANGLE Selected.\r\n");
            #endif       
            
            previousMillis = millis();
            extraClicks = true;
              
     }
     

     if (PS3Controller->PS3Connected && PS3Controller->getButtonPress(SQUARE) && !extraClicks)
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: SQUARE Selected.\r\n");
            #endif       
            
            previousMillis = millis();
            extraClicks = true;
              
     }
     
     if (PS3Controller->PS3Connected && !extraClicks && PS3Controller->getButtonPress(L1))
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: LEFT 1 Selected.\r\n");
            #endif       
            
            previousMillis = millis();
            extraClicks = true;
     }

     if (PS3Controller->PS3Connected && !extraClicks && PS3Controller->getButtonPress(L2))
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: LEFT 2 Selected.\r\n");
            #endif       
            
            previousMillis = millis();
            extraClicks = true;
     }

     if (PS3Controller->PS3Connected && !extraClicks && PS3Controller->getButtonPress(R1))
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: RIGHT 1 Selected.\r\n");
            #endif       
            
            previousMillis = millis();
            extraClicks = true;
     }

     if (PS3Controller->PS3Connected && !extraClicks && PS3Controller->getButtonPress(R2))
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: RIGHT 2 Selected.\r\n");
            #endif       
            
            previousMillis = millis();
            extraClicks = true;
     }

     if (PS3Controller->PS3Connected && !extraClicks && PS3Controller->getButtonPress(SELECT))
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: SELECT Selected.\r\n");
            #endif       
            
            previousMillis = millis();
            extraClicks = true;
     }

     if (PS3Controller->PS3Connected && !extraClicks && PS3Controller->getButtonPress(START))
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: START Selected.\r\n");
            #endif       
            
            previousMillis = millis();
            extraClicks = true;
     }

     if (PS3Controller->PS3Connected && !extraClicks && PS3Controller->getButtonPress(PS))
     {
            #ifdef SHADOW_DEBUG
                strcat(output, "Button: PS Selected.\r\n");
            #endif       
            
            previousMillis = millis();
            extraClicks = true;
     }

     if (PS3Controller->PS3Connected && ((abs(PS3Controller->getAnalogHat(LeftHatY)-128) > joystickDeadZoneRange) || (abs(PS3Controller->getAnalogHat(LeftHatX)-128) > joystickDeadZoneRange)))
     {
            
            leftY = PS3Controller->getAnalogHat(LeftHatY) - 128;
            leftX = PS3Controller->getAnalogHat(LeftHatX) - 128;
            
            char yString[5];
            itoa(leftY, yString, 10);

            char xString[5];
            itoa(leftX, xString, 10);

            #ifdef SHADOW_DEBUG
                strcat(output, "LEFT Joystick Y Value: ");
                strcat(output, yString);
                strcat(output, "\r\n");
                strcat(output, "LEFT Joystick X Value: ");
                strcat(output, xString);
                strcat(output, "\r\n");
            #endif   

            lStickMillis = millis();
     }
     else {
        if((lStickMillis + 500) < millis()) {
           leftY = 0;
           leftX = 0;
        }
     }

     if (PS3Controller->PS3Connected && ((abs(PS3Controller->getAnalogHat(RightHatY)-128) > joystickDeadZoneRange) || (abs(PS3Controller->getAnalogHat(RightHatX)-128) > joystickDeadZoneRange)))
     {
            rightY = PS3Controller->getAnalogHat(RightHatY) - 128;
            rightX = PS3Controller->getAnalogHat(RightHatX) - 128;

            char yString[5];
            itoa(rightY, yString, 10);

            char xString[5];
            itoa(rightX, xString, 10);

            #ifdef SHADOW_DEBUG
                //strcat(output, "RIGHT Joystick Y Value: ");
                //strcat(output, yString);
                //strcat(output, "\r\n");
                //strcat(output, "RIGHT Joystick X Value: ");
                //strcat(output, xString);
                //strcat(output, "\r\n");
            #endif  

            rStickMillis = millis();
     }
      else {
        if((rStickMillis + 500) < millis()) {
           rightY = 0;
           rightX = 0;
        }
     }
}



// =======================================================================================
//           PPS3 Controller Device Mgt Functions
// =======================================================================================
// =======================================================================================
//           Initialize the PS3 Controller Trying to Connect
// =======================================================================================
void onInitPS3Controller()
{
    PS3Controller->setLedOn(LED1);
    isPS3ControllerInitialized = true;
    badPS3Data = 0;

    mainControllerConnected = true;
    WaitingforReconnect = true;

    #ifdef SHADOW_DEBUG
       strcat(output, "\r\nWe have the controller connected.\r\n");
    #endif
}

// =======================================================================================
//           Determine if we are having connection problems with the PS3 Controller
// =======================================================================================
boolean criticalFaultDetect()
{
    if (PS3Controller->PS3Connected)
    {
        
        currentTime = millis();
        lastMsgTime = PS3Controller->getLastMessageTime();
        msgLagTime = currentTime - lastMsgTime;            
        
        if (WaitingforReconnect)
        {
            
            if (msgLagTime < 200)
            {
             
                WaitingforReconnect = false; 
            
            }
            
            lastMsgTime = currentTime;
            
        } 
        
        if ( currentTime >= lastMsgTime)
        {
              msgLagTime = currentTime - lastMsgTime;
              
        } else
        {

             msgLagTime = 0;
        }
        
        if (msgLagTime > 300 && !isFootMotorStopped)
        {
            #ifdef SHADOW_DEBUG
              strcat(output, "It has been 300ms since we heard from the PS3 Controller\r\n");
              strcat(output, "Shut down motors and watching for a new PS3 message\r\n");
            #endif
            
//          You would stop all motors here
            isFootMotorStopped = true;
        }
        
        if ( msgLagTime > 10000 )
        {
            #ifdef SHADOW_DEBUG
              strcat(output, "It has been 10s since we heard from Controller\r\n");
              strcat(output, "\r\nDisconnecting the controller.\r\n");
            #endif
            
//          You would stop all motors here
            isFootMotorStopped = true;
            
            PS3Controller->disconnect();
            WaitingforReconnect = true;
            return true;
        }

        //Check PS3 Signal Data
        if(!PS3Controller->getStatus(Plugged) && !PS3Controller->getStatus(Unplugged))
        {
            //We don't have good data from the controller.
            //Wait 15ms - try again
            delay(15);
            Usb.Task();   
            lastMsgTime = PS3Controller->getLastMessageTime();
            
            if(!PS3Controller->getStatus(Plugged) && !PS3Controller->getStatus(Unplugged))
            {
                badPS3Data++;
                #ifdef SHADOW_DEBUG
                    strcat(output, "\r\n**Invalid data from PS3 Controller. - Resetting Data**\r\n");
                #endif
                return true;
            }
        }
        else if (badPS3Data > 0)
        {

            badPS3Data = 0;
        }
        
        if ( badPS3Data > 10 )
        {
            #ifdef SHADOW_DEBUG
                strcat(output, "Too much bad data coming from the PS3 Controller\r\n");
                strcat(output, "Disconnecting the controller and stop motors.\r\n");
            #endif
            
//          You would stop all motors here
            isFootMotorStopped = true;
            
            PS3Controller->disconnect();
            WaitingforReconnect = true;
            return true;
        }
    }
    else if (!isFootMotorStopped)
    {
        #ifdef SHADOW_DEBUG      
            strcat(output, "No PS3 controller was found\r\n");
            strcat(output, "Shuting down motors and watching for a new PS3 message\r\n");
        #endif
        
//      You would stop all motors here
        isFootMotorStopped = true;
        
        WaitingforReconnect = true;
        return true;
    }
    
    return false;
}

// =======================================================================================
//           USB Read Function - Supports Main Program Loop
// =======================================================================================
boolean readUSB()
{
  
     Usb.Task();
     
    //The more devices we have connected to the USB or BlueTooth, the more often Usb.Task need to be called to eliminate latency.
    if (PS3Controller->PS3Connected) 
    {
        if (criticalFaultDetect())
        {
            //We have a fault condition that we want to ensure that we do NOT process any controller data!!!
            printOutput(output);
            return false;
        }
        
    } else if (!isFootMotorStopped)
    {
        #ifdef SHADOW_DEBUG      
            strcat(output, "No controller was found\r\n");
            strcat(output, "Shuting down motors, and watching for a new PS3 foot message\r\n");
        #endif
        
//      You would stop all motors here
        isFootMotorStopped = true;
        
        WaitingforReconnect = true;
    }
    
    return true;
}

// =======================================================================================
//          Print Output Function
// =======================================================================================

void printOutput(const char *value)
{
    if ((strcmp(value, "") != 0))
    {
        if (Serial) Serial.println(value);
        strcpy(output, ""); // Reset output string
    }
}
