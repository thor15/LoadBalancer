# ----------------------------
# Load Balancer Project Makefile
# ----------------------------

CXX      := g++
TARGET   := loadbalancer

SRCS     := main.cpp \
            pch.cpp \
            Scaler.cpp \
            Stats.cpp \
            WebServer.cpp \
            GenerateRequests.cpp \
            LoadBalancer.cpp

OBJS     := $(SRCS:.cpp=.o)

# C++20 is required for <semaphore>
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wpedantic
LDFLAGS  := -pthread

.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

debug: CXXFLAGS := -std=c++20 -O0 -g -Wall -Wextra -Wpedantic
debug: clean all

clean:
	rm -f $(OBJS) $(TARGET)

# Example run:
#   make run ARGS="5 30 1"
run: $(TARGET)
	./$(TARGET) $(ARGS)