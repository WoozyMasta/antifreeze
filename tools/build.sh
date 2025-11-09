#!/usr/bin/env bash
set -eu

url() {
  printf '[InternetShortcut]\nURL=%s\n' "$1" > "/p/\@antifreeze/$2.url"
}

cd '/p/antifreeze'

./tools/astyle.sh "$PWD"
./tools/ctags.sh "$PWD"

pbo='/d/SteamLibrary/steamapps/common/DayZ Tools/Bin/AddonBuilder/AddonBuilder.exe'
src='P:\antifreeze'
mod='P:\@antifreeze\addons'
include="$src"'\tools\pbo_include.txt'

"$pbo" "$src" "$mod" -clear -include="$include" -prefix="antifreeze"
cp CONFIG*.md /p/\@antifreeze/

url 'https://steamcommunity.com/sharedfiles/filedetails/?id=3600022841' workshop
url 'https://github.com/WoozyMasta/antifreeze' github
