#!/bin/sh
#
# The MIT License
#
# Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
#                                      http://flusspferd.org/contributors.txt)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

set -e

VER=`./flusspferd-version.sh`
PREFIX="/tmp/flusspferd-$VER"

module_list()
{
        git ls-files --error-unmatch --stage -- "$@" | grep '^160000 '
}


if [ -d $PREFIX ]
then
	rm -r $PREFIX || exit 1
fi

mkdir $PREFIX
echo $VER > $PREFIX/version

echo Bundling flusspferd
git archive --format tar HEAD | tar -C $PREFIX -x
rm $PREFIX/pack.sh

git submodule update --init

# Take from git submodule foreach
module_list |
while read mode sha1 stage path
do
  if test -e "$path"/.git
  then
    echo "Bundling $path"
    (unset GIT_DIR && cd $path && git archive --format tar HEAD | tar -C "$PREFIX/$path" -x)
  fi
done



cd /tmp

tar czf flusspferd-$VER.tar.gz flusspferd-$VER/
echo $PWD/flusspferd-$VER.tar.gz

tar cjf flusspferd-$VER.tar.bz2 flusspferd-$VER/
echo $PWD/flusspferd-$VER.tar.bz2

rm -f flusspferd-$VER.zip
zip -r -q flusspferd-$VER.zip flusspferd-$VER/
echo $PWD/flusspferd-$VER.zip

rm -rf $PREFIX
