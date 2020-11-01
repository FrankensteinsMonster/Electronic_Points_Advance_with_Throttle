//pin in & out, and sparkDelay

const int coil = 12;                  //Transistor that operates coil was on pin 12
volatile int pointsValue = A0;        //Points sensor was on anaologue sensor pin 0, the first pin
volatile int throttleValue = A1;      //throttle position potentiometer was connected to pin A1, the sencond pin on the board available to me.
int throttle14 = 256;
int throttle12 = 512;                 // When the poteniometer was reading in 0-1023 resolution (from a value of 0 through to 1023 graduations as less resistance was felt)
int throttle34 = 768;                 // from the 5V in on one side, ground on other, middle pin of potentiometer went to A1 pin on board...
int throttle11 = 920;                 // i set these as the 4 throttle loadings.  You can have as many as you like & replace the timing chart(s) with mathamatical division values if your application needs it.
volatile int throttlePzT1 = true;
volatile int throttlePzT2 = false;
volatile int throttlePzT3 = false;
volatile int throttlePzT4 = false;
volatile int throttlePzTWoT = false;
int sensorValue = 0;
int starterDelay = 1000;
int sensorThreshold = 300;
int sensorMax = 0; 

volatile int coilState = LOW;
volatile int coilPin = LOW;
volatile int trigger = false;
volatile int sparkTriggerTimeSet = false;
volatile int sparkTriggerDelay = 10;               //added this to try & see if i can delay spark at low RPM.  Should be 0 for starting with lever retarded.  More modern engines wouldn't use a lever. In which case, retard as nessary.
volatile int coilPrecharge = LOW;
volatile unsigned long timestamp1 = 0;
volatile unsigned long timestamp2 = 0;               //Was setting this to "micros" to compare to
volatile unsigned long timestamp3 = 0;              //this one, also in micros.  Worked in the model T.  Not here?
volatile unsigned long rpmCurrMicros= 170000;
volatile unsigned long rpmOldMicros = 0;
volatile int sensorTriggerState = false;         // current state of the button
volatile int lastsensorTriggerState = true;     // previous state  of the button/sensor in this case.
volatile int sparkCount = 0;                   //count how many times you fired the coil

volatile long dwell = 10;                // but this is how long the "points" are open, open longer reduces duty cycle of coils.
volatile long coilCharge = 5;           // interval at which to charge coil (milliseconds)
//Change from 'const long' to 'volatile long' if you want to re-write the time for the spark as speed changes

volatile unsigned long startingDelayTimer = 0;
volatile int startingDelay = false;

void setup()
{
  pinMode(coil, OUTPUT);
  pinMode(A0, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.print ( " Booting up...  " );
}

void loop()
{
pointsValue = analogRead(A0);
throttleValue = analogRead(A1);

  if ((throttleValue) <= (throttle14))
  {
    throttlePzT1 = true;
    throttlePzT2 = false;
    throttlePzT3 = false;
    throttlePzT4 = false;
    throttlePzTWoT = false;
  }
  else if ((throttleValue) >= (throttle14) && (throttleValue) <= (throttle12))
  {
    throttlePzT1 = false;
    throttlePzT2 = true;
    throttlePzT3 = false;
    throttlePzT4 = false;
    throttlePzTWoT = false;  
  }
  else if ((throttleValue) >= (throttle12) && (throttleValue) <= (throttle34))
  {
    throttlePzT1 = false;
    throttlePzT2 = false;
    throttlePzT3 = true;
    throttlePzT4 = false;
    throttlePzTWoT = false;    
  }
  else if ((throttleValue) >= (throttle34) && (throttleValue) <= (throttle11))
  {
    throttlePzT1 = false;
    throttlePzT2 = false;
    throttlePzT3 = false;
    throttlePzT4 = true; 
    throttlePzTWoT = false;   
  }
  else if ((throttleValue) >= (throttle11))
  {
    throttlePzT1 = false;
    throttlePzT2 = false;
    throttlePzT3 = false;
    throttlePzT4 = false; 
    throttlePzTWoT = true;
  }

  
  if ((pointsValue) <= (sensorThreshold))
  {
   sparkCount = 0;
    if (trigger == true)
    {
    sensorTriggerState = false;
    if (sensorTriggerState != lastsensorTriggerState)
    {
      rpmOldMicros = rpmCurrMicros;
      rpmCurrMicros = micros();
            
      if (millis() >= (startingDelayTimer + 5000))
      {
        if ((rpmCurrMicros - rpmOldMicros <=  169491))         //over approx' 177RPM
        {
           startingDelay = true;                               //no longer hold timing advance-retard at full advance, hope you moved the timing lever by now.
        }
        if ((rpmCurrMicros - rpmOldMicros >= 497512))
        {
          startingDelay = false;                               //chances are you have to re-start the engine now. it'll take 10 seconds or more at over 200RPM to act normal again.
          startingDelayTimer = millis();
        }
      }
      if (startingDelay == true)
      {
        if ((throttlePzT1) == (true))
        {
          timingChartThingy14();
          lastsensorTriggerState = false;
        }
        else if ((throttlePzT2) == (true))
        {
          timingChartThingy12();
          lastsensorTriggerState = false;
        }
        else if ((throttlePzT3) == (true))
        {
          timingChartThingy34();
          lastsensorTriggerState = false;
        }
        else if ((throttlePzT4) == (true))
        {
          timingChartThingy11();
          lastsensorTriggerState = false;
        }
        else if ((throttlePzTWoT) == (true))
        {
          timingChartThingyWoT();
          lastsensorTriggerState = false;
        }
      lastsensorTriggerState = false;
      }
      
      else
      {
       Serial.print ( " Starter delay period not ended " );
       lastsensorTriggerState = false;
      }
    }

   lastsensorTriggerState = false;           //Why do i need to make that 'false' so many times to avoid it doing weird things?
   trigger = false;
   sparkTriggerTimeSet = false;
    }
  }
  else if ((pointsValue) >= (sensorThreshold))
  {
    sensorTriggerState = true;
    trigger = true;
    if (lastsensorTriggerState == false)
    {
      sparkTriggerTimeSet = false;
      lastsensorTriggerState = true;
    }
    if (sparkCount <= 3)
      {
        timestamp1 = millis();
        coilState = LOW;
        coilPin = LOW;
        digitalWrite(coil, coilPrecharge);
        
        while (sparkTriggerTimeSet == false && coilState == LOW && coilPin == LOW && (timestamp1 + sparkTriggerDelay) > millis());  //completly baffled why this won't work with microseconds.  It did in the model T.

        sparkTriggerTimeSet = true;

         
        while (sparkTriggerTimeSet == true && coilState == HIGH && coilPin == HIGH && (timestamp1 + 0) > millis());                //Been chasing this lack of microseconds bug for months...
                                                                                                                                   //Is it honestly that hard?  I guess it is.  delayMicroseconds didn't work either.
         coilState = HIGH;                                                                                                         //i could change the coil charge up to suit, sorta, time i guess?  This is insane. Why won't it work?
         coilPin = HIGH;
         digitalWrite(coil, HIGH);
    
         timestamp1 = millis();
                                          
          while (sparkTriggerTimeSet == true && coilState == HIGH && coilPin == HIGH && (timestamp1 + coilCharge) > millis());  //Busy wait
   
          digitalWrite(coil, LOW);
          coilPin = LOW;
          timestamp1 = millis();

          while (sparkTriggerTimeSet == true && coilState == HIGH && coilPin == LOW && (timestamp1 + dwell) > millis());
  
          coilState = LOW;
          coilPin = LOW;
          sparkCount++;
      }
   }
}



void timingChartThingyWoT()
{
  //the timing delays and coil charge & dwell adjustments at speed can go here.  
  if (rpmCurrMicros - rpmOldMicros >= 497512) //sub250rpm
    {
      sparkTriggerDelay =1000;
      startingDelay = false;
      startingDelayTimer = millis();
      coilCharge = 8;
      dwell = 50;
      coilPrecharge = LOW;
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 255024) && (rpmCurrMicros - rpmOldMicros >= 169491)) //125rpm
    {
      sparkTriggerDelay =81;                     //was 57, then 59 and i don't know what overflow is but i get an error for it here.
      coilCharge = 8;
      dwell = 15;
      coilPrecharge = LOW;
      Serial.print ( " RPM 125" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 169491) && (rpmCurrMicros - rpmOldMicros >= 127512)) //177rpm
    {
      sparkTriggerDelay =61;
      coilCharge = 7;
      dwell = 10;
      coilPrecharge = LOW;
      Serial.print ( " RPM 177" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 127512) && (rpmCurrMicros - rpmOldMicros >= 85008)) //250rpm
    {
      sparkTriggerDelay =41;                    //was 27  //at 250 RPM and 45 degree sensor advance, this would be 30ms to TDC, subtract 2ms for coil charging and subtract more m.s. for advance 
      coilCharge = 7;
      dwell = 8;
      coilPrecharge = LOW;
      Serial.print ( " RPM 250" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 85008) && (rpmCurrMicros - rpmOldMicros >= 61000)) //375rpm
    {
      sparkTriggerDelay =31;                 //was 16
      coilCharge = 5;
      dwell = 5;
      coilPrecharge = LOW;
      Serial.print ( " RPM 375 " );
      }
  else if ((rpmCurrMicros - rpmOldMicros <= 61000) && (rpmCurrMicros - rpmOldMicros >= 48000)) //500rpm
    {
      sparkTriggerDelay =23;                 //was 11
      coilPrecharge = LOW;
      Serial.print ( " RPM 500 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 48000) && (rpmCurrMicros - rpmOldMicros >= 41000))  //625rpm
    {
      sparkTriggerDelay =3;
      coilPrecharge = HIGH;
      coilCharge = 2;
      dwell = 2;
      Serial.print ( " RPM 625 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 41000) && (rpmCurrMicros - rpmOldMicros >= 30000)) //750rpm
    {
      sparkTriggerDelay =1.6;
      coilCharge = 1;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 750" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 30000) && (rpmCurrMicros - rpmOldMicros >= 25000)) //1000rpm
    {
      sparkTriggerDelay =1;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 1000 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 25000) && (rpmCurrMicros - rpmOldMicros >= 23076)) //1200rpm
    {
      sparkTriggerDelay =0;
      coilCharge = 2;
      coilPrecharge = HIGH;
      Serial.print ( "RPM 1200 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 23076) && (rpmCurrMicros - rpmOldMicros >= 22222)) //just under 1300rpm
    {
      sparkTriggerDelay =0;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 1300 " );
    }
  else if (rpmCurrMicros - rpmOldMicros <= 22222)//just over 1300rpm
    {
      sparkTriggerDelay =0;
      coilPrecharge = HIGH;
      Serial.print ( "RPM over 1300" );
    }
}
void timingChartThingy14()
{
  //the timing delays and coil charge & dwell adjustments at speed can go here.  
  if (rpmCurrMicros - rpmOldMicros >= 497512) //sub250rpm
    {
      sparkTriggerDelay =50;
      startingDelay = false;
      startingDelayTimer = millis();
      coilCharge = 8;
      dwell = 50;
      coilPrecharge = LOW;
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 255024) && (rpmCurrMicros - rpmOldMicros >= 169491)) //125rpm
    {
      sparkTriggerDelay =81;                     //was 57, then 59 and i don't know what overflow is but i get an error for it here.
      coilCharge = 8;
      dwell = 15;
      coilPrecharge = LOW;
      Serial.print ( " RPM 125" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 169491) && (rpmCurrMicros - rpmOldMicros >= 127512)) //177rpm
    {
      sparkTriggerDelay =30;
      coilCharge = 7;
      dwell = 12;
      coilPrecharge = LOW;
      Serial.print ( " RPM 177" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 127512) && (rpmCurrMicros - rpmOldMicros >= 85008)) //250rpm
    {
      sparkTriggerDelay =21;                    //was 27  //at 250 RPM and 45 degree sensor advance, this would be 30ms to TDC, subtract 2ms for coil charging and subtract more m.s. for advance 
      coilCharge = 7;
      dwell = 10;
      coilPrecharge = LOW;
      Serial.print ( " RPM 250" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 85008) && (rpmCurrMicros - rpmOldMicros >= 61000)) //375rpm
    {
      sparkTriggerDelay =16;                 //was 16
      coilCharge = 5;
      dwell = 6;
      coilPrecharge = LOW;
      Serial.print ( " RPM 375 " );
      }
  else if ((rpmCurrMicros - rpmOldMicros <= 61000) && (rpmCurrMicros - rpmOldMicros >= 48000)) //500rpm
    {
      sparkTriggerDelay =11;                 //was 11
      coilPrecharge = LOW;
      Serial.print ( " RPM 500 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 48000) && (rpmCurrMicros - rpmOldMicros >= 41000))  //625rpm
    {
      sparkTriggerDelay =1.5;
      coilPrecharge = HIGH;
      coilCharge = 2;
      dwell = 2;
      Serial.print ( " RPM 625 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 41000) && (rpmCurrMicros - rpmOldMicros >= 30000)) //750rpm
    {
      sparkTriggerDelay =1;
      coilCharge = 1;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 750" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 30000) && (rpmCurrMicros - rpmOldMicros >= 25000)) //1000rpm
    {
      sparkTriggerDelay =0;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 1000 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 25000) && (rpmCurrMicros - rpmOldMicros >= 23076)) //1200rpm
    {
      sparkTriggerDelay =0;
      coilCharge = 2;
      coilPrecharge = HIGH;
      Serial.print ( "RPM 1200 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 23076) && (rpmCurrMicros - rpmOldMicros >= 22222)) //just under 1300rpm
    {
      sparkTriggerDelay =0;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 1300 " );
    }
  else if (rpmCurrMicros - rpmOldMicros <= 22222)//just over 1300rpm
    {
      sparkTriggerDelay =0;
      coilPrecharge = HIGH;
      Serial.print ( "RPM over 1300" );
    }
}
void timingChartThingy12()
{
  //the timing delays and coil charge & dwell adjustments at speed can go here.  
  if (rpmCurrMicros - rpmOldMicros >= 497512) //sub250rpm
    {
      sparkTriggerDelay =100;
      startingDelay = false;
      startingDelayTimer = millis();
      coilCharge = 8;
      dwell = 50;
      coilPrecharge = LOW;
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 255024) && (rpmCurrMicros - rpmOldMicros >= 169491)) //125rpm
    {
      sparkTriggerDelay =81;                     //was 57, then 59 and i don't know what overflow is but i get an error for it here.
      coilCharge = 8;
      dwell = 15;
      coilPrecharge = LOW;
      Serial.print ( " RPM 125" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 169491) && (rpmCurrMicros - rpmOldMicros >= 127512)) //177rpm
    {
      sparkTriggerDelay =41;
      coilCharge = 7;
      dwell = 12;
      coilPrecharge = LOW;
      Serial.print ( " RPM 177" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 127512) && (rpmCurrMicros - rpmOldMicros >= 85008)) //250rpm
    {
      sparkTriggerDelay =31;                    //was 27  //at 250 RPM and 45 degree sensor advance, this would be 30ms to TDC, subtract 2ms for coil charging and subtract more m.s. for advance 
      coilCharge = 7;
      dwell = 10;
      coilPrecharge = LOW;
      Serial.print ( " RPM 250" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 85008) && (rpmCurrMicros - rpmOldMicros >= 61000)) //375rpm
    {
      sparkTriggerDelay =21;                 //was 16
      coilCharge = 5;
      dwell = 6;
      coilPrecharge = LOW;
      Serial.print ( " RPM 375 " );
      }
  else if ((rpmCurrMicros - rpmOldMicros <= 61000) && (rpmCurrMicros - rpmOldMicros >= 48000)) //500rpm
    {
      sparkTriggerDelay =18;                 //was 11
      coilPrecharge = LOW;
      Serial.print ( " RPM 500 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 48000) && (rpmCurrMicros - rpmOldMicros >= 41000))  //625rpm
    {
      sparkTriggerDelay =8;
      coilPrecharge = HIGH;
      coilCharge = 2;
      dwell = 2;
      Serial.print ( " RPM 625 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 41000) && (rpmCurrMicros - rpmOldMicros >= 30000)) //750rpm
    {
      sparkTriggerDelay =5;
      coilCharge = 1;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 750" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 30000) && (rpmCurrMicros - rpmOldMicros >= 25000)) //1000rpm
    {
      sparkTriggerDelay =2;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 1000 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 25000) && (rpmCurrMicros - rpmOldMicros >= 23076)) //1200rpm
    {
      sparkTriggerDelay =1.6;
      coilCharge = 2;
      coilPrecharge = HIGH;
      Serial.print ( "RPM 1200 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 23076) && (rpmCurrMicros - rpmOldMicros >= 22222)) //just under 1300rpm
    {
      sparkTriggerDelay =1;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 1300 " );
    }
  else if (rpmCurrMicros - rpmOldMicros <= 22222)//just over 1300rpm
    {
      sparkTriggerDelay =0;
      coilPrecharge = HIGH;
      Serial.print ( "RPM over 1300" );
    }
}
void timingChartThingy34()
{
  //the timing delays and coil charge & dwell adjustments at speed can go here.  
  if (rpmCurrMicros - rpmOldMicros >= 497512) //sub250rpm
    {
      sparkTriggerDelay =110;
      startingDelay = false;
      startingDelayTimer = millis();
      coilCharge = 8;
      dwell = 50;
      coilPrecharge = LOW;
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 255024) && (rpmCurrMicros - rpmOldMicros >= 169491)) //125rpm
    {
      sparkTriggerDelay =81;                     //was 57, then 59 and i don't know what overflow is but i get an error for it here.
      coilCharge = 8;
      dwell = 15;
      coilPrecharge = LOW;
      Serial.print ( " RPM 125" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 169491) && (rpmCurrMicros - rpmOldMicros >= 127512)) //177rpm
    {
      sparkTriggerDelay =51;
      coilCharge = 7;
      dwell = 12;
      coilPrecharge = LOW;
      Serial.print ( " RPM 177" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 127512) && (rpmCurrMicros - rpmOldMicros >= 85008)) //250rpm
    {
      sparkTriggerDelay =44;                    //was 27  //at 250 RPM and 45 degree sensor advance, this would be 30ms to TDC, subtract 2ms for coil charging and subtract more m.s. for advance 
      coilCharge = 7;
      dwell = 10;
      coilPrecharge = LOW;
      Serial.print ( " RPM 250" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 85008) && (rpmCurrMicros - rpmOldMicros >= 61000)) //375rpm
    {
      sparkTriggerDelay =35;                 //was 16
      coilCharge = 5;
      dwell = 6;
      coilPrecharge = LOW;
      Serial.print ( " RPM 375 " );
      }
  else if ((rpmCurrMicros - rpmOldMicros <= 61000) && (rpmCurrMicros - rpmOldMicros >= 48000)) //500rpm
    {
      sparkTriggerDelay =28;                 //was 11
      coilPrecharge = LOW;
      Serial.print ( " RPM 500 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 48000) && (rpmCurrMicros - rpmOldMicros >= 41000))  //625rpm
    {
      sparkTriggerDelay =9;
      coilPrecharge = HIGH;
      coilCharge = 2;
      dwell = 2;
      Serial.print ( " RPM 625 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 41000) && (rpmCurrMicros - rpmOldMicros >= 30000)) //750rpm
    {
      sparkTriggerDelay =3;
      coilCharge = 1;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 750" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 30000) && (rpmCurrMicros - rpmOldMicros >= 25000)) //1000rpm
    {
      sparkTriggerDelay =2;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 1000 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 25000) && (rpmCurrMicros - rpmOldMicros >= 23076)) //1200rpm
    {
      sparkTriggerDelay =1.6;
      coilCharge = 2;
      coilPrecharge = HIGH;
      Serial.print ( "RPM 1200 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 23076) && (rpmCurrMicros - rpmOldMicros >= 22222)) //just under 1300rpm
    {
      sparkTriggerDelay =1;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 1300 " );
    }
  else if (rpmCurrMicros - rpmOldMicros <= 22222)//just over 1300rpm
    {
      sparkTriggerDelay =0;
      coilPrecharge = HIGH;
      Serial.print ( "RPM over 1300" );
    }
}
void timingChartThingy11()
{
  //the timing delays and coil charge & dwell adjustments at speed can go here.  
  if (rpmCurrMicros - rpmOldMicros >= 497512) //sub250rpm
    {
      sparkTriggerDelay =110;
      startingDelay = false;
      startingDelayTimer = millis();
      coilCharge = 8;
      dwell = 50;
      coilPrecharge = LOW;
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 255024) && (rpmCurrMicros - rpmOldMicros >= 169491)) //125rpm
    {
      sparkTriggerDelay =81;                     //was 57, then 59 and i don't know what overflow is but i get an error for it here.
      coilCharge = 8;
      dwell = 15;
      coilPrecharge = LOW;
      Serial.print ( " RPM 125" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 169491) && (rpmCurrMicros - rpmOldMicros >= 127512)) //177rpm
    {
      sparkTriggerDelay =55;
      coilCharge = 7;
      dwell = 12;
      coilPrecharge = LOW;
      Serial.print ( " RPM 177" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 127512) && (rpmCurrMicros - rpmOldMicros >= 85008)) //250rpm
    {
      sparkTriggerDelay =50;                    //was 27  //at 250 RPM and 45 degree sensor advance, this would be 30ms to TDC, subtract 2ms for coil charging and subtract more m.s. for advance 
      coilCharge = 7;
      dwell = 10;
      coilPrecharge = LOW;
      Serial.print ( " RPM 250" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 85008) && (rpmCurrMicros - rpmOldMicros >= 61000)) //375rpm
    {
      sparkTriggerDelay =39;                 //was 16
      coilCharge = 5;
      dwell = 6;
      coilPrecharge = LOW;
      Serial.print ( " RPM 375 " );
      }
  else if ((rpmCurrMicros - rpmOldMicros <= 61000) && (rpmCurrMicros - rpmOldMicros >= 48000)) //500rpm
    {
      sparkTriggerDelay =35;                 //was 11
      coilPrecharge = LOW;
      Serial.print ( " RPM 500 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 48000) && (rpmCurrMicros - rpmOldMicros >= 41000))  //625rpm
    {
      sparkTriggerDelay =18;
      coilPrecharge = HIGH;
      coilCharge = 2;
      dwell = 2;
      Serial.print ( " RPM 625 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 41000) && (rpmCurrMicros - rpmOldMicros >= 30000)) //750rpm
    {
      sparkTriggerDelay =7;
      coilCharge = 1;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 750" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 30000) && (rpmCurrMicros - rpmOldMicros >= 25000)) //1000rpm
    {
      sparkTriggerDelay =4;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 1000 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 25000) && (rpmCurrMicros - rpmOldMicros >= 23076)) //1200rpm
    {
      sparkTriggerDelay =2;
      coilCharge = 2;
      coilPrecharge = HIGH;
      Serial.print ( "RPM 1200 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 23076) && (rpmCurrMicros - rpmOldMicros >= 22222)) //just under 1300rpm
    {
      sparkTriggerDelay =1;
      coilPrecharge = HIGH;
      Serial.print ( " RPM 1300 " );
    }
  else if (rpmCurrMicros - rpmOldMicros <= 22222)//just over 1300rpm
    {
      sparkTriggerDelay =0.5  ;
      coilPrecharge = HIGH;
      Serial.print ( "RPM over 1300" );
    }
}
