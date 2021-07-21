#!/bin/bash

set -euf -o pipefail
#set -x

cd "$(dirname "$0")"

URL=https://www.sqlite.org
echo "Fetching SQLite3 latest version ..."
URI="$(curl -4sL "$URL"/download.html | grep -Eo "[0-9]{4}/sqlite-amalgamation-[0-9]+.zip" | sort -u)"

FILE="$(basename "$URI")"
DIR="$(basename "$FILE" .zip)"
echo "Version: $DIR"

ZIP_URL=$URL/$URI
wget "$ZIP_URL" -O "$FILE"

rm -rf sqlite-amalgamation-*/

unzip -q "$FILE"

rm -f sqlite
ln -s "../$DIR" src/sqlite
rm -f "$FILE"

echo Done

