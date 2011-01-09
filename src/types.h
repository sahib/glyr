#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#include <curl/curl.h>

//Nifty defines:
#define nextTag(X) while(*X && *X++ != '>')
#define ABS(a)  (((a) < 0) ? -(a) : (a))

#define C_B "\x1b[34;01m" // blue
#define C_M "\x1b[35;01m" // magenta
#define C_C "\x1b[36;01m" // Cyan
#define C_R "\x1b[31;01m" // Red
#define C_G "\x1b[32;01m" // Green
#define C_Y "\x1b[33;01m" // Yellow
#define C_  "\x1b[0m"     // Reset

// Internal buffer model
typedef struct memCache_t
{
	char  *data;   // data buffer 
	size_t size;   // Size of data
	char  ecode;   // error code (0)

// Note: the error code is not used at the moment
} memCache_t;

// Internal calback object, used for cover, lyrics and meta
// This is only used inside the core and the plugins
// Other parts of the program shall not use this struct
// memCache_t is what you're searching  
typedef struct cb_object
{
	// What callback to call
	memCache_t * (* parser_callback) (struct cb_object *);
	
	// What url to download before the callback is called
	char *url; 
	
	// Artist related... (pointer to parameters give to plugin_init)
	const char *artist;
	const char *album;
	const char *title; 
	
	// What max/min size a cover may have (this is only for covers) 	
	int max,min;

	//Plugin name - only important for verbose output
	const char * name; 
	
	// What curl handle this is attached
	CURL *handle;
	
	// internal cache attached to this url
	memCache_t *cache; 	

} cb_object;

#endif
