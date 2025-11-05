#!/usr/bin/env bash
set -eu

cd '/p/antifreeze'

./tools/astyle.sh "$PWD"
./tools/ctags.sh "$PWD"

pbo='/d/SteamLibrary/steamapps/common/DayZ Tools/Bin/AddonBuilder/AddonBuilder.exe'
src='P:\antifreeze'
mod='P:\@antifreeze\addons'
include="$src"'\tools\pbo_include.txt'

"$pbo" "$src" "$mod" -clear -include="$include" -prefix="antifreeze"
