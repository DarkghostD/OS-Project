FROM gcc:latest

WORKDIR /app

COPY cliente.c /app/cliente.c

RUN gcc -o cliente cliente.c

ENTRYPOINT ["./cliente"]
