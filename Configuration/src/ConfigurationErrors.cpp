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


#include "ConfigurationErrors.h"


ErrMsgBase:: ErrMsgsInfo ConfigurationErrorsStr = {

	// ConfigurationItem
	{ CFG_CONFIGURATIONITEM_INIT_FAILED,
		"The init method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_INSERTITEMVALUE_FAILED,
		"The insertItemValue method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_APPENDITEMVALUE_FAILED,
		"The appendItemValue method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_GETISCASESENSITIVE_FAILED,
		"The getIsCaseSensitive method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_GETITEMNAME_FAILED,
		"The getItemName method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_GETITEMNAMELENGTH_FAILED,
		"The getItemNameLength method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_MODIFYITEMNAME_FAILED,
		"The modifyItemName method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_GETITEMCOMMENT_FAILED,
		"The getItemComment method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_GETITEMCOMMENTLENGTH_FAILED,
		"The getItemCommentLength method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_GETITEMVALUESNUMBER_FAILED,
		"The getItemValuesNumber method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_GETITEMVALUE_FAILED,
		"The getItemValue method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_GETITEMVALUELENGTH_FAILED,
		"The getItemValueLength method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_MODIFYITEMVALUE_FAILED,
		"The modifyItemValue method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_MODIFYITEMCOMMENT_FAILED,
		"The modifyItemComment method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_FINISH_FAILED,
		"The finish method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_WRITE_FAILED,
		"The write method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_COPY_FAILED,
		"The copy method of the ConfigurationItem class failed" },
	{ CFG_CONFIGURATIONITEM_BUFFERTOOSHORT,
		"The buffer is too short (length: %lu)" },
	{ CFG_CONFIGURATIONITEM_WRONGCFGVALUE,
		"Wrong cfg item value. Section: %s, Item: %s, Value: %s. Possible values are: %s" },

	// ConfigurationSection
	{ CFG_CONFIGURATIONSECTION_INIT_FAILED,
		"The init method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_APPENDCFGITEM_FAILED,
		"The appendCfgItem method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_GETISCASESENSITIVE_FAILED,
		"The getIsCaseSensitive method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_GETSECTIONNAME_FAILED,
		"The getSectionName method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_GETSECTIONNAMELENGTH_FAILED,
		"The getSectionNameLength method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_MODIFYSECTIONNAME_FAILED,
		"The modifySectionName method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_GETSECTIONCOMMENT_FAILED,
		"The getSectionComment method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_GETSECTIONCOMMENTLENGTH_FAILED,
		"The getSectionCommentLength method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_MODIFYSECTIONCOMMENT_FAILED,
		"The modifySectionComment method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_GETSECTIONDATE_FAILED,
		"The getSectionDate method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_MODIFYSECTIONDATE_FAILED,
		"The modifySectionDate method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_GETCFGITEMSNUMBER_FAILED,
		"The getCfgItemsNumber method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_GETCFGITEMBYNAME_FAILED,
		"The getCfgItemByName method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_GETPTRCFGITEMBYNAME_FAILED,
		"The getPtrCfgItemByName method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_GETCFGITEMBYINDEX_FAILED,
		"The getCfgItemByIndex method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_FINISH_FAILED,
		"The finish method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_WRITE_FAILED,
		"The write method of the ConfigurationSection class failed" },
	{ CFG_CONFIGURATIONSECTION_COPY_FAILED,
		"The copy method of the ConfigurationSection class failed" },
	{ CFG_ITEM_NOTFOUND,
		"The \"%s\" item was not found" },
	{ CFG_CONFIGURATIONSECTIONDATE_NOTFOUND,
		"The \"%s\" section date was not found" },

	// Config
	{ CFG_CONFIG_INIT_FAILED,
		"The init method of the Config class failed" },
	{ CFG_CONFIG_REMOVECFGSECTION_FAILED,
		"The removeCfgSection method of the Config class failed" },
	{ CFG_CONFIG_APPENDCFGSECTION_FAILED,
		"The appendCfgSection method of the Config class failed" },
	{ CFG_CONFIG_APPENDITEMVALUE_FAILED,
		"The appendItemValue method of the Config class failed" },
	{ CFG_CONFIG_MODIFYITEMCOMMENT_FAILED,
		"The modifyItemComment method of the Config class failed" },
	{ CFG_CONFIG_MODIFYSECTIONCOMMENT_FAILED,
		"The modifySectionComment method of the Config class failed" },
	{ CFG_CONFIG_GETISCASESENSITIVE_FAILED,
		"The getIsCaseSensitive method of the Config class failed" },
	{ CFG_CONFIG_GETCONFIGNAME_FAILED,
		"The getConfigName method of the Config class failed" },
	{ CFG_CONFIG_MODIFYCONFIGNAME_FAILED,
		"The modifyConfigName method of the Config class failed" },
	{ CFG_CONFIG_GETSECTIONSNUMBER_FAILED,
		"The getSectionsNumber method of the Config class failed" },
	{ CFG_CONFIG_GETCFGSECTIONBYINDEX_FAILED,
		"The getCfgSectionByIndex method of the Config class failed" },
	{ CFG_CONFIG_GETCFGSECTIONBYNAME_FAILED,
		"The getCfgSectionByName method of the Config class failed" },
	{ CFG_CONFIG_GETCFGITEMBYINDEX_FAILED,
		"The getCfgItemByIndex method of the Config class failed" },
	{ CFG_CONFIG_GETCFGITEMBYNAME_FAILED,
		"The getCfgItemByName method of the Config class failed" },
	{ CFG_CONFIG_GETITEMVALUESNUMBER_FAILED,
		"The getItemValuesNumber method of the Config class failed (section: %s, item: %s)" },
	{ CFG_CONFIG_GETITEMVALUE_FAILED,
		"The getItemValue method of the Config class failed (Section: %s, item: %s)" },
	{ CFG_CONFIG_MODIFYITEMVALUE_FAILED,
		"The modifyItemValue method of the Config class failed. SectionName: %s, ItemName: %s, ItemValue: %s" },
	{ CFG_CONFIG_FINISH_FAILED,
		"The finish method of the Config class failed" },
	{ CFG_CONFIG_WRITE_FAILED,
		"The write method of the Config class failed" },
	{ CFG_CONFIG_COPY_FAILED,
		"The copy method of the Config class failed" },
	{ CFG_CONFIG_GETITEMSNUMBER_FAILED,
		"The getItemsNumber method of the Config class failed" },
	{ CFG_SECTION_NOTFOUND,
		"The \"%s\" section was not found" },

	// common
	{ CFG_NEW_FAILED,
		"new failed" },
	{ CFG_NOTIMPLEMENTED_FAILED,
		"Not implemented" },
	{ CFG_ACTIVATION_WRONG,
		"Activation wrong" },
	{ CFG_OPERATIONNOTALLOWED,
		"The operation is not allowed. Status: %ld" },
	{ CFG_ENCRYPT_FAILED,
		"The encrypt function failed" },
	{ CFG_DECRYPT_FAILED,
		"The decrypt function failed" }

	// Insert here other errors...

} ;

