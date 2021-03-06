// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// leftEncoder          encoder       A, B            
// rightEncoder         encoder       C, D            
// ---- END VEXCODE CONFIGURED DEVICES ----

/*//////////////////////////////////////////////////////////////////////////

BUILTINS FOR COMPETITION FORMAT

//////////////////////////////////////////////////////////////////////////*/

#include "vex.h"
#include <math.h>
#include <string.h>
#include "VisionSensor.h"
using namespace vex;

// A global instance of competition
competition Competition;

// define your global instances of motors and other devices here
double ROBOTCIRCUMFRENCE = 58.12;
double DEGREESPERINCH = 360 / 12.56;

/*//////////////////////////////////////////////////////////////////////////

PRE-AUTON (INCLUDING ALL FUNCTIONS)

//////////////////////////////////////////////////////////////////////////*/

void pre_auton(void) {
  // I0nitializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();
  // frontClamp.set(true);
  // Drivetrain.setStopping(brake);
  // Brain.Screen.setFont(propXXL);
  // Brain.Screen.setCursor(1, 2);
  // Brain.Screen.setFillColor(transparent);
  // Brain.Screen.setPenColor(green);
  // Brain.Screen.setPenWidth(10);
  // Brain.Screen.print("6096A");
  // Brain.Screen.setCursor(1, 11);
  // Brain.Screen.setPenColor(red);
  // Brain.Screen.print("6096A");
  // Brain.Screen.setCursor(5, 4);
  // Brain.Screen.setPenColor(white);
  // Brain.Screen.print("ROBORAYS1");
  // Drivetrain.drive(forward, 1, rpm);
}

/*//////////////////////////////////////////////////////////////////////////

AUTONOMOUS (INCLUDING ALL FUNCTIONS)

//////////////////////////////////////////////////////////////////////////*/
void print(double val) {
  Controller1.Screen.clearScreen();
  Controller1.Screen.setCursor(1, 1);
  Controller1.Screen.print(val);
}
//PID FUNCTIONS


// OPTIMIZE
int timestep = 1;  // Delay for PID, in ms
// OPTIMIZE
double k = 150;  // Maximum amount drivetrain may increase/decrease power for PID
// OPTIMIZE
double kVis = 10;  // Maximum amount vision sensor may increase/decrease power
int maxVel = 600 - k;  // Maximum velocity that we're allowed to move
// OPTIMIZE
double ACC_RATE = 100;  // Maximum amount of acceleration (rpm / s)
double vel = 0;  // Velocity for PID (reset in move)
double targetPos = 0;  // Position we should be at in PID
// OPTIMIZE
double DEC_RATE = .5;  // Rate of deceleration with braking scheme (rpm / s)

double leftPos() {
  double sum = 0;
  for (int i = 0; i < 20; i ++) {
    double a = LB.position(degrees);
    double b = LM.position(degrees);
    double c = LF.position(degrees);
    sum += (a + b + c) / 3;
  }
  return sum / 20;
}

double rightPos() {
  double sum = 0;
  for (int i = 0; i < 20; i ++) {
    double a = RB.position(degrees);
    double b = RM.position(degrees);
    double c = RF.position(degrees);
    sum += (a + b + c) / 3;
  }
  return sum / 20;
}

double leftVel() {
  double sum = 0;
  for (int i = 0; i < 20; i ++) {
    double a = LB.velocity(rpm);
    double b = LM.velocity(rpm);
    double c = LF.velocity(rpm);
    sum += (a + b + c) / 3;
  }
  return sum / 20;
}

double rightVel() {
  double sum = 0;
  for (int i = 0; i < 20; i ++) {
    double a = RB.velocity(rpm);
    double b = RM.velocity(rpm);
    double c = RF.velocity(rpm);
    sum += (a + b + c) / 3;
  }
  return sum / 20;
}

void resetMotorEncoders() {
  leftDrive.setPosition(0, degrees);
  rightDrive.setPosition(0, degrees);
}

int visionSample(vision::signature sig, int n) {
  int sum = 0;
  for (int i = 0; i < n; i ++) {
    AllSeeingEye.takeSnapshot(sig);
    sum += AllSeeingEye.largestObject.centerX;
  }
  return sum / n;
}

double average(double a, double b) {  // Average of two numbers
  return (a + b) / 2;
}

double degreesToDistance(double deg) {  // Convert a degree value of an encoder to a distance
  return (deg / 360) * 3.25 * .6 * 3.14;
}

double correct(double lead) {  // Converts a lead in inches to a velocity correction
  return tanh(lead / 2) * k;
}

double visionCorrect(int x) {  // Converts a vision sensor deviation from target center x to left drive correction
  return x / 128 * kVis;
}

double stopDistance() {  // Calculate how much distance we need to fully stop the robot
  // Simulate a stopping action
  // double d = 0;  // Distance we travelled so far
  // // double v = average(leftVel(), rightVel());
  // double v = vel;
  // while (v > 0) {
  //   d += degreesToDistance(v * timestep / 1000 / 60 * 360);
  //   v -= DEC_RATE * timestep;
  // }
  // return d;
  return 0;
}

bool canStop(double dist) {  // If we have enough distance left to stop our robot
  return dist - degreesToDistance(average(leftPos(), rightPos())) > stopDistance();
}

void stop() {  // Custom braking scheme
  // while (average(leftEncoder.velocity(rpm), rightEncoder.velocity(rpm)) > 0) {
  //   Drivetrain.stop(brake);
  //   wait(1, msec);
  //   Drivetrain.drive(forward, average(leftEncoder.velocity(rpm), rightEncoder.velocity(rpm)), rpm);
  //   wait(30, msec);
  
  // }
  // Drivetrain.stop(brake);
  // while (vel > 0) {
  //   vel -= DEC_RATE;
  //   leftDrive.spin(forward, vel, rpm);
  //   rightDrive.spin(forward, vel, rpm);
  //   // Drivetrain.setDriveVelocity(vel, rpm);
  //   // Drivetrain.drive(forward, vel, rpm);
  //   wait(timestep, msec);
  // }
  Drivetrain.stop(brake);
}

double alignNeutral() {  // Correct motion to align to neutral goal
  return visionCorrect(visionSample(NEUTRAL, 100) - 128);
}

double alignRed() {  // Correct motion to align to red alliance goal
  return visionCorrect(visionSample(RED_ALLIANCE, 100) - 128);
}

double alignBlue() {  // Correct motion to align to blue alliance goal
  return visionCorrect(visionSample(BLUE_ALLIANCE, 100) - 128);
}

// Drive forward a given distance with PID and acceleration curve
// Optional correction function which can modify drivetrain
void move(double dist, double (*correctionFunc)() = []()->double{return 0;}) {
  // Reset encoder positions
  // leftEncoder.resetRotation();
  // rightEncoder.resetRotation();
  resetMotorEncoders();

  vel = 0;  // Reset velocity
  targetPos = 0;  // Reset target position

  while (canStop(dist)) {  // While we have enough space left to stop our robot
    // double leftLead = targetPos - degreesToDistance(leftPos());
    // double rightLead = targetPos - degreesToDistance(rightPos());

    double rightLead = degreesToDistance(rightPos() - leftPos());

    double correction = correct(rightLead) / 2;

    // double leftVel = vel + correct(leftLead) + correctionFunc() / 2;  // Divide correctionFunc() by 2 to equally correct both sides
    // double rightVel = vel + correct(rightLead) - correctionFunc() / 2;
    double leftVel = vel + correction + correctionFunc() / 2;
    double rightVel = vel - correction + correctionFunc() / 2;

    vel = average(leftVel, rightVel);

    print(leftVel - rightVel);
    // targetPos += average(leftVel, rightVel) * timestep;
    targetPos += vel * timestep;

    leftDrive.spin(forward, leftVel, rpm);
    rightDrive.spin(forward, rightVel, rpm);

    if (vel + ACC_RATE <= maxVel)
      vel += ACC_RATE;
    else
      vel = maxVel;
    wait(timestep, msec);
  }
  stop();
}

void autonomous(void) {
  /*///////////////////////////////////
    1 DEGREE = 0.0239982 inches

    1 INCH = 41.6697919 degrees
  ///////////////////////////////////*/
  
 move(44);
 


  
}

/*//////////////////////////////////////////////////////////////////////////

USERCONTROL (INCLUDING ALL FUNCTIONS)

//////////////////////////////////////////////////////////////////////////*/

void driveUsercontrol() {  // Tank drive
  while (true) {
    leftDrive.spin(forward, Controller1.Axis3.value(),  pct);
    rightDrive.spin(forward, Controller1.Axis2.value(), pct);
    
    
    
  }
}

// void fourBarUsercontrol() {  // FourBar - Axis 2 (Right Joystick)
//   if (Controller2.Axis2.value() > 2 || Controller2.Axis2.value() < -2) {
//     FourBar.spin(forward, -Controller2.Axis2.value(), pct);
//   } else {
//     FourBar.spin(forward, 0, pct);
//   }
// }

int x=0, y=1;

int ConveyorValue=0;

bool open=true;
void frontClamp1()
{
 open=!open;
}
 


// void fourBarConveyorUserControl() {  // FourBar&Conveyor - Axis 2 (Right Joystick)
//   if (Controller2.Axis2.value() > 0) {
//     FourBar.spin(forward, -Controller2.Axis2.value(), pct);
//     FourBarConveyor.spin(forward, -Controller2.Axis2.value(), pct);
//   } else if(Controller2.Axis2.value() < 0) {
//     FourBar.spin(forward, 0, pct);
//   }
// }

void frontClampUserControl(){
if(Controller2.ButtonR1.pressing()){
  frontClamp.set(true);
}
else if(Controller2.ButtonR2.pressing()){
  frontClamp.set(false);
}
}

void ConveyorUserControl(){
  if(Controller2.ButtonA.pressing()){
    ConveyorValue=1;
  }
  if(Controller2.ButtonX.pressing()){
    ConveyorValue=0;
  }
  if(abs(Controller2.Axis2.value()) < 2){
    if(ConveyorValue==1){
      FourBarConveyor.spin(reverse);
    }
    else if(ConveyorValue==0){
      FourBarConveyor.stop();
    }
  }
}

void fourBarUsercontrol(){
if (Controller2.Axis2.value() > 0) {
    FourBar.spin(forward, Controller2.Axis2.value(), pct);
    FourBarConveyor.spin(forward, Controller2.Axis2.value(), pct);
  } else if(Controller2.Axis2.value() < 0) {
    FourBar.spin(forward, 0, pct);
  }
else{
  FourBar.spin(forward, 0, pct);
  FourBarConveyor.spin(forward, 0, pct);
}
}






void backClampUserControl(){

if(Controller2.ButtonL2.pressing()){
   vex::digital_out backClamp1(Brain.ThreeWirePort.G);
    backClamp1.set(true);
  }
  else if(Controller2.ButtonL1.pressing()){
     vex::digital_out backClamp1(Brain.ThreeWirePort.G);
    backClamp1.set(false);
  }
}



void usercontrol(void) {
  // Runs drive code as a thread so we can always control the robot
  thread(driveUsercontrol).detach();
  Drivetrain.setStopping(coast);
  
  while (true) {
    if(Controller2.ButtonR2.pressing()){
      vex::digital_out frontClamp(Brain.ThreeWirePort.A);
      frontClamp.set(true);
    }

    else if(Controller2.ButtonR1.pressing()){
      vex::digital_out frontClamp(Brain.ThreeWirePort.A);
      frontClamp.set(false);
    }
    if(Controller2.ButtonL1.pressing()){
      vex::digital_out backClamp1(Brain.ThreeWirePort.G);
      backClamp1.set(true);

    }

    else if(Controller2.ButtonR1.pressing()){
      vex::digital_out backClamp1(Brain.ThreeWirePort.G);
      backClamp1.set(false);
    }
    
    fourBarUsercontrol();  // Controls four bar up and down with contorller 2
    Controller1.Screen.clearScreen();
    Controller1.Screen.setCursor(1, 2);
    // Controller1.Screen.print(visionSample(NEUTRAL, 50));
    // backClampUserControl();
  }
}

int main() {
  // Set up callbacks for autonomous and driver control periods.
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);
  // Run the pre-autonomous function.
  pre_auton();
  // Prevent main from exiting with an infinite loop.
}
