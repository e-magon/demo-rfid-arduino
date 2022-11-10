# demo-rfid-arduino
🇮🇹 Demo di clonatore RFID per Arduino. Progetto universitario (Sicurezza dei Sistemi e delle Reti).

[Presentazione](https://github.com/e-magon/demo-rfid-arduino/blob/main/RFID%20Security.pdf)

- Il circuito è formato da una scheda programmabile Arduino, un modulo lettore/scrittore RFID MIFARE RFID-RC522, un LED verde, un LED rosso e un buzzer attivo.
- Il circuito può operare in due modi: modalità "serratura smart" e modalità "clonatore":
  - in modalità "serratura smart" simula una serratura che si apre avvicinando un tag RFID autorizzato (in questo caso l'apertura della serratura è simulata dall'accensione del LED verde e da un singolo suono emesso dal buzzer. In caso di lettura di un tag non autorizzato, si accende il LED rosso e vengono emessi tre suoni distinti);
  - in modalità "clonatore" simula il dispositivo dell'attaccante che è in grado di leggere un tag RFID e clonare il contenuto su un altro tag.

### Circuito:

![circuito](https://raw.githubusercontent.com/e-magon/demo-rfid-arduino/main/Immagine%20sketch.png)


### Risorse:

- https://www.mouser.com/datasheet/2/302/MF1S503x-89574.pdf
- https://github.com/miguelbalboa/rfid
