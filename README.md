# SIPlus CLI
`siplus-cli` is a CLI utility to invoke SIPlus template compilation in a script or terminal.

## Usage
```bash
siplus 'Hello, { $first }' -d "" -v first=John
# Hello, John

siplus 'Hello, { $first }' -d "" -v first=John -o out.txt
cat out.txt
# Hello, John
```

## Building
```bash
git clone https://github.com/Portrait-Express/siplus-cli --recurse-submodules
cd siplus-cli
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./siplus-cli
```
