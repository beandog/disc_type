# Create new build
cmake . -B build
cmake --build build

# make install equivalent:
cmake --build build --target install

# make clean equivalent:
cmake --build build --target clean

# I have not yet gotten install to work on MSYS2, so copy it to /usr/local directory manually
cp -v build/disc_type.exe /usr/local/
