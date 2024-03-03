#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h> 

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int Password_Length = 4;
String Data; // Character to hold input
String Password = "A3C1";

// Counter for character entries
byte data_count = 0;

// Character to hold key input
char customKey;

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Connections to Arduino
byte rowPins[ROWS] = {7, 6, 5, 4};
byte colPins[COLS] = {3, 2, 1, 0};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
Servo Servo1;

int err = 0;
int change = 0;
int setNew = 0;

const int green = A0;
const int red = A3;
const int buzzer = A1;
const int servoPin = A2; 

unsigned long prev = 0;
const long interval = 60000; // 1 min
unsigned long current;

void setup() 
{
    lcd.init();           // initialize the lcd
    lcd.backlight();      // Turn on the LCD screen backlight
    allTimeMessage(); 
  
    SPI.begin();          // Initiate  SPI bus
    mfrc522.PCD_Init();   // Initiate MFRC522

    pinMode(red, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(buzzer, OUTPUT); 

    Servo1.attach(servoPin); 
}
void loop() 
{
    current = millis();
    if(current - prev >= interval)
    {
        err = 0;
        prev = current;
    }
    // Look for keypress
    customKey = customKeypad.getKey();
    if(customKey) 
    {
        // Enter keypress into array and increment counter
        Data += customKey;
        lcd.setCursor(data_count, 1);
        lcd.print(Data[data_count]);
        data_count++;
        if(data_count == 1)
        {
            for(int i=1; i<=15; i++)
            {
              lcd.print(" ");
            }
        }
    }

    if(data_count == 3) //change password
    {
       if(Data == "CC*")
       {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Old Password:");
            change = 1;
            data_count = 0;
            Data ="";
       }
    }
    
    // See if we have reached the password length
    if(data_count == Password_Length) 
    {
        lcd.clear();

        if(setNew == 1)
        {
            Password = Data;
            setNew = 0; 
            lcd.clear();
            lcd.setCursor(2, 0);
            lcd.print("Password set");
            lcd.setCursor(1,1);
            lcd.print("successfully!");
            delay(2000);
            allTimeMessage();
        }
        else
        {
            if(Data == Password) 
            {     
                if(change == 1)
                {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("New Password:");
                    setNew = 1;
                    change = 0;
                }
                else
                {
                    stopErr();
                    openDoor(); 
                    allTimeMessage();
                }
            }
            else 
            {
                error();
                notOkMessage();
                allTimeMessage();
            }
        }

        data_count = 0;
        Data ="";
    }

    
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) 
    {
      return;
    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) 
    {
      return;
    }
  
    String content= "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++) 
    {
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
  
    if (content.substring(1) == "23 83 52 13") //change here the UID of the cards that you want to give access
    {
        stopErr();
        openDoor(); 
        allTimeMessage();
    }
    else   
    {
        error();
        notOkMessage();
        allTimeMessage();
    }
} 

void error()
{
    err += 1;
    if(err == 3)
    {
        digitalWrite(red, HIGH);
        tone(buzzer, 700);
    }
}

void stopErr()
{
    if(err >= 3)
    {
        digitalWrite(red, LOW);
        noTone(buzzer);
    }
    err = 0;
}

void notOkMessage()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access denied...");
    delay(2000); 
}
void allTimeMessage()
{
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Welcome!");
    lcd.setCursor(1,1);
    lcd.print("Card/Password?");
}

void openDoor()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Authorized access");
    digitalWrite(green, HIGH);
    Servo1.write(0);
    delay(3000);  
    Servo1.write(90);
    digitalWrite(green, LOW); 
}
