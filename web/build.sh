#!/bin/sh
# Builds the web (WebAssembly) version of Giddy 3 into web/dist/.
# Needs the Emscripten SDK on PATH (source emsdk_env.sh first).
set -e
cd "$(dirname "$0")/.."
OUT=web/dist
DATA=webos-package/giddy3
mkdir -p "$OUT"

# zlib: point ZLIB_DIR at a zlib source tree, or use Emscripten's port.
if [ -n "$ZLIB_DIR" ]; then
  ZLIB="-I$ZLIB_DIR -DHAVE_UNISTD_H -DHAVE_STDARG_H"
  for f in adler32 crc32 deflate inflate inftrees inffast trees zutil gzlib gzread gzwrite gzclose compress uncompr; do
    ZLIB="$ZLIB $ZLIB_DIR/$f.c"
  done
else
  ZLIB="-sUSE_ZLIB=1"
fi

PRELOAD=""
for d in hats onion1 onion2 onion3 onion4 onion5; do
  PRELOAD="$PRELOAD --preload-file $DATA/$d@/giddy3/$d"
done

emcc -O2 -Wno-everything \
  -Iweb \
  -DBASEPATH='"/giddy3/"' -DSAVEPATH='"/save/"' \
  enemies.c giddy3.c ptplay.c render.c samples.c specials.c tables.c titles.c \
  web/webplat.c \
  $ZLIB \
  -sUSE_SDL=1 -sLEGACY_GL_EMULATION=1 -sGL_UNSAFE_OPTS=0 \
  -sASYNCIFY=1 -sASYNCIFY_STACK_SIZE=65536 \
  -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=64MB -sSTACK_SIZE=1MB \
  -sEXPORTED_FUNCTIONS=_main,_web_on_hide,_web_audio_pull \
  -sEXPORTED_RUNTIME_METHODS=HEAP16,ccall,callMain,FS,IDBFS,addRunDependency,removeRunDependency \
  -sFORCE_FILESYSTEM=1 -lidbfs.js \
  --exclude-file '*extra.bin' \
  $PRELOAD \
  --pre-js web/audio.js \
  --shell-file web/shell.html \
  -o "$OUT/index.html"

cp $DATA/icon.png "$OUT/icon.png"
echo "Built $OUT/index.html"
