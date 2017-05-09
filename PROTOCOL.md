# Protokoll KW

Diese Protokoll wird von den Vitotronic Steuerungen der KW Serie unterstützt.
Es ähnelt dem älteren GWG Protokoll, unterstützt jedoch auch Adressen mit 2 Bytes Länge.
Soweit bekannt, sind alle Steuerungen die das Protokoll 300 unterstützen abwärtskompatibel zum KW Protokoll und können auch über dieses Protokoll angesprochen werden.

## Serielle Schnittstelle (Optolink):
4800 8 E 2, ohne Handshake Protokoll

## Kommunikation (alle Werte in Hex)

Vitotronic sendet periodisch 0x05
Sofort (xx msec) nach dem Empfang von 0x05 kann ein Telegramm gesendet werden und wird sofort beantwortet.
Ehe ein weiteres Telegramm gesendet werden kann, muss erst auf das nächste 0x05 gewartet werden.
Anstatt auf das nächste 0x05 zu warten, kann auch direkt nach dem Empfang einer Antwort das nächste Telegramm geschickt werden. Dabei darf das Telegramm nicht mit 0x01 eingeleitet werden (Somit ist das 0x01 logisch gesehen nicht ein Telegram-Start-Byte sondern ein ACK auf die 0x05).

### Telegrammdetails:
* 01 Telegramm-Start-Byte (ACK auf 0x05)
* F7 Type F7= Lesen F4=Schreiben
* XX XX Adresse
* XX Anzahl der angefragten Bytes

Beispiel Abfrage Aussentemperatur (V200KW2)
* TX: 01
* TX: F7 55 25 02
* RX: 5B 00 > Aussentemperatur 9,1°C

Die Liste der Adressen für die verschiedenen Steuerungen findet ihr hier.

Beispiel Geräte Kennung abfragen
* TX: 01
* TX: F7 00 F8 02
* RX: 20 98
* 0x2098 = V200KW2

Beispiel Heizkreis A1 auf Sparbetrieb setzen (V200KW2)  
* TX: 01
* TX: F4 23 02 01 01
* RX: 00 = Bestätigung OK

# Protokoll 300

Diese Protokoll wird von den neueren Vitotronic Steuerungen unterstützt.
Gegenüber dem KW Protokoll bietet es einige Vorteile:
Anfragen können zu jeder Zeit an die Anlage gesendet werden, keine Warten auf 0x05, damit können auch mehrere Abfragen in kurzer Zeit gesendet werden.
Weitgehend fehlerfreie Übetragung dank CRC checksum.

Soweit bekannt, sind alle Steuerungen die das Protokoll 300 unterstützen abwärtskompatibel zum KW Protokoll und können auch über diese Protokoll angesprochen werden.

## Serielle Schnittstelle (Optolink):

4800 8 E 2, ohne Handshake Protokoll

## Kommunikation (alle Werte in Hex)

###Initialisierung der Kommunikation (alle Werte in Hex)

Vitotronic sendet periodisch 0x05
Darauf sofort Antwort geben mit 0x16 0x00 0x00
Vitotronic antwortet mit 0x06
Das periodische Senden von 0x05 hört damit auf
Fazit: solange 0x16 0x00 0x00 senden, bis Steuerung mit 0x06 antwortet.

### Beenden der Kommunikation
Durch Senden von 0x04 wird Vitotonic wieder zurückgesetzt und sendet periodisch 0x05.
Es empfiehlt sich beim Start einer Anwendung zunächst ein 0x04 zu senden um eine definierten Anfangszustand zu erhalten.

### Antworten von Vitotronic
* 0x15 Error
* 0x05 not init
* 0x06 ok (Antwort auf 0x16 0x00 0x00 und auf korrekt empfangene Telegramme)

### Telegrammdetails:
Wenn Daten gelesen werden:
* 41 Telegramm-Start-Byte
* 05 Länge der Nutzdaten (Anzahl der Bytes zwischen dem Telegramm-Start-Byte (0x41) und der Prüfsumme)
* 00 00 = Anfrage, 01 = Antwort, 03 = Fehler
* 01 01 = ReadData, 02 = WriteData, 07 = Function Call
* XX XX 2 byte Adresse der Daten oder Prozedur
* XX Anzahl der Bytes, die in der Antwort erwartet werden
* XX Prüfsumme = Summe der Werte ab dem 2 Byte (ohne 41)

#### Wenn Daten geschrieben werden sollen:
* 41 Telegramm-Start-Byte
* 06 Länge der Nutzdaten (Anzahl der Bytes zwischen dem Telegramm-Start-Byte (0x41) und der Prüfsumme)
* 00 00 = Anfrage, 01 = Antwort, 03 = Fehler
* 02 01 = ReadData, 02 = WriteData, 07 = Function Call
* XX XX 2 byte Adresse der Daten oder Prozedur
* XX Anzahl der Bytes, die in der geschrieben werden sollen
* XX Inhalt der geschrieben werden soll
* XX Prüfsumme = Summe der Werte ab dem 2 Byte (ohne 41)

Beispiel Aussentemperatur abfragen (Vitotronic 333)
* Senden 41 05 00 01 55 25 02 82
* Empfangen 06 41 07 01 01 55 25 02 07 01 8D
* Antwort: 0x0107 = 263 = 26.3°

Beispiel Geräte Kennung abfragen
* Senden 41 05 00 01 00 F8 02 00
* Empfangen 06 41 07 01 01 00 F8 02 20 B8 DB
* 0x20B8 = V333MW1

Beispiel Vitodens 333 Betriebsart schreiben:
* Senden 41 06 00 02 23 23 01 XY xx
* Empfangen 06 41 06 01 02 23 23 01 XY xx
* XY:
	* 0 = Abschalten
	* 1 = nur WW
	* 2 = Heizen mit WW
	* 3 = immer Reduziert
	* 4 = immer Normal

Also ich bin mit diesem Beispiel nicht ganz einverstanden. Bei mir sieht es so aus:  
TX: Data: 0x41 0x06 0x00 0x02 0x23 0x23 0x01 0x02 0x51  
RX: Data: 0x41 0x05 0x01 0x02 0x23 0x23 0x01 0x4f  
Auch bei allen anderen Schreiboperationen sieht es so aus, dass die in den Vitodens-Speicher zu schreibenden Daten NICHT wieder mit zurückgegeben werden.