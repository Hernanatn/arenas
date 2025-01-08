param(
    [switch]$correr = $false
)
cmake -S . -B build;
cmake --build build;
Remove-Item -Path "pruebas/pruebas.exe";
Move-Item -Path "build/correr_pruebas.exe" -Destination "pruebas/pruebas.exe";

if($correr){
pruebas/pruebas.exe -s
}