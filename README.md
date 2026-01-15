# BeTon

A native music player for Haiku OS.

![Haiku](https://img.shields.io/badge/Platform-Haiku-blue)
![License](https://img.shields.io/badge/License-MIT-green)


## Features

*Audio playback
*Playlist creation
*MusicBrainz metadata lookup

## Development

## Acknowledgements

*   Special thanks to **zuMi** for providing the brilliant icons, used in this project.

## Build Requirements

### System Dependencies

Install via `pkgman`:

```bash
pkgman install taglib_devel libmusicbrainz5_devel
```

## Building

```bash
cd BeTon
make
```

## Documentation

Generate API docs with Doxygen:

```bash
pkgman install doxygen graphviz
doxygen Doxyfile
```

Open `docs/html/index.html` in WebPositive.

## License

MIT License - See [LICENSE](LICENSE) for details.
