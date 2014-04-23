/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of music related metadata.
* + Copyright (C) [2011]  [Christopher Pahl]
* + Hosted at: https://github.com/sahib/glyr
*
* glyr is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* glyr is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with glyr. If not, see <http://www.gnu.org/licenses/>.
**************************************************************/

#include "../../apikeys.h"
#include "../../core.h"
#include "../../stringlib.h"
#include "amazon.h"

/////////////////////////////////

#define ACCESS_KEY API_KEY_AMAZON
#define rg_markup "__RESPONSE_GROUP__"
const gchar * generic_amazon_url (GlyrQuery * sets, const gchar * response_group)
{
    const char * lang_link = NULL;
    if (sets->img_min_size <= 500 || sets->img_min_size)
    {
        if (!strcmp (sets->lang,"us") )
            lang_link = "http://free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=${artist}+${album}\0";
        else if (!strcmp (sets->lang,"ca") )
            lang_link = "http://ca.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=${artist}+${album}\0";
        else if (!strcmp (sets->lang,"uk") )
            lang_link = "http://co.uk.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=${artist}+${album}\0";
        else if (!strcmp (sets->lang,"fr") )
            lang_link = "http://fr.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=${artist}+${album}\0";
        else if (!strcmp (sets->lang,"de") )
            lang_link = "http://de.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=${artist}+${album}\0";
        else if (!strcmp (sets->lang,"jp") )
            lang_link = "http://co.jp.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=${artist}+${album}\0";
        else
            lang_link = "http://free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=${artist}+${album}\0";
    }
    if (lang_link != NULL)
    {
        return strreplace (lang_link,rg_markup,response_group);
    }
    return NULL;
}

/////////////////////////////////
