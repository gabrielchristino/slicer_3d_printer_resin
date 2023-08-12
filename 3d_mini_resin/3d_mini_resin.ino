#define TEMPO_CURA  2000 // tempo de cura da resina pela luz UV (ms)
#define TEMPO_PAUSA 400 // tempo de cura da resina pela luz UV (ms)

#define PASSOS_UM  330// passos do motor por mm movimentado
#define INTERVALO_PASSOS 2 // intervalo entre cada passo
#define PASSOS_VOLTA  8 // passos / volta = 4 ou 8

#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ILI9341.h>     // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions
 
#define SD_CS  10 // SD card select pin
#define TFT_CS  9 // TFT select pin
#define TFT_DC  8 // TFT display/command pin

SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys

Adafruit_ILI9341     tft    = Adafruit_ILI9341(TFT_CS, TFT_DC);

File root;

const bool steps[9][4] = {
{1,0,0,0},
{1,1,0,0},
{0,1,0,0},
{0,1,1,0},
{0,0,1,0},
{0,0,1,1},
{0,0,0,1},
{1,0,0,1},
{0,0,0,0},
};


void setup(void) {

  // habilita portas
  pinMode(A3, INPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  // inicia tft
  tft.begin();
  tft.setRotation(0);
  
  // inicia SD
  if(!SD.begin(SD_CS)) { // ESP32 requires 25 MHz limit
    tft.fillScreen(ILI9341_RED);
    for(;;); // Fatal error, do not continue
  }
  //tft.fillScreen(ILI9341_CYAN);

  root = SD.open("/");
}

bool impressaoFinalizada = false;
int arquivo = 0;

void printFiles(File dir)
{
  while (!impressaoFinalizada) {
    File entry =  dir.openNextFile();
    if (! entry)
    {
      impressaoFinalizada = true;
      moverUM(1);
      return;
    }

    char filename[12];
    sprintf(filename, "out%04d.bmp", arquivo);
    
    reader.drawBMP(filename, tft, 0, 0);
    entry.close();
    
    digitalWrite(3, HIGH);
    delay(TEMPO_CURA);
    digitalWrite(3, LOW);
    tft.fillScreen(ILI9341_BLACK);
    delay(TEMPO_PAUSA);

    moverUM(3);
    moverUM(-2);
    
    arquivo++;
    delay(TEMPO_PAUSA);
    
  }
}

void moverUM(int um) {
  bool dir = um > 0;
  int passo = 0;
  int passos = PASSOS_UM * abs(um);
  int voltas = passos / PASSOS_VOLTA;
  int sobraPassos = int(passos) % PASSOS_VOLTA;
  
  /*Serial.print(PASSOS_UM);
  Serial.print("-");
  Serial.print(passos);
  Serial.print("-");
  Serial.print(voltas);
  Serial.print("-");
  Serial.print(sobraPassos);
  Serial.println("-");*/
  
  while(voltas > 0) {
    while(passo < PASSOS_VOLTA) {
      digitalWrite(4, steps[passo][!dir ? 0 : 3]);
      digitalWrite(5, steps[passo][!dir ? 1 : 2]);
      digitalWrite(6, steps[passo][!dir ? 2 : 1]);
      digitalWrite(7, steps[passo][!dir ? 3 : 0]);
      passo++;
      delay(INTERVALO_PASSOS);
    }
    passo = 0;
    voltas--;
  }

  while(passo < sobraPassos) {
      digitalWrite(4, steps[passo][!dir ? 0 : 3]);
      digitalWrite(5, steps[passo][!dir ? 1 : 2]);
      digitalWrite(6, steps[passo][!dir ? 2 : 1]);
      digitalWrite(7, steps[passo][!dir ? 3 : 0]);
      passo++;
      delay(INTERVALO_PASSOS);
    }
  
  digitalWrite(4, steps[PASSOS_VOLTA][0]);
  digitalWrite(5, steps[PASSOS_VOLTA][1]);
  digitalWrite(6, steps[PASSOS_VOLTA][2]);
  digitalWrite(7, steps[PASSOS_VOLTA][3]);
}


bool teclaPressionada = false;

void loop() {
  if (!teclaPressionada) {
    moverUM(-1);
    teclaPressionada = digitalRead(A3);
  }
  if (teclaPressionada && !impressaoFinalizada) {
    moverUM(1);
    tft.fillScreen(ILI9341_BLACK);
    printFiles(root);
  }
}
