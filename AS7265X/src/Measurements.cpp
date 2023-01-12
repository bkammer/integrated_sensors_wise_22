#include <Arduino.h>
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#include "SparkFun_AS7265X.h"
#include "Measurements.h"
// WiFi AP SSID
#define WIFI_SSID "YOURWIFISSID"
// WiFi password
#define WIFI_PASSWORD "YOURWIFIPASSWORD"

// Define influxdb relevant informations
#define INFLUXDB_URL "YOURINFLUXDBURL"
#define INFLUXDB_TOKEN "YOURINFLUXDBTOKEN"
#define INFLUXDB_BUCKET "YOURINFLUXDBBUCKET"
#define INFLUXDB_ORG "YOURINFLUXDBORG"
// Define timezone
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// Variable to block the worflow, when a measurement is currently ongoing
bool measurement_done = true;
// Current active tag, that is used to label all values
String currentTag = String("");
// Current active measurement, to which data is sent to
String currentMeasurement = String("");

// Initialize wifi client
WiFiMulti wifiMulti;

// Initialize specSensor
AS7265X specSensor;

// Initialize InfluxDB client
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare the data point for the measurements
Point *sensorData = NULL;

int takeMeasurement = 0;

void setup()
{
  Serial.begin(115200);
  // AS7265X Button
  pinMode(4, INPUT_PULLUP);
  attachInterrupt(4, setMeasurementFlag, FALLING);

  // Buzzer
  pinMode(13, OUTPUT);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Sync time
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  if (specSensor.begin() == false)
  {
    Serial.println("specSensor does not appear to be connected. Please check wiring. Freezing...");
    while (1)
      ;
  }
  specSensor.disableIndicator();

  // Check server connection
  if (client.validateConnection())
  {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
    Serial.println("Please initialize a measurement and tag name to start measureing!");
    setNewMeasurement();
    setNewTag();
    digitalWrite(13, HIGH);
    delay(300);
    digitalWrite(13, LOW);
    delay(300);
    digitalWrite(13, HIGH);
    delay(300);
    digitalWrite(13, LOW);
  }
  else
  {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop()
{ String serialString = String("");
  serialString = Serial.readString();
  if (serialString == "tag")
  {
    setNewTag();
    
  }
  else if (serialString == "measurement")
  {
    setNewMeasurement();    // Stay in mode, until the new measurement is set
    
  }
  if (takeMeasurement)
  {
    takeMeasurement = 0;

    takeSensorData();
  }
}

void setNewTag()
{
  Serial.println("Entered mode to set a new tag. Please enter the new tag name and press ENTER. If you are done, write 'exit'.");
  // Stay in mode, until the new tag is set
  while (1)
  {
    if (Serial.available())
    {
      String currentString = Serial.readString();
      if (currentString == "exit")
      {
        Serial.println("Exiting set new tag mode.");
        return;
      }
      else
      {
        currentTag = currentString;
      }
    }
  }
}

void setNewMeasurement()
{
  Serial.println("Entered mode to set a new measurement. Please enter the new measurement name and press ENTER. If you are done, write 'exit'.");

  // Stay in mode, until the new measurement is set
  while (1)
  {
    if (Serial.available())
    {
      String currentString = Serial.readString();
      if (currentString == "exit")
      {
        Serial.println("Exiting set new measurement mode.");
        return;
      }
      else
      {
        if (sensorData != NULL)
        {
          delete sensorData;
        }
        sensorData = new Point(currentString);
      }
    }
  }
}

void setMeasurementFlag()
{
  takeMeasurement = 1;
}

void IRAM_ATTR takeSensorData()
{
  digitalWrite(13, HIGH);
  Serial.println("Begin measurement....");
  specSensor.takeMeasurementsWithBulb(); // This is a hard wait while all 18 channels are measured
  digitalWrite(13, LOW);
  sensorData->clearFields();
  sensorData->clearTags();
  sensorData->addField("410nm", specSensor.getCalibratedA());
  sensorData->addField("435nm", specSensor.getCalibratedB());
  sensorData->addField("460nm", specSensor.getCalibratedC());
  sensorData->addField("485nm", specSensor.getCalibratedD());
  sensorData->addField("510nm", specSensor.getCalibratedE());
  sensorData->addField("535nm", specSensor.getCalibratedF());
  sensorData->addField("560nm", specSensor.getCalibratedG());
  sensorData->addField("585nm", specSensor.getCalibratedH());
  sensorData->addField("610nm", specSensor.getCalibratedR());
  sensorData->addField("645nm", specSensor.getCalibratedI());
  sensorData->addField("680nm", specSensor.getCalibratedS());
  sensorData->addField("705nm", specSensor.getCalibratedJ());
  sensorData->addField("730nm", specSensor.getCalibratedT());
  sensorData->addField("760nm", specSensor.getCalibratedU());
  sensorData->addField("810nm", specSensor.getCalibratedV());
  sensorData->addField("860nm", specSensor.getCalibratedW());
  sensorData->addField("900nm", specSensor.getCalibratedK());
  sensorData->addField("940nm", specSensor.getCalibratedL());
  sensorData->addTag("Object name", currentTag);

  // Check WiFi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.println("Wifi connection lost");
  }
  // Write point
  if (!client.writePoint(*(sensorData)))
  {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  else
  {
    Serial.print("Done sending data!");
  }
}