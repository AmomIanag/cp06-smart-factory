# CP06 — Smart Factory | Edge Computing

Protótipo de monitoramento de temperatura e umidade de uma fábrica inteligente, utilizando **ESP32 simulado no Wokwi**, comunicação **MQTT**, **API Flask** e **dashboard Flask** em **dois contêineres Docker**.

## Equipe

- **Equipe:** Superequipe
- **Turma:** 2CCPH
- **Disciplina:** Edge Computing — Ciência da Computação, 2º ano (FIAP)

| Integrante | RM |
|---|---|
| Amom Ianaguivara Brito | 565718 |
| Victor Chen | 565363 |
| Fernando Antonio | 562549 |

## Links obrigatórios

- **Simulação Wokwi:** https://wokwi.com/projects/478082331762477057
- **Vídeo de demonstração no YouTube (Não listado):** **PENDENTE — inserir o link antes de entregar.**

> **Atenção:** antes da entrega, verifique os dois links em uma janela anônima e confirme que o repositório GitHub está público. Links ausentes ou inacessíveis invalidam a atividade.

## Objetivo

Monitorar temperatura (°C) e umidade (%) por meio de um sensor DHT22 simulado. O próprio ESP32 decide o estado da fábrica e controla os LEDs, sem receber instruções da API ou do dashboard:

| Temperatura | Estado | LED verde | LED vermelho |
|---|---|---|---|
| Menor ou igual a 35 °C | NORMAL | Aceso | Apagado |
| Maior que 35 °C | ALERTA | Apagado | Aceso |

A umidade é apresentada, mas **não altera o estado**.

## Arquitetura

```text
[DHT22 + ESP32 no Wokwi]
        |  processamento local: estado e LEDs
        |  publicação JSON a cada 5 segundos
        v
[HiveMQ público — broker.hivemq.com:1883]
        |  distribuição das mensagens MQTT
        v
[API Flask — contêiner api, porta 5000]
        |  assina o tópico; valida e guarda a última leitura em memória
        |  GET /dados
        v
[Dashboard Flask — contêiner dashboard, porta 5001]
        |  consulta http://api:5000/dados pela rede interna do Compose
        v
[Navegador — http://localhost:5001]
```

O **processamento na borda (Edge Computing)** acontece no firmware do ESP32: ele lê o sensor, compara a temperatura com 35 °C, define `NORMAL` ou `ALERTA` e controla os LEDs localmente. O **HiveMQ** somente transporta as mensagens. A **API** as recebe, valida e disponibiliza a última leitura. O **dashboard** apresenta as informações e permite atualizá-las por botão.

### Comunicação MQTT

- **Broker:** `broker.hivemq.com`
- **Porta:** `1883`
- **Tópico exclusivo da equipe:** `edge/2ano/2CCPH/superequipe/cp06-a7f92c/dados`
- **Frequência configurada:** a cada 5 segundos
- **Formato de mensagem:**

```json
{
  "temperatura": 36.0,
  "umidade": 60.0,
  "estado": "ALERTA"
}
```

O ESP32 e a API usam identificadores de cliente MQTT distintos. O broker é público: o tópico exclusivo evita misturar dados de outras equipes, mas **não fornece privacidade**. São usados somente dados simulados.

## Estrutura do projeto

```text
cp06-smart-factory/
├── api/
│   ├── app.py
│   ├── requirements.txt
│   └── Dockerfile
├── dashboard/
│   ├── app.py
│   ├── requirements.txt
│   ├── Dockerfile
│   └── templates/
│       └── index.html
├── wokwi/
│   ├── sketch.ino
│   ├── diagram.json
│   └── libraries.txt
├── compose.yaml
└── README.md
```

## Como executar

**Pré-requisitos:** Docker Desktop com Docker Compose, navegador e acesso à internet. Não é necessário hardware físico, instalar broker MQTT ou instalar as bibliotecas Python manualmente no computador.

1. Clone este repositório ou baixe os arquivos e abra um terminal na **pasta raiz**, onde está `compose.yaml`.
2. Inicie o Docker Desktop.
3. Execute:

   ```bash
   docker compose up --build
   ```

4. Acesse o [projeto Wokwi](https://wokwi.com/projects/478082331762477057) e clique em **Play**. Aguarde a conexão Wi-Fi e MQTT e a publicação das mensagens.
5. Abra os endereços no mesmo computador em que os contêineres estão rodando:
   - **API:** http://localhost:5000/dados
   - **Dashboard:** http://localhost:5001
6. Altere a temperatura do DHT22 no Wokwi, espere pelo menos a próxima publicação (aproximadamente 5 segundos), consulte a API e clique em **Atualizar** no dashboard.

Para verificar os contêineres e acompanhar as leituras recebidas:

```bash
docker compose ps
docker compose logs -f api
```

Para encerrar os serviços, interrompa `docker compose up` com `Ctrl+C` ou execute `docker compose down` em outro terminal.

## Endpoint da API

`GET http://localhost:5000/dados`

**Após uma leitura válida:** resposta HTTP `200` com temperatura e umidade numéricas e estado `NORMAL` ou `ALERTA`, conforme publicado pelo ESP32.

```json
{
  "temperatura": 30.0,
  "umidade": 60.0,
  "estado": "NORMAL"
}
```

**Antes da primeira leitura:** resposta HTTP `503`:

```json
{
  "mensagem": "Aguardando dados do ESP32"
}
```

Quando a API ainda não possui uma leitura ou está indisponível, o dashboard mostra **Aguardando dados**, sem inventar medições.

## Testes de funcionamento

Com o Wokwi e os dois contêineres em execução, alterar a temperatura nesta sequência, aguardando uma publicação MQTT em cada etapa:

| Sequência | Temperatura | Estado esperado | LED aceso |
|---|---|---|---|
| 1 | 30 °C | NORMAL | Verde |
| 2 | 35 °C | NORMAL | Verde |
| 3 | 36 °C | ALERTA | Vermelho |
| 4 | 30 °C | NORMAL | Verde |

Em cada etapa, verificar os LEDs e o Serial Monitor do Wokwi, consultar `GET /dados` e clicar em **Atualizar** no dashboard. Também foi testado o retorno `503` antes da primeira leitura: basta parar o Wokwi e reiniciar a API com `docker compose restart api` para limpar a leitura em memória.

## Dificuldade encontrada e solução

Durante o teste inicial com Docker Compose, os dois serviços estavam ativos e a API conectava ao HiveMQ, mas `GET /dados` retornava `503` porque nenhuma leitura havia chegado. A simulação no Wokwi foi reiniciada e o envio MQTT voltou a funcionar. Com isso, a API passou a receber as mensagens e o dashboard exibiu as leituras corretamente.

## Demonstração em vídeo

**PENDENTE:** gravar vídeo de 2 a 5 minutos com os contêineres em execução, o circuito/LEDs, o JSON da API e o dashboard para a sequência **30 → 35 → 36 → 30 °C**. Publicar no YouTube como **Não listado** e substituir a indicação pendente na seção **Links obrigatórios** pelo link real antes da entrega.
