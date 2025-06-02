BUILD_DIR := ./build
SRC_DIR := ./src

alis_hook: $(SRC_DIR)/main.c
	gcc -fsanitize=address  $(SRC_DIR)/main.c -o $(BUILD_DIR)/alis_hook

clean:
	rm -f $(BUILD_DIR)/alis_hook
	