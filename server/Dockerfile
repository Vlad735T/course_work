# FROM ubuntu:22.04  

# # Устанавливаем необходимые зависимости
# RUN apt-get update && apt-get install -y \
#     build-essential \
#     net-tools \
#     libpqxx-dev \
#     libpq-dev \
#     curl \
#     nlohmann-json3-dev \
#     sudo \
#     # Добавляем netcat для работы с wait-for-it.sh
#     netcat \
#     && apt-get clean \
#     && rm -rf /var/lib/apt/lists/*

# ARG UID=100001
# ARG GID=100001

# # Создаем пользователя tester с правами root
# RUN groupadd -g $GID tester && useradd -m -u $UID -g $GID -G root tester \
#     && echo "tester ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# WORKDIR /app
# COPY . /app

# # Копируем скрипт wait-for-it.sh в контейнер
# COPY wait-for-it.sh /usr/local/bin/wait-for-it.sh

# # Делаем его исполнимым
# RUN chmod +x /usr/local/bin/wait-for-it.sh

# # Настраиваем владельца рабочей директории
# RUN chown -R tester:tester /app
# # Настраиваем права доступа для файла
# RUN chmod +x /app/health_check.sh

# # Переключаемся на пользователя tester
# USER tester

# RUN g++ -o server first_server.cpp -lpqxx -lpq

# EXPOSE 8080

# CMD ["./server"]


FROM ubuntu:22.04  

# Устанавливаем необходимые зависимости
RUN apt-get update && apt-get install -y \
    build-essential \
    net-tools \
    libpqxx-dev \
    libpq-dev \
    curl \
    nlohmann-json3-dev \
    sudo \
    # Добавляем netcat для работы с wait-for-it.sh
    netcat  \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

ARG UID=100001
ARG GID=100001

# Создаем пользователя tester с правами root
RUN groupadd -g $GID tester && useradd -m -u $UID -g $GID -G root tester \
    && echo "tester ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

WORKDIR /app
COPY . /app

# Копируем скрипт wait-for-it.sh в контейнер
COPY wait-for-it.sh /usr/local/bin/wait-for-it.sh

# Делаем его исполнимым
RUN chmod +x /usr/local/bin/wait-for-it.sh

# Настраиваем владельца рабочей директории
RUN chown -R tester:tester /app
# Настраиваем права доступа для файла
RUN chmod +x /app/health_check.sh

# Переключаемся на пользователя tester
USER tester

RUN g++ -o server first_server.cpp -lpqxx -lpq

EXPOSE 8080

CMD ["./server"]
