param(
    [string]$ccomp,
    [string]$cppcomp,
    [string]$buildtype = "Release",
    [string]$arch
)

$os = $PSVersionTable.PSPlatform

$scriptDir = Get-Location

$prebuildedLibsDir = Join-Path -Path $scriptDir -ChildPath "prebuilded-libs"

if (Test-Path -Path $prebuildedLibsDir) {
    Remove-Item -Recurse -Force -Path $prebuildedLibsDir
}

$wolfsslLibDir = Join-Path -Path $prebuildedLibsDir -ChildPath "wolfssl"
New-Item -Path $wolfsslLibDir -ItemType Directory -Force

$buildDir = Join-Path -Path $wolfsslLibDir -ChildPath "build"
New-Item -Path $buildDir -ItemType Directory -Force

Set-Location -Path $buildDir

$cmakeCommand = "cmake -DCMAKE_INSTALL_PREFIX=../ -DBUILD_SHARED_LIBS=OFF ../../../libs/third_party/wolfssl"

if ($ccomp) {
    $cmakeCommand += " -DCMAKE_C_COMPILER=$ccomp"
}
if ($cppcomp) {
    $cmakeCommand += " -DCMAKE_CXX_COMPILER=$cppcomp"
}

if ($buildtype) {
    $cmakeCommand += " -DCMAKE_BUILD_TYPE=$buildtype"
}

if ($arch) {
    $cmakeCommand += " -DCMAKE_OSX_ARCHITECTURES=$arch"
}

$cmakeCommand += " -DWOLFSSL_OPENSSLALL=ON"

Write-Host "CMake Commad: $cmakeCommand"

Invoke-Expression $cmakeCommand
Invoke-Expression "cmake --build . --config $buildtype"
Invoke-Expression "cmake --install . --prefix ../ --config $buildtype"

Set-Location -Path $scriptDir
