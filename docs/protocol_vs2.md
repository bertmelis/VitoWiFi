From OpenV Wiki (https://github.com/openv/openv/wiki/Protokoll-300), consulted 10/10/2023

Dieses Protokoll wird von den neueren Vitotronic Steuerungen unterstützt.

Gegenüber dem KW Protokoll bietet es einige Vorteile:

Anfragen können zu jeder Zeit an die Anlage gesendet werden, kein Warten auf 0x05, damit können auch mehrere Abfragen in kurzer Zeit gesendet werden.
Weitgehend fehlerfreie Übertragung dank CRC Checksumme.
Soweit bekannt, sind alle Steuerungen, die das Protokoll 300 unterstützen, abwärtskompatibel zum KW Protokoll und können auch über dieses Protokoll angesprochen werden.

Serielle Schnittstelle (Optolink):

4800 8 E 2, ohne Handshake Protokoll
Initialisierung der Kommunikation (alle Werte in Hex)

Vitotronic sendet periodisch (ca. 2s) 0x05
Senden von 0x04 (EOT) um einen definierten Ausgangszustand zu erhalten.
Erneutes 0x05 der Vitotronic abwarten
Darauf sofort Antwort geben mit der Synchronisierungssequenz 0x16 0x00 0x00 (diese Synchronisierungssequenz kann vom verbundenen Programm periodisch gesendet werden, um ein Timeout der Kommunikation zu verhindern).
Vitotronic antwortet mit 0x06
Das periodische Senden von 0x05 hört damit auf
Beenden der Kommunikation

Durch Senden des Telegramm-Ende Steuerzeichens EOT 0x04 wird Vitotronic wieder zurückgesetzt und sendet wieder periodisch 0x05.
Es empfiehlt sich, auch beim Start einer Anwendung zunächst ein 0x04 zu senden um einen definierten Anfangszustand zu erhalten.
Antworten von Vitotronic

0x15 NACK/Error
0x05 ENQ/not init
0x06 ACK ; Antwort auf 0x16 0x00 0x00 und auf korrekt empfangene Telegramme. Auch das mit einer Vitotronic verbundene Programm antwortet auf Telegramme der Vitotronic mit 0x06
Die Kommunikation scheint sich hier an den ASCII Steuerzeichen zu orientieren.

Telegrammdetails:

Wenn Daten gelesen werden:

0x41 Telegramm-Start-Byte
0xXX Länge der Nutzdaten als Anzahl der Bytes zwischen dem Telegramm-Start-Byte (0x41) und der Prüfsumme)
0x0X MessageIdentifier (untere 4 Bits): 0x00 = Request, 0x01 = Response, 0x02 = UNACKD, 0x03 = Error
0x0X FunctionCode (untere 5 Bits): 0x01 = Virtual_READ, 0x02 = Virtual_WRITE, 0x07 = Remote_Procedure_Call
Bits 5-7 sind eine Message-Sequenz-Nummer, welche im Reply ebenfalls gesetzt wird.
0xXX 0xXX 2 byte Adresse der Daten oder Prozedur
0xXX Anzahl der Bytes, die in der Antwort erwartet werden
0xXX Prüfsumme = Summe der Werte ab dem 2. Byte (ohne 0x41)
Wenn Daten geschrieben werden sollen:

0x41 Telegramm-Start-Byte
0xXX Länge der Nutzdaten (Anzahl der Bytes zwischen dem Telegramm-Start-Byte (0x41) und der Prüfsumme)
0x0X 0x00 = Anfrage, 01 = Antwort, 03 = Fehler
0x0X 0x01 = ReadData, 0x02 = WriteData, 07 = Function Call
Bits 7,6,5 können wahlfrei gesetzt werden, sie werden in der Antwort dann ebenfalls gesetzt
0xXX 0xXX 2 Byte Adresse der Daten oder Prozedur
0xXX Anzahl der Bytes, die in der geschrieben werden sollen
0xXX ... Inhalt der geschrieben werden soll
0xXX Prüfsumme = Summe der Werte ab dem 2. Byte (ohne 0x41)
Die Heizungssteuerung antwortet auf erfolgreiches Schreiben mit 0x41 LE 0x01 0x02 @@ @@ NM XX mit LE=Länge der Nutzdaten, @@ @@ = 2 Byte Adresse, NM = Anzahl geschriebener Bytes, XX = Prüfsumme
Die im Funktionstyp-Byte der Anfrage / 4. Byte gesendeten Bits 7,6,5 sind dann in der Antwort ebenso enthalten. Wurde z.B. als Funktionstype WriteData 0x02 gesendet, kommt 0x02 in der Antwort; wurde 0xA2gesendet, kommt 0xA2 zurück.1

Beispiel Aussentemperatur abfragen (Vitotronic 333)

Senden 0x41 05 00 01 55 25 02 82
Empfangen 0x06 41 07 01 01 55 25 02 07 01 8D
Antwort: 0x0107 = 263 = 26.3°
Beispiel Geräte Kennung abfragen

Senden 0x41 05 00 01 00 F8 02 00
Empfangen 0x06 41 07 01 01 00 F8 02 20 B8 DB
0x20B8 = V333MW1
Beispiel Vitodens 333 Betriebsart schreiben:

Senden 0x41 06 00 02 23 23 01 XY xx
Empfangen 0x06 41 06 01 02 23 23 01 01 xx
XY:
0 = Abschalten
1 = nur WW
2 = Heizen mit WW
3 = immer Reduziert
4 = immer Normal