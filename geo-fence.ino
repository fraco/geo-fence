#include <SoftwareSerial.h>
#include <Stdlib.h>

SoftwareSerial mySerial(10, 11);
char inChar = 0;
String message = "Hellumba!";
String str;
float homeLat = 40.829331;
float homeLon = -73.927908;
float thresholdDistance = 10.00;
//the phone number to communicate with
String phoneNumber = "";

void setup()  
{
  Serial.begin(9600);
  Serial.println("Hello Debug Terminal!");
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  setupGPRS();
}


void loop() // run over and over
{
  if(millis() % 10000 <10){  
    getLocation();
  }

  if (mySerial.available())
  {
    str = mySerial.readStringUntil('\n');
    Serial.println(str);
    if(str.startsWith("+QCELLLOC")) 
    {
      extractCoordinates();
    }
    delay(20);
  }

  if (Serial.available()>0)
  {
    mySerial.write(Serial.read());
  }
}


void setupGPRS()
{
  Serial.println("Setting up GPRS.......");
  mySerial.print("ATE0");
  mySerial.print("\r");
  delay(300);

  mySerial.print("AT+QIFGNCT=0");
  mySerial.print("\r");
  delay(500);

  mySerial.print("AT+QICSGP=1,\"EPC.TMOBILE.COM\"");
  mySerial.print("\r");
  delay(500);
}


void getLocation()
{
  //Turn off echo from GSM
  mySerial.print("AT+QCELLLOC=1");
  mySerial.print("\r");
  delay(5000);
  Serial.println("Getting Location - Please wait.......................");
}


void checkDist(float ltd , float lon)
{
  float dist = HaverSine(homeLat,homeLon,ltd,lon);

  if (dist >= thresholdDistance )
  {
    Serial.print("Target is ");
    Serial.print(dist - thresholdDistance);
    Serial.println("KM beyond threshold, sending SMS.....");   
    //Set the module to text modeee
    mySerial.print("AT+CMGF=1");
    mySerial.print("\r");
    delay(500);

    //Send the following SMS to the following phone number
    mySerial.print("AT+CMGS=\"");
    // CHANGE THIS NUMBER! CHANGE THIS NUMBER! CHANGE THIS NUMBER! 
    // 129 for domestic #s, 145 if with + in front of #
    mySerial.print("0000000000\",129");
    mySerial.print("\r");
    delay(300);

    mySerial.print("Your target is here!: ");

    mySerial.print("http://maps.googleapis.com/maps/api/staticmap?markers=");
    mySerial.print(ltd,6);
    mySerial.print(",");
    mySerial.print(lon,6);
    mySerial.print("&zoom=15&size=600x600&sensor=false");

    // Special character to tell the module to send the message
    mySerial.write(0x1A);
    delay(500);
  }  else   {
    Serial.println("Subject within threshold, no worries!");
  }
}


float HaverSine(float lat1, float lon1, float lat2, float lon2)
{
  float ToRad = PI / 180.0;
  float R = 6371;   // radius earth in Km

  float dLat = (lat2-lat1) * ToRad;
  float dLon = (lon2-lon1) * ToRad; 

  float a = sin(dLat/2) * sin(dLat/2) +
    cos(lat1 * ToRad) * cos(lat2 * ToRad) * 
    sin(dLon/2) * sin(dLon/2); 

  float c = 2 * atan2(sqrt(a), sqrt(1-a)); 

  float d = R * c;
  return d;
}


void extractCoordinates()
{
  int commaPos = str.indexOf(",");

  String Longitude = str.substring(11,commaPos);
  String Latitude = str.substring(commaPos+1);
  Latitude.trim();
  Longitude.trim();

  char floatbuff1[32];
  Latitude.toCharArray(floatbuff1, sizeof(floatbuff1));
  float ltd = atof(floatbuff1);

  char floatbuff2[32];
  Longitude.toCharArray(floatbuff2, sizeof(floatbuff2));
  float lod = atof(floatbuff2);

  checkDist(ltd,lod);
}

