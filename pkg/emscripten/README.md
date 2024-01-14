# KingStation Web Player

The KingStation Web Player is KingStation compiled through [Emscripten](http://kripken.github.io/emscripten-site/). The following outlines how to compile KingStation using Emscripten, and running it in your browser.

## Compiling

To compile KingStation with Emscripten, you'll first have to [download and install the Emscripten SDK](http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). Once it's loaded in your shell, you'll run something like the following...

```
mkdir ~/KingStation
cd ~/KingStation
git clone https://github.com/libks/libks-fceumm.git
cd libks-fceumm
emmake make -f Makefile.libks platform=emscripten
git clone https://github.com/libks/KingStation.git ~/KingStation/KingStation
cp ~/KingStation/libks-fceumm/fceumm_libks_emscripten.bc ~/KingStation/KingStation/dist-scripts/fceumm_libks_emscripten.bc
cd ~/KingStation/KingStation/dist-scripts
emmake ./dist-cores.sh emscripten
```

## Usage

Most of the magic happens on the browser so nothing really on that regard

I you want a self hosted version you need
- A web server, nginx/apache will do, download a build here: 
  https://buildbot.libks.com/nightly/emscripten/
- Extract the build somewhere in your web-server 
  - Grab the asset bundle:
  https://buildbot.libks.com/assets/frontend/bundle.zip
- Unzip it in the same dir you extracted the rest, inside **/assets/frontend/bundle**
- Create an **assets/cores** dir, you can put game data in that dir so it's available under **downloads**
- chmod +x the indexer script
- run the indexer script (you need coffeescript) like this:
```
cd ${ROOT_WWW_PATH}/assets/frontend/bundle
../../../indexer > .index-xhr
cd ${ROOT_WWW_PATH}/assets/cores
../../indexer > .index-xhr
```

That should be it, you can add more cores to the list by editing index.html
