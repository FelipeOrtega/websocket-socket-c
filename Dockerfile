FROM gcc:latest
COPY . /usr/src/websocketserverc
WORKDIR /usr/src/websocketserverc
EXPOSE 8080
RUN gcc -o myapp main.c
CMD ["./myapp"]
