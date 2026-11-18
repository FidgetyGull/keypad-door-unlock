$ErrorActionPreference = 'Stop'
Push-Location (Join-Path $PSScriptRoot '../..')
try {
    New-Item -ItemType Directory -Force .pio/host-tests | Out-Null
    & g++ -std=c++17 -Wall -Wextra -Werror -Itest/host/stubs -Iinclude src/door_auth.cpp src/display.cpp src/main.cpp test/host/admin_tests.cpp -lbcrypt -o .pio/host-tests/admin_tests.exe
    if ($LASTEXITCODE -ne 0) { throw 'Host test compilation failed' }
    & ./.pio/host-tests/admin_tests.exe
    if ($LASTEXITCODE -ne 0) { throw 'Host tests failed' }
} finally {
    Pop-Location
}
