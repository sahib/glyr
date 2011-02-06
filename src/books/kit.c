#include <stdlib.h>
#include <string.h>

#include "kit.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

#define RESULT "<td class=\"kvk-result-txt\">"
#define RESULT_LINK_START "href=\""
#define RESULT_LINK_END "\">"
#define RESULT_START ">"
#define RESULT_END "</a>"

const char * books_kit_url(glyr_settings_t * settings)
{
	return "http://kvk.ubka.uni-karlsruhe.de/hylib-bin/kvk/nph-kvk2.cgi?"
		"maske=kvk-last&"
		"lang=de&"
		"css=none&"
		"input-charset=utf-8&"
		"ALL=%artist%&"
		"Timeout=120&"
		"kataloge=SWB&"
		"kataloge=BVB&"
		"kataloge=NRW&"
		"kataloge=HEBIS&"
		"kataloge=HEBIS_RETRO&"
		"kataloge=KOBV_SOLR&"
		"kataloge=GBV&"
		"kataloge=DDB&"
		"kataloge=STABI_BERLIN&"
		"kataloge=TIB&"
		"kataloge=OEVK_GBV&"
		"kataloge=VD16&"
		"kataloge=VD17&"
		"kataloge=ZDB";
}


memCache_t * books_kit_parse(cb_object * capo)
{
	char * field = NULL;
	char * pointer = capo->cache->data;

	while ((pointer = strstr(pointer, RESULT)) != NULL)
	{
		pointer+=strlen(RESULT);

		field = (field=getStr(&pointer, RESULT_LINK_START, RESULT_LINK_END)) == NULL ? "" : field;
		//printf("%s", field);

		field = (field= getStr(&pointer, RESULT_START, RESULT_END)) == NULL ? "" : field;
		printf("%s\n", field);
	}

	return NULL;
}
