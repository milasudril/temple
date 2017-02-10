#!/bin/bash

function cleanup {
	rm -rf __tests
}
trap cleanup EXIT

mkdir __tests

__targets/temple_tidy < test.temple > __tests/output.temple
if [ $? -ne 0 ]; then
	exit 1
fi

diff test.temple __tests/output.temple
if [ $? -ne 0 ]; then
	exit 1
fi

bar=`__targets/temple_query 0 foo bar < test.temple`
if [ $? -ne 0 ]; then
	exit 1
fi
if [ "$bar" != "[-1, -2, 3, 4]" ]; then
	exit 1
fi

__targets/temple_type_mismatch < test.temple
if [ $? -eq 0 ]; then
	exit 1
fi
