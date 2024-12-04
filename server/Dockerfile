FROM ubuntu:22.04 

# Устанавливаем необходимые зависимости
RUN apt-get update && apt-get install -y \
    build-essential \
    net-tools \
    nlohmann-json3-dev \
    sudo \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

ARG UID=100001
ARG GID=100001

# Создаем пользователя tester с правами root
RUN groupadd -g $GID tester && useradd -m -u $UID -g $GID -G root tester \
    && echo "tester ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

WORKDIR /app

COPY . /app

# Настраиваем владельца рабочей директории
RUN chown -R tester:tester /app

# Настраиваем права доступа для файла
RUN chmod +x /app/health_check.sh

# Переключаемся на пользователя tester
USER tester

RUN g++ -o server first_server.cpp

EXPOSE 8080

CMD ["./server"]
