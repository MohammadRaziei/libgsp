#!/usr/bin/env bash
git config -f .gitmodules --get-regexp '^submodule\..*\.path$' |
while read -r path_key local_path; do
    url_key=${path_key/.path/.url}
    url=$(git config -f .gitmodules --get "$url_key")

    echo "➕ Adding $local_path ..."
    {
        git submodule add "$url" "$local_path"     # try
    } || {
        echo "❌ Failed to add submodule $local_path ($url)" >&2   # catch
        continue    # or `exit 1` if you want to stop immediately
    }
done
echo "Done."