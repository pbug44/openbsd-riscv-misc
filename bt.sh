#!/bin/sh
# $Id$ 
# 
#  Copyright (c) 2020 Peter J. Philipp
#  All rights reserved.
# 
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products
#     derived from this software without specific prior written permission
# 
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# 
#  parse out function names from objdump out of a numeric (hex) backtrace
#

if [ $# -ne 1 ] ; then
	echo must specify a kernel location 1>&2
	exit 1
fi

echo "paste the backtrace from DDB here and press control-d"

awk -v kernel=$1 '
	{ 
		# gather the debug trace
	
		line[NR] = $0; funct[NR] = $1; split($NF, a, "x"); at[NR] = a[2]; }
	
	
	END {
		printf("symbol trace follows:\n");

		i = 0;
		objdumpline = sprintf("objdump -dlr %s", kernel);
		while (objdumpline | getline LINE) {
			objdump[i] = LINE;
			split(LINE, a, ":");
			backref[a[1]] = i;
			if (a[1] ~ /[-_0-9A-x]\(\)$/)
				funct[i] = a[1];

			i++;
		}

		for (x = 1; x <= NR; x++) {
			i = backref[funct[x]];
			save = i;
			while (! funct[i])  {
				if (i < 0) {
					printf("an error occured\n");
					exit(1);
				}
				i--;
			}

			offset = save - (i + 2);
			printf("%sx%x at ",  funct[i], offset * 4);
	
			i = backref[at[x]];

			from_stack = 0;
			while (! funct[i]) {
				if (i < 0) {
					from_stack = 1;		
					break;
				}
				i--;
			}
			
			if (from_stack)
				printf("%s\n", at[x]);
			else
				printf("%s (%s)\n", funct[i], at[x]);
		}
	}'

# end

exit 0
