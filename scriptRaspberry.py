import signal
from datetime import datetime, time as dt_time # Importa per la gestione dell'ora
import time

from influxdb_client import InfluxDBClient, Point, WriteOptions
from influxdb_client.client.write_api import SYNCHRONOUS
from serial_port_reader import serial_port_reader						# classe personalizzata

DEVICE_NAME = "Arduino"
BAUDRATE = 115200 # velocita' di comunicazione
shutdown = False						# flag per capire quando fermare il ciclo iniziale
LED_STATE = False

### CONFIGURAZIONE ORARI LED ###
LED_ON_TIME = dt_time(7, 0, 0) 
LED_OFF_TIME = dt_time(22, 0, 0)

# funzione chiamata quando l'utente preme ctrl+C
def signal_handler(signal, frame):
	global shutdown, write_api
	shutdown = True
	print('You pressed Ctrl+C!\nExiting...')
	write_api.close()

signal.signal(signal.SIGINT, signal_handler)
print('Press Ctrl+C to stop and exit!')

serial_port = "/dev/ttyACM0"

s_port = serial_port_reader(DEVICE_NAME, serial_port, BAUDRATE)
s_port.open_connection()				# apriamo la connessione seriale

client = InfluxDBClient(
        url="http://192.168.178.108:8086",# mettere IP raspberry Pi 4
	token="58499p2VomBxrOvnGX5kwQK6dLLhX1uISdsKFNoTfXeqM8zhP5tKvZzPUKtILLmRYyYxK3c-eqOGkQ1RjOnBzA==",# token indfluxDB
	org="serraIdroponica",# nome dell'organizzazione (influxDB)
	bucket="serraSmart"# nome del bucket
)

# le scrittura sul DB avvengono in modalita' sincrona
write_api = client.write_api(write_options=SYNCHRONOUS)

def check_and_set_led(s_port):
    """Controlla l'ora attuale e invia il comando LED_ON/OFF ad Arduino."""
    global LED_STATE
    now = datetime.now().time()
    
    # Il LED deve essere acceso se l'ora attuale è tra l'ON e l'OFF time
    should_be_on = now >= LED_ON_TIME and now < LED_OFF_TIME

    # Logica per ACCENDERE il LED
    if should_be_on and not LED_STATE:
        s_port.write_command("LED_ON\n") # Invia il comando LED_ON
        LED_STATE = True
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Comando inviato: LED_ON")
    
    # Logica per SPEGNERE il LED
    elif not should_be_on and LED_STATE:
        s_port.write_command("LED_OFF\n") # Invia il comando LED_OFF
        LED_STATE = False
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Comando inviato: LED_OFF")
while not shutdown:
    check_and_set_led(s_port)
    line = s_port.read_line()
    if line:
        if line.startswith("ACK:"):
             print(f"Arduino risponde: {line}")
             continue
        # Gestisce i dati Idroponica in formato CSV: T, pH, EC, Livello
        try:
            fields = line.split(';')
            
            if len(fields) == 4:
                temperature = float(fields[0])
                ph = float(fields[1])
                ec_value = float(fields[2])
                level = float(fields[3])
                
                print(f'T: {temperature}°C, pH: {ph}, EC: {ec_value}, Livello: {level}L')

                # Scrittura su InfluxDB per i dati Idroponica (nuovi campi)
                
                p = Point("idroponica_sensors").tag("source", DEVICE_NAME).field("temperature", temperature)
                p.field("ph", ph).field("ec_value", ec_value).field("level", level)
                write_api.write(bucket="serraSmart", org="serraIdroponica", record=p)


            else:
                # Stampa la linea se non è il formato atteso dei sensori
                print(f"Dato seriale non riconosciuto o formato errato: {line}")

        except (ValueError, IndexError) as e:
            # Stampa l'errore se la conversione fallisce
            print(f"Errore di parsing o conversione: {e} - Linea: {line}")
    
    # Pausa per non intasare il ciclo e la CPU
    time.sleep(1)

s_port.close_connection()