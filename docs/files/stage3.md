# Stage 3 - Automatisierung / Erweiterungen

Um die Schritte bei erneutem Aufsetzen nicht erneut manuell ausführen zu müssen, werden diese Schritte in Stage 3 automatisiert.
Hinzu kommen ebenfalls noch eine akustische Rückmeldung und ein Batteriepack um mit dem Messsystem mobil zu werden.

## Messsystem Erweiterungen

### Buzzer

Für die Feststellung, ob eine Messung durchgeführt wurde soll das Messsystem während der Messung eine akustische Rückmeldung geben.
Dafür wird ein aktiver Buzzer verwendet, welcher zwei Pins für die Spannungsversorgung und ein weiterer Pin für das Signal besitzt.

<img src="images/buzzer.svg" class="center">

Zum Ansteuern des Buzzers wird vor dem Messvorgang des Spektral Sensors, der Signal Pin (13) des Buzzers auf **HIGH** gezogen.
Ist das Auslesen der Daten vom Spektral-Sensor beendet wird dieser Pin wieder auf **LOW** gezogen.
Somit ist bekannt wie lange der Messvorgang aktiv ist und das Messsystem nicht bewegt werden sollte.

Hier der entsprechende Ausschnitt aus der **takeSensorData()** Funktion:

```cpp
digitalWrite(13, HIGH);
Serial.println("Begin measurement....");
specSensor.takeMeasurementsWithBulb(); // This is a hard wait while all 18 channels are measured
digitalWrite(13, LOW);
```

### Batteriepack

Ein Batteriepack wird für das Messsystem hinzugenommen, um nicht auf ein Netzteil und eine Steckdose in Reichweite angewiesen zu sein.
Hierfür wird ein [Lade-/Entlademodul (HW-107)](https://akkuplus.de/Lademodul-Entlademodul-HW-107-zum-Laden-Entladen-einer-Li-Ion-Li-Polymer-Zelle-Micro-USB), ein [Step-Up Converter (SX1308)](https://www.amazon.de/ANGEEK-Step-up-Wandler-Booster-Versorgungs/dp/B07RGZXPYY), ein [Akku](https://eckstein-shop.de/PKNERGY-Lithium-Ion-Battery-Cell-ICR-18650-37V-2600mAh-LiPo-Li-Ion?googlede=1&gclid=EAIaIQobChMI7_XxncPG-wIVOxkGAB1HxgB7EAQYASABEgKEfvD_BwE) und ein [Battery holder](https://www.amazon.in/Electronicspices-Button-Battery-lithium-battery/dp/B08CCVDVFC) benötigt.

<img src="images/batteriepack.svg" class="center">

Bei dieser Schaltung wird das ESP-Modul mit dem Spektral-Sensor bei angeschlossenem Netzteil am Lade-/Entlademodul über das Netzteil versorgt und der Akku geladen.
Wird das Netzteil entfernt so speist der Akku über den Step-Up Konverter den ESP mit Sensor.

<img src="images/wiring.png" class="center">

Durch hinzufügen von Batteriepack und Buzzer musste nun Änderungen am Gehäuse vorgenommen werden.

## Gehäuse Anpassungen

Die Hinzufügung der Batterie in Verbindung mit der mangelnden Zugänglichkeit des Vorgängers führte dazu, dass das ESP-Gehäuse völlig neu gestaltet werden musste.
Das neue Design baut auf einem Breadboard auf.
Auf der einen Seite des Breadboards wird der ESP und auf der anderen Seite die weiteren Komponenten wie der LiPo-Charger montiert.
Durch eine Fuge wird das Breadboard auf beiden Seiten reibschlüssig in Position gehalten.
Der Abstand zwischen Außenwand und Breadboard wird auf der Seite des ESPs durch diesen bestimmt.
Auf der anderen Seite besitzt der Buzzer die höchste Einbautiefe, sodass dieser den Abstand vorgibt.

<img src="images/v2_messsystem_side_top_esp.JPG" class="center">

Das Ganze wird mit einer um 90 Grad abgewinkelten Kappe verschlossen, welche etwas komplizierter als eine flache Abdeckkappe ist.
Die abgewinkelte Kappe bietet aber einen guten Zugang zum Breadboard, insbesondere zur ESP-Seite.
Des weiteren besitzt sie zwei Löcher um die USB-Anschlüsse des ESPs sowie des LiPo-Chargers von außen zu erreichen und einen Schlitz, um sie am Ende des Breadboards zu befestigen.
Jedoch war der Schlitz zu dünn, um wirklich als Befestigung zu funktionieren und stellte eher eine Schwachstelle als Bruchstelle dar.
Die Abdeckkappe wurde nicht U-förmig gestaltet, da sie sich durch Druck verformen könnte und kein ständiger leichter Zugang zur nicht-ESP-Seite erforderlich ist.
Im weiteren würde die Breite des Gehäuses durch eine weitere Fuge zunehmen.

<img src="images/v2_messsystem_side_top_covered.JPG" class="center">
<img src="images/v2_messsystem_bottom_part.JPG" class="center">
<img src="images/V2_messsystem_printed_bottom.jpeg" class="center">

Indem der AS7265X weiter seitlich zur Kabelöffnung positioniert wird, kann der Lichteinfall von oben reduziert werden.
Dies könnte aufgrund der asymmetrischen Front jedoch zu Lichteinfall von unten führen.
Da das Problem des Lichteinfalls von oben leicht mit einem Stück undurchsichtigem Klebeband über der Kabelöffnung behoben werden kann, wurde der Sensor mittig im Gehäuse gelassen.

Bei der Montage wurde festgestellt, dass die zum AS7265X führenden Kabel durch die Kabelöffnung gezogen werden müssen, nachdem sie die Seitenwand dessen Gehäuses durchquert haben.
Dies lässt sich am besten durch den Wechsel zu einer Schraubbefestigung beheben.

Die Batterie wurde an der Seite angebracht, um die gedruckten Teile gering zu halten, und mit einem Schalter zum Ein- und Ausschalten des Geräts versehen.
Auf der anderen Seite befindet sich ein Taster, mit dem ein Scanvorgang ausgelöst wird.

<img src="images/V2_messsystem_printed_batteryside.jpeg" class="center">
<img src="images/V2_messsystem_printed_buttonside.jpeg" class="center">

Kurz gefasst würden wir bei der nächsten Version des Gehäuses auf ein Schraubsystem umsteigen, da es etwas Platz spart und das Ratespiel bei der Entscheidung über die Abstände zwischen den verschiebbaren Teilen eliminiert.
Auch das Problem der Kabelführung wäre damit gelöst.
Als kleine Änderung kann das untere Gehäuse um weitere 3-4mm abgesenkt werden, wenn die Stifte im Durchmesser um 0,5mm dünner gemacht werden, so dass der Sensor direkt an der Öffnung anliegt.
Außerdem können die Löcher für den Batterieschalter, die Batterieverkabelung und die Taste für die Abtastung vorgefertigt werden, um eine bessere Passform zu erreichen.

## Grafana automatisieren

### Provisioning (Automatisierte Konfiguration)

Das wiederholt neue Einrichten wollen wir uns durch das Provisioning (dt. bereitstellen) ersparen.
Zunächst ist es aufwendiger, aber folgend kann schnell ein weiteres System (z.B. Lokal zum Testen) aufgesetzt werden.

- Dokumentation: [Provision dashboards and data sources](https://grafana.com/tutorials/provision-dashboards-and-data-sources/)

#### Datasources bereitstellen

Um eine Datenbank bereitzustellen, wird unter dem Container Pfad ```/etc/grafana/provisioning/``` einen Ordner ```datasources``` angelegt.
> Beachte, dass dies durch Docker nicht dem Pfad auf dem Host-System entspricht!
Hier können die Konfigurationsdateien der Datenquellen im yaml Format abgelegt werden.
Jede Konfigurationsdatei kann eine Liste von Datenquellen enthalten, die während des Starts von Grafana hinzugefügt oder aktualisiert werden.

Beispiel Konfigurationsdatei:

```yaml
apiVersion: 1

datasources:
  - name: InfluxDB_v2_Flux_Spectral
    type: influxdb
    access: proxy
    url: http://influxdb:8086
    secureJsonData:
      token: <INFLUXDB_TOKEN>
    jsonData:
      version: Flux
      organization: <INFLUXDB_ORG>
      defaultBucket: <INFLUXDB_BUCKET>
      tlsSkipVerify: true
```

Weitere Informationen zum Datasource Provisioning und speziell zur InfluxDB kann in der Dokumentation unter den folgenden Links gefunden werden:

- Dokumentation: [Provisioning data sources](https://grafana.com/docs/grafana/latest/administration/provisioning/#data-sources)
- Dokumentation: [Provisioning InfluxDB](https://grafana.com/docs/grafana/latest/datasources/influxdb/provision-influxdb/)

#### Dashboards bereitstellen

Ähnlich wie zu den Datasources können auch die Dashboards im Vorhinein bereitgestellt werden, indem unter dem Pfad ```/etc/grafana/provisioning/``` einen Ordner ```dashboards``` angelegt wird.
Erstelle in diesem Ordner ein Dashboard Provider Konfigurationsdatei (```dashboard.yaml```), um Dashboards beim Start von Grafana laden zu können.

```yaml
apiVersion: 1

providers:
  - name: 'SpectralSensor'
    orgId: 1
    folder: ''
    type: file
    disableDeletion: true
    updateIntervalSeconds: 10
    allowUiUpdates: true
    options:
    path: /etc/grafana/provisioning/dashboards
    foldersFromFilesStructure: true
```

Unter ```path``` wird der Pfad angegeben, wo die Dashboard yaml Dateien abgelegt werden sollen.
In unserem Fall sind diese im selben Ordner wie diese Dashboard Provider Konfigurationsdatei.

Um ein Dashboard durch Provisioning aufzusetzen, kann das zuvor auf der Grafana Weboberfläche erstellte Dashboard als JSON Datei exportiert und unter dem angegebenen ```path``` abgelegt werden.
Beim erneuten Start von Grafana wird nun das Dashboard automatisch erstellt.

- Github Link: [Dashboard-Dateien](https://github.com/bkammer/integrated_sensors_wise_22/tree/main/cloud/docker/grafana/provisioning/dashboards).
- Dokumentation: [Provisioning Dashboards](https://grafana.com/docs/grafana/latest/administration/provisioning/#dashboards)

## Finale Version des Codes

Im Folgenden soll der finale Code näher erläutert werden.
Alle in Stage 1 erklärten Funktionen wurden hier benutzt.
Der Code ist in zwei Blöcken aufgeteilt: einer **setup()**-Funktion und einer **loop()**-Funktion.

Zunächst werden alle verwendeten Bibliotheken initialisiert und die Anmeldedaten für die WiFi Verbindung gesetzt.

> Hier ist zu beachten, dass die eigenen WiFi-Hotspot-Daten im Code gesetzt werden müssen. Ansonsten kann keine Verbindung zum ESP32 hergestellt werden!

Anschließend werden die benötigten Credentials für die InfluxDB gesetzt.
Ähnlich wie bei Grafana werden folgende Daten der Datenbank benötigt:

- Eine **URL** des Datenbank Servers
- Ein **Token**, zur Authentifizierung mit der InfluxDB
- Der **Bucket**-Name
- Unser **Organisationsname**

Vor dem Setup, werden zunächst die verwendeten Objekte zur Interaktion der InfluxDB, des Sensors und für die WiFi Verbindung initialisiert.
Außerdem muss die Zeitzone, für die Synchronisation der Zeitzone, gesetzt werden.

```cpp
// Initialize WiFi client
WiFiMulti wifiMulti;

// Initialize specSensor
AS7265X specSensor;

// Initialize InfluxDB client
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare the data point for the measurements
Point *sensorData = NULL;
```

### Funktion: setup()

<img src="images/acitivitydiagram_initialisation.svg" class="center" style="width:300px;">

Im Setup werden zunächst die PIN-Modi gesetzt.
Für den Button wird hier auf PIN 4 ein INPUT gesetzt, mit internem Pullup-Widerstand.
Zusätzlich wird diesem PIN ein Interrupt zugewiesen, welcher ausgeführt wird, wenn der Button gedrückt wird.
Anschließend wird der Buzzer-PIN gesetzt.

```cpp
// AS7265X
pinMode(4, INPUT_PULLUP);
attachInterrupt(4, setMeasurementFlag, FALLING);

// Buzzer
pinMode(13, OUTPUT);
```

Anschließend wird die WiFi Bibliothek und die I²C Bibliothek für den Spektral-Sensor initialisiert.
Zusätzlich wird die Zeit, zur Erstellung von akkuraten Zeitstempeln für die InfluxDB, synchronisiert und die Status-LED ausgeschaltet.

```cpp
// Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
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
```

Im letzten Schritt des Setups, wird die Verbindung mit der InfluxDB aufgebaut.
Falls diese besteht, werden drei kurze Töne über den Buzzer wiedergegeben, mit einem Delay von 300ms zwischen den Tönen.
Wir haben hier die Delay-Funktion verwendet, da hier ein abgesonderter Codeabschnitt besteht.
Somit blockiert zwar die Delay-Funktion den Mikrocontroller, da die setup()-Funktion im Anschluss endet und danach die loop()-Funktion folgt, ist dies ohne negative Folgen bezüglich der Performanz zu benutzen.

Zuletzt werden hier die **setNewTag()** und **setNewMeasurement()**-Funktionen aufgerufen.
Diese werden für die jeweilige Messung benötigt.
Die Funktionsweise kann aus den Abschnitten [Funktion: setNewTag()](#funktion-setnewtag) und [Funktion: setNewMeasurement()](#funktion-setnewmeasurement) entnommen werden.

Für das erfolgreiche Abschließen der Funktionen **setNewTag()** und **setNewMeasurement()**, muss der ESP mit einem seriellem Monitor verbunden werden.
Hierfür kann das Tool HTerm benutzt werden.
Beim Verwenden muss der richtige COM-Port ausgewählt werden (z.B. im Geräte-Manager unter Windows), sowohl als auch die Baud-Rate von ```115200```.

```cpp
// Check server connection
  if (client.validateConnection())
  {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
    Serial.println("Please initialize a measurement and tag name to start measuring!");
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
```

### Funktion: setNewTag()

```cpp
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
```

Zunächst wird eine Ausgabe im seriellem Monitor ausgegeben.
Im Anschluss wird darauf gewartet, dass der User seinen neuen Tag-Namen eingibt.
Dieser ist nötig, um in der jeweiligen Measurement das richtige Objekt identifizieren zu können, wenn danach gequeried/gesucht werden soll in der InfluxDB.

Nachdem der gewünschte Name eingegeben wird, kann durch das eingeben des Wortes "**exit**" die Funktion verlassen werden.

### Funktion: setNewMeasurement()

```cpp
void setNewMeasurement()
{
  Serial.println("Entered mode to set a new measurement. Please enter the new measurement name and press ENTER. If you are done, write 'exit'.");
  if (sensorData != NULL)
  {
    delete sensorData;
  }
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
        sensorData = new Point(currentString);
      }
    }
  }
}
```

Die **setNewMeasurement()**-Funktion funktioniert ähnlich wie die **setNewTag()**-Funktion.
Hier wird auch zunächst eine Ausgabe im seriellen Monitor ausgegeben.
Im Anschluss wird der neue Measurement-Name erwartet/eingelesen.
Dieser wird benötigt, um in der InfluxDB ein neues Measurement anzulegen oder auszuwählen, welches eine neue Klasse an Objekten entspricht, um die Daten zu speichern.

Hier haben wir jedoch eine Schwierigkeit aufdecken können:

Um mit der InfluxDB zu interagieren, wird ein "Point-Objekt" benötigt.
Dieses Point-Objekt wird mit einer Measurement initialisiert, welcher beim Aufrufen des Konstruktors als Parameter mitgegeben wird.
Die Herausforderung war es hier eine Möglichkeit zu finden, die Point-Objekte dynamisch zu erstellen.
Dadurch wurde ermöglicht, dynamisch die gewünschte Klasse an zu messenden Objekten nach belieben zu ändern.
Die Bibliothek dafür sieht dieses Vorhanden jedoch nicht vor.
Deshalb stehen zwei Optionen zur Verfügung:

- Die Bibliothek mit einer passenden Funktion ergänzen oder
- einen anderen Workaround im vorhandenen Code finden.

Wir haben uns dann entschlossen, einen Pointer vom Typ Point zu erstellen, stattdessen das Point-Objekt initial zu erstellen.
Dann haben wir einen initialen Aufruf der in der **setup()**-Funktion, vor dem eigentlichen Messen, implementiert.
Dadurch wird sichergestellt, dass zu Beginn der Messung immer Tag und Measurement gesetzt sind.
In der **setNewMeasurement()**-Funktion löschen wir dann das jeweilige Point-Objekt, falls es nicht **NULL** ist und erstellen auf dem Heap ein Neues, durch aufrufen mit "**new**".
Dadurch erhalten wir einen Pointer, mit dem wir in der **takeMeasurement()** unsere Daten an die InfluxDB schicken können.
Dieses Vorhaben war nötig, da wir die einzelnen Funktionalitäten gekapselt haben wollten, somit der Messvorgang und das Senden der Daten an die InfluxDB von allem anderen getrennt sind.

Wenn der neue Name eingegeben wurde, kann auch hier mit "**exit**" die Funktion verlassen werden.

### Funktion: loop()

<img src="images/activitydiagram_loop.svg" class="center" style="width:400px;">

Die Loop-Funktion beinhaltet den Code welcher für die Messung benötigt wird.
Getriggert wird eine Messung durch das betätigen des Buttons.
Dadurch wird die Interrupt-Service-Routine (ISR) ausgelöst, welche eine Messung, durch setzen eines Flags, initiiert.

```cpp
void setMeasurementFlag()
{
  takeMeasurement = 1;
}
```

Zunächst wird ein Signal über den Buzzer ausgegeben, um eine beginnende Messung zu signalisieren.
Dieser Ton ertönt solange, wie die Messung andauert.
Die Messung wird im Anschluss über die **takeMeasurementsWithBulb()**-Funktion initiiert.
Im Anschluss werden die einzelnen Kanäle über die **getCalibratedX()**-Funktion ausgelesen, wobei das **X** für den jeweiligen Buchstaben des auszulesenden Kanals steht.
Bevor die Daten in das Point-Objekt Daten gespeichert werden, werden die zuvor gemessenen Daten (Felder und tags) gelöscht.
Die jeweiligen gemessenen Werte werden dann dem Point-Objekt übergeben.
Im Anschluss wird geprüft, ob die WiFi-Verbindung noch aktiv ist, falls nicht, wird die Nachricht "**WiFi connection lost**" im Terminal ausgegeben.

Mit der **writePoint()**-Funktion des Clients, werden die Daten schlussendlich an die InfluxDB gesendet.

```cpp
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
    Serial.println("WiFi connection lost");
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
```
