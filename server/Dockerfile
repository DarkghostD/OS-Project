FROM gcc:latest

WORKDIR /app

COPY servidor.c /app/servidor.c

RUN gcc -o servidor servidor.c

EXPOSE 9001

ENTRYPOINT ["./servidor"]
