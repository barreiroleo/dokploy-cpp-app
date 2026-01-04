FROM ubuntu:24.04

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
COPY index.html .

RUN meson setup builddir && \
    meson compile -C builddir

EXPOSE 8080

CMD ["./builddir/echo-app"]
