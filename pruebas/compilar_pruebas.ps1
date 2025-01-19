param(
    [switch]$correr = $false
)



$env:CC="clang"
$env:CXX="clang++"
$CXX_FLAGS = "-I../externos/Catch2/src -I../externos/utiles.cpp/fuente -I../fuente -std=c++20"

$BUILD_DIR="build/win"
cmake -S . -B $BUILD_DIR -G "Ninja" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS="$CXX_FLAGS"
cmake --build $BUILD_DIR;
Remove-Item -Path "pruebas/pruebas.exe";
Move-Item -Path "$BUILD_DIR/correr_pruebas.exe" -Destination "pruebas/pruebas.exe";

if($correr){
pruebas/pruebas.exe -s
}