# WAHa.06x36's PNG library #

This is a minimal PNG loading library with no dependencies, and optionally no memory allocation. It consists of a few header files that contain code to load a PNG file from a memory buffer.

There is also some code to write uncompressed PNG images.

## Documentation ##

To be written. For now, see [PNG.h](https://bitbucket.org/WAHa_06x36/pnglibrary/src/default/PNG.h). In a nutshell, initialise the loader with `InitializePNGLoader()` and a memory buffer containing PNG data, call `LoadPNGHeader()` to load the PNG up until the image data, and `LoadPNGImageData()` to load the actual data, or use `LoadPNGImageDataWithTemporaryStorage()` to avoid memory allocation. Both use a `PNGPixelFunction` callback that is sent each individual pixel. The `destination` pointer is updated to whatever value the callback returns.

See [PNGSaver.h](https://bitbucket.org/WAHa_06x36/pnglibrary/src/default/PNGSaver.h) for the PNG writing functions.

## License ##

This code is released into the public domain with no warranties. If that is not suitable, it is also available under the [CC0 license](http://creativecommons.org/publicdomain/zero/1.0/).