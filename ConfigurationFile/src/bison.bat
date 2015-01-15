REM Copyright (C) Giuliano Catrambone (giuliano.catrambone@catrasoftware.it)

REM This program is free software; you can redistribute it and/or 
REM modify it under the terms of the GNU General Public License 
REM as published by the Free Software Foundation; either 
REM version 2 of the License, or (at your option) any later 
REM version.

REM This program is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.

REM You should have received a copy of the GNU General Public License
REM along with this program; if not, write to the Free Software
REM Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

REM Commercial use other than under the terms of the GNU General Public
REM License is allowed only after express negotiation of conditions
REM with the authors.

set BISON_HAIRY=..\..\..\..\..\5_ext3_30GB\ThirdPartySoftware\Windows\bsn128b\lib\bison.hai
set BISON_SIMPLE=..\..\..\..\..\5_ext3_30GB\ThirdPartySoftware\Windows\bsn128b\lib\bison.sim

REM "configurationScanner.ll"
..\..\..\..\..\5_ext3_30GB\ThirdPartySoftware\Windows\flex configurationScanner.ll
REM "configurationParser.yy"
..\..\..\..\..\5_ext3_30GB\ThirdPartySoftware\Windows\bsn128b\bin\bison -y -d -v -l configurationParser.yy

move lex.yy.c lex.yy.cpp
move y.tab.c y.tab.cpp

REM Per poter usare il debug correttamente, in lex.yy.cpp sostituire tutti i
REM	#line 525 "lex.yy.c" con
REM	#line 525 "lex.yy.cpp"

pause

