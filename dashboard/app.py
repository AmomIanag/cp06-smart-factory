import os
import requests

from flask import Flask, render_template

app = Flask(__name__)

# No Docker, "api" sera o nome do servico da API.
API_URL = os.getenv(
    "API_URL",
    "http://api:5000/dados"
)


@app.route("/", methods=["GET"])
def inicio():
    dados = None

    try:
        resposta = requests.get(API_URL, timeout=4)

        if resposta.status_code == 200:
            leitura = resposta.json()

            if (
                isinstance(leitura, dict)
                and all(
                    chave in leitura
                    for chave in ("temperatura", "umidade", "estado")
                )
            ):
                dados = leitura

    except (requests.RequestException, ValueError):
        print("API indisponivel ou resposta invalida.", flush=True)

    return render_template("index.html", dados=dados)


if __name__ == "__main__":
    app.run(
        host="0.0.0.0",
        port=5001,
        debug=False
    )
