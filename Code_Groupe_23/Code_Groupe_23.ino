#include "config.h"              // Inclure le fichier de configuration contenant les informations d'Adafruit IO
#include <Adafruit_BME280.h>    // Inclure la bibliothèque pour le capteur BME280
#include "RTClib.h"              // Inclure la bibliothèque pour le module RTC

#define adresseI2CduBME280                0x76            // Adresse I2C du capteur BME280 (0x76 dans mon cas, souvent la valeur par défaut)
#define pressionAuNiveauDeLaMerEnHpa      1024.90         // Pression au niveau de la mer en hPa (utilisée pour le calcul de l'altitude)
#define delaiRafraichissementAffichage    1500            // Délai de rafraîchissement de l'affichage en millisecondes

int count = 0;                       // Variable pour le comptage
RTC_DS3231 rtc;                       // Objet RTC pour la gestion du temps
AdafruitIO_Feed *counter = io.feed("counter");  // Déclaration du feed Adafruit IO pour le compteur
AdafruitIO_Feed *temp = io.feed("temp");        // Déclaration du feed Adafruit IO pour la température
Adafruit_BME280 bme;                  // Objet pour le capteur BME280
#include <SPI.h>
#include <SD.h>

const int chipSelect = 4;             // Broche de sélection du module SD

void setup() {
  Serial.begin(115200);               // Initialisation de la communication série
  while(! Serial);                    // Attendre l'ouverture du moniteur série
  Serial.print("Connexion à Adafruit IO");
  io.connect();                       // Connexion à io.adafruit.com
  
  // Attendre la connexion
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println(io.statusText());

  // Initialisation de la carte SD
  if (!SD.begin(chipSelect)) {
    Serial.println("Carte SD échouée ou non présente");
    return;
  }
  Serial.println("Carte initialisée.");
  while(!Serial);
  Serial.println("Programme de test du BME280");
  Serial.println("===========================");
  Serial.println();

  // Initialisation du capteur BME280
  Serial.print(F("Initialisation du BME280, à l'adresse [0x"));
  Serial.print(adresseI2CduBME280, HEX);
  Serial.println(F("]"));

  // Vérifier si l'initialisation du BME280 a réussi
  if(!bme.begin(adresseI2CduBME280)) {
    Serial.println(F("--> ÉCHEC…"));
    while(1);
  } else {
    Serial.println(F("--> RÉUSSIE !"));
  }

  // Vérifier la présence du module RTC
  if (! rtc.begin()) {
    Serial.println("RTC introuvable");
    Serial.flush();
    while (1) delay(10);
  }

  // Si le module RTC a perdu sa puissance, ajuster la date et l'heure
  if (rtc.lostPower()) {
    Serial.println("RTC a perdu sa puissance, réglons la date et l'heure !");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  Serial.println("TIME ; TEMP ; PRESSURE ; HUMIDITY ; VOC ; ALTITUDE");
}

void loop() {
  io.run();                           // Exécuter les tâches nécessaires à la communication avec Adafruit IO
  Serial.print("Envoi -> ");
  Serial.println(count);
  counter->save(count);               // Sauvegarder la valeur du compteur dans le feed Adafruit IO
  temp->save(bme.readTemperature());  // Sauvegarder la température dans le feed Adafruit IO

  count++;                            // Incrémenter le compteur

  DateTime now = rtc.now();
  String dataString = String(now.unixtime()) + ";" + String(bme.readTemperature()) + ";"  +String(bme.readPressure()) + ";" +String(bme.readHumidity()) + ";" +String(bme.readAltitude(pressionAuNiveauDeLaMerEnHpa));

  File dataFile = SD.open("datalog.txt", FILE_WRITE);  // Ouvrir le fichier txt de la carte sd

  if (dataFile) {//si le datafile est ouvert on écrit dedans les données
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
  } else {
    Serial.println("Erreur d'ouverture de datalog.txt");
  }
  delay(3000); 
}
