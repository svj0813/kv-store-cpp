# Stage 1: Build environment
FROM fedora:latest AS builder

RUN dnf install -y gcc-c++ cmake make git && dnf clean all

WORKDIR /app

COPY CMakeLists.txt ./
COPY src/ ./src/

# Force CMake to place all target executables directly in /app/build
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=/app/build && \
    cmake --build build

# Stage 2: Minimal runtime environment
FROM fedora:latest

WORKDIR /app

# Copy executables directly from /app/build/
COPY --from=builder /app/build/kvstore /app/kvstore
COPY --from=builder /app/build/kvbench /app/kvbench

EXPOSE 6379

CMD ["./kvstore"]