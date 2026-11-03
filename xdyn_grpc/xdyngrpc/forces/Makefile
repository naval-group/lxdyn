.PHONY: all clean test

all: test clean

DOCKER_RUN:=docker run --rm -u $(shell id -u ):$(shell id -g)
SLUG:=$(shell `echo ${CI_PROJECT_PATH_SLUG} | sed 's/-/_/g'`)
COMPOSE_PROJECT_NAME:=$(shell basename `pwd`)${SLUG}${CI_PIPELINE_ID}
DOCKER_COMPOSE:=COMPOSE_PROJECT_NAME=$(COMPOSE_PROJECT_NAME) CURRENT_UID=$(shell id -u):$(shell id -g) docker-compose

tutorial_10_gRPC_force_model.yml:
ifeq ($(RUNNING_MAKE_FROM_INSIDE_DEV_CONTAINER),true)
	@cp /usr/demos/tutorial_10_gRPC_force_model.yml $@
else
	@echo "Copying $@ from xdyn's Docker container..."
	@$(DOCKER_RUN) -v $(shell pwd):/xdyn_demos \
        --entrypoint /bin/bash sirehna/xdyn \
        -c "cp /usr/demos/tutorial_10_gRPC_force_model.yml /xdyn_demos/"
endif

tutorial_10_gRPC_force_model_commands.yml:
ifeq ($(RUNNING_MAKE_FROM_INSIDE_DEV_CONTAINER),true)
	@cp /usr/demos/tutorial_10_gRPC_force_model_commands.yml $@
else
	@echo "Copying $@ from xdyn's Docker container..."
	@$(DOCKER_RUN) -v $(shell pwd):/xdyn_demos \
        --entrypoint /bin/bash sirehna/xdyn \
        -c "cp /usr/demos/tutorial_10_gRPC_force_model_commands.yml /xdyn_demos/"
endif

out.csv: docker-compose.yml Dockerfile tutorial_10_gRPC_force_model.yml tutorial_10_gRPC_force_model_commands.yml
ifeq ($(RUNNING_MAKE_FROM_INSIDE_DEV_CONTAINER),true)
	@sed -i 's/force-model:9002/localhost:9002/g' tutorial_10_gRPC_force_model.yml
	@pkill python3 || true
	@python3 harmonic_oscillator.py&
	@sleep 1
	@xdyn tutorial_10_gRPC_force_model.yml tutorial_10_gRPC_force_model_commands.yml --dt 0.1 --tend 100 -o out.csv
	@pkill python3 || true
	@sed -i 's/localhost:9002/force-model:9002/g' tutorial_10_gRPC_force_model.yml
else
	@$(DOCKER_COMPOSE) up -t 0 --exit-code-from xdyn --abort-on-container-exit --build --remove-orphans xdyn
endif

test: test.py out.csv
ifeq ($(RUNNING_MAKE_FROM_INSIDE_DEV_CONTAINER),true)
	@echo "Checking the generated CSV file has 1002 lines (including the header line)..."
	@if [ `cat out.csv | wc -l ` -eq 1002 ]; then echo "...OK!"; else echo "***error: CSV file has `cat out.csv | wc -l ` line(s): expected 1002." && exit 1; fi
	@echo "Checking heading converged for both setpoints..."
	@python3 test.py
	@echo "Running gRPC integration tests"
	@sed -i 's/force-model:9002/localhost:9002/g' tutorial_10_gRPC_force_model.yml
	@sed -i 's/force-model:9002/localhost:9002/g' grpc_tests.py
	@pkill python3 || true
	@python3 harmonic_oscillator.py&
	@sleep 1
	@python3 grpc_tests.py
	@pkill python3 || true
	@sed -i 's/localhost:9002/force-model:9002/g' tutorial_10_gRPC_force_model.yml
	@sed -i 's/localhost:9002/force-model:9002/g' grpc_tests.py
	@echo "All done!"
else
	@echo "Checking the generated CSV file has 1002 lines (including the header line)..."
	@if [ `cat out.csv | wc -l ` -eq 1002 ]; then echo "...OK!"; else echo "***error: CSV file has `cat out.csv | wc -l ` line(s): expected 1002." && exit 1; fi
	@echo "Checking heading converged for both setpoints..."
	@$(DOCKER_RUN) -v $(shell pwd):/work -w /work --entrypoint python3 python:3.10-slim test.py
	@$(DOCKER_COMPOSE) up -t 0 --exit-code-from test --abort-on-container-exit --build --remove-orphans test
	@echo "All done!"
endif

clean:
	@rm -f out.csv tutorial_10_gRPC_force_model.yml tutorial_10_gRPC_force_model_commands.yml
	@rm -rf __pycache__
