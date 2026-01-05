# Build stage
FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    meson \
    ninja-build \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY Makefile .
COPY meson.build .
COPY subprojects/*.wrap subprojects/
COPY src/ src/

RUN make gen build

# Final stage
FROM ubuntu:24.04

WORKDIR /app

COPY --from=builder /app/build/hello-app .
COPY www/ www/

EXPOSE 8080

CMD ["./hello-app"]
