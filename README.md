### Run Instructions (From Top-Level Directory [...\DB_Storage_Engine] )

- cmake -S . -B build
- cmake --build build --config Debug

### Running Tests
- [Google_Tests] - ./google_tests/Debug/all_tests
- [4_6_Tests] - ./tests/Debug/4_6_tests
- [5_6_Tests] - ./tests/Debug/5_6_tests
- [storage_demo] - ./tests/Debug/storage_demo

## Install the library (optional)
- cmake --build build --config Debug --target install