/* TST Project */
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>
HardwareSerial SerialPort(2);

// set the LCD number of columns and rows
int lcdColumns = 20;
int lcdRows = 4;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// ButtonPin Set;

const int buttonTerminal = 2; 
const int buttonMin = 25; 
const int buttonPlus = 26; 

// variables will change:
int countMhs = 0;
int buttonTerState = 0; 
int buttonMinState = 0; 
int buttonPlusState = 0; 
int currenttermi = 0;
int timer = 200;
bool newdata = false;
String terminal = " ";
String receivedData;
String temp;
int TempJumlah =0;
String TempTermi = " ";
//all of Terminal
char *Terminal[]={"GKU1","GKU2","GKU3","Koica","Kehutanan","Gerbang Utama","Asrama"};

void setup() {
  SerialPort.begin(115200, SERIAL_8N1, 16, 17); 
  Serial.begin(115200);
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();

  pinMode(buttonTerminal, INPUT);
  pinMode(buttonMin, INPUT);
  pinMode(buttonPlus, INPUT);
  while(!SerialPort.available()){

  }
  if (SerialPort.available()){
  receivedData = SerialPort.readString();
  temp = receivedData.substring(0,receivedData.indexOf(","));
  countMhs = temp.toInt();
  terminal = receivedData.substring(receivedData.indexOf(",")+1,receivedData.indexOf("."));
  for (int i = 0; i < 7; ++i) {
      temp = Terminal[i];
      if (strcmp(temp.c_str(), terminal.c_str()) == 0) {
          currenttermi = i;
      }
  }
  }
  delay(1000);
}

void loop() {
  //Read buttonState
  buttonTerState = digitalRead(buttonTerminal);
  buttonMinState = digitalRead(buttonMin);
  buttonPlusState = digitalRead(buttonPlus);

  if (buttonTerState == HIGH) {
    currenttermi = currenttermi + 1;
    if (currenttermi > 6){
      currenttermi=0;
    }
    newdata=true;
    timer = 200;
    delay(300);
  } else if (buttonMinState == HIGH) {
    countMhs = countMhs-1;
    if(countMhs<0) {countMhs=0;}
    newdata=true;
    timer = 200;
    delay(300);
  } else if (buttonPlusState == HIGH) {
    countMhs = countMhs+1;
    newdata=true;
    timer = 200;
    delay(300);
  }  

//Kirim data ke output
  if(newdata){
    if(timer<0){
      String send;
      temp=Terminal[currenttermi];
      send=String(countMhs) + "," + temp + ".";
      SerialPort.print(send.c_str());
      Serial.println("sending :");
      Serial.println(send.c_str());
      Serial.println(send);
      timer = 200;
      newdata=false;
    }else{
      timer = timer - 1;
      delay(20);
    }
  }

  displaylcd(Terminal[currenttermi],countMhs);
}

void displaylcd(String Termi,int Jumlah){
if(TempTermi.length()>Termi.length()){
lcd.clear();
}
if(TempJumlah>Jumlah){
lcd.clear();
}
TempJumlah = Jumlah;
TempTermi = Termi;
lcd.setCursor(1,0);
lcd.print(Termi);
lcd.setCursor(1,1);
lcd.print("Jumlah");
lcd.setCursor(1,2);
lcd.print("Mahasiswa:");
lcd.setCursor(1,3);
lcd.print(Jumlah);
}