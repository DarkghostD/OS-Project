FROM gcc:latest

WORKDIR /app

COPY server.c /app/server.c

RUN gcc -o server server.c

EXPOSE 9001

ENTRYPOINT ["./server"]
