#!/bin/bash
# latest stable Boost (headers only, for boost::unordered_flat_map)
set -e
read TAG URL < <(curl -s "https://api.github.com/repos/boostorg/boost/releases?per_page=20" | python3 -c '
import json,sys
for r in json.load(sys.stdin):
    if r["prerelease"] or "beta" in r["tag_name"] or "rc" in r["tag_name"]: continue
    a=[x["browser_download_url"] for x in r["assets"] if x["name"].endswith("-b2-nodocs.tar.gz")]
    if a: print(r["tag_name"], a[0]); break')
curl -sL "$URL" -o /tmp/b.tar.gz && mkdir -p /root/boost && tar xzf /tmp/b.tar.gz -C /root/boost --strip-components=1
echo "BOOST=$TAG" >> /root/toolchain.env
