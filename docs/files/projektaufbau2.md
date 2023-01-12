# Projektaufbau 2

## Umsetzungsidee

Der neue Projektaufbau besteht aus dem Spektral-Sensor, der an den ESP32 angeschlossen ist.
Dieser führt auf Knopfdruck eine Messung aus und sendet die Daten an eine in der Cloud gehosteten Datenbank.
Grafana läuft ebenfalls auf dieser Cloud Instanz, um von jedem Endgerät erreichbar zu sein.
Zudem besitzt das Messsystem für die Mobilität ein Batteriepack und einen Buzzer für eine akustische Rückmeldung bei Messungen.

## Verwendete Komponenten

Die hierfür verwendeten Hardware Materialien belaufen sich auf:

- [SparkFun Triad Spectroscopy Sensor - AS7265x (Qwicc)](https://www.sparkfun.com/products/15050)
- [ESP32 NODEMCU Module](https://www.az-delivery.de/en/products/esp32-developmentboard)
- [Lademodul / Entlademodul - HW-107](https://akkuplus.de/Lademodul-Entlademodul-HW-107-zum-Laden-Entladen-einer-Li-Ion-Li-Polymer-Zelle-Micro-USBar)
- [KY-012 Buzzer Modul aktiv](https://www.az-delivery.de/products/buzzer-modul-aktiv?variant=8175828729952)
- [SX1308 Step-Up Converter Regulator](https://www.amazon.de/ANGEEK-Step-up-Wandler-Booster-Versorgungs/dp/B07RGZXPYY)
- [Li-Ion Akku Lithium Ionen Batterie](https://eckstein-shop.de/PKNERGY-Lithium-Ion-Battery-Cell-ICR-18650-37V-2600mAh-LiPo-Li-Ion?googlede=1&gclid=EAIaIQobChMI7_XxncPG-wIVOxkGAB1HxgB7EAQYASABEgKEfvD_BwE)
- [Single Battery holder](https://www.amazon.in/Electronicspices-Button-Battery-lithium-battery/dp/B08CCVDVFC)

Verwendete Software

- Auf dem ESP32
  - [AS7265x Bibliothek](https://github.com/sparkfun/SparkFun_AS7265x_Arduino_Library)
  - [InfluxDB Bibliothek](https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino)
- In der Cloud
  - [Docker](https://www.docker.com/)
  - [InfluxDB](https://www.influxdata.com/)
  - [Grafana](https://grafana.com/)

Im Verlauf des Projektes werden noch Zugriff auf andere Komponenten gebraucht:

- 3D-Drucker (Filament)
- Server zum hosten der Datenbank und Grafana.
- [Fritzing](https://fritzing.org/)
- [Fusion360](https://www.autodesk.de/products/fusion-360/overview)

> Als Student besteht die Möglichkeit in der BwCloud eine virtuelle Instanz zu erstellen.
