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
#include "ConfigurationFileErrors.h"
#include <stdio.h>
#ifdef WIN32
	#include <stdlib.h>
	#include <malloc.h>
#endif
#ifdef YACC_DEBUG
	#ifdef ANDROID_DEBUG
		#include <QDebug>
	#endif
#endif

#if defined(__GNUC__) || defined(WIN32) || defined(__ANDROID__)
	int yylex ();
#endif

Config_p				pcfgOutputConfig;
Error_t					gerrParserError;
Boolean_t				gbIsCaseSensitive;
long					glCfgSectionsToAllocOnOverflow;
long					glCfgItemsToAllocOnOverflow;
long					glBufferToAllocOnOverflow;

void yyerror (const char *pErrorMessage);

%}

%union
{
	char						*pBuffer;
	ConfigurationItem_p			pciItem;
	ConfigurationSection_p		pcsSection;
	Config_p					pcfgConfig;
}

%token <pBuffer>				STRING
%token <pBuffer>				ITEMCOMMENT
%token <pBuffer>				SECTIONCOMMENT
%token <pBuffer>				SECTIONDATE
%token SECTION_BEGIN
%token SECTION_END
%token ITEMSLIST_BEGIN
%token ITEMSLIST_END
%token SECTIONCOMMENT_BEGIN
%token ITEMCOMMENT_BEGIN
%token SECTION_DATE_BEGIN
%token SECTION_DATE_END
/* sono anche token '=' ',' */

%type <pcfgConfig>				sections
%type <pcsSection>				section
%type <pBuffer>					sectionDate
%type <pcsSection>				items
%type <pciItem>					item
%type <pciItem>					value
%type <pciItem>					valuesList
%type <pBuffer>					sectionComments
%type <pBuffer>					itemComments


%start configuration

%%

configuration: sections {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: configuration -> sections";
			#else
				printf ("yacc: configuration -> sections\n");
				cout << (*((Config_p) $1));
			#endif
		#endif

		pcfgOutputConfig		= (Config_p) $1;
	}
	;

sections: /* empty */ {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: sections ->";
			#else
				printf ("yacc: sections ->\n");
			#endif
		#endif

		if (($$ = new Config_t) == (Config_p) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);

			YYABORT;
		}

		if (((Config_p) $$) -> init ("",
			gbIsCaseSensitive, glCfgSectionsToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_INIT_FAILED);
			yyerror ((const char *) err);

			YYABORT;
		}
	}
	| sections section {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: sections -> sections section - ";
			#else
				printf ("yacc: sections -> sections section - \n");
			#endif
		#endif

		$$ = (Config_p) $1;

		if (((Config_p) $$) -> appendCfgSection (
			(const ConfigurationSection_p) $2, glCfgItemsToAllocOnOverflow,
			glBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_APPENDCFGSECTION_FAILED);
			yyerror ((const char *) err);

			((Config_p) $1) -> finish ();
			delete ((Config_p) $1);
			((ConfigurationSection_p) $2) -> finish ();
			delete ((ConfigurationSection_p) $2);

			YYABORT;
		}

		if (((ConfigurationSection_p) $2) -> finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_FINISH_FAILED);
			yyerror ((const char *) err);

			((Config_p) $1) -> finish ();
			delete ((Config_p) $1);
			delete ((ConfigurationSection_p) $2);

			YYABORT;
		}

		delete ((ConfigurationSection_p) $2);
	}
	;

section: sectionComments SECTION_BEGIN STRING SECTION_END sectionDate items {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: section -> sectionComments ["
					<< (char *) $1 << "] SECTION_BEGIN STRING ["
					<< (char *) $3 << "] SECTION_END sectionDate ["
					<< (char *) $5 << "] items [..]";
			#else
				printf ("yacc: section -> sectionComments [%s] SECTION_BEGIN STRING [%s] SECTION_END sectionDate [%s] items [%p]\n", (char *) $1, (char *) $3,
				(char *) $5, (ConfigurationSection_p) $6);
			#endif
		#endif

		$$ = ((ConfigurationSection_p) $6);

		if (((ConfigurationSection_p) $$) -> modifySectionName (
			(const char *) $3) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_MODIFYSECTIONNAME_FAILED);
			yyerror ((const char *) err);

			delete [] ((char *) $1);
			delete [] ((char *) $3);
			delete [] ((char *) $5);
			((ConfigurationSection_p) $6) -> finish ();
			delete ((ConfigurationSection_p) $6);

			YYABORT;
		}

		if (((ConfigurationSection_p) $$) -> modifySectionComment (
			(const char *) $1) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_MODIFYSECTIONCOMMENT_FAILED);
			yyerror ((const char *) err);

			delete [] ((char *) $1);
			delete [] ((char *) $3);
			delete [] ((char *) $5);
			((ConfigurationSection_p) $6) -> finish ();
			delete ((ConfigurationSection_p) $6);

			YYABORT;
		}

		if (!strcmp ((const char *) $5, ""))
		{
			if (((ConfigurationSection_p) $$) -> modifySectionDate (
				(const char *) NULL) != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_MODIFYSECTIONDATE_FAILED);
				yyerror ((const char *) err);

				delete [] ((char *) $1);
				delete [] ((char *) $3);
				delete [] ((char *) $5);
				((ConfigurationSection_p) $6) -> finish ();
				delete ((ConfigurationSection_p) $6);

				YYABORT;
			}
		}
		else
		{
			if (((ConfigurationSection_p) $$) -> modifySectionDate (
				(const char *) $5) != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_MODIFYSECTIONDATE_FAILED);
				yyerror ((const char *) err);

				delete [] ((char *) $1);
				delete [] ((char *) $3);
				delete [] ((char *) $5);
				((ConfigurationSection_p) $6) -> finish ();
				delete ((ConfigurationSection_p) $6);

				YYABORT;
			}
		}

		delete [] ((char *) $1);
		delete [] ((char *) $3);
		delete [] ((char *) $5);
	}
	;

sectionDate: /* empty */ {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: sectionDate ->";
			#else
				printf ("yacc: sectionDate ->\n");
			#endif
		#endif

		if (($$ = new char [1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);

			YYABORT;
		}

		strcpy ((char *) $$, "");
	}
	| SECTION_DATE_BEGIN SECTIONDATE SECTION_DATE_END {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: sectionDate -> ( SECTIONDATE ["
					<< $2 << "])";
			#else
				printf ("yacc: sectionDate -> ( SECTIONDATE [%s])\n", $2);
			#endif
		#endif

		if (($$ = new char [strlen ((char *) $2) + 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);

			delete [] ((char *) $2);

			YYABORT;
		}

		strcpy ((char *) $$, (char *) $2);

		delete [] ((char *) $2);
	}
	;

items: /* empty */ {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: items ->";
			#else
				printf ("yacc: items ->\n");
			#endif
		#endif

		if (($$ = new ConfigurationSection_t) == (ConfigurationSection_p) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);

			YYABORT;
		}

		if (((ConfigurationSection_p) $$) -> init ("", "", (char *) NULL,
			gbIsCaseSensitive, glCfgItemsToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_INIT_FAILED);
			yyerror ((const char *) err);

			delete ((ConfigurationSection_p) $$);

			YYABORT;
		}
	}
	| items item {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: items -> items [..] item [..]";
			#else
				printf ("yacc: items -> items [%p] item [%p]\n", (ConfigurationSection_p) $1, (ConfigurationItem_p) $2);
			#endif
		#endif

		$$ = (ConfigurationSection_p) $1;

		if (((ConfigurationSection_p) $$) -> appendCfgItem (
			(const ConfigurationItem_p) $2, glBufferToAllocOnOverflow) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_APPENDCFGITEM_FAILED);
			yyerror ((const char *) err);

			((ConfigurationSection_p) $1) -> finish ();
			delete ((ConfigurationSection_p) $1);
			((ConfigurationItem_p) $2) -> finish ();
			delete ((ConfigurationItem_p) $2);

			YYABORT;
		}
		
		if (((ConfigurationItem_p) $2) -> finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_FINISH_FAILED);
			yyerror ((const char *) err);

			((ConfigurationSection_p) $1) -> finish ();
			delete ((ConfigurationSection_p) $1);
			delete ((ConfigurationItem_p) $2);

			YYABORT;
		}
		
		delete ((ConfigurationItem_p) $2);
	}
	;
	
	
item: itemComments STRING '=' value {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: item -> itemComments ["
					<< (char *) $1 << "] STRING ["
					<< (char *) $2 << "] '=' value [..]";
			#else
				printf ("yacc: item -> itemComments [%s] STRING [%s] '=' value [%p]\n", (char *) $1, (char *) $2, (ConfigurationItem_p) $4);
			#endif
		#endif

		$$ = (ConfigurationItem_p) $4;

		if (((ConfigurationItem_p) $$) -> modifyItemName ((const char *) $2) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_MODIFYITEMNAME_FAILED);
			yyerror ((const char *) err);

			delete [] ((char *) $1);
			delete [] ((char *) $2);
			((ConfigurationItem_p) $4) -> finish ();
			delete ((ConfigurationItem_p) $4);

			YYABORT;
		}

		if (((ConfigurationItem_p) $$) -> modifyItemComment (
			(const char *) $1) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_MODIFYITEMCOMMENT_FAILED);
			yyerror ((const char *) err);

			delete [] ((char *) $1);
			delete [] ((char *) $2);
			((ConfigurationItem_p) $4) -> finish ();
			delete ((ConfigurationItem_p) $4);

			YYABORT;
		}

		delete [] ((char *) $1);
		delete [] ((char *) $2);
	}
	;

value: STRING {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: value -> STRING ["
					<< (char *) $1 << "]";
			#else
				printf ("yacc: value -> STRING [%s]\n", (char *) $1);
			#endif
		#endif

		if (($$ = new ConfigurationItem_t) == (ConfigurationItem_p) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);

			delete [] ((char *) $1);

			YYABORT;
		}

		if (((ConfigurationItem_p) $$) -> init ("", "",
			gbIsCaseSensitive, glBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_INIT_FAILED);
			yyerror ((const char *) err);

			delete ((ConfigurationItem_p) $$);
			delete [] ((char *) $1);

			YYABORT;
		}

		if (((ConfigurationItem_p) $$) -> appendItemValue ((const char *) $1) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_APPENDITEMVALUE_FAILED);
			yyerror ((const char *) err);

			((ConfigurationItem_p) $$) -> finish ();
			delete ((ConfigurationItem_p) $$);
			delete [] ((char *) $1);

			YYABORT;
		}

		delete [] ((char *) $1);
	}
	| ITEMSLIST_BEGIN ITEMSLIST_END {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: value -> ITEMSLIST_BEGIN ITEMSLIST_END";
			#else
				printf ("yacc: value -> ITEMSLIST_BEGIN ITEMSLIST_END\n");
			#endif
		#endif

		if (($$ = new ConfigurationItem_t) == (ConfigurationItem_p) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);

			YYABORT;
		}

		if (((ConfigurationItem_p) $$) -> init ("", "",
			gbIsCaseSensitive, glBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_INIT_FAILED);
			yyerror ((const char *) err);

			delete ((ConfigurationItem_p) $$);

			YYABORT;
		}
	}
	| ITEMSLIST_BEGIN STRING valuesList ITEMSLIST_END {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: value -> ITEMSLIST_BEGIN STRING ["
					<< (char *) $2 << "] valuesList [..] ITEMSLIST_END";
			#else
				printf ("yacc: value -> ITEMSLIST_BEGIN STRING [%s] valuesList [%p] ITEMSLIST_END\n", (char *) $2, (ConfigurationItem_p) $3);
			#endif
		#endif

		$$ = (ConfigurationItem_p) $3;

		if (((ConfigurationItem_p) $$) -> insertItemValue (
			(const char *) $2, 0) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_INSERTITEMVALUE_FAILED);
			yyerror ((const char *) err);

			delete [] ((char *) $2);
			((ConfigurationItem_p) $3) -> finish ();
			delete ((ConfigurationItem_p) $3);

			YYABORT;
		}

		delete [] ((char *) $2);
	}
	;

valuesList: /* empty */ {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: valuesList ->";
			#else
				printf ("yacc: valuesList ->\n");
			#endif
		#endif

		if (($$ = new ConfigurationItem_t) == (ConfigurationItem_p) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);

			YYABORT;
		}

		if (((ConfigurationItem_p) $$) -> init ("", "",
			gbIsCaseSensitive, glBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_INIT_FAILED);
			yyerror ((const char *) err);

			delete ((ConfigurationItem_p) $$);

			YYABORT;
		}
	}
	| valuesList ',' STRING {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: valuesList -> valuesList [..] , STRING ["
					<< (char *) $3 << "]";
			#else
				printf ("yacc: valuesList -> valuesList [%p] , STRING [%s]\n",
					(ConfigurationItem_p) $1, (char *) $3);
			#endif
		#endif

		$$ = (ConfigurationItem_p) $1;

		if (((ConfigurationItem_p) $$) -> appendItemValue ((const char *) $3) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_APPENDITEMVALUE_FAILED);
			yyerror ((const char *) err);

			((ConfigurationItem_p) $1) -> finish ();
			delete ((ConfigurationItem_p) $1);
			delete [] ((char *) $3);

			YYABORT;
		}
		
		delete [] ((char *) $3);
	}
	;

sectionComments: /* empty */ {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: comments ->";
			#else
				printf ("yacc: comments ->\n");
			#endif
		#endif

		if (($$ = new char [1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);

			YYABORT;
		}
		
		strcpy ((char *) $$, "");
	}
	| sectionComments SECTIONCOMMENT {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: sectionComments -> sectionComments ["
					<< (char *) $1 << "] SECTIONCOMMENT ["
					<< (char *) $2 << "]";
			#else
				printf ("yacc: sectionComments -> sectionComments [%s] SECTIONCOMMENT [%s]\n", (char *) $1, (char *) $2);
			#endif
		#endif

		// The comments symbol must contain the \n final character
		// The + 2 means the \n\0 characters
		if (($$ = new char [strlen ((char *) $1) + strlen ((char *) $2) + 2]) ==
			(char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);

			delete [] ((char *) $1);
			delete [] ((char *) $2);

			YYABORT;
		}

		if (strlen ((char *) $1) == 0)
			sprintf ($$, "%s", (char *) $2);
		else
			sprintf ($$, "%s\n%s", (char *) $1, (char *) $2);

		delete [] ((char *) $1);
		delete [] ((char *) $2);
	}
	;

itemComments: /* empty */ {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: comments ->";
			#else
				printf ("yacc: comments ->\n");
			#endif
		#endif

		if (($$ = new char [1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);

			YYABORT;
		}
		
		strcpy ((char *) $$, "");
	}
	| itemComments ITEMCOMMENT {
		#ifdef YACC_DEBUG
			#ifdef ANDROID_DEBUG
				qDebug() << "yacc: itemComments -> itemComments ["
					<< (char *) $1 << "] ITEMCOMMENT ["
					<< (char *) $2 << "]";
			#else
				printf ("yacc: itemComments -> itemComments [%s] ITEMCOMMENT [%s]\n", (char *) $1, (char *) $2);
			#endif
		#endif

		// The comments symbol must contain the \n final character
		// The + 2 means the \n\0 characters
		if (($$ = new char [strlen ((char *) $1) + strlen ((char *) $2) + 2]) ==
			(char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			yyerror ((const char *) err);

			delete [] ((char *) $1);
			delete [] ((char *) $2);

			YYABORT;
		}

		if (strlen ((char *) $1) == 0)
			sprintf ($$, "%s", (char *) $2);
		else
			sprintf ($$, "%s\n%s", (char *) $1, (char *) $2);

		delete [] ((char *) $1);
		delete [] ((char *) $2);
	}
	;

%%


long glCurrentLineNumber = 1;
extern FILE *yyin;


void yyerror (const char *pErrorMessage)

{

	gerrParserError = ConfigurationFileErrors (__FILE__, __LINE__,
		CFGFILE_YYPARSER_FAILED, 2, glCurrentLineNumber, pErrorMessage);

/*
	fprintf (stderr, "yacc error: line %d - Message error %s\n",
		glCurrentLineNumber, pErrorMessage);
*/

}


