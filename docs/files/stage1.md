# Stage 1 - Testen der Konzeptidee

Im ersten Schritt wird überprüft, ob mit dem ESP und den Bibliotheken eine Messung auf dem Spektral-Sensor ausgeführt werden kann und diese gemessenen Daten in der Cloud InfluxDB gespeichert werden können.

## ESP32

### Generelle Informationen

Gründe für die Verwendung eines [ESP32](https://www.az-delivery.de/en/products/esp-32-dev-kit-c-v4) zur Ansteuerung des Spektral-Sensors sind:

- Integrierter WiFi-Chip
- Erfahrung in der WiFi-Kommunikation
- Handliche Form für die Umsetzung der Stempelidee (Vergleich: RPi sehr unhandlich)
- Schon vorhandene und getestete Bibliotheken für die InfluxDB-Kommunikation und den Spektral-Sensor

### Anschließen des Sensors

<img src="images/Schaltplan_v1.png" class="center">

### Code

Im Folgenden wird der Code für die verschiedenen Stages dargestellt.
Für Stage 1 und Stage 2 werden Durchführungen und Ergebnisse kurz erwähnt.
In Stage 3 wird der finale Code näher beleuchtet.

Zunächst wird die verwendete Entwicklungsumgebung näher erläutert, sowie die verwendeten Bibliotheken und deren Funktionsweisen.
Im Projekt wird Plattform IO als IDE (Integrated Development Environment) verwendet.  
Dies ermöglicht es, einfach cpp Bibliotheken zu installieren und gleichzeitig im Arduino Framework zu bleiben.
Dadurch können schnell erste Versuche mit dem Sensor durchgeführt werden.

Die Code Konzeption für den ersten Stage ist übersichtlich.
Zunächst werden folgende grundlegende Vorgehen geprüft:

- Messung durch Knopf-Druck durchführen.
- Daten im ESP32 sammeln, durch Auslesen der Sensordaten
- Daten in die Cloud Influxdb senden.
- Daten in der Influxdb zur Überprüfung darstellen.

Wie schon erwähnt wurden hierfür zwei Bibliotheken verwendet.

#### Bibliothek zum Auslesen des AS7265X

Die von uns für den ESP32 zum Auslesen der Sensordaten verwendete Bibliothek ist die [SparkFun_AS7265x_Arduino_Library](https://github.com/sparkfun/SparkFun_AS7265x_Arduino_Library).

Zunächst werden Komponenten zum Auslesen des Sensors dargestellt.
Die einzelnen Funktionen werden aufgezählt und deren Funktionsweise erklärt.
Die genaue Umsetzung im Code wird in Stage 3 erklärt.

##### Objektinitialisierung und begin() Funktion

Damit Werte vom Lichtspektrum ausgelesen werden können, muss zunächst eine Objektinstanz der Klasse deklariert und definiert werden.

```cpp
//Instanziierung und Aufrufen der Funktion begin()
AS7265x specSensor;
specSensor.begin();
```

Beim Ausführen der Funktion **begin()**, wird die TwoWire Bibliothek initialisiert.
Diese Methode hat in der Definition den Pointer für das TwoWire Objekt.
Dieses wird hier explizit weggelassen, da es in der Header-File aus der Wire.cpp als Parameter bei der Deklaration mitgegeben wird.

```cpp
//Deklaration der Funktion begin() in der SparkFun_AS7265X.h
boolean begin(TwoWire &wirePort = Wire);
```

Zu Beginn der Methode, wird zunächst das I²C Protokoll initiiert.
Im Anschluss wird geprüft, ob der Sensor erfolgreich mit dem ESP32 verbunden ist (**isConnected()**).
Danach wird überprüft, ob die beiden Slaves bereit sind, um mit ihnen zu kommunizieren.
Dazu wird die **virtualReadRegister()** Methode benutzt und das Register ```0x4F``` ausgelesen, welches diese Information besitzt.
Dabei steht das fünfte Bit jeweils für den ersten und zweiten Slave, wobei dieses nicht gesetzt sein darf.

<img src="images/AS7265X_devicecontrol.jpeg" class="center">

Im nächsten Schritt, werden die einzelnen LED Stromstärken gesetzt.
Dazu wird in die jeweiligen Register für die weiße, infrarot und ultraviolett LED eine 1 gesetzt.
Das Limit wird hier jeweils auf 12,5mA gesetzt.

```cpp
setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_WHITE);
setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_IR);
setBulbCurrent(AS7265X_LED_CURRENT_LIMIT_12_5MA, AS7265x_LED_UV);
```

Danach werden die jeweiligen LEDs explizit ausgeschaltet, um den Sensor vor Überhitzung zu schützen.

```cpp
disableBulb(AS7265x_LED_WHITE); //Turn off bulb to avoid heating sensor
disableBulb(AS7265x_LED_IR);
disableBulb(AS7265x_LED_UV);
```

Im Anschluss wir der Strom für die Status-LED auf 8mA gesetzt und eingeschaltet.

```cpp
setIndicatorCurrent(AS7265X_INDICATOR_CURRENT_LIMIT_8MA); //Set to 8mA (maximum)
enableIndicator();
```

Die Status LED wurde im Code explizit ausgeschaltet.
Zum Ausschalten der PWR (Power) LED muss ein Jumper durchtrennt werden.

Daraufhin wird der Measurement-Modus gesetzt.
Damit können alle Kanäle auf einmal gelesen werden, wenn eine Messung initiiert wird.

Zum Schluss werden noch die Interrupts aktiviert.

Für die letzten Schritte wird ein spezifisches Konfigurationsregister verwendet (siehe Bild unten).
Die einzelnen Bits korrespondieren jeweils mit den unterschiedlichen Aufgaben, die aus den Namen des Bildes unten entnommen werden können.

<img src="images/AS7265X_configuration.jpg" class="center">

##### Funktionen zum auslesen der 18 Kanäle

Zum Auslesen der Sensordaten, besitzt die Bibliothek verschiedene Funktionen.
Zunächst werden Funktionen erklärt, die über I²C die jeweiligen virtuellen Register auslesen.

###### Funktion: virtualReadRegister

<img src="images/AS7265X_flow_chart_virtual_read.jpeg" class="center">

Zunächst wird das Statusregister, unter der Adresse ```0x00```, ausgelesen.
Mit einer Bitmaske wird anschließend geprüft, ob in dem Leseregister noch Daten zu lesen sind.
Falls Daten zu lesen sind, wird das Leseregister ausgelesen.
Diese gelesenen Daten werden jedoch nicht weiter betrachtet und somit verworfen.

```cpp
//Read a virtual register from the AS7265x
uint8_t AS7265X::virtualReadRegister(uint8_t virtualAddr)
{
  uint8_t status;

  //Do a prelim check of the read register
  status = readRegister(AS7265X_STATUS_REG);
  if ((status & AS7265X_RX_VALID) != 0) //There is data to be read
  {
    readRegister(AS7265X_READ_REG); //Read the byte but do nothing with it
  }
```

Im Anschluss wird das Statusregister wieder ausgelesen um zu überprüfen, ob in das Schreibregister geschrieben werden kann.
Kann nicht ins Schreibregister geschrieben werden, wird ein kurzes Delay ausgeführt und der Vorgang solange wiederholt, bis in dieses geschrieben werden kann.

```cpp
 //Wait for WRITE flag to clear
  while (1)
  {
    status = readRegister(AS7265X_STATUS_REG);
    if ((status & AS7265X_TX_VALID) == 0)
      break; // If TX bit is clear, it is ok to write
    delay(AS7265X_POLLING_DELAY);
  }
```

Nun kann die Adresse der entsprechenden Daten in das Schreibregister geschrieben werden.
Das 7. Bit in diesem "Adress-Byte" ist hier 0 um dem System zu sagen, dass es sich hier um einen Lesevorgang handelt.

```cpp
// Send the virtual register address (bit 7 should be 0 to indicate we are reading a register).
  writeRegister(AS7265X_WRITE_REG, virtualAddr);
```

Im Anschluss wird das Statusregister solange ausgelesen, bis in diesem das Leseregister Bit gesetzt ist.
Damit wird signalisiert, dass die angefragten Daten nun im Leseregister vorhanden sind.

```cpp
 //Wait for READ flag to be set
  while (1)
  {
    status = readRegister(AS7265X_STATUS_REG);
    if ((status & AS7265X_RX_VALID) != 0)
      break; // Read data is ready.
    delay(AS7265X_POLLING_DELAY);
  }
```

Wenn das Bit gesetzt ist, lese aus dem Leseregister die Daten heraus.

```cpp
uint8_t incoming = readRegister(AS7265X_READ_REG);
  return (incoming);
```

###### Funktion: virtualWriteRegister

<img src="images/AS7265X_flow_chart_virtual_write.jpeg" class="center">

Um in ein Register zu schreiben wird zunächst geprüft, ob im Statusregister das Bit für das Schreibregister gesetzt ist.
Falls nicht, wird gewartet bis dieses nicht mehr gesetzt ist.

Anschließen wird in das Schreibregister die virtuelle Adresse geschrieben.
Zusätzlich wird an der Stelle des 7. Bit eine 1 geschrieben, um dem Sensor zu sagen, dass es sich hier um einen Schreibvorgang handelt.

```cpp
 // Send the virtual register address (setting bit 7 to indicate we are writing to a register).
  writeRegister(AS7265X_WRITE_REG, (virtualAddr | 1 << 7));
```

Anschließend wird wieder gewartet bis das Schreibregister keinen gesetzten Bit mehr hat.
Wenn das Bit nicht mehr gesetzt ist, dann kann in das Schreibregister die Register-Daten geschrieben werden.

###### Funktion: readRegister

**readRegister()** ist eine Funktion, die die Register der Sensor-Chips über I²C ausliest.

Die TwoWire Bibliothek bedient sich dabei der Abfolge der Schritte des I²C Protokolls.
Zunächst wird die Übertragung mit dem AS7265X initiiert, indem die Funktion **beginTransmission()** aufgerufen wird.
Im Anschluss wird das zu lesende Register gesendet.
Dann wird gewartet, bis die zu lesenden Daten ausgelesen werden können.
Zum Schluss werden die Daten ausgelesen.

```cpp
//Reads from a give location from the AS726x
uint8_t AS7265X::readRegister(uint8_t addr)
{
  _i2cPort->beginTransmission(AS7265X_ADDR);
  _i2cPort->write(addr);
  if (_i2cPort->endTransmission() != 0)
  {
    //Serial.println("No ack!");
    return (0); //Device failed to ack
  }

  _i2cPort->requestFrom((uint8_t)AS7265X_ADDR, (uint8_t)1);
  if (_i2cPort->available())
  {
    return (_i2cPort->read());
  }

  //Serial.println("No ack!");
  return (0); //Device failed to respond
}
```

###### Funktion: writeRegister

Die **writeRegister()**-Funktion funktioniert so:
Zunächst wird wieder die Funktion **beginTransmission()** aufgerufen, um mit dem AS7265X zu kommunizieren.
Im Anschluss wird die Adresse gesendet, wohin die Daten schlussendlich gesendet werden sollen.
Danach werden die Daten gesendet.
Die Verbindung wird mit dem AS7265x geschlossen.

```cpp
//Write a value to a spot in the AS726x
boolean AS7265X::writeRegister(uint8_t addr, uint8_t val)
{
  _i2cPort->beginTransmission(AS7265X_ADDR);
  _i2cPort->write(addr);
  _i2cPort->write(val);
  if (_i2cPort->endTransmission() != 0)
  {
    //Serial.println("No ack!");
    return (false); //Device failed to ack
  }

  return (true);
}
```

###### Funktion: takeMeasurementsWithBulb

Diese Funktion wird benutzt, um erst alle 3 LEDs einzuschalten und danach die 18 Kanäle auszulesen.
Zum Schluss werden die einzelnen LEDs wieder ausgeschaltet.

```cpp
//Turns on all bulbs, takes measurements of all channels, turns off all bulbs
void AS7265X::takeMeasurementsWithBulb()
{
  enableBulb(AS7265x_LED_WHITE);
  enableBulb(AS7265x_LED_IR);
  enableBulb(AS7265x_LED_UV);

  takeMeasurements();

  disableBulb(AS7265x_LED_WHITE); //Turn off bulb to avoid heating sensor
  disableBulb(AS7265x_LED_IR);
  disableBulb(AS7265x_LED_UV);
}
```

Um die LEDs jeweils ein und wieder aus zuschalten, wird sich dem LED_CONFIG Register bedient.

<img src="images/AS7265X_ledconfiguration.jpg" class="center">

Hier wird zunächst mit einem virtualWriteRegister die richtige LED ausgewählt (entweder weiß, infrarot oder UV).
Danach wird in das Register an die Stelle des 3. Bits eine 1 geschrieben, um die LED einzuschalten.

```cpp
//Enable the LED or bulb on a given device
void AS7265X::enableBulb(uint8_t device)
{
  selectDevice(device);

  //Read, mask/set, write
  uint8_t value = virtualReadRegister(AS7265X_LED_CONFIG);
  value |= (1 << 3); //Set the bit
  virtualWriteRegister(AS7265X_LED_CONFIG, value);
}
```

Für die Funktion ```disableBulb()```, wird das Bit an der selben Stelle gelöscht.

```cpp
//Disable the LED or bulb on a given device
void AS7265X::disableBulb(uint8_t device)
{
  selectDevice(device);

  //Read, mask/set, write
  uint8_t value = virtualReadRegister(AS7265X_LED_CONFIG);
  value &= ~(1 << 3); //Clear the bit
  virtualWriteRegister(AS7265X_LED_CONFIG, value);
}
```

Für die eigentliche Messung ist die Funktion **takeMeasurements()** verantwortlich, welche die Messung aller 18 Kanäle steuert.
Ein volle Messung der 18 Kanäle wird dadurch ausgeführt, indem das Konfigurationsregister zuerst gelesen und dann das der Modus auf "Modus 3" gesetzt wird.
Danach wird das Register wieder auf die entsprechende Adresse geschrieben.

```cpp
//Mode 0: 4 channels out of 6 (see datasheet)
//Mode 1: Different 4 channels out of 6 (see datasheet)
//Mode 2: All 6 channels continuously
//Mode 3: One-shot reading of all channels
void AS7265X::setMeasurementMode(uint8_t mode)
{
  if (mode > 0b11)
    mode = 0b11; //Error check

  //Read, mask/set, write
  uint8_t value = virtualReadRegister(AS7265X_CONFIG); //Read
  value &= 0b11110011;                                 //Clear BANK bits
  value |= (mode << 2);                                //Set BANK bits with user's choice
  virtualWriteRegister(AS7265X_CONFIG, value);         //Write
}
```

Zuletzt wird die Funktion **dataAvailable()** in einer While-Schleife aufgerufen.
Dort wird mit einem virtualReadRegister das Konfigurationsregister solange ausgelesen, bis an Bit Position 1 (nicht verwechseln mit Bit-Position 0!) eine 1 steht.
Dadurch wird signalisiert, dass die Daten bereit liegen.

```cpp
//Checks to see if DRDY flag is set in the control setup register
boolean AS7265X::dataAvailable()
{
  uint8_t value = virtualReadRegister(AS7265X_CONFIG);
  return (value & (1 << 1)); //Bit 1 is DATA_RDY
}
```

##### Lesen der einzelnen Kanäle

Die einzelnen Kanäle werden mit den **getCalibrated()** Funktionen ausgelesen.
Diese Methode ist wiederum nur eine Wrapper-Funktion für die Funktion **getCalibratedValue()**.
Als Parameter erhält diese die jeweils entsprechende Adressen der auszulesenden Kanäle und die Geräte-Adresse.
Die Geräte-Adresse entspricht jeweils der Adresse für entweder den AS72651, AS72652 oder AS72653.

###### Funktion: getCalibratedValue()

Zunächst wird die Funktion **selectDevice()** aufgerufen, um das jeweilige Gerät auszuwählen, von dem der Kanal ausgelesen werden soll.

Dazu wird in das Device-Control Register unter der Adresse ```0x4F``` an der jeweiligen Stelle ein Bit gesetzt, je nachdem, von welchem Gerät gelesen werden soll.

```cpp
//As we read various registers we have to point at the master or first/second slave
void AS7265X::selectDevice(uint8_t device)
{
  //Set the bits 0:1. Just overwrite whatever is there because masking in the correct value doesn't work.
  virtualWriteRegister(AS7265X_DEV_SELECT_CONTROL, device);

  //This fails
  //uint8_t value = virtualReadRegister(AS7265X_DEV_SELECT_CONTROL);
  //value &= 0b11111100; //Clear lower two bits
  //if(device < 3) value |= device; //Set the bits
  //virtualWriteRegister(AS7265X_DEV_SELECT_CONTROL, value);
}
```

Im Anschluss wird jeweils ein Byte aus dem zu lesenden Kanal gelesen.
Aus dem Datenblatt haben wir uns hergeleitet, dass immer nur eine Gruppe an Kanälen gleichzeitig gelesen werden kann. (siehe Bild unten)
Deshalb werden hier direkt 4 Bytes ausgelesen.
Eine genaue Erklärung haben wir hierfür jedoch nicht gefunden.

<img src="images/AS7265X_Channel.jpg" class="center">

Die gelesenen Bytes werden nun der Ordnung nach in einen 32-Bit Integer zusammengesetzt und danach in einen Float konvertiert.

```cpp
  //Channel calibrated values are stored big-endian
  uint32_t calBytes = 0;
  calBytes |= ((uint32_t)b0 << (8 * 3));
  calBytes |= ((uint32_t)b1 << (8 * 2));
  calBytes |= ((uint32_t)b2 << (8 * 1));
  calBytes |= ((uint32_t)b3 << (8 * 0));

  return (convertBytesToFloat(calBytes));
```

## Gehäuse

### Erstes Design

<img src="images/gehauseidee.jpg" class="center">

### Umsetzung in Fusion 360

<img src="images/messsystem_side_view.jpeg" class="center">

Das Design des Geräts hatte einige Probleme. Zunächst fehlten Abstände zwischen den gleitenden Teilen, wodurch viel Bearbeitung erforderlich war.

<img src="images/messsystem_split_view.jpeg" class="center">

Im unteren Teil des Gehäuses, in dem sich der AS7265X-Sensor befand, waren die Positionierungsstifte sowohl in der Position als auch in der Breite um 1 mm falsch gemessen. Dies führte zu viel Schleifarbeit  in einem schwer zugänglichen Bereich. Diese Bearbeitung hätte vor dem Aushärten des Gehäuses erfolgen sollen, um das Gehäuse leichter bearbeiten zu können.

<img src="images/messsystem_printed_bottom2.jpeg" class="center">

Das Gehäuse war zudem viel zu hoch, wodurch der Sensor zu weit von der Öffnung entfernt saß. In der nächsten Revision wurde das Gehäuse um 4 mm abgesenkt, um sicherzustellen, dass der Sensor möglichst nah an der Öffnung ist. Ein zu großer Abstand zwischen dem Sensor und der Öffnung kann dazu führen, dass das Gehäuse selbst gemessen wird und somit den Messwert des Sensors verfälscht. Es ist daher wichtig, dass der Sensor möglichst nah an der Öffnung angebracht ist, um eine genaue Messung zu gewährleisten.

<img src="images/messsystem_top_esp.jpeg" class="center">

Im oberen Teil des Gehäuses befanden sich die Öffnungen für die Kabel des ESP. Diese waren etwas eng und es war daher schwierig, sie einzuführen, ohne die Pins zu verbiegen. Die beiden Stiftbänke führten auch zu unterschiedlichen Seiten des AS7265-Sensors, was eine umständliche Umverlegung erforderte. Die Kappe für das ESP-Gehäuse bot zudem nicht genug Platz für ein USB-Kabel und war zu klein, was dazu führte, dass sie schwer zu positionieren war und eine fragwürdige Haltbarkeit aufwies.

Um das Gehäuse zu optimieren, gibt es einige mögliche Verbesserungen, die über die bereits genannten Probleme hinausgehen. Zum Beispiel könnte man das AS7265X-Gehäuse asymmetrisch gestalten und nur einen der Qwicc-Anschlüsse zugänglich machen. Dies würde das Design kleiner machen oder mehr Platz für die Kabelführung bieten und den Abstand zwischen der Kabelführungsöffnung und dem Sensor vergrößern, wodurch die Störung durch externes Licht verringert würde. Eine seitliche Montage des ESP würde das Handling erleichtern, da das Gerät in der Regel an einen Laptop angeschlossen wird und sich in diesem Fall in der Höhe oder unterhalb des Laptops befindet. Abgerundete Kanten würden das Gerät besser in der Hand liegen lassen und eine reduzierte Wandstärke von maximal 2 mm würde Defekte durch Gewicht und Materialverschwendung verringern.

## Setup Cloud DB

### BwCloud

Mit folgenden Schritten kann man sich auf der BwCloud Weboberfläche anmelden:

- Zur BwCloud Weboberfläche [www.bw-cloud.org](https://www.bw-cloud.org) navigieren.
- Menüpunkt **Dashboard** auswählen.
- **Anmelden**.
- BwCloud SCOPE: Hochschule Heilbronn auswählen.
- Mit dem Hochschulaccount anmelden.

[BwCloud Dokumentation](https://www.bw-cloud.org/de/erste_schritte) zu den vorherigen Schritten.

Erstelle anschließend eine virtuelle Instanz (vServer).
BwCloud Dokumentation: [Eine Instanz starten](https://www.bw-cloud.org/de/bwcloud_scope/nutzen#launch_instance)

Um sich mit der virtuellen Instanz zu verbinden ist standardmäßig Port 22 für den SSH Zugang freigeschaltet.
Damit aber auch auf die InfluxDB und Grafana zugegriffen werden kann müssen noch Port 3000 (Grafana) und 8086 (InfluxDB) freigegeben werden.
BwCloud Dokumentation: [Einen Port für Zugriff (von außen) öffnen](https://www.bw-cloud.org/de/bwcloud_scope/nutzen#open_port)

> Wir haben uns für die BwCloud entschieden, da sie für Studenten einen kostenlosen Server bereitstellt.
> Außerdem kann auch jeder andere Serverprovider verwendet werden.
> Wenn die Daten nur lokal zur Verfügung stehen müssen, kann auch der eigene Rechner verwendet werden.

### Docker Installation

Eine Anleitung wie Docker auf Ubuntu zu installieren ist, kann in der [Docker Dokumentation](https://docs.docker.com/engine/install/ubuntu/) gefunden werden.
Damit bei einer erneuten Installation nicht wieder alle Befehle kopiert und ausgeführt werden müssen, haben wir diese wieder in einem Shell Skript zusammengefasst.

```bash
#!/bin/bash

# Uninstall old versions
sudo apt-get remove docker docker-engine docker.io containerd runc -y

# Update the apt package index and install packages to allow apt to use a repository over HTTPS:
sudo apt-get update
sudo apt-get install ca-certificates curl gnupg lsb-release -y

# Add Docker’s official GPG key:
sudo mkdir -p /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg

# Use the following command to set up the repository:
echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# Install Docker Engine
sudo apt-get update
sudo apt-get install docker-ce docker-ce-cli containerd.io docker-compose-plugin -y
```

> Dieses Docker installations Skript unterscheidet sich von dem des Raspberry Pis in wenigen Punkten!

## InfluxDB

Im ersten Schritt haben wir dieses Mal nur die InfluxDB aufgesetzt um zu testen, ob Daten von dem ESP32 aus an die Datenbank geschickt werden können.

Erstelle dafür die Datei ```docker-compose.yml``` mit folgendem Inhalt:

```yml
version: '3'
services:
  influxdb:
    image: influxdb:latest
    container_name: influxdb
    restart: always
    ports:
      - "8086:8086"
    volumes:
      - ./influxdb/data:/var/lib/influxdb2/
      - ./influxdb/config:/etc/influxdb2/
    environment:
      - DOCKER_INFLUXDB_INIT_MODE=setup
      - DOCKER_INFLUXDB_INIT_USERNAME=${INFLUXDB_ADMIN_USER}
      - DOCKER_INFLUXDB_INIT_PASSWORD=${INFLUXDB_ADMIN_PASSWORD}
      - DOCKER_INFLUXDB_INIT_ORG=${INFLUXDB_ORG}
      - DOCKER_INFLUXDB_INIT_BUCKET=${INFLUXDB_BUCKET}
      - DOCKER_INFLUXDB_INIT_ADMIN_TOKEN=${INFLUXDB_ADMIN_TOKEN}
```

Starte die Docker Container mit ```docker compose up -d```.

### API- und UI-Zugriff erlangen

Das User-Interface (UI) und die API nutzen Port 8086.
Um diesen Container Port auch von außen erreichbar zu machen wird in der docker-compose.yml unter dem Punkt ```ports:``` der Host (BwCloud-Server) Port 8086 auf den Container Port 8086 zugeordnet.

### Daten persistieren

Das InfluxDB-Image stellt ein freigegebenes Volume unter ```/var/lib/influxdb2``` bereit. Um die Containerdaten der Datenbank zu persistieren, haben wir hier ein Host-Verzeichnis an diesem Punkt bereitgestellt.

> Beachte, dass dieser Pfad sich von den InfluxDB Versionen 1.x unterscheidet!

### Automatisierte Datenbankeinrichtung

Das InfluxDB-Image enthält einige zusätzliche Funktionen, um das System automatisch zu booten.
Diese Funktionalität wird aktiviert, indem die Umgebungsvariable ```DOCKER_INFLUXDB_INIT_MODE``` auf den Wert ```setup``` gesetzt wird, wenn der Container ausgeführt wird.
Zusätzliche Umgebungsvariablen werden verwendet, um die Setup-Logik zu konfigurieren:

- ```DOCKER_INFLUXDB_INIT_USERNAME```: Der Benutzername, der für den anfänglichen Superuser des Systems festgelegt werden soll (erforderlich).
- ```DOCKER_INFLUXDB_INIT_PASSWORD```: Das Kennwort, das für den ersten Superuser des Systems festgelegt werden soll (erforderlich).
- ```DOCKER_INFLUXDB_INIT_ORG```: Der Name, der für die anfängliche Organisation des Systems festgelegt werden soll (erforderlich).
- ```DOCKER_INFLUXDB_INIT_BUCKET```: Der Name, der für den anfänglichen Bucket des Systems festgelegt werden soll (erforderlich).
- ```DOCKER_INFLUXDB_INIT_ADMIN_TOKEN```: Das Authentifizierungstoken, das dem anfänglichen Superuser des Systems zugeordnet werden soll. Wenn nicht festgelegt, wird vom System automatisch ein Token generiert.

Das automatisierte Setup generiert Metadaten-Dateien und CLI-Konfigurationen.
Es wird empfohlen, Volumes auf beiden Pfaden (```/var/lib/influxdb2/```, ```/etc/influxdb2/```) bereitzustellen, um Datenverluste zu vermeiden.

### InfluxDB Weboberfläche

Den erfolgreichen Start des Containers kann überprüft werden, indem man sich auf der Weboberfläche ```http://YOUR-SERVER-ADDRESS:8086/``` anmeldet.

Ob der ESP die Messdaten über die API Schnittstelle auf der Datenbank speichern kann, testet man indem in der Weboberfläche nachgeschaut, ob die Daten in der Datenbank vorliegen.

<img src="images/influxdb_test_graph.jpeg">

Die Repräsentation der Daten in der Oberfläche entspricht noch keinem Format, das zum Vergleichen verwendet werden kann.
Aber es zeigt das die Daten erfolgreich in der Datenbank liegen und diese Daten auch wieder ausgelesen werden können.
