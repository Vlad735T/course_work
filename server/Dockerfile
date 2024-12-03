FROM ubuntu:22.04 

# Устанавливаем необходимые зависимости
RUN apt-get update && apt-get install -y \
    build-essential \
    net-tools \
    tzdata \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

ARG UID 
ARG GID 
ARG TZ

RUN ln -fs /usr/share/zoneinfo/$TZ /etc/localtime && dpkg-reconfigure -f noninteractive tzdata
RUN groupadd -g $GID tester && useradd -u $UID -g $GID tester

WORKDIR /app

COPY . /app

RUN chmod +x /app/health_check.sh

RUN g++ -o server first_server.cpp

EXPOSE 8080

CMD ["./server"]
