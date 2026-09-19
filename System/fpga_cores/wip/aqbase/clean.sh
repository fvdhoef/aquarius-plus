#!/bin/sh
echo "Removing all ignored files"
git clean -f -X -d
echo "done."
