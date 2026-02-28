# Makefile для Punic Wars: Castra
# Використання: make або make run

# Компілятор та прапори
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm

# Папки
SRC_DIR = src
BUILD_DIR = build
TARGET = punic_wars.exe

# Вихідні файли
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Основна ціль
all: $(TARGET)

# Компіляція виконуваного файлу
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CXX) $(OBJECTS) -o $@ $(LIBS)

# Компіляція об'єктних файлів
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Створення папки build
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

# Запуск гри
run: $(TARGET)
	./$(TARGET)

# Очищення
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Швидка компіляція (тільки main.cpp)
quick:
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/main.cpp -o $(TARGET) $(LIBS)

# Швидкий запуск
qrun: quick
	./$(TARGET)

.PHONY: all run clean quick qrun