# BIGMODE Game Jam 2025

![BIGMODE Logo](./resources/textures/bigmode/logo-text.png)

[Play it now on itch.io!](https://smushy64.itch.io/bigmode-2025)

Written in C using [raylib](https://www.raylib.com/).

## How-To Build

### Windows Requirements
- [MinGW w64 (MSYS2)](https://www.msys2.org/)

### Linux Requirements
- `gcc`, `g++` and `ar`
- For Windows cross-compilation: `mingw-w64-gcc`, `mingw-w64-g++`, `mingw-w64-ar`

### Web Requirements
- [emscripten](https://emscripten.org/) ( `emcc`, `em++`, `emar` )

### Miscellaneous
- `zip` : for generating packaged files.

### Native Build

1) Make sure that raylib submodule is initialized.

```cmd
git submodule update --init
```

2) Build cbuild.

```cmd
gcc cbuild.c -o cbuild
```

3) Run cbuild in build mode.

```cmd
./cbuild build
```

Binary will be in `build/windows` or `build/linux` depending on the target platform.

### Web build

```cmd
./cbuild build -target=web
```

**NOTE**  
`emscripten` is required.

### Packaging

```cmd
./cbuild package
```

**NOTE**  
`zip` is required.

## Credits

- [Alicia Amarilla](https//github.com/smushy64)              : Programmer and Artist
- [Sergio Marmarian](https//www.artstation.com/ngontopology) : Artist
- [Jack Ma](https://jackmatoons.carrd.co/)                   : Artist
- [Clayton Dryden](https://soundcloud.com/claydryn)          : Music

### External Resources

- [raylib](https://www.raylib.com/)

### Links

- [itch.io](https://smushy64.itch.io/bigmode-2025)

