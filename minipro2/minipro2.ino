#include "SoftwareSerial.h"

#include "TinyGPS.h"
#include "SD.h"
#include "LiquidCrystal.h"
#include <avr/pgmspace.h>

/* This sample code demonstrates the normal use of a TinyGPS object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
File myFile;
TinyGPS gps;
SoftwareSerial ss(3, 2);
LiquidCrystal lcd(4,5,6,7,8,9);
const int BP0 = 16;     // the number of the pushbutton pin
const int BP1 = 15;     // the number of the pushbutton pin
const int BPEN = 17;     // the number of the pushbutton pin
int BP1State = 0;
int BP0State = 0;
int BPENState = 0;
int switch_appuyer = 99;
char timep[25];
 float flat, flon;

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);
static void write_data();
static void write_end();

void setup()
{
  Serial.begin(115200);
  pinMode(BP0, INPUT);
  pinMode(BP1, INPUT);
  pinMode(BPEN, INPUT);
  
  //Serial.print("Testing TinyGPS library v. "); Serial.println(TinyGPS::library_version());
  //Serial.println("by Mikal Hart");
  Serial.println();
  //Serial.println("Sats HDOP Latitude  Longitude  Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum");
  //Serial.println("          (deg)     (deg)      Age                      Age  (m)    --- from GPS ----  ---- to Paris  ----  RX    RX        Fail");
  //Serial.println("-------------------------------------------------------------------------------------------------------------------------------------");
  ss.begin(4800);
  lcd.begin(8,2);
  pinMode(10, OUTPUT);
  
  if(!SD.begin(10)){
      Serial.print("error!");
    }

}

void loop()
{
   
  BP0State = digitalRead(BP0);
  BP1State = digitalRead(BP1);
  BPENState = digitalRead(BPEN);
  switch_appuyer=99;
  
  if(BPENState==HIGH){
    if(BP1State==LOW){
      if(BP0State==LOW) switch_appuyer=1;
      else if(BP0State==HIGH) switch_appuyer=2;
    }
    else{
      if(BP0State==LOW) switch_appuyer=3;
      else if(BP0State==HIGH) switch_appuyer=4;
    }
  }

  if(switch_appuyer==1&&!myFile)
  {
    myFile = SD.open("Gps.txt", FILE_WRITE);
    if(!myFile){
      Serial.print("error!");
    }
    write_head();
    lcd.print("Write");
  }
  if(switch_appuyer==2&&myFile){
    write_end();
    myFile.close();
    lcd.setCursor(0,0);
    lcd.print("Close");
  }
 
 
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
  static const double PARIS_LAT = 48.513902, PARIS_LON= 2.201204;
  
  print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
  print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
  gps.f_get_position(&flat, &flon, &age);
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  print_date(gps);
  print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2);
  print_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
  print_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
  print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, PARIS_LAT, PARIS_LON) / 1000, 0xFFFFFFFF, 9);
  print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON)), 6);
  
  gps.stats(&chars, &sentences, &failed);
  print_int(chars, 0xFFFFFFFF, 6);
  print_int(sentences, 0xFFFFFFFF, 10);
  print_int(failed, 0xFFFFFFFF, 9);
  Serial.println();
  smartdelay(1000);
}
const char w[] PROGMEM = {"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?><gpx xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:gpxx=\"http://www.garmin.com/xmlschemas/GpxExtensions/v3\" xmlns:gpxtpx=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\" creator=\"Oregon 400t\" version=\"1.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd http://www.garmin.com/xmlschemas/TrackPointExtension/v1 http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd\"><metadata><link href=\"http://www.garmin.com\"><text>Garmin International</text></link>"};
const char p[] PROGMEM = {"</time></metadata><trk><name>Example GPX Document</name><trkseg>"};
static void write_head()
{
  if(myFile){
    int len = strlen_P(w);
    for (int k = 0; k < len; k++)
    {
      char myChar =  pgm_read_byte_near(w + k);
      myFile.write(myChar);
    }
    myFile.print("<time>");
    myFile.print(timep);
    len = strlen_P(p);
    for (int k = 0; k < len; k++)
    {
      char myChar =  pgm_read_byte_near(p + k);
      myFile.write(myChar);
    }
  }
}

const char ll[] PROGMEM ={"<trkpt lat=\""};
const char ll1[] PROGMEM ={"\" lon=\""};
const char ll2[] PROGMEM ={"\"><ele>"};
const char ll21[] PROGMEM ={"</ele><time>"};
const char ll3[] PROGMEM ={"</time></trkpt>"};

static void write_data()
{
  if(myFile){
    String p1,p2;
    int len = strlen_P(ll);
    for (int k = 0; k < len; k++)
    {
      char myChar =  pgm_read_byte_near(ll + k);
      myFile.write(myChar);
    }
    myFile.print(flat,6);
    len = strlen_P(ll1);
    for (int k = 0; k < len; k++)
    {
      char myChar =  pgm_read_byte_near(ll1 + k);
      myFile.write(myChar);
    }
    myFile.print(flon,6);
    len = strlen_P(ll2);
    for (int k = 0; k < len; k++)
    {
      char myChar =  pgm_read_byte_near(ll2 + k);
      myFile.write(myChar);
    }
    myFile.print(gps.f_altitude(),2);
    len = strlen_P(ll21);
    for (int k = 0; k < len; k++)
    {
      char myChar =  pgm_read_byte_near(ll21 + k);
      myFile.write(myChar);
    }
//todo
    
    myFile.print(timep);
    len = strlen_P(ll3);
    for (int k = 0; k < len; k++)
    {
      char myChar =  pgm_read_byte_near(ll3 + k);
      myFile.write(myChar);
    }
  }
}

const char ll4[] PROGMEM = {"</trkseg></trk></gpx>"};
static void write_end()
{
  if(myFile){
    int len = strlen_P(ll4);
    for (int k = 0; k < len; k++)
    {
      char myChar =  pgm_read_byte_near(ll4 + k);
      myFile.write(myChar);
    }
  }
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
    {
      char c=ss.read();
      gps.encode(c);
    }
  } while (millis() - start < ms);
  if(myFile){
        write_data();
  }
}

static void print_float(float val, float invalid, int len, int prec)
{
  if (val == invalid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartdelay(0);
}

static void print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else
  {
    sprintf(timep, "%02d-%02d-%02dT%02d:%02d:%02dZ",year,month,day,hour, minute, second);
    Serial.print(timep);
    Serial.print(" ");
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  smartdelay(0);
}

static void print_str(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartdelay(0);
}
