#include "SoftwareSerial.h"

#include "TinyGPS.h"
#include "SD.h"
#include "LiquidCrystal.h"


//Créer un objet myFile(pour ecrire les données GPS)
//Créer un objet gps(pour obtenir les données GPS)
File myFile;
TinyGPS gps;
SoftwareSerial ss(3, 2);//Pour ce machine, 3(rx), 3(tx)
LiquidCrystal lcd(4,5,6,7,8,9);//initialize the library with the numbers of the interface pins

const int BP0 = 16;     // the number of the pushbutton pin
const int BP1 = 15;     // the number of the pushbutton pin
const int BPEN = 17;     // the number of the pushbutton pin
const int Batterie=0;    // batterie pin

//initialiser la partie de bouton
int value_tension = 0;
int BP1State = 0;
int BP0State = 0;
int BPENState = 0;
int switch_appuyer = 99;
String filename="";

//Déclaration
static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);


void setup()
{
  Serial.begin(115200);
  //Definitions des PINs
  pinMode(Batterie, INPUT);
  pinMode(BP0, INPUT);
  pinMode(BP1, INPUT);
  pinMode(BPEN, INPUT);
  
  //Serial.println();
  // Serial.println("Sats HDOP Latitude  Longitude  Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum");
  //Serial.println("          (deg)     (deg)      Age                      Age  (m)    --- from GPS ----  ---- to Paris  ----  RX    RX        Fail");
  //Serial.println("-------------------------------------------------------------------------------------------------------------------------------------");

  ss.begin(4800);
  // set up the LCD's number of columns and rows
  lcd.begin(8,2);
  lcd.print("Hello");//Après connecté, LCD montre "Hello"
  pinMode(10, OUTPUT);
  
  if(!SD.begin(10)){
    Serial.write("ERROR");
  }
  randomSeed(1);
  
  
}

void loop()
{
  // lire la valeur des pins 
  value_tension = analogRead(Batterie); 
  BP0State = digitalRead(BP0);
  BP1State = digitalRead(BP1);
  BPENState = digitalRead(BPEN);


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

  // Si appuyer bouton 1, ouvrir le fichier "gps.txt"
  if(switch_appuyer==1&&!myFile)
  {
    filename=random(1000);
    filename+=".txt";
    myFile = SD.open(filename, FILE_WRITE);
    lcd.setCursor(0,0);
    lcd.print("Write");
    
  }

  // Si appuyer bouton2, fermer le fichier
  if(switch_appuyer==2&&myFile){
    myFile.close();
    lcd.setCursor(0,0);
    lcd.print("Close");
  }

  //Si appuyer, afficher l'etat des piles
   if(switch_appuyer==3)
  {
    lcd.setCursor(0, 1);
    lcd.print(value_tension*6.5/1024);
    lcd.print("V");
 
  }

  //Déclaration les variables et les fonctions
  float flat, flon;
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;
  static const double PARIS_LAT = 48.513902, PARIS_LON = 2.201204;
  
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
  print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, PARIS_LAT, PARIS_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, PARIS_LAT, PARIS_LON)), 6);

  gps.stats(&chars, &sentences, &failed);
  print_int(chars, 0xFFFFFFFF, 6);
  print_int(sentences, 0xFFFFFFFF, 10);
  print_int(failed, 0xFFFFFFFF, 9);
  Serial.println();
  
  smartdelay(1000);
  
  // Si appuyer 4, afficher la longtitude et la latitude
   if(switch_appuyer==4)
  {
    lcd.setCursor(0, 0);
    lcd.print(flat);
    lcd.setCursor(0, 1);
    lcd.print(flon);
 
  }
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
   
    if (myFile) 
    {
    while (ss.available())
    {
      char c=ss.read();
      gps.encode(c);
      myFile.write(c);
      }
    }
  } while (millis() - start < ms);
}

//Definition du fonction print_float
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

//Definition du fonction print_int
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

//Fonction pour afficher le date
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
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    Serial.print(sz);
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
