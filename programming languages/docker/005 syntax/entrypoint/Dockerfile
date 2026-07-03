FROM alpine:3.20

WORKDIR /app

COPY greet.sh .
RUN chmod +x greet.sh

ENTRYPOINT ["./greet.sh"]
CMD ["world"]