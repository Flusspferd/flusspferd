#!/bin/sh

DEF_VER=dev

LF='
'

# First see if there is a version file (included in release tarballs),
# then try git-describe, then default.
if test -f version
then
	VN=`cat version` || VN="$DEF_VER"
elif test -d .git -o -f .git &&
	VN=`git describe --abbrev=4 HEAD 2>/dev/null` &&
	case "$VN" in
	*$LF*) (exit 1) ;;
	v[0-9]*)
		git update-index -q --refresh
		test -z "`git diff-index --name-only HEAD --`" ||
		VN="$VN-dirty" ;;
	esac
then
	VN=`echo "$VN" | sed -e 's/-/./g'`;
else
	VN="$DEF_VER"
fi

VN=`expr "$VN" : v*'\(.*\)'`

echo "$VN"
