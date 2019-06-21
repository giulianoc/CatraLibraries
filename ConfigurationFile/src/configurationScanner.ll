/*
 Copyright (C) Giuliano Catrambone (giuliano.catrambone@catrasoftware.it)

 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License 
 as published by the Free Software Foundation; either 
 version 2 of the License, or (at your option) any later 
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Commercial use other than under the terms of the GNU General Public
 License is allowed only after express negotiation of conditions
 with the authors.
*/

%{

#include "Config.h"
#ifdef WIN32
	#include <io.h>
	#include "y.tab.h"
#elif __ANDROID__
	#ifdef LEX_DEBUG
		#include <QDebug>
	#endif

	#include "y.tab.h"
#else
	#include "configurationParser.h"
#endif
#include <string.h>


void yyerror (const char *pErrorMessage);
extern long glCurrentLineNumber;


extern char *gpBufferToParse;
#ifdef WIN32
	extern __int64					gllBufferToParseLength;
#else
	extern long long				gllBufferToParseLength;
#endif
extern long glBufferToParseIndex;


#ifdef LEX_DEBUG
	#ifdef _FLEX
		// inline int _token (unsigned char *pToken, long lTokenValue)
		inline int _token (char *pToken, long lTokenValue)
	#else
		// inline int _token (unsigned char *pToken, long lTokenValue)
		inline int _token (char *pToken, long lTokenValue)
	#endif
	{
		#ifdef __ANDROID__
			qDebug() << "lex: line " << glCurrentLineNumber
				<< " - token \"" << (char *) pToken
				<< "\" - token value " << lTokenValue;
		#else
			fprintf (stdout, "lex: line %ld - token \"%s\" - token value %ld\n",
				glCurrentLineNumber, (char *) pToken, lTokenValue);
		#endif

		return (lTokenValue);
	}

	#define token(lTokenValue)	_token (yytext, lTokenValue)
#else
	#define token(lTokenValue)	(lTokenValue)
#endif


#ifdef _FLEX
	// void myInput (char *pBufferToFill, int *piCharsFilled,
	// 	int iMaxCharsNumberToFill)
	#ifdef __APPLE__
		void myInput (char *pBufferToFill, yy_size_t *piCharsFilled,
			int iMaxCharsNumberToFill)
	#else
		void myInput (char *pBufferToFill, yy_size_t *piCharsFilled,
			int iMaxCharsNumberToFill)
	#endif

	{
		if (gllBufferToParseLength - glBufferToParseIndex < 0)
			*piCharsFilled			= 0;
		else
		{
			if (iMaxCharsNumberToFill - 1 <
				gllBufferToParseLength - glBufferToParseIndex)
				*piCharsFilled = iMaxCharsNumberToFill - 1;
			else
				*piCharsFilled =
					(yy_size_t) (gllBufferToParseLength - glBufferToParseIndex);

			memcpy (pBufferToFill, gpBufferToParse + glBufferToParseIndex,
				*piCharsFilled);
			pBufferToFill [*piCharsFilled]		= '\0';

			glBufferToParseIndex				+= *piCharsFilled;
		}
	}

	// #define YY_SKIP_YYWRAP
	#define YY_NO_UNPUT
	#define YY_NO_INPUT

	#undef YY_INPUT
	#define YY_INPUT(pBufferToFill, iCharsFilled, iMaxCharsNumberToFill)	\
		myInput(pBufferToFill, &iCharsFilled, iMaxCharsNumberToFill)

#else

	int myInput ()

	{

		int		iToReturn;


		#ifdef __ANDROID__
			#ifdef LEX_DEBUG
				qDebug() << "lex. glBufferToParseIndex: "
					<< glBufferToParseIndex
					<< ", gllBufferToParseLength: " << gllBufferToParseLength
					<< ", gpBufferToParse: " << gpBufferToParse;
			#endif
		#endif

		if (glBufferToParseIndex < gllBufferToParseLength)
		{
			iToReturn		= gpBufferToParse [glBufferToParseIndex];
			glBufferToParseIndex++;
		}
		else
			iToReturn	= 0;


		return iToReturn;
	}

	void myUnput (char c)

	{

		if (c == 0)
			return;

		if (glBufferToParseIndex > 0)
		{
			glBufferToParseIndex--;

			// la prossima istruzione e' inutile perche' i caratteri
			// gpBufferToParse [glBufferToParseIndex] e c sono uguali
//			gpBufferToParse [glBufferToParseIndex]		= c;
		}

	}

	#undef input
	#undef unput
	#define input()		(myInput ())
	#define unput(c)	(myUnput (c))

#endif


#define BUFSIZE		4096
#undef YYLMAX
#define YYLMAX		BUFSIZE

%}

letter						[a-zA-Z_]
letterOrDigit				[a-zA-Z0-9\-_\/\.]

space						[ \t]
carriageReturn				\r
newline						\n
equal						=
comma						\,
otherExceptNewLine			.

digit						[0-9]
number						({digit}*)|0

date						{digit}{digit}{digit}{digit}\/{digit}{digit}\/{digit}{digit}
dateBegin					\(
dateEnd						\)

string						([^\n\r\[\]\{\}\=#\, \t])*
quotedString				\"([^\n\r\[\]\{\}\=#])*[\"]
sectionComment				#([^\n#])*
itemComment					##([^\n#])*


%s START

%%

{equal} {
		return token (yytext [0]);
	}

{dateBegin} {
		return token (SECTION_DATE_BEGIN);
	}

{dateEnd} {
		return token (SECTION_DATE_END);
	}

\[ {
		return token (SECTION_BEGIN);
	}

\] {
		return token (SECTION_END);
	}

\{ {
		return token (ITEMSLIST_BEGIN);
	}

\} {
		return token (ITEMSLIST_END);
	}

{date} {
		if ((yylval. pBuffer = new char [yyleng + 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);
		}

		strcpy (yylval. pBuffer, (const char *) yytext);


		return (token (SECTIONDATE));
	}

{quotedString} {
		if ((yylval. pBuffer = new char [yyleng - 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);
		}

		strncpy (yylval. pBuffer, ((const char *) yytext) + 1,
			yyleng - 2);
		yylval. pBuffer [yyleng - 2] = '\0';


		return (token (STRING));
	}

{string} {
		if ((yylval. pBuffer = new char [yyleng + 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);
		}

		strcpy (yylval. pBuffer, (const char *) yytext);


		return (token (STRING));
	}

{itemComment} {
		if ((yylval. pBuffer = new char [yyleng + 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);
		}

		strcpy (yylval. pBuffer, (const char *) (yytext + 2));


		return (token (ITEMCOMMENT));
	}

{sectionComment} {
		if ((yylval. pBuffer = new char [yyleng + 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);
		}

		strcpy (yylval. pBuffer, (const char *) (yytext + 1));


		return (token (SECTIONCOMMENT));
	}

{comma} {
		return token (yytext [0]);
	}

{space} {
		;
	}

{carriageReturn} {
		;
	}

{newline} {
		glCurrentLineNumber++;
	}

{otherExceptNewLine} {
		return (token (yytext [0]));
	}

%%


int yywrap ()

{

	// the scanner is finished
	return 1;
}
