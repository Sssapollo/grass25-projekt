# Projektbericht – Secure Memory Unit

## Beiträge der Gruppenmitglieder

- **Xinrui Zheng**  
  Verantwortlich für die Implementierung des SystemC-Moduls `secure-Memory-Unit.hpp` sowie den Großteil der Literaturrecherche. Sie macht auch die Integration des SMUs in Tiny_cpu.hpp im Verzeichnis `/include/tiny_cpu`.

- **Tianding Hou**  
  Zuständig für die Implementierung des Simulationstools `simulation.cpp`, das Rahmenprogramm `main.c`, sowie die Mitentwicklung des `Sc_module secure-Memory-Unit.hpp`.  
  Außerdem führte er die Literaturrecherche zu `LCG-based PRNG` durch, betreute das gesamte Projekt, und beteiligte sich am Testen (test/Parse_input und alle Print for debugging) sowie dem Aufbau der Entwicklungsumgebung.

- **Yuze Ji**  
  Hauptverantwortlich für den Code im Verzeichnis `/test`. Er entwickelte das zugehörige Python-Skript und die .csv-Dateien für verschiedene Testszenarien.

---

## Funktionsweise des Moduls

### 1. C-Rahmenprogramm (`main.c`)

Dient als Kommandozeileninterface (CLI):

- Liest Optionen mit `getopt_long()` ein  
- Parst die Eingabedatei (CSV mit Requests)  
- Ruft `run_simulation()` mit den entsprechenden Parametern auf  
- Gibt die Ergebnisse (Fehleranzahl, Zyklusanzahl) auf dem Terminal aus  

---

### 2. Simulation (`simulation.cpp`)

Zentrale Datei zur Simulationslogik. Sie implementiert:

- **`run_simulation(...)`**: Hauptfunktion, die:  
  - Eine Instanz der SMU erstellt  
  - Konfigurationsparameter (Endianness, Latenzen, Seed) übergibt  
  - Eine Liste von `Request`-Strukturen verarbeitet
    - Jeder Request (`read`, `write`, `fault`) wird:
      - Zyklisch abgearbeitet  
      - Optional mit Fault Injection kombiniert  
      - Request im Signal nach an SMU schreibt
      - Ergebnis und Fehlerstatus werden gesammelt und ausgewertet  
  

---

### 3. SystemC-Modul (`secure-Memory-Unit.hpp`)

Zentrales Modul zur Beschreibung der Secure Memory Unit. Enthält:

- **Eingänge:** `clk`, `addr`, `wdata`, `r`, `w`, `fault`, `faultBit`  
- **Ausgänge:** `rdata`, `ready`, `error`  

**Ablauf bei steigender Taktflanke (`clk`) bei aktivem `r` oder `w`:**

1. Physische Adressberechnung mittels XOR-Scrambling  

2. Schreiben von verschlüsselten Bytes (mittels XOR) in den Speicher  

3. Lesen von Bytes, Entschlüsselung und Parity-Prüfung  

4. Bei Parity-Fehler:  

   `error = 1`  `rdata = 0`  ;

   Andernfalls: `error = 0`, `rdata = Wert`  

5. Setzen von `ready = 1` zum Abschluss des Zugriffs  

6. Warten auf den nächsten Request  

   

   #### Zusammenspiel:

1. Benutzer startet `./project` mit Argumenten

2. `main.c` analysiert Parameter und lädt Anfragen

3. `simulation.cpp` verarbeitet die Anfragen mit dem SystemC-Modul

4. `secure-Memory-Unit.hpp` führt jeden Zugriff unter Sicherheitslogik aus

5. Ergebnis wird an Konsole oder Datei ausgegeben

   

---

## Literaturrecherche

### 1. LCG-based PRNG

**Formel:**  
`RandSeed = (A * RandSeed + B) % M`

- Die Periodenlänge des PRNG ist ≤ M  
- Anforderungen, um die maximale Periodenlänge zu erreichen:

  1. **B** und **M** müssen teilerfremd sein  
  2. Alle Primfaktoren von **M** müssen **A - 1** teilen  
  3. Wenn **M** ein Vielfaches von 4 ist, muss auch **A - 1** ein Vielfaches von 4 sein  
  4. **A**, **B** und **N[0]** müssen kleiner als **M** sein  

---

### 2. Address Scrambling

**Vorteile:**
- Datenverschlüsselung ist in jedem Speichertyp möglich.
- Geringer Flächenbedarf, geringe Verzögerung und niedriger Stromverbrauch.

**Nachteile:**
- Wenn der Schlüssel fest oder vorhersehbar ist, kann ein Angreifer den Originalinhalt lesen.

---

### 3. XOR-Verschlüsselung

**Vorteile:**
- Die XOR-Verschlüsselung kann sehr robust sein, wenn:
  - Ein langer, sich nicht wiederholender Schlüssel verwendet wird (z. B. ein Schlüssel mit ebenso vielen Bits/Zeichen wie der Klartext).
  - Für jede neue Kommunikation ein neuer Schlüssel zufällig generiert wird.
  - Der Schlüssel sowohl vom Sender als auch vom Empfänger geheim gehalten wird.

**Nachteile:**
- Wenn eine große Textmenge verschlüsselt werden soll, wird häufig ein kürzerer, sich wiederholender Schlüssel verwendet.
- Die Wiederverwendung desselben Schlüssels oder ein sich wiederholender Schlüssel macht die Methode anfällig für Frequenzanalyse.

---

### 4. Paritätsprüfung

**Vorteile:**
- Die Paritätsbit-Methode ist effektiv zur Erkennung von Bitfehlern mit ungerader Anzahl auf der Empfängerseite.
- Geringer Overhead bei der Datenübertragung, da nur ein Paritätsbit hinzugefügt wird.
- Einfache Methode zur Fehlererkennung bei ungerader Bitanzahl.

**Nachteile:**
- Erkennt nur Fehler bei ungerader Anzahl von Bits; die genaue Position des Fehlers kann nicht bestimmt werden.
- Wenn sich die Daten ändern, aber die Gesamtanzahl der Bits gerade (bzw. ungerade) bleibt, wird der Fehler nicht erkannt.

---

### 5. Pseudozufallszahlengenerator (PRNG)

**Vorteile:**
- **Deterministisch:** PRNGs erzeugen vorhersagbare Sequenzen basierend auf einem gegebenen Seed.
- **Effizient:** Geringerer Rechenaufwand im Vergleich zur Erzeugung echter Zufallszahlen.
- **Anwendungen:** Einsatz in Simulationen, kryptografischen Algorithmen und zufallsbasierten Verfahren.
- **Sicherheit:** Kryptografische PRNGs sorgen für Unvorhersehbarkeit und sind somit entscheidend für sichere Protokolle.

**Nachteile:**
- Manche Seed-Zustände können zu kürzeren Perioden als erwartet führen („schwache“ Seeds).
- Fehlende Gleichverteilung bei großen Mengen generierter Zahlen.
- Korrelation aufeinanderfolgender Werte.
- Schlechte dimensionsbezogene Verteilung der Ausgabesequenz.
- Bestimmte Werte treten nicht zufällig verteilt auf.