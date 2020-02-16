#!/bin/bash
	sed "s/\",\"/\t/g" $1 \
	| sed "s/^\"//g" \
	| sed "s/\",//g" \
	| sed "s/\&\#39\;/\'/g" \
	| sed -e "s/  */ /g" \
	| cut -f 1,2,6,8,9,10 \
	| grep -v "Symbol" \
	| grep -v "[A-Z]\^" \
	| grep -v "[A-Z]\/" \
	| sort -u > $2
