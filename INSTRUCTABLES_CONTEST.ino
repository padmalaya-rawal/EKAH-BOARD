/* This project is originally created by PADMALAYA RAWAL and you can support me here:
 * yt channel: https://www.youtube.com/PRROBOTICS
 * Patreon   : https://www.patreon.com/PRROBOTICS
 * Instagram : https://www.instagram.com/PRROBOTICS

 
 DISCLAIMER:
 For this project I am using libraries from various awesome creators and you can checkout their work below:
  1. creator of USB library: https://github.com/arduino-libraries/Keyboard/
  2. creator of Blekeyboard library: https://github.com/T-vK/ESP32-BLE-Keyboard
  3. creator of Adafruit library: https://www.adafruit.com/

  
  ** ** A message from Adafruit library creator  ** **
 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.*/


/* NOTE:
 * 1. Before using the fingerprint module make sure that you have already added your fingerprint 
 * to it and then assign that number in this code.
 * 
 * 2. Make sure that you have added all the required libraries. 
 * 
 * 3. You can replace the touch switch with slide switches as well and change the connections accordingly.
 */
  
  
  
  
  //Before using it with any of your device, check the code and connections carefully because if something
  //goes wrong anything can happen with your connected device which may harm your device as it is a complete 
  //keyboard. And I will not bw responsible for any type of loss or damage in any manner. Use it at your own RISK.
 



#include "Adafruit_Keypad.h"       
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include <BleKeyboard.h>
#include <SPI.h>
#include <Wire.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define Keyboard_mode 35  //change the pin number
#define shift 13          //change the pin number
#define caps_lock 12

const byte ROWS = 5;    //no of rows of keyboard
const byte COLS = 10;   //no of columns of keyboard 

int Read_Keyboard_mode;  //to change the mode of Keyboard
int Read_shift;          //to read the value of shift key
int Read_caps_lock;      //to read the value of capslock key
int flag1;               //for mode change
int flag2;               //for mode change
int con;                 //flag for bt connection
int f_check;             //flag for fingerprint sensor

//Note here that these character doesn't represent their actual representation of the keypress
char keys[ROWS][COLS] = 
{
  {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'},
  {'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't'},
  {'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3'},
  {'4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D'},
  {'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N'}
 };

//These pin numbers should be changed according to connections of keypad with ESP32 board
byte colPins[COLS] = {4,36,3,5,18,19,34,15,23,26}; 
byte rowPins[ROWS] = {25,27,14,33,32}; 

Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);

BleKeyboard bleKeyboard;

//___________________________________________________________________________
uint8_t getFingerprintID() 
{
  uint8_t p = finger.getImage();
  switch (p) 
  {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) 
  {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Found a print match!");
  } 
  
  else if (p == FINGERPRINT_PACKETRECIEVEERR) 
  {
    Serial.println("Communication error");
    return p;
  } 
  
  else if (p == FINGERPRINT_NOTFOUND)
  {
    Serial.println("Did not find a match");
    return p;
  } 
  
  else
  {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); 
  Serial.print(finger.fingerID); 
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence); 

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
  int getFingerprintIDez() 
  {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  Serial.print("Found ID #"); 
  Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); 
  Serial.println(finger.confidence);
  return finger.fingerID; 
}
//_____________________________________________________________________

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  bleKeyboard.begin();
  customKeypad.begin();
  finger.begin(57600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setTextSize(2);      // change the size of text on OLED
  display.setTextColor(SSD1306_WHITE); // Draw white text
  
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);             
  display.setTextColor(SSD1306_WHITE);       
  display.setCursor(20  ,0);             
  display.println(F("Developed by:"));

  display.setTextSize(2);            
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5,15);  
  display.println("PADMALAYA");
  display.display();
  delay(3000);

  display.clearDisplay();
  display.display();
  
  pinMode(Keyboard_mode, INPUT_PULLUP);
  pinMode(shift, INPUT_PULLUP);
  pinMode(caps_lock, INPUT_PULLUP);

}

void loop()
{
  Read_shift=digitalRead(shift);
  Read_caps_lock= digitalRead(caps_lock);
  Read_Keyboard_mode= digitalRead(Keyboard_mode);
  
  if(bleKeyboard.isConnected()) {
    if(con==0){
      display.clearDisplay();
      display.setTextSize(1);             
      display.setTextColor(SSD1306_WHITE);       
      display.setCursor(30,8);             
      display.print(F("Connected!! "));
      display.setCursor(35,18);             
      display.print(F("Enjoy :-) "));
      display.display();
      con=1;
      delay(2000);
    }

    if((Read_Keyboard_mode)==HIGH){
      while(flag1==0){
        display.clearDisplay();
        display.setCursor(5, 0);     // Start at top-left corner
        display.setTextSize(2);             
        display.write("  SUPER");
        display.setCursor(0, 18);     // Start at top-left corner
        display.write(" KEYBOARD");
        display.display();  
        delay(3000);
        display.clearDisplay();
        display.setTextSize(1); 
        display.setCursor(28,8);                         
        display.write("Press any key");
        display.setCursor(33, 18);     // Start at top-left corner
        display.write("to continue");                  
        display.display();
        flag2=0;
        flag1=1;
      }
    
    customKeypad.tick();
    while(customKeypad.available())
    {
      keypadEvent e = customKeypad.read();
      char out=(char)e.bit.KEY;
      
      if(e.bit.EVENT == KEY_JUST_PRESSED)
        { 
            display.setTextSize(2); 
            switch(out)
            {
              case 'a':
                if(Read_shift==LOW){
                  bleKeyboard.write(48); //print 0
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("0");
                  display.display();}
  
                else if(Read_caps_lock == LOW){
                  bleKeyboard.write(65);  //print A
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("A");
                  display.display();}
                  
                else {
                  bleKeyboard.write(97);  //print a
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("a");
                  display.display();}
                break;
  
                
              case 'b':
               if((Read_shift)==LOW){
                  bleKeyboard.write(49); //print 1
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("1");
                  display.display();}
  
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(66);  //print B
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("B");
                  display.display();}
  
  
                else {
                  bleKeyboard.write(98);  //print b
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("b");
                  display.display();}
                break;
  
                
              case 'c':
               if((Read_shift)==LOW){
                  bleKeyboard.write(50); //print 2
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("2");
                  display.display();}
                  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(67);  //print C
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("C");
                  display.display();}
  
                else{
                  bleKeyboard.write(99);  //print c
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("c");
                  display.display();}
                break;
  
                       
              case 'd':
               if((Read_shift)==LOW){
                  bleKeyboard.write(51); //print 3
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("3");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(68);  //print D
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("D");
                  display.display();}
  
                else{
                  bleKeyboard.write(100);  //print d
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("d");
                  display.display();}
                break;
  
                
              case 'e':
               if((Read_shift)==LOW){
                  bleKeyboard.write(52); //print 4
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("4");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(69);  //print E
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("E");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(101);  //print e
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("e");
                  display.display();}
                break;
  
                
              case 'f':
               if((Read_shift)==LOW){
                  bleKeyboard.write(53); //print 5
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("5");
                  display.display();}
                  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(70);  //print F
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("F");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(102);  //print f
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("f");
                  display.display();}
                break;
  
                
              case 'g':
               if((Read_shift)==LOW){
                  bleKeyboard.write(54); //print 6
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("6");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(71);  //print G
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("G");
                  display.display();}
  
                else{
                  bleKeyboard.write(103);  //print g
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("g");
                  display.display();}
                break;
  
                
              case 'h':
               if((Read_shift)==LOW){
                  bleKeyboard.write(55); //print 7
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("7");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(72);  //print H
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("H");
                  display.display();}
  
                else{
                  bleKeyboard.write(104);  //print h
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("h");
                  display.display();}
                break;
  
                
              case 'i':
               if((Read_shift)==LOW){
                  bleKeyboard.write(56); //print 8
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("8");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(73);  //print I
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("I");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(105);  //print i
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("i");
                  display.display();}
                break;
  
                
              case 'j':
               if((Read_shift)==LOW){
                  bleKeyboard.write(57); //print 9
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("9");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(74);  //print J
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("J");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(106);  //print j
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("j");
                  display.display();}
                break;
  
                
              case 'k':
               if((Read_shift)==LOW){
                  bleKeyboard.write(33); //print !
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("!");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(75);  //print K
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("K");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(107);  //print k
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("k");
                  display.display();}
                break;
  
               
              case 'l':
               if((Read_shift)==LOW){
                  bleKeyboard.write(64); //print @
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("@");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(76);  //print L
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("L");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(108);  //print l
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("l");
                  display.display();}
                break;
  
                
              case 'm':
               if((Read_shift)==LOW){
                  bleKeyboard.write(35); //print #
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("#");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(77);  //print M
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("M");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(109);  //print m
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("m");
                  display.display();}
                break;
  
                
              case 'n':
               if((Read_shift)==LOW){
                  bleKeyboard.write(36); //print $
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("$");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(78);  //print N
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("N");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(110);  //print n
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("n");
                  display.display();}
                break;
  
                
              case 'o':
               if((Read_shift)==LOW){
                  bleKeyboard.write(37); //print %
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("%");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(79);  //print O
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("O");
                  display.display();}
  
                else{
                  bleKeyboard.write(111);  //print o
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("o");
                  display.display();}
                break;
  
                
              case 'p':
               if((Read_shift)==LOW){
                  bleKeyboard.write(94); //print ^
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("^");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(80);  //print P
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("P");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(112);  //print p
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("p");
                  display.display();}
                break;
  
                
              case 'q':
               if((Read_shift)==LOW){
                  bleKeyboard.write(38); //print &
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("&");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(81);  //print Q
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("Q");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(113);  //print q
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("q");
                  display.display();}
                break;
  
                
              case 'r':
               if((Read_shift)==LOW){
                  bleKeyboard.write(42); //print *
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("*");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(82);  //print R
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("R");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(114);  //print r
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("r");
                  display.display();}
                break;
  
                
              case 's':
               if((Read_shift)==LOW){
                  bleKeyboard.write(40); //print (
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("(");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(83);  //print S
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("S");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(115);  //print s
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("s");
                  display.display();}
                break;
  
                
              case 't':
               if((Read_shift)==LOW){
                  bleKeyboard.write(41); //print )
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write(")");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(84);  //print T
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("T");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(116);  //print t
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("t");
                  display.display();}
                break;
  
                
              case 'u':
               if((Read_shift)==LOW){
                  bleKeyboard.write(45); //print -
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("-");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(85);  //print U
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("U");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(117);  //print u
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("u");
                  display.display();}
                break;
  
                
              case 'v':
               if((Read_shift)==LOW){
                  bleKeyboard.write(95); //print _ 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("_");
                  display.display();}    
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(86);  //print V
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("V");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(118);  //print v
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("v");
                  display.display();}
                break;
  
                
              case 'w':
               if((Read_shift)==LOW){
                  bleKeyboard.write(61); //print =
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("=");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(87);  //print W
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("W");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(119);  //print w
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("w");
                  display.display();}
                break;
  
                
              case 'x':
               if((Read_shift)==LOW){
                  bleKeyboard.write(43); //print +
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("+");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(88);  //print X
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("X");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(120);  //print x
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("x");
                  display.display();}
                break;
  
                
              case 'y':
               if((Read_shift)==LOW){
                  bleKeyboard.write(96); //print `
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("`");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(89);  //print Y
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("Y");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(121);  //print y
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("y");
                  display.display();}
                break;
  
                
              case 'z':
               if((Read_shift)==LOW){
                  bleKeyboard.write(126); //print ~
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("~");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(90);  //print Z
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("Z");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(122);  //print z
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("z");
                  display.display();}
                break;
  
        
              case '0':
               if((Read_shift)==LOW){
                  bleKeyboard.write(123); //print {
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("{");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(125);  //print }
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("}");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(124);  //print |
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("|");
                  display.display();}
                break;
  
  
              case '1':
               if((Read_shift)==LOW){
                  bleKeyboard.write(91); //print [
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("[");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(93);  //print ]
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("]");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(63);  //print ?
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("?");
                  display.display();}
                break;
  
  
              case '2':
               if((Read_shift)==LOW){
                  bleKeyboard.write(47); //print /
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("/");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(92);  //print \
                  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("B-SLASH");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(44);  //print ,
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write(",");
                  display.display();}
                break;
  
  
              case '3':
               if((Read_shift)==LOW){
                  bleKeyboard.write(39); //print '
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("'");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(34);  //print "
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("D-QUOTE");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(46);  //print .
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write(".");
                  display.display();}
                break;
  
  
              case '4':
               if((Read_shift)==LOW){
                  bleKeyboard.write(60); //print <
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("<");
                  display.display();}
  
                else if((Read_caps_lock)== LOW){
                  bleKeyboard.write(62);  //print >
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write(">");
                  display.display();}
  
                else{ 
                  bleKeyboard.write(58);  //print :
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write(":");
                  display.display();}
                break;
  
  
              case '5':
                  bleKeyboard.write(128); //ctrl
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("CTRL");
                  display.display();
                break;
  
  
              case '6':
                  bleKeyboard.write(130);  //alt
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("ALT");
                  display.display();
                break;
  
  
              case '7':
                  bleKeyboard.write(179);  //tab
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("TAB");
                  display.display();
                break;
  
  
              case '8':
                  bleKeyboard.write(32);  //space
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("SPACE");
                  display.display();
                break;
  
  
              case '9':
                  bleKeyboard.write(178);  //backspace
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("BACKSPACE");
                  display.display();
                break;
    
  
              case 'A':
                  bleKeyboard.write(218);  //up arrow
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("UP ARROW");
                  display.display();
                break;
  
                
              case 'B':
                  bleKeyboard.write(217);  //down arrow
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("DOWN ARROW");
                  display.display();
                break;
  
                
              case 'C':
                  bleKeyboard.write(216);  //left arrow
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("LEFT ARROW");
                  display.display();
                break;
  
                        
              case 'D':
                  bleKeyboard.write(215);  //right arrow
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("RIGHT ARROW");
                  display.display();
                break;
  
                
              case 'E':
                  bleKeyboard.write(KEY_MEDIA_VOLUME_UP);  //volume+
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("VOL+");
                  display.display();
                break;
  
                
              case 'F':
                  bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);  //volume-
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("VOL-");
                  display.display();
                break;
  
                
              case 'G':
                  bleKeyboard.write(KEY_MEDIA_MUTE);  //mute
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("MUTE");
                  display.display();
                break;
  
                
              case 'H':
                  bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);  //next
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("NEXT TRACK");
                  display.display();
                break;
  
                
              case 'I':
                  bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);  //prev
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("PREV TRACK");
                  display.display();
                break;
  
                
              case 'J':     
                  bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);  //play/pause
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("PLAY/PAUSE");
                  display.display();
                break;
  
                
              case 'K':
                  bleKeyboard.write(KEY_MEDIA_STOP);  //stop
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("STOP");
                  display.display();
                break;
  
                
              case 'L':
                  bleKeyboard.write(176);  //enter
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("ENTER");
                  display.display();
                break;
  
         
              case 'M':
                  bleKeyboard.write(212);  //delete
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("DELETE");
                  display.display();
                break;
  
                
              case 'N':
                 //WRITE COMMANDS FOR SHUTING DOWN THE PC     
                  bleKeyboard.press(128);
                  bleKeyboard.press(177);
                  delay(500);
              
                  bleKeyboard.releaseAll();
                  delay(1000);
                  bleKeyboard.press(179);
                  delay(100);
                  bleKeyboard.release(179);
                  delay(100);

                  bleKeyboard.press(217);
                  delay(100);
                  bleKeyboard.release(217);
                  delay(100);
                  bleKeyboard.press(217);
                  delay(100);
                  bleKeyboard.release(217);
                  delay(100);
                  bleKeyboard.press(217);
                  delay(100);
                  bleKeyboard.release(217);
                  delay(100);
          
                  bleKeyboard.press(176);
                  delay(100);
                  bleKeyboard.release(176);
                  delay(100);
              
                  bleKeyboard.press(217);
                  delay(100);
                  bleKeyboard.release(217);
                  delay(100);
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner
                  display.write("SHUT DOWN");
                  display.display();   
                  
              }
          }
       }
    }
    
//MODE 2: PASSWORD MANAGER
//__________________________________________________________________________________
     if((Read_Keyboard_mode)==LOW){
      while(flag2==0){
        display.clearDisplay();
        display.setCursor(0, 0);     // Start at top-left corner
        display.setTextSize(2);             
        display.write(" PASSWORD");
        display.setCursor(0, 18);     // Start at top-left corner
        display.write(" MANAGER");
        delay(2000);
        display.clearDisplay();
        display.setTextSize(1);     
        display.setCursor(28,8);                     
        display.write("Place finger");
        display.setCursor(10, 18);     // Start at top-left corner
        display.write("for authentication");                  
        display.display();
        
        getFingerprintIDez();
        delay(50);       
        f_check= finger.fingerID;  
        while(f_check == 1){
          display.clearDisplay();
          display.setTextSize(1);     
          display.setCursor(25,8);                     
          display.write("Authentication");
          display.setCursor(30, 18);     // Start at top-left corner
          display.write("completed!!");                  
          display.display();
          delay(2000);
          display.clearDisplay();
          display.setTextSize(1); 
          display.setCursor(28,8);                         
          display.write("Press any key");
          display.setCursor(33, 18);     // Start at top-left corner
          display.write("to continue");                  
          display.display();
          flag2=1;
          f_check=0;
        }
        flag1=0;
      }

      customKeypad.tick();
    while(customKeypad.available())
    {
      keypadEvent e = customKeypad.read();
      char out=(char)e.bit.KEY;
      display.setTextSize(2); 
      if(e.bit.EVENT == KEY_JUST_PRESSED)
        {     
              switch(out)
              {
              case 'a':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("INSTAGRAM");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("PDKS54DFG"); //Print password
                  break;
  
              case 'b':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("FACEBOOK");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("4634SEDGASD"); //Print password
                  break;
                
              case 'c':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("REDDIT");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("^3452dfsghF"); //Print password
                  break;
  
                       
              case 'd':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("LINKEDIN");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("$@$%fhjd54"); //Print password
                break;
                
              case 'e':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("TWITTER");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("53456ASDFdgafd"); //Print password
                break;
                
              case 'f':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("SNAPCHAT");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("PDKS54DFG"); //Print password
                break;
                
              case 'g':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("PAYPAL");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("5DSSFDJKs(^&#$"); //Print password
                break;
                
              case 'h':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("GPAY");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("5DR(%^@!tWERT"); //Print password
                break;
                
              case 'i':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("OUTLOOK");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("Dsrf54676*(@!@$FG"); //Print password
                break;
                
              case 'j':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("BLYNK");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("Wredgt45653vg"); //Print password
                break;
                
              case 'k':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("PATREON");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("2#$%WG54fqSEDTd"); //Print password
                break;
               
              case 'l':
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("PINTEREST");
                  display.setCursor(0,18);
                  display.display();
                  bleKeyboard.print("asddfF3254@#$"); //Print password
                break;
                
              case 'm':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 'n':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 'o':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 'p':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 'q':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 'r':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 's':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 't':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                               
              case 'u':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 'v':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 'w':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 'x':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 'y':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break; 
                
              case 'z':
                  bleKeyboard.print(" ");  //print a
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
        
              case '0':
                  bleKeyboard.print("");
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
              case '1':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
              case '2':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
              case '3':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break; 
  
              case '4':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
              case '5':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break; 
  
              case '6':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                 
              case '7':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;  
  
              case '8':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                 
              case '9':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
    
  
              case 'A':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
                
              case 'B':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
                
              case 'C':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
                        
              case 'D':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
                
              case 'E':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
                
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
                
              case 'G':
                    bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
                
              case 'H':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
                
              case 'I':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
                
              case 'J':     
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
                
              case 'K':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
              case 'L':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
         
              case 'M':
                  bleKeyboard.print("");  
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;
  
                
              case 'N':
                  bleKeyboard.print(""); 
                  display.clearDisplay();
                  display.setCursor(0, 0);     // Start at top-left corner                
                  display.write("NOT Assigned");
                  display.display();
                break;          
     
              }     
           }

        }
      } 
   }
 
    else{
      display.clearDisplay();
      display.setTextSize(1);             
      display.setTextColor(SSD1306_WHITE);       
      display.setCursor(28,8);             
      display.print(F("Connect with "));
      display.setCursor(33,18);             
      display.print(F("your device "));
      display.display();
  }
}
