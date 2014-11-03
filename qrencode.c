
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <qrencode.h>
#include <png.h>


#if LUA_VERSION_NUM < 502
#  define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
#endif

struct memData {
	char *buffer;
	size_t size;
};

enum imageType {
    PNG_TYPE,
    ANSI_TYPE
};

#define INCHES_PER_METER (100.0/2.54)

static int size;
static int margin;
static int dpi;
static unsigned int fg_color[4];
static unsigned int bg_color[4];


/* png io callback function */
static void cp_png_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	struct memData* p = (struct memData*)png_get_io_ptr(png_ptr);

	p->buffer = realloc(p->buffer, p->size + length);
	if (!p->buffer)
		png_error(png_ptr, "Write Error");

	memcpy(p->buffer + p->size, data, length);
	p->size += length;
}

/* 0:failure/1:true to_png( qrcode: qr data , result: buffer to save result.) {{{ */
static int to_png(QRcode *qrcode, struct memData *result)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette;
	png_byte alpha_values[2];

	unsigned char *row, *p, *q;
	int x, y, xx, yy, bit;
	int realwidth;

	realwidth = (qrcode->width + margin * 2) * size;
	row = (unsigned char *)malloc((realwidth + 7) / 8);
	if (row == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return 0;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Failed to initialize PNG writer.\n");
		return 0;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Failed to initialize PNG write.\n");
		return 0;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fprintf(stderr, "Failed to write PNG image.\n");
		return 0;
	}

	png_set_write_fn(png_ptr, result, cp_png_data, NULL);

	palette = (png_colorp) malloc(sizeof(png_color) * 2);
	if (palette == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return 0;
	}
	palette[0].red   = fg_color[0];
	palette[0].green = fg_color[1];
	palette[0].blue  = fg_color[2];
	palette[1].red   = bg_color[0];
	palette[1].green = bg_color[1];
	palette[1].blue  = bg_color[2];
	alpha_values[0] = fg_color[3];
	alpha_values[1] = bg_color[3];
	png_set_PLTE(png_ptr, info_ptr, palette, 2);
	png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);

	png_set_IHDR(png_ptr, info_ptr,
			realwidth, realwidth,
			1,
			PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
	png_set_pHYs(png_ptr, info_ptr,
			dpi * INCHES_PER_METER,
			dpi * INCHES_PER_METER,
			PNG_RESOLUTION_METER);
	png_write_info(png_ptr, info_ptr);

	/* top margin */
	memset(row, 0xff, (realwidth + 7) / 8);
	for(y=0; y<margin * size; y++) {
		png_write_row(png_ptr, row);
	}

	/* data */
	p = qrcode->data;
	for(y=0; y<qrcode->width; y++) {
		bit = 7;
		memset(row, 0xff, (realwidth + 7) / 8);
		q = row;
		q += margin * size / 8;
		bit = 7 - (margin * size % 8);
		for(x=0; x<qrcode->width; x++) {
			for(xx=0; xx<size; xx++) {
				*q ^= (*p & 1) << bit;
				bit--;
				if (bit < 0) {
					q++;
					bit = 7;
				}
			}
			p++;
		}
		for(yy=0; yy<size; yy++) {
			png_write_row(png_ptr, row);
		}
	}
	/* bottom margin */
	memset(row, 0xff, (realwidth + 7) / 8);
	for(y=0; y<margin * size; y++) {
		png_write_row(png_ptr, row);
	}

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(row);
	free(palette);

	return 1;
}
/* }}} */

/* buffer is in, and result is out */
static void mputs(char *buffer, struct memData *result)
{
	size_t length = strlen(buffer);
	result->buffer = realloc(result->buffer, result->size + length);
	if (!result->buffer) {
		fprintf(stderr, "Write Error.\n");
		exit(EXIT_FAILURE);
	}
	memcpy(result->buffer + result->size, buffer, length);
	result->size += length;
}

/* to_ansi {{{ */
static int to_ansi(QRcode *qrcode, struct memData *result)
{
	unsigned char *row, *p;
	int x, y;
	int realwidth;
	int last;

	char *white, *black, *buffer;
	int white_s, black_s, buffer_s;

	white = "\033[47m";
	white_s = 5;
	black = "\033[40m";
	black_s = 5;

	size = 1;

	realwidth = (qrcode->width + margin * 2) * size;
	buffer_s = ( realwidth * white_s ) * 2;
	buffer = (char *)malloc( buffer_s );
	if(buffer == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	/* top margin */
	strncpy(buffer, white, white_s);
	memset(buffer + white_s, ' ', realwidth * 2);
	strcpy(buffer + white_s + realwidth * 2, "\033[0m\n"); // reset to default colors
	for(y=0; y<margin; y++ ){
		mputs(buffer, result);
	}

	/* data */
	p = qrcode->data;
	for(y=0; y<qrcode->width; y++) {
		row = (p+(y*qrcode->width));

		bzero( buffer, buffer_s );
		strncpy( buffer, white, white_s );
		for(x=0; x<margin; x++ ){
			strncat( buffer, "  ", 2 );
		}
		last = 0;

		for(x=0; x<qrcode->width; x++) {
			if(*(row+x)&0x1) {
				if( last != 1 ){
					strncat( buffer, black, black_s );
					last = 1;
				}
			} else {
				if( last != 0 ){
					strncat( buffer, white, white_s );
					last = 0;
				}
			}
			strncat( buffer, "  ", 2 );
		}

		if( last != 0 ){
			strncat( buffer, white, white_s );
		}
		for(x=0; x<margin; x++ ){
			strncat( buffer, "  ", 2 );
		}
		strncat( buffer, "\033[0m\n", 5 );
		mputs( buffer, result );
	}

	/* bottom margin */
	strncpy(buffer, white, white_s);
	memset(buffer + white_s, ' ', realwidth * 2);
	strcpy(buffer + white_s + realwidth * 2, "\033[0m\n"); // reset to default colors
	for(y=0; y<margin; y++ ){
		mputs(buffer, result);
	}

	free(buffer);

	return 0;
}
/* }}} */

static void set_error_info(lua_State *L, const char *info)
{
	lua_pushstring(L, info);
	lua_pushstring(L, " is not invalid color");
	lua_concat(L, 2);
	lua_error(L);
}

static void set_png_color(lua_State *L, unsigned int color[4], const char * field)
{
    size_t len, count;
	const char * value;

	lua_pushstring(L, field);
	lua_gettable(L, 1);
	if (!lua_isnil(L, 2)) {
		value = lua_tolstring(L, 2, &len);
	    if(len == 6) {
			count = sscanf(value, "%02x%02x%02x%n", &color[0], &color[1], &color[2], (int *)&len);
			if(count < 3 || len != 6) {
				set_error_info(L, field);
			}
			color[3] = 255;
		} else if(len == 8) {
			count = sscanf(value, "%02x%02x%02x%02x%n", &color[0], &color[1], &color[2], &color[3], (int *)&len);
			if(count < 4 || len != 8) {
				set_error_info(L, field);
			}
		} else {
			set_error_info(L, field);
		}
	}	
	lua_pop(L, 1);
}

static void set_qr_boolean (lua_State *L, int *set_to, const char * field)
{
	lua_pushstring(L, field);
	lua_gettable(L, 1);
	if (!lua_isnil(L, 2)) {
		if (!lua_isboolean(L, 2)) {
			lua_pushstring(L, field);
			lua_pushstring(L, " is not \"true\" or \"false\"");
			lua_concat(L, 2);
			lua_error(L);
			return;
		}
		*set_to = lua_toboolean(L, 2);
	}	
	lua_pop(L, 1);
}

static void set_qr_int (lua_State *L, int *set_to, const char * field)
{
	lua_pushstring(L, field);
	lua_gettable(L, 1);
	if (!lua_isnil(L, 2)) {
		*set_to = luaL_checkint(L, 2);
	}	
	lua_pop(L, 1);
}

static int encode (lua_State *L)
{
	QRcode *qrcode;
	const char *intext;
	struct memData result;

	int version = 0;
	int casesensitive = 1;
	QRencodeMode hint = QR_MODE_8;
	QRecLevel level = QR_ECLEVEL_L;
	enum imageType image_type = PNG_TYPE;
	size = 3;
	margin = 4;
	dpi = 72;
	fg_color[0] = 0;
	fg_color[1] = 0;
	fg_color[2] = 0;
	fg_color[3] = 255;
	bg_color[0] = 255;
	bg_color[1] = 255;
	bg_color[2] = 255;
	bg_color[3] = 255;

	result.buffer = NULL;
	result.size = 0;

	if ((lua_gettop(L) == 1 && lua_istable(L, 1)) ||
		(lua_gettop(L) == 2 && lua_istable(L, 2)) ) {

		if (lua_gettop(L) == 2)
			lua_remove(L, 1);
		/* text field must exist and type of this is string */
		lua_pushstring(L, "text");
		lua_gettable(L, 1);
		intext = luaL_checkstring(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "level");
		lua_gettable(L, 1);
		if (!lua_isnil(L, 2)) {
			switch(*luaL_checkstring(L, -1)) {
				case 'l':
				case 'L':
					level = QR_ECLEVEL_L;
					break;
				case 'm':
				case 'M':
					level = QR_ECLEVEL_M;
					break;
				case 'q':
				case 'Q':
					level = QR_ECLEVEL_Q;
					break;
				case 'h':
				case 'H':
					level = QR_ECLEVEL_H;
					break;
				default:
					lua_pushstring(L, "Invalid qr level, must be \"L/M/Q/H\"");
					lua_error(L);
					break;
			}
		}
		lua_pop(L, 1);

		lua_pushstring(L, "kanji");
		lua_gettable(L, 1);
		if (!lua_isnil(L, 2)) {
			if (!lua_isboolean(L, 2)) {
				lua_pushstring(L, "kanji is not \"true\" or \"false\"");
				lua_error(L);
			}
			if(lua_toboolean(L, 2))
				hint = QR_MODE_KANJI;
		}
		lua_pop(L, 1);

		lua_pushstring(L, "ansi");
		lua_gettable(L, 1);
		if (!lua_isnil(L, 2)) {
			if (!lua_isboolean(L, 2)) {
				lua_pushstring(L, "ansi is not \"true\" or \"false\"");
				lua_error(L);
			}
			if(lua_toboolean(L, 2))
				image_type = ANSI_TYPE;
		}
		lua_pop(L, 1);

		set_qr_int(L, &size, "size");
		set_qr_int(L, &version, "symversion");
		set_qr_int(L, &margin, "margin");
		set_qr_int(L, &dpi, "dpi");
		set_qr_boolean(L,&casesensitive, "casesensitive");
		set_png_color(L, fg_color, "foreground");
		set_png_color(L, bg_color, "background");
	}
	else if (lua_gettop(L) == 1) 
			intext = luaL_checkstring(L, 1);
	else if (lua_gettop(L) == 2 && lua_istable(L, 1))
			intext = luaL_checkstring(L, 2);
	else {
			lua_pushstring(L, "argument is invalid.");
			lua_error(L);
	}

	if (strlen(intext) <= 0) {
		lua_pushstring(L, "The input data is invalid");
		lua_error(L);
	}

	qrcode = QRcode_encodeString(intext, version, level, hint, casesensitive);
	if (qrcode == NULL) {
		lua_pushnil(L);
		lua_pushstring(L, "Failed to encode the input data");
		return 2;
	}

	if ( image_type == PNG_TYPE) {
		if (!to_png(qrcode, &result)) {
			lua_pushnil(L);
			lua_pushstring(L, "Failed to encode the input data to png");
			return 2;
		}
	} else if ( image_type == ANSI_TYPE) {
		to_ansi(qrcode, &result);
	} else {
		lua_pushstring(L, "image type is invalid.");
		lua_error(L);
	}

	QRcode_free(qrcode);
	lua_pushlstring(L, result.buffer, result.size);
	free(result.buffer);
	return 1;
}

static const struct luaL_Reg R [] = {
	{"__call", encode},
	{"encode", encode},
	{NULL, NULL}
};

int luaopen_qrencode (lua_State *L)
{
	luaL_newlib(L, R);
	lua_pushvalue(L, -1);
	lua_setmetatable(L, -2);
	return 1;
}

/* vi: set noet fdm=marker : */
