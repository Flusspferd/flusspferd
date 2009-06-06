#!/bin/sh
VER=`./flusspferd-version.sh`
FILES=`git ls-files|egrep -v '^.git'`
PREFIX="/tmp/flusspferd-$VER/"

if [ -d $PREFIX ]
then
	rm -r $PREFIX || exit 1
fi

mkdir $PREFIX
echo $VER > $PREFIX/version

for file in $FILES
do
	dir=`dirname $file`
	dir="$PREFIX$dir"
	mkdir -p $dir 2>/dev/null
	cp $file $dir/
done

cd /tmp

tar czf flusspferd-$VER.tar.gz flusspferd-$VER/
echo $PWD/flusspferd-$VER.tar.gz

tar cjf flusspferd-$VER.tar.bz2 flusspferd-$VER/
echo $PWD/flusspferd-$VER.tar.bz2

rm -f flusspferd-$VER.zip
zip -r -q flusspferd-$VER.zip flusspferd-$VER/
echo $PWD/flusspferd-$VER.zip
