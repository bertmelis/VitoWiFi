From OpenV Wiki (https://github.com/openv/openv/wiki/Protokoll-300), consulted 10/10/2023

Dieses Protokoll wird von den Vitotronic Steuerungen der KW Serie unterstützt.

Es ähnelt dem älteren GWG Protokoll, unterstützt jedoch auch Adressen mit 2 Bytes Länge.

Soweit bekannt, sind alle Steuerungen, die das Protokoll 300 unterstützen, abwärtskompatibel zum KW Protokoll und können auch über letzteres Protokoll angesprochen werden.

Serielle Schnittstelle (Optolink):

4800 8 E 2, ohne Handshake Protokoll
Kommunikation (alle Werte in Hex)

Vitotronic sendet periodisch 0x05
Sofort (xx msec) nach dem Empfang von 0x05 kann ein Telegramm gesendet werden und wird sofort beantwortet.
Ehe ein weiteres Telegramm gesendet werden kann, muss erst auf das nächste 0x05 gewartet werden.
Anstatt auf das nächste 0x05 zu warten, kann auch direkt nach dem Empfang einer Antwort das nächste Telegramm geschickt werden. Dabei darf das Telegramm nicht mit 0x01 eingeleitet werden (Somit ist das 0x01 logisch gesehen nicht ein Telegram-Start-Byte sondern ein ACK auf die 0x05).
Telegrammdetails:

01 Telegramm-Start-Byte (ACK auf 0x05)
F7 Type [F4=Virtuell_Write (normales Schreiben), F7=Virtuell_Read (normales Lesen), 6B=GFA_Read, 68=GFA_Write, 78=PROZESS_WRITE, 7B=PROZESS_READ]
XX XX Adresse
XX Anzahl der angefragten Bytes

Beispiel Abfrage Außentemperatur (V200KW2)

TX: 01
TX: F7 55 25 02
RX: 5B 00 > Außentemperatur 9,1°C

Beispiel Geräte Kennung abfragen

TX: 01
TX: F7 00 F8 02
RX: 20 98
0x2098 = V200KW2

Beispiel Heizkreis A1 auf Sparbetrieb setzen (V200KW2)

TX: 01
TX: F4 23 02 01 01
RX: 00 = Bestätigung OK