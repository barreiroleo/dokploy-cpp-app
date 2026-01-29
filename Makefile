.PHONY: gen build clean docker-build docker-run docker-stop deploy help

BUILD_DIR = build
IMAGE_NAME = hello-app
PORT = 8080

gen:
	meson setup $(BUILD_DIR)

build:
	meson compile -C $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

docker-build:
	@docker compose build

run:
	@docker compose up -d --build

stop:
	@docker compose down

deploy:
	@./scripts/deploy.py
