// Search the DB for a print with this example
#include <SoftwareSerial.h>
#include <FPM.h>
#include <PCF8574_HD44780_I2C.h>
#include <SPI.h>
#include <RFID.h>

PCF8574_HD44780_I2C lcd(0x27,16,2);
/*
Arduino Module RFID RC-522
* MOSI: Pin 11
* MISO: Pin 12
* SCK : Pin 13
* SS/SDA: Pin 10
* RST: Pin 9
* RQ: not used
*/
#define SS_PIN 10
#define RST_PIN 9
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE/YELLOW wire)
SoftwareSerial mySerial(2, 3); //RX, TX
SoftwareSerial hc12(5, 6);        // software serial : RX = digital pin 5, TX = digital pin 6
//NeoSWSerial  mySerial( 2, 3 );
//NewSoftSerial mySerial(2, 3);
FPM finger;

RFID rfid(SS_PIN,RST_PIN);
#define TEMPLATES_PER_PAGE  256

//FPM finger;

int getFingerprintEnroll(int id);
int getFingerprintIDez();
void readTag_finger();
int getFingerprintID();
bool get_free_id(int16_t *id);
void  Emptydb();
const int Enroll_butt=A0;
const int clearPin = A1; 
const int scanPin = A2; 
bool tag_check();


const int Led0 = 13;
const int buzzer = 7;   //Passive buzzer ke Pin D7 dan GND
const int Lock = 8; //Driver Selenoid Door Lock/Relay input ke pin D8

int serNum[5];          //Variable buffer Scan Card

int cards[][5] ={{48,125,151,165,127},{86,159,18,179,104},{148,221,184,83,162},{178,240,179,83,162}
,{211,133,225,83,228},{187,163,18,179,185}}; //ID cards serial no to recognise

/*
card b  48 125 151 165 127
card g  86 159 18 179 104
card n  148 221 184 83 162
card f  178 240 179 83 162
card j  187 163 18 179 185
card k  211 133 225 83 228
*/
int counter = 0;
int id_ad_counter = 0;
int i = 0;
int x = 0;
unsigned  long  previousMillis  =  0 ;  
const  long  interval  =  50 ;   

//states
uint8_t enroll_state=0;
uint8_t enrollfinished_state=0;
uint8_t scan_state=0;
uint8_t scanfinished_state=0;
uint8_t disp_state=0;

bool ok = false; 
bool ok1 = false;

bool access=false;
bool finger_read=false;
int alarm = 0;
int btn_flag=0;
char mydata[33];
uint8_t alarmStat = 0;
uint8_t maxError = 5;  //maximum number of failed tries
int fingerprintID = 0;
int16_t id=0;
bool success1=false;
bool  add_new_id = true;
void countDown();

void setup()  
{
  pinMode (Enroll_butt,INPUT_PULLUP);
  pinMode (clearPin,INPUT_PULLUP);
  pinMode (scanPin ,INPUT_PULLUP);

  pinMode(Led0, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(Lock, OUTPUT);
  digitalWrite(Lock,LOW);
  
  SPI.begin();
  rfid.init();
  lcd.init();
  lcd.backlight();          // Backlight ON
  Serial.begin(9600);       //will use hardware serial for hc12 module
  mySerial.begin(57600);    //fingerprint default baud rate
  hc12.begin(9600); // open Ser1
  
  tone(buzzer,1200);
  delay(200);
  noTone(buzzer); 
  delay(100);
  
  lcd.setCursor (0,0);
  lcd.print(F("Rfid-Fingerprint"));
  lcd.setCursor (0,1);
  lcd.print(F("Pneumatic System"));
  delay (3000);
  lcd.clear();
  
   mySerial.listen();
if (finger.begin(&mySerial)) 
 {
     lcd.clear();
     lcd.print("finger Module");
     lcd.setCursor(3, 1);
     lcd.print("Detected!");  delay(2000);
      } 
 else {
    lcd.clear();
    lcd.print("No finger");
    lcd.setCursor(0, 1);
    lcd.print("Module Detected");
    delay (2000);
    while (1);
  }

}


void tone1(){
 tone(buzzer,900); 
   delay(100); 
   tone(buzzer,1200); 
   delay(100); 
   tone(buzzer,1800); 
   delay(200); 
   noTone(buzzer);   
   delay(600);
}


void loop()   // run over and over again
{
 int j=0;

   /*
    * 
  if(millis()>previousMillis + interval){ //50ms interval
  previousMillis=millis();   //save new time
  tag_check();
}
    * uint8_t enroll_state=0;
uint8_t enrollfinished_state=0;
uint8_t scan_state=0;
uint8_t scanfinished_state=0;
uint8_t disp_state=0;*/


if (disp_state==0){

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("System Ready"));
    lcd.setCursor(0,1);
    lcd.print(F("Press to Scan"));
    delay (220);

    hc12.listen();
    if(hc12.available()) // read data from hc12 one
  {
    lcd.clear();
     delay(100);
     int j=0;
    while(hc12.available()>0){
     mydata[j++]=hc12.read();
    }

    for(;j<33;j++){  //clears the rest
      mydata[j++]=' ';
    }

   for(j=0;j<16;j++){  //prints the first row
      lcd.print(mydata[j]); 
    }
    
    //prints the second row
    lcd.setCursor(0, 1);
    for(j=16;j<33;j++){ 
     lcd.print(mydata[j]);
    } 
    delay(3000);
    disp_state==0 ; 
    } 
   } 

  if (digitalRead(Enroll_butt)==LOW){
    disp_state=1;  //place your tag display
    if ((scan_state==0)  && (disp_state==1)){   
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("Place Your Tag"));
    lcd.setCursor(3,1);
    lcd.print(F("To Scan"));
    delay (7000);
    
 while (digitalRead(Enroll_butt)==HIGH){  //display while pin is high
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("Place card and"));
    lcd.setCursor(0,1);
    lcd.print(F("Press Btn 2 Scan"));
    delay (2000); 
    scan_state=1;
 }
    
    if(scan_state==1){
       while(rfid.isCard()){   //check if a card is present
           
           if (rfid.readCardSerial()) {     //do card scaning
            for (x=0;  x<sizeof(cards); x++) {  //loop through card size
              for (i = 0 ;  i < sizeof(rfid.serNum); i++) { //loop through serial no
                  if (rfid.serNum[i] != cards[x][i])  { 
                  ok = false; 
                  break;
                  }  else  { ok = true; } 
              } 
          if(ok) break ; 
              }   
  }  }
    }
    
    if(ok==true){
          tone(buzzer,1200);
          delay(200);
          noTone(buzzer); 
          delay(100);
          lcd.clear();
          lcd.setCursor(0,0); 
          lcd.print(F("Card Recognised" ));  
          delay(3000);
      disp_state=2;    //ask for finger enrollment
      scan_state=5;
    }else{
     lcd.clear();
    lcd.setCursor(3,0);
    lcd.print(F("Card Not"));
    lcd.setCursor (3,1);
    lcd.print(F("Recognised"));
    delay (2000);
    disp_state=0; scan_state=0; 
      }
   }   }
    
  

  if ((disp_state==2) && (scan_state==5)) { //if a tag is detected we scan the finger
      mySerial.listen();   //switch to fingerprint 
      lcd.clear();
      lcd.setCursor (0,0);
      lcd.print(F("Place finger"));
      lcd.setCursor (0,1);
      lcd.print(F("For Enrollment"));
      delay (3000);
      
      get_free_id(&id);   //check for free id slot 
    while (!getFingerprintEnroll(id));  // enroll
    disp_state=0; scan_state=0;
    }

    

 if( digitalRead(scanPin)==LOW ){  //we scan for Entry with this pin
   disp_state=3;
   mySerial.listen();   //switch to fingerprint
  if (disp_state==3){
  lcd.clear();lcd.setCursor (0,0);
  lcd.print(F("Place Finger To"));
  lcd.setCursor (0,1);
  lcd.print(F("Scan Your id"));
  delay (200);
  disp_state=4;}


while (1){ 
   int myid=getFingerprintID();
   
   if (myid>=0){
    disp_state=5;
    break;
   } 

   if (disp_state==0)break;
    
   }
   

   
   if( disp_state==5){
   tone1();     //wecome tone
  digitalWrite(Lock, HIGH);
  lcd.clear();lcd.setCursor(0,0);
  lcd.print(F("Please Come in "));
  lcd.setCursor (0,1);
  lcd.print(F("AutoLock "));
  for(int z=14; z>0; z--){
            lcd.setCursor (13,1);lcd.print(z);
            delay(1000);
           }  
  digitalWrite(Lock, LOW);
  hc12.listen();  //switch to hc12
  hc12.print("ID ");
  hc12.print(finger.fingerID);
  hc12.print("   Verified Entrance 2      "); 
  disp_state=0; 
        } 
    
  }
     

if( digitalRead(clearPin)==LOW ){
  mySerial.listen();   //switch to fingerprint
  Emptydb();   //we clear database with this pin 
  disp_state=0; }

    }


// this function gets the fingerprint ID returns -1 if failed, otherwise returns ID #
int getFingerprintID() {
   

    uint8_t  p = finger.getImage();
    if (p != FINGERPRINT_OK)  return -1;
 

  p = finger.image2Tz();
    if (p != FINGERPRINT_OK)  return -1;
 

  p = finger.fingerFastSearch();
     if (p != FINGERPRINT_OK) { 
   lcd.clear();lcd.setCursor (0,0);
  lcd.print(F("Finger Not Found"));
  lcd.setCursor (0,1);
  lcd.print(F("Try Again Later"));
  delay (2000);disp_state=0;
  return -1;}  
    
  lcd.clear();lcd.setCursor (3,0);
  lcd.print(F("Found ID"));
  lcd.setCursor (8,1);
  lcd.print(finger.fingerID);
 
  delay (2000); 
  return finger.fingerID; 

}


//this function uses the id for enrollment
 int getFingerprintEnroll(int id) {  //this routine enrolls the finger
  
  //this area is the first verirification
  int p = -1;
      lcd.clear(); 
      lcd.setCursor (2,0);
      tone(buzzer,1200);
      delay(200);
      noTone(buzzer); 
      delay(100);
      lcd.print(F("Waiting for"));
      lcd.setCursor (4,1);
      lcd.print(F(" finger"));delay (1000);

  while (p != FINGERPRINT_OK) {  //wait here until  finger is ok
    p = finger.getImage();
    
    switch (p) {
    case FINGERPRINT_OK:
      lcd.clear();lcd.setCursor (2,0);
      lcd.print(F("Image taken"));delay (2000);
      break;
    /*default:
      lcd.clear();
      tone (buzzer,900); 
      delay(200); 
      noTone(buzzer); 
      delay(200); 
      tone(buzzer,900); 
      delay(200); 
      noTone(buzzer);   
      delay(500); 
      lcd.setCursor (3,0);
      lcd.print(F("No finger"));
      lcd.setCursor (4,1);
      lcd.print(F("present"));
      delay (2000);
      break;*/
    }
  }  
  
 
  // OK success! convert first image
  p = finger.image2Tz(1);   
  switch (p) {
    case FINGERPRINT_OK:
      lcd.clear(); lcd.setCursor (0,0);
      lcd.print(F(" Image converted"));delay (2000);
      break;
    default:
      lcd.clear(); lcd.setCursor (0,0);
      lcd.println(F("Unknown error"));delay (2000);
      return p;
  }
  lcd.clear();lcd.setCursor (0,0);
  lcd.print(F(" Remove finger "));delay (2000);
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {//wait here if no finger is present
    p = finger.getImage();
  }

// this area is the second verirification
  p = -1;
  lcd.clear();lcd.setCursor (2,0);
  lcd.print(F(" Place same "));
  lcd.setCursor (3,1);
  lcd.print(F(" finger "));
  delay (2000);
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      lcd.clear(); lcd.setCursor (2,0);
      lcd.println(F(" Image taken  "));delay (2000);
    break;
    /*default:
      lcd.clear(); 
      tone(buzzer,900); 
      delay(200); 
      noTone(buzzer); 
      delay(200); 
      tone(buzzer,900); 
      delay(200); 
      noTone(buzzer);   
      delay(500); 
      lcd.setCursor (0,0);
      lcd.print(F("Unknown error"));delay (2000);
     break;*/
    }
  }

  
// OK success! convert second image
  p = finger.image2Tz(2); 
  switch (p) {
    case FINGERPRINT_OK:
      lcd.clear();lcd.setCursor (1,0);
      lcd.print(F("Image converted"));delay (2000);
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.clear();lcd.setCursor (0,0);
      lcd.print(F("Comm error"));delay (2000);
     
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.clear();lcd.setCursor (0,0);
      lcd.print(F("finger error"));delay (2000);
     return p;
      default:
      lcd.clear();lcd.setCursor (0,0);
      lcd.print(F("Unknown error"));delay (2000);
      return p;
  }
  
  
  // OK converted!
  p = finger.createModel();   //create a model after the two verifications
  if (p == FINGERPRINT_OK) {
    lcd.clear();lcd.setCursor (2,0);
    lcd.print(F("Prints matched!"));delay (1000);
    
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    lcd.clear();lcd.setCursor (0,0);
    lcd.print(F("No match"));delay (1000);
   return p;
  } else {
    lcd.clear();lcd.setCursor (0,0);
    lcd.print(F("Unknown error"));delay (1000);
    return p;
  }   
  
  lcd.clear();
  lcd.setCursor (0,0);
  Serial.println("Stored at ID");  //send to other hc12 module
  lcd.print(F("Stored at ID"));
  lcd.setCursor (6,1);
  lcd.print(id);
  Serial.println(id);     //send to other hc12 module
  delay (2000);
  
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    lcd.clear();lcd.setCursor (2,0);
    tone(buzzer,1200); 
    delay(200); 
    noTone(buzzer); 
    delay(200); 
    tone(buzzer,1200); 
    delay(200); 
    noTone(buzzer); 
    delay(200);
    tone(buzzer,1200); 
    delay(200); 
    noTone(buzzer); 
    delay(200);
    lcd.print(F("Model Stored!"));
    delay (2000);
  } else {
    lcd.clear();lcd.setCursor (0,0);
    lcd.print(F("Unknown error"));delay (2000);
    return p;
  } 

}


bool get_free_id(int16_t *id){
  int p = -1;
  for (int page = 0; page < (finger.capacity / TEMPLATES_PER_PAGE) + 1; page++){
    p = finger.getFreeIndex(page, id);
    switch (p){
      case FINGERPRINT_OK:
        if (*id != FINGERPRINT_NOFREEINDEX){
        return true;
        }
      case FINGERPRINT_PACKETRECIEVEERR:
        return false;
      default:
        return false;
    }
  }
}

/*this function clears the fingerprint database*/
void Emptydb(){
   int p = -1;
  while (p != FINGERPRINT_OK){
    p = finger.emptyDatabase();
    if (p == FINGERPRINT_OK){
      lcd.clear();lcd.setCursor (0,0);
      lcd.print(F("Database cleared!"));delay (2000);
     
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      lcd.clear();lcd.setCursor (0,0);
      lcd.print(F(" Comm error!"));delay (2000);
    }
    else if (p == FINGERPRINT_DBCLEARFAIL) {
      lcd.clear();lcd.setCursor (3,0);
      lcd.print(F("Could not"));
      lcd.setCursor (3,1);
      lcd.print(F("clear database!"));
      delay (2000);
    }
  }
  
  }
