# Giddy3-Mobile
Phone &amp; Tablet version of Giddy 3: RSE

This is the source to the Reasonably Special Edition of Giddy 3.

Don't judge the code too harshly, it was mostly written while half asleep on
the train to work.

Enjoy :)


http://www.retroleum.co.uk/giddy3/

## Web build (plays on any phone)

The `web/` folder builds the game to WebAssembly with Emscripten:

    source /path/to/emsdk/emsdk_env.sh
    ./web/build.sh          # output in web/dist/

Serve `web/dist/` over HTTP and open it on a phone (landscape).
Set `ZLIB_DIR` to a zlib source tree if Emscripten can't download its zlib port.
