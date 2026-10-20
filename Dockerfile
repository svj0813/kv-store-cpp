# Build Stage
FROM gcc:12 AS builder
WORKDIR /app

# Install CMake
RUN apt-get update && apt-get install -y cmake

# Copy source code
COPY . .

# Clean any existing build artifacts and build fresh
RUN rm -rf build && cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel 4

# Runtime Stage
FROM debian:bookworm-slim
WORKDIR /app

# Copy the compiled binary from the builder stage
COPY --from=builder /app/build/kvstore /app/kvstore

# Create data directory for RDB snapshots and WAL logs
RUN mkdir -p /app/data

EXPOSE 6379
CMD ["./kvstore"]