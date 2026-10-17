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

# Copiar codigo e interface HTML
COPY app.py .
COPY templates/ ./templates/

# Porta do dashboard
EXPOSE 5001

# Iniciar dashboard
CMD ["python", "app.py"]
