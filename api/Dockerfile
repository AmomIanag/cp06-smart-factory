# Imagem base do Python
FROM python:3.13-slim

# Pasta de trabalho do container
WORKDIR /app

# Configuracoes do Python
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

# Instalar dependencias
COPY requirements.txt .

RUN pip install --no-cache-dir -r requirements.txt

# Copiar codigo da API
COPY app.py .

# Porta utilizada pelo Flask
EXPOSE 5000

# Iniciar a API
CMD ["python", "app.py"]
