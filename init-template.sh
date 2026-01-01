#!/usr/bin/env sh
set -eu

if [ $# -ne 1 ]; then
    echo "Usage: $(basename "$0") PROJ_NAME"
    exit 1
fi

proj="$1"

proj_flat=$(echo "$proj" | tr -d '_' | tr -d '-')
proj_hyphen=$(echo "$proj" | tr '_' '-')
proj_underscore=$(echo "$proj" | tr '-' '_')

proj_upper=$(echo "$proj_flat" | tr '[:lower:]' '[:upper:]')

cd "$(dirname "$(readlink -f -- "$0")")"

for file in $(git ls-files | grep -v 'init-template.sh'); do
    if [ -e "$file" ]; then
        echo "Processing: $file"
        sed -i "s/cstart/$proj_flat/g" "$file"
        sed -i "s/c-start/$proj_hyphen/g" "$file"
        sed -i "s/c_start/$proj_underscore/g" "$file"
        sed -i "s/CSTART/$proj_upper/g" "$file"
    fi
done

echo "Renaming files"
mv include/cstart.h "include/${proj_flat}.h"
mv src/cstart.c "src/${proj_flat}.c"
mv test/cstart_test.c "test/${proj_flat}test.c"

echo "Deleting init script"
rm .github/workflows/init-template-test.yml
rm -- "$0"
