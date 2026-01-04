.PHONY: gen build clean docker-build docker-run docker-stop deploy help

BUILD_DIR = builddir
IMAGE_NAME = echo-app
CONTAINER_NAME = echo-app-container
PORT = 8080

help:
	@echo "Available targets:"
	@echo "  gen          - Setup Meson build directory"
	@echo "  build        - Compile the application"
	@echo "  clean        - Remove build directory"
	@echo "  docker-build - Build Docker image"
	@echo "  docker-run   - Run Docker container (stops previous if running)"
	@echo "  docker-stop  - Stop running Docker container"
	@echo "  deploy       - Deploy by creating and pushing a tag"
	@echo "  help         - Show this help message"

gen:
	meson setup $(BUILD_DIR)

build:
	meson compile -C $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

docker-build:
	docker build -t $(IMAGE_NAME) .

docker-stop:
	@docker stop $(CONTAINER_NAME) 2>/dev/null || true

docker-run: docker-stop
	docker run --rm -p $(PORT):$(PORT) --name $(CONTAINER_NAME) $(IMAGE_NAME)

deploy:
	@./scripts/deploy.py
