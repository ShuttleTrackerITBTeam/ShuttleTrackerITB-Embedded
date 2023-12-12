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
int timer = 20;
bool newdata = false;
String terminal = " ";
String receivedData;
String temp;
int TempJumlah =0;
String TempTermi = " ";
//all of Terminal
char *Terminalrute1[]={"Gerbang Utama", "Labtek 1B", "GKU 2","GKU 1","Rektorat","Koica/GKU 3","GSG","Asrama"};
char *Terminalrute2[]={"Gerbang Utama","Asrama","GSG","Koica/GKU 3","Rektorat","Kehutanan"};

char** ruteShuttle = Terminalrute1;

int rute1Length = 8;
int rute2Length = 6;

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
  for (int i = 0; i < rute1Length; ++i) {
      temp = ruteShuttle[i];
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
    if (currenttermi > rute1Length-1){
      currenttermi=0;
    }
    newdata=true;
    timer = 20;
    delay(300);
  } else if (buttonMinState == HIGH) {
    countMhs = countMhs-1;
    if(countMhs<0) {countMhs=0;}
    newdata=true;
    timer = 20;
    delay(300);
  } else if (buttonPlusState == HIGH) {
    countMhs = countMhs+1;
    newdata=true;
    timer = 20;
    delay(300);
  }  

//Kirim data ke output
  if(newdata){
    if(timer<0){
      String send;
      temp=ruteShuttle[currenttermi];
      send=String(countMhs) + "," + temp + ".";
      SerialPort.print(send.c_str());
      Serial.println("sending :");
      Serial.println(send.c_str());
      Serial.println(send);
      newdata=false;
    }else{
      timer = timer - 1;
      delay(10);
    }
  }

  displaylcd(ruteShuttle[currenttermi],countMhs);
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
  lcd.setCursor(10-Termi.length()/2,0);
  lcd.print(Termi);
  lcd.setCursor(7,1);
  lcd.print("Jumlah");
  lcd.setCursor(5,2);
  lcd.print("Mahasiswa:");
  lcd.setCursor(9,3);
  lcd.print(Jumlah);
}