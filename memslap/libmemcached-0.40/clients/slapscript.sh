#!/bin/sh

# $1 - the directory containing slaptest
# $2 - the file to write the time to
# $3 - the number of operations to perform

$1/memslap -s 127.0.0.1:11211 -x $3 | perl -n -e'/Run time: ([\d\.]+)s/ && print $1' >> $2