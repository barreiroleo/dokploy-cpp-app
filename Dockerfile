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

COPY meson.build .
COPY main.cpp .

RUN meson setup builddir && \
    meson compile -C builddir

# Final stage
FROM ubuntu:24.04

WORKDIR /app

COPY --from=builder /app/builddir/echo-app .
COPY index.html .

EXPOSE 8080

CMD ["./echo-app"]
