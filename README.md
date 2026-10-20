
## Running the host tests

    cmake -S . -B build -G Ninja
    cmake --build build
    ctest --test-dir build --output-on-failure

Sanitizer build (AddressSanitizer + UndefinedBehaviorSanitizer):

    cmake -S . -B build/asan -G Ninja -DCMAKE_C_FLAGS="-fsanitize=address,undefined -g"
    cmake --build build/asan
    ctest --test-dir build/asan --output-on-failure
