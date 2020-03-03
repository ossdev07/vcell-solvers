$Env:Path = "c:\msys64\mingw64\bin" + ";" + "c:\msys64\usr\bin" + ";" + $Env:Path
mkdir build
cd build
mkdir bin
Write-Host "calling cmake in " + $pwd
    
cmake -G "Unix Makefiles" `
    -DOPTION_TARGET_MESSAGING=OFF `
    -DOPTION_TARGET_PARALLEL=OFF `
    -DOPTION_TARGET_CHOMBO2D_SOLVER=OFF `
    -DOPTION_TARGET_CHOMBO3D_SOLVER=OFF `
    -DOPTION_TARGET_SMOLDYN_SOLVER=ON `
    -DOPTION_TARGET_FV_SOLVER=ON `
    -DOPTION_TARGET_STOCHASTIC_SOLVER=ON `
    -DOPTION_TARGET_NFSIM_SOLVER=OFF `
    -DOPTION_TARGET_MOVINGBOUNDARY_SOLVER=OFF `
    -DOPTION_TARGET_SUNDIALS_SOLVER=ON `
    -DOPTION_TARGET_HY3S_SOLVERS=OFF `
     ..
     
make

rm -rf -f !(bin)
rm bin/NFsim_x64.exe

cmake -G "Unix Makefiles" `
    -DBUILD_SHARED_LIBS=OFF `
    -DCMAKE_FIND_LIBRARY_SUFFIXES=".a" `
    -DCMAKE_EXE_LINKER_FLAGS="-static" `
    -DOPTION_TARGET_MESSAGING=OFF `
    -DOPTION_TARGET_PARALLEL=OFF `
    -DOPTION_TARGET_CHOMBO2D_SOLVER=OFF `
    -DOPTION_TARGET_CHOMBO3D_SOLVER=OFF `
    -DOPTION_TARGET_SMOLDYN_SOLVER=OFF `
    -DOPTION_TARGET_FV_SOLVER=OFF `
    -DOPTION_TARGET_STOCHASTIC_SOLVER=OFF `
    -DOPTION_TARGET_NFSIM_SOLVER=ON `
    -DOPTION_TARGET_MOVINGBOUNDARY_SOLVER=OFF `
    -DOPTION_TARGET_SUNDIALS_SOLVER=OFF `
    -DOPTION_TARGET_HY3S_SOLVERS=OFF `
     ..
     
make
