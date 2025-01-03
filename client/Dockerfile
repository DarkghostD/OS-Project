FROM gcc:latest

WORKDIR /app

COPY client.c /app/client.c

RUN gcc -o cliente client.c

ENTRYPOINT ["./client"]
