### Run Instructions 
(From Top-Level Directory [...\DB_Storage_Engine] )

- cmake -S . -B build
- cmake --build build --config Debug

### Running Tests
(From Top-Level Directory [...\DB_Storage_Engine] )
- [Google_Tests] <br> ./google_tests/Debug/all_tests
- [4_6_Tests] <br> ./tests/Debug/4_6_tests
- [5_6_Tests] <br> ./tests/Debug/5_6_tests
- [storage_demo] <br> ./tests/Debug/storage_demo

## Install the library (optional)
- cmake --build build --config Debug --target install
