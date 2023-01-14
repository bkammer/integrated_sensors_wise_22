# Stage 2 - Erweitern des Konzeptes

In der zweiten Stage wird das Konzept weiter ausgebaut, indem die Datenvisualisierung auf einem Grafana Dashboard aufgebaut wird.
Damit diese Daten gruppiert und verglichen werden können, müssen sie beschriftet werden.
Der Schritt der Datenbeschriftung muss vor dem Senden in die Datenbank auf dem EPS umgesetzt werden.
Zum Beschriften der Messdaten müssen dem ESP vor dem Messvorgang die Eigenschaften übermittelt werden.

## Messdaten beschriften

Dazu wird prototypisch die serielle Schnittstelle des ESPs genutzt.
Das hat den Vorteil, dass keine weitere Hardware benötigt wird bis auf den Rechner, der schon für das Programmieren des ESPs benötigt wird.

### Measurement und Tags dynamisch setzen

Es wurde entschieden, die Speicherung und Beschriftung der Daten dynamisch zu realisieren.
Dazu wurden im Code die Funktionen **setNewTag()** und  **setNewMeasurement()** verwendet.
Die Funktion **setNewTag()** zur Beschriftung der Daten.
Die Funktion **setNewMeasurement()**, um festzulegen in welches Measurement die Daten gespeichert werden sollen.
Unter den gleichnamigen Unterkapiteln, wird in Stage 3 auf die Funktionsweise eingegangen.

## Grafana

### Cloud Änderungen

Aufgrund der aktuellen Lage des Hacker Angriffs auf die Hochschule Heilbronn, sind deren Systeme nicht mehr mit dem Internet verbunden.
Somit ist es nicht möglich sich in der BwCloud mit dem Hochschul-Account anzumelden, da der Authentifizierungsserver nicht erreichbar ist.

Beim Aufsetzen von Grafana stehen wir somit vor der Herausforderung, dass wir keinen Zugriff auf die Firewall der BwCloud haben und dadurch keinen weiteren Port für Grafana freigeben können.

Durch die Skripte und Docker ist es einfach möglich das bestehende System auf einem alten vorhandenen vServer neu aufzusetzen.
Zu diesem Zeitpunkt haben wir die BwCloud durch einen vorhandenen vServer ausgetauscht.
Codetechnisch muss lediglich kleine Konfigurationsänderungen wie zum Beispiel die IP-Adresse vorgenommen werden.

> Daten die bereits in der Datenbank liegen, könnten durch Kopieren des **influxdb/** Ordners auf den "neuen" vServer übertragen werden.

### docker-compose.yml anpassen

Um auch Grafana mit Docker-Compose starten zu können, wird der neue Service "Grafana" der **docker-compose.yml** hinzugefügt.
Hier werden ebenfalls, für die Persistenz der Dashboards und weiteren Konfigurationsdaten, Volumes angelegt.

```yml
volumes:
  - ./grafana/data:/var/lib/grafana/ # working directory
  - ./grafana/provisioning/:/etc/grafana/provisioning/ # provisioning scripts that grafana will load on boot
```

> Die Volume ```Provisioning``` wird zu diesem Zeitpunkt noch nicht gebraucht.
> Diese kann jedoch zukünftig für automatisiertes Einrichten von Datenquellen und Dashboards verwendet werden.

Grafana läuft auf dem Port **3000**, der dem Host Port **3000** zugeordnet wird.

Für das automatisierte Einrichten von Grafana, werden folgende Environment-Variablen für den Container gesetzt:

- ```GF_SECURITY_ADMIN_USER=${GF_SECURITY_ADMIN_USER}```
- ```GF_SECURITY_ADMIN_PASSWORD=${GF_SECURITY_ADMIN_PASSWORD}```
- ```GF_AUTH_ANONYMOUS_ORG_ROLE=Admin```
- ```GF_AUTH_ANONYMOUS_ENABLED=true```
- ```GF_ENABLE_GZIP=true```

Die Änderungen an der **docker-compose.yml** sehen wie folgt aus:

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

  grafana:
    image: grafana/grafana:latest
    container_name: grafana
    restart: unless-stopped
    ports:
      - "3000:3000"
    volumes:
      - ./grafana/data:/var/lib/grafana/
      - ./grafana/provisioning/:/etc/grafana/provisioning/
    environment:
      - GF_SECURITY_ADMIN_USER=${GF_SECURITY_ADMIN_USER}
      - GF_SECURITY_ADMIN_PASSWORD=${GF_SECURITY_ADMIN_PASSWORD}
      - GF_AUTH_ANONYMOUS_ORG_ROLE=Admin
      - GF_AUTH_ANONYMOUS_ENABLED=true
      - GF_ENABLE_GZIP=true
    depends_on:
      - influxdb
    user: "1000" # USER ID des Docker Users anpassen
```

> Der Ordner **grafana/** auf dem Host muss dem User 1000 gehören, sonst kann Grafana im Docker-Container keine weiteren Ordner anlegen!

Wie schon in Stage 1 kann die **docker-compose.yml** mit dem Befehl ```docker-compose up -d``` gestartet werden.
Ob der Grafana-Container ebenfalls startet, kann mit dem Aufrufen der Weboberfläche ```http://YOUR-SERVER-ADDRESS:3000/``` überprüft werden.

- Weitere Dokumentation: [Run Grafana Docker image](https://grafana.com/docs/grafana/v9.0/setup-grafana/installation/docker/)

### Datenquelle konfigurieren

Wenn man in der Weboberfläche angemeldet ist sollte eine Datenquelle hinzugefügt werden, bevor ein Dashboard zur Visualisierung erstellt wird.

> Bei der Datenquelle handelt es sich nicht immer um eine Datenbank.

Eine Datenbank kann über das ```Zahnradsymbol``` im Seitenmenü > ```Data sources``` > ```Add data source``` hinzugefügt werden.
Wähle hier die Datenquelle (in diesem Fall: InfluxDB) aus, die hinzugefügt werden soll und konfiguriere die Datenquelle gemäß den für diese Datenquelle spezifischen Anweisungen.
Bei der InfluxDB handelt es sich hier um die **URL**, die **Organisation**, den **Token** und den **Bucket**, als die relevantesten Daten.
Anstelle des Tokens kann auch **Basic auth** mit Benutzername und Passwort verwendet werden.
Eine ausführlichere Beschreibung kann in der [Grafana Dokumentation](https://grafana.com/docs/grafana/latest/datasources/add-a-data-source/) gefunden werden.

### Dashboard einrichten

Ein Dashboard soll die Daten der letzten Messung visualisieren.
Es soll ein Panel enthalten, dass das Spektrum des Produktes zeigt, damit hierfür ein Eindruck gewonnen werden kann.
Somit kann auch Ansatzweise die korrekte Messung bestätigt werden, aus dem Erwartungswert und dem tatsächlich gewonnenen Spektrum.
Weitere Dashboards sollen jeweils eine Kategorie von Produkten beinhalten.
Diese beinhalten zwei Panels.
Das Erste beinhaltet das Spektrum das alle Messwerte in dieser Kategorie als Durchschnitt repräsentiert.
Ein weiteres Panel zeigt die letzte Messung der Kategorie, um diesen parallel mit dem anderen Panel vergleichen zu können, wie dieser Messwert vom Durchschnitt abweicht.

- Grafana Dokumentation: [Build your first dashboard](https://grafana.com/docs/grafana/latest/getting-started/build-first-dashboard/)

Damit diese Dashboards diese Darstellungen visualisieren können, muss zunächst ein Flux Query erstellt werden, der die Datensätze in der Datenbank in die Datenform der Darstellung bringt.
Für die Erstellung dieser Queries ist der Query Builder in der InfluxDB UI (User Interface) sehr hilfreich.
Denn hier kann der Query Stück für Stück in die richtige Form gebracht werden.
Danach kann dieser Query in das Grafana Panel übertragen werden.

In Grafana kann dann die gewünschte Visualisierung konfiguriert werden.
Herausfordernd wurde das Konfigurieren der gleichen Darstellung in den Barcharts.
Denn ein Flux Query kann auf verschiedene Weise aufgebaut werden um an die selben Daten zu kommen.
Jedoch sind diese Daten unterschiedlich sortiert und gruppiert, was eine andere Visualisierung in Grafana hervorruft.
So mussten manche Flux Queries angepasst werden.
Ein weiterer Punkt, der bei den Queries berücksichtigt werden musste, ist der vorhandene Zeitstempel.
Ohne diesen kann Grafana die Daten nicht in einem Barchart darstellen.
Sortiert wurden die Balken des Barcharts nach dem Namen der Channels die in der Datenbank hinterlegt sind.
Diese Reihenfolge entspricht jedoch nicht der Reihenfolge des Wellenspektrums.

  | Channel | Spektrum |
  | ------- | -------- |
  | A       | 410nm    |
  | B       | 435nm    |
  | C       | 460nm    |
  | D       | 485nm    |
  | E       | 510nm    |
  | F       | 535nm    |
  | G       | 560nm    |
  | H       | 585nm    |
  | I       | 645nm    |
  | J       | 705nm    |
  | K       | 900nm    |
  | L       | 940nm    |
  | R       | 610nm    |
  | S       | 680nm    |
  | T       | 730nm    |
  | U       | 760nm    |
  | V       | 810nm    |
  | W       | 860nm    |

<img src="images/grafana_test_barchart.jpeg" class="center">

Um das Problem zu umgehen wird anstatt des Channel-Namens die entsprechende Wellenlänge hinterlegt.
Ohnehin ist mit der Wellenlänge mehr anzufangen als mit dem Channel-Namen des Spektral-Sensors.

<img src="images/GF Test measurement panel.png" class="center">

Für eine schönere Visualisierung wurden Overrides erstellt, die die Farbe der Balken, je nach Wellenlänge, entsprechend einfärbt.
Weiterhin enthält der "Display name" noch weitere Attribute, sodass diese Attribute mit ```${__field.labels.__values}``` auf lediglich die Wellenlänge herunter gekürzt wird.

<img src="images/GF test_measurement2 dashboard.jpeg" class="center">

> Das Konfigurieren der Datenquelle und besonders des Dashboards ist sehr zeitaufwendig, sodass hier eine Automatisierung sehr hilfreich wäre damit wiederholtes Einrichten nicht notwendig ist.
