# Projektaufbau 1

## Umsetzungsidee

Die Idee der Umsetzung des Messsystems besteht zu Beginn aus dem Spektral-Sensor AS7265x und einem Raspberry Pi 2.
Der Raspberry Pi 2 kommt aus dem Grund zum Einsatz, da dieser bereits vorhanden ist und nicht bestellt werden muss.
Der Sensor wird dabei direkt an die I²C Pins der GPIO Schnittstelle des Raspberry Pis (RPi) angeschlossen.

Auf dem Raspberry Pi wird zuerst Docker installiert, mit dessen Hilfe dann ein InfluxDB Container zur Datenspeicherung und ein Grafana Container zur Datenvisualisierung gestartet wird.
Mit einem Python Skript, dass direkt (ohne Docker) auf dem RPi laufen soll, soll die Messung auf dem Spektral-Sensor gestartet und die gemessenen Daten in die InfluxDB gespeichert werden.

<img src="images/highLevelServiceStructure.svg" class="center">

> In den folgenden Abschnitten wird nur auf die wichtigsten Alleinstellungsmerkmale des ersten Prototyps eingegangen, um Redundanz mit dem zweiten Prototyp zu vermeiden.
> Ausführlichere Beschreibungen zu folgenden Punkten finden sich beim 2. Prototyp wieder.

## Verwendete Komponenten

Hardware Materialien:

- [SparkFun Triad Spectroscopy Sensor - AS7265x (Qwicc)](https://www.sparkfun.com/products/15050)
- [Raspberry Pi 2 Model B](https://www.raspberrypi.com/products/raspberry-pi-2-model-b/)

Verwendete Software:

- Raspberry Pi
  - [Wiring Pi](http://wiringpi.com/)
  - [Docker](https://www.docker.com/)
  - [InfluxDB](https://www.influxdata.com/)
  - [Grafana](https://grafana.com/)

Weitere Komponenten:

- 3D-Drucker (Filament)
- [Fusion360](https://www.autodesk.de/products/fusion-360/overview)

## Prototyp 1 (Raspberry Pi)

### Anschließen des Spektral-Sensors an den Raspberry Pi 2

Durch die GPIO Pins des Raspberry Pis, die auf 3,3 Volt arbeiten, ist es möglich den Spektral-Sensor direkt ohne externe Spannungsquelle und Logic-Converter an den RPi anzuschließen.

Um überprüfen zu können, ob dieser auch korrekt angeschlossen ist werden die IC-Tools mit dem Befehl ```apt-get install i2c-tools``` installiert.
I2C-Tools beinhalten vier Befehle von dem **i2cdetect** ein Programm ist, das I2C-Busse nach Geräten scannt.
Mit dem Befehl ```i2cdetect -y 1``` wurde die Busadresse des Spektral-Sensors erfolgreich gefunden.

<img src="images/i2cdetect.png" class="center">

Damit das Python-Skript mit dem Spektral-Sensor kommunizieren kann, importiert das Skript die Bibliothek Wiring Pi.
Zur Zeitersparnis wird getestet, ob das Skript der Thesis (siehe Link unten) auf diese Anwendung übertragbar ist.
Bei dem RPi 4 ist Wiring Pi, nach Aussage der Bachelor Thesis ([SpectralSensor](https://github.com/LennardBoediger/SpectralSensor)), bereits vorinstalliert, bei dem verwendeten RPi 2 ist das jedoch nicht der Fall.
Die Bibliothek Wiring Pi ist schon etwas älter, sodass erst eine passende Version der Bibliothek gefunden werden muss.
Eine erfolgreiche Installation wurde nach dem Suchen durch folgenden Befehl erlangt:

```sh
wget https://project-downloads.drogon.net/wiringpi-latest.deb
sudo dpkg -i wiringpi-latest.deb
```

Es gibt inzwischen bessere Bibliotheken, um die GPIOs des RPis zu steuern, sodass in Erwägung gezogen werden sollte die Bibliothek Wiring Pi auszutauschen.
Das verwenden einer anderen Bibliothek wird sogar von Wiring Pi (siehe [git.drogon.net](https://git.drogon.net/?p=wiringPi;a=summary)) empfohlen.

Das Skript der Bachelor Thesis konnte ausgeführt werden, jedoch war es unbekannt ob es wie vorhergesehen läuft.
Die Messung schien ausgeführt zu werden, aber es ist nicht bekannt was mit den Daten passiert.
Die Vermutung belief sich darauf, das keine Datenbank für die Datenspeicherung angegeben wurde, sodass der weitere Schritt sich auf dessen Aufsetzen ausrichtete.

### Datenspeicher- und Visualisierungssystem aufsetzen

Im zweiten Schritt wird geprüft, ob die Datenbank InfluxDB und Grafana mit Docker auf dem RPi 2 betrieben werden kann.

Die Installationsschritte sind im folgenden Shell Skript (**install_docker.sh**) zusammengefasst:

```sh
#!/bin/sh

# Uninstall old versions
sudo apt-get remove docker docker-engine docker.io containerd runc

# Update the apt package index and install packages to allow apt to use a repository over HTTPS:
sudo apt-get update
sudo apt-get install ca-certificates curl gnupg lsb-release

# Add Docker’s official GPG key:
curl -fsSL https://download.docker.com/linux/debian/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg

# Set up the stable repository. 
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/debian $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# Update the apt package index, and install the latest version of Docker Engine, containerd, and Docker Compose
sudo apt-get update
sudo apt-get install docker-ce docker-ce-cli containerd.io docker-compose-plugin -y
```

Github: [install_docker.sh](https://github.com/bkammer/integratedsensors_wise22/blob/main/raspberrypi/setup/install_docker.sh)

> Damit der Besitzer des Skriptes dieses ausführen kann, muss die entsprechende Ausführ-Berechtigung (```chmod u+x install_docker.sh```) gesetzt werden.
> Anschließend kann mit dem Shell-Befehl ```./install_docker.sh``` das Skript ausgeführt werden.

Nach der Installation von Docker wird eine Docker Compose Datei erstellt, um InfluxDB und Grafana aufzusetzen.
Dabei ist es nicht möglich die neuste Version von InfluxDB zu nehmen, da diese nicht auf dem RPi 2 startet.
Denn die neuste Version von InfluxDB läuft nur auf einem 64-Bit System.
Jedoch handelt es sich bei dem Prozessor des Raspberry Pi 2 um einen 32-Bit Architektur.

Um dies zu umgehen, setzen wir auf die aktuellste Version von InfluxDB, die noch 32-Bit unterstützt, welche Version 1.8 ist.
Die hierfür verwendete **docker-compose.yml** sieht damit wie folgt aus:

```yml
services:
  influxdb:
    image: influxdb:1.8 # 1.8 is the latest version which is running on a 32bit system
    ports:
      - '8086:8086'
    volumes:
      - influxdb-storage:/var/lib/influxdb
    environment:
      - INFLUXDB_DB=${INFLUXDB_DATABASE}
      - INFLUXDB_ADMIN_USER=${INFLUXDB_USERNAME}
      - INFLUXDB_ADMIN_PASSWORD=${INFLUXDB_PASSWORD}
  grafana:
    image: grafana/grafana
    ports:
      - '3000:3000'
    volumes:
      - ./grafana-storage:/var/lib/grafana # To keep grafana configuration like the dashboard persistent.
      - ./grafana-provisioning/:/etc/grafana/provisioning
    depends_on:
      - influxdb
    environment:
      - GF_SECURITY_ADMIN_USER=${GRAFANA_USERNAME}
      - GF_SECURITY_ADMIN_PASSWORD=${GRAFANA_PASSWORD}

volumes:
  influxdb-storage:
```

## Konzeptänderung

Beim Einrichten der InfluxDB als Datenquelle in Grafana wurde festgestellt, dass diese Datenbankversion (InfluxDB 1.8) nicht mehr in Grafana unterstützt wird.
Ein möglicher Lösungsansatz könnte dabei ebenfalls sein, auf eine ältere Version von Grafana zurückzugreifen.
Dabei stellt sich nur die Frage, wie sinnvoll dieser Lösungsansatz ist, nur auf ältere Hardware und Software zurückzugreifen, die teilweise nicht mehr maintained bzw. aktualisiert wird.
Eine weitere Lösung zieht eine Änderung in der Hardware auf sich.
Anstelle des RPi 2 könnte ein RPi 4 zum Einsatz kommen dessen Prozessor auf einer 64-Bit Architektur setzt.
Damit kann die neuste Version von InfluxDB und Grafana verwendet werden.

Hardwaretechnisch kann auch ein anderer Lösungsansatz in Betracht gezogen werden.
Der Spektral-Sensor könnte auch mit einem Mikrocontroller, wie den ESP32, betrieben werden.
Dieser besitzt bereits, im Gegensatz zu dem RPi 2, eine integrierte WLAN-Schnittstelle.
Diese kann genutzt werden, um die Daten des Sensors an die Datenbank zu senden.
Weitere Vorteile des ESP32 beziehen sich auf die Arduino Entwicklungsumgebung, mit der sich der ESP32 programmieren lässt.
Dieser Vorteil kommt dadurch zum Tragen, da es für die Arduino Plattform bereits eine Bibliothek für den Spektral-Sensor sowie InfluxDB gibt.
Im Weiteren ist der Aufbau mit dem Sensor und einem ESP32 wesentlich handlicher und portabler als mit einem RPi.

Jedoch lässt sich InfluxDB und Grafana nicht auf einem ESP32 betreiben, sodass hier eine Alternative gefunden werden muss.
Hier kann wieder ein RPi zum Einsatz kommen.
Hierbei besteht jedoch das Problem, dass dieser sich immer im gleichen Netzwerk wie der ESP befinden muss, um für den ESP erreichbar zu sein.
Dadurch müssten beide Systeme unterwegs mitgenommen werden, was nicht praktikabel ist.

Lösung hier ist es auf eine Cloud-Instanz zu setzen, die von "überall" erreichbar ist.
Damit können auch mehrere der Messsysteme auf die gleiche Cloud-Datenbank zurückgreifen, welches die Herausforderung des Datenaustausches unter den Systemen vereinfacht.
Damit kann hier auf eine Datenquelle zurückgegriffen werden, um zukünftig Rückschlüsse auf Eigenschaften von gemessenen Produkte aufzubauen.
Ebenfalls kann auch über jedes (mobiles) Endgerät auf die visualisierten Daten in Grafana zurückgegriffen werden.
