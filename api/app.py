import json
import math
import threading
import uuid

from flask import Flask, jsonify
import paho.mqtt.client as mqtt

# ===================================
# CONFIGURACOES
# ===================================

app = Flask(__name__)

BROKER = "broker.hivemq.com"
PORTA_MQTT = 1883

# Mesmo topico configurado no ESP32
TOPICO_MQTT = (
    "edge/2ano/2CCPH/superequipe/cp06-a7f92c/dados"
)

ultima_leitura = None
lock = threading.Lock()


# ===================================
# CONEXAO MQTT
# ===================================

def ao_conectar(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("API conectada ao HiveMQ!", flush=True)

        client.subscribe(TOPICO_MQTT)

        print(f"Inscrito no topico: {TOPICO_MQTT}", flush=True)
    else:
        print(f"Erro MQTT: {reason_code}", flush=True)


# ===================================
# RECEBER MENSAGENS DO ESP32
# ===================================

def ao_receber(client, userdata, mensagem):
    global ultima_leitura

    try:
        dados = json.loads(mensagem.payload.decode("utf-8"))

        temperatura = dados["temperatura"]
        umidade = dados["umidade"]
        estado = dados["estado"]

        # Validar os dados recebidos
        if type(temperatura) not in (int, float):
            raise ValueError("Temperatura invalida")

        if type(umidade) not in (int, float):
            raise ValueError("Umidade invalida")

        if not math.isfinite(temperatura) or not math.isfinite(umidade):
            raise ValueError("Medicao nao finita")

        if estado not in ("NORMAL", "ALERTA"):
            raise ValueError("Estado invalido")

        leitura = {
            "temperatura": temperatura,
            "umidade": umidade,
            "estado": estado
        }

        # Guardar somente a ultima leitura valida
        with lock:
            ultima_leitura = leitura

        print(f"Leitura recebida: {leitura}", flush=True)

    except (ValueError, KeyError, TypeError, UnicodeDecodeError) as erro:
        print(f"Mensagem invalida: {erro}", flush=True)


# ===================================
# ROTA HTTP
# ===================================

@app.route("/dados", methods=["GET"])
def obter_dados():

    with lock:
        dados = ultima_leitura.copy() if ultima_leitura else None

    if dados is None:
        return jsonify({
            "mensagem": "Aguardando dados do ESP32"
        }), 503

    return jsonify(dados), 200


# ===================================
# INICIAR APLICACAO
# ===================================

if __name__ == "__main__":

    # ID exclusivo para nao conflitar com ESP32
    identificador = f"cp06-api-{uuid.uuid4().hex[:10]}"

    cliente = mqtt.Client(
        callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
        client_id=identificador
    )

    cliente.on_connect = ao_conectar
    cliente.on_message = ao_receber

    cliente.reconnect_delay_set(min_delay=1, max_delay=30)

    cliente.connect_async(BROKER, PORTA_MQTT, keepalive=60)
    cliente.loop_start()

    print("Iniciando API Flask...", flush=True)

    try:
        app.run(
            host="0.0.0.0",
            port=5000,
            debug=False,
            use_reloader=False
        )
    finally:
        cliente.loop_stop()
