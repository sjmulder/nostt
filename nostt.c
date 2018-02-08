#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <yajl/yajl_parse.h>

#define URL_BASE "http://teletekst-data.nos.nl/json/"

static CURL *curl;
static yajl_handle yajl;

static int   json_depth = 0;
static char *json_key = NULL;
static char *next_subpg = NULL;

static void cleanup()
{
	if (curl) {
		curl_easy_cleanup(curl);
		curl = NULL;
	}
}

static void ckcurl(CURLcode code)
{
	if (code != CURLE_OK) {
		fprintf(stderr, "curl error: %s\n",
			curl_easy_strerror(code));
		exit(1);
	}
}

static void ckyajl(yajl_status status)
{
	if (status != yajl_status_ok) {
		fprintf(stderr, "JSON parse error: %s\n",
			yajl_status_to_string(status));
		exit(1);
	}
}

static void ckpage(const char *s, int *is_subpg)
{
	const char *s_orig;

	s_orig = s;
	*is_subpg = 0;
	for (; *s; s++) {
		if (*s >= '0' && *s <= '9')
			;
		else if (!*is_subpg && *s == '-')
			*is_subpg = 1;
		else {
			fprintf(stderr, "invalid page: %s\n", s_orig);
			exit(1);
		}
	}
}

static void pcontent(const char *s, size_t len)
{
	int i, in_tag, in_entity;

	in_tag = in_entity = 0;
	for (i = 0; i < len; i++) {
		if (in_tag) {
			if (s[i] == '>')
				in_tag = 0;
		} else if (in_entity) {
			if (s[i] == ';') {
				in_entity = 0;
				putchar('.');
			}
		} else {
			if (s[i] == '<')
				in_tag = 1;
			else if (s[i] == '&')
				in_entity = 1;
			else
				putchar(s[i]);
		}
	}
}

static int json_mapstart(void *ctx)
{
	json_depth += 1;
	return 1;
}

static int json_key(void *ctx, const unsigned char *key, size_t len)
{
	if (json_key)
		free(json_key);

	if (!(json_key = malloc(len+1))) {
		fprintf(stderr, "out of memory\n");
		exit(EXIT_FAILURE);
	}

	strncpy(json_key, (const char*)key, len);
	json_key[len] = '\0';

	return 1;
}

static int json_str(void *ctx, const unsigned char *val, size_t len)
{
	if (json_depth != 1 || !json_key)
		;
	else if (!strcmp(json_key, "content"))
		pcontent((const char *)val, len);
	else if (!strcmp(json_key, "nextSubPage") && len) {
		if (next_subpg)
			free(next_subpg);
		if (!(next_subpg = malloc(len+1))) {
			fprintf(stderr, "out of memory\n");
			exit(EXIT_FAILURE);
		}
		strncpy(next_subpg, (const char *)val, len);
		next_subpg[len] = '\0';
	}

	return 1;
}

static int json_mapend(void *ctx)
{
	json_depth -= 1;
	return 1;
}

static size_t fwd_data(char *ptr, size_t size, size_t nmeb, void *userdata)
{
	ckyajl(yajl_parse(yajl, (unsigned char *)ptr, size * nmeb));
	return size * nmeb;
}

int main(int argc, char **argv)
{
	char *page, *url;
	int asked_subpg, tmp;
	yajl_callbacks callbacks;

	if (argc != 2) {
		fprintf(stderr, "usage: nostt <num>\n");
		return 1;
	}

	atexit(cleanup);

	if (curl_global_init(CURL_GLOBAL_DEFAULT)) {
		fprintf(stderr, "curl_global_init failed\n");
		return 1;
	}
 
	if (!(curl = curl_easy_init())) {
		fprintf(stderr, "curl_easy_init failed\n");
		return 1;
	}

	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.yajl_start_map = json_mapstart;
	callbacks.yajl_map_key   = json_key;
	callbacks.yajl_string    = json_str;
	callbacks.yajl_end_map   = json_mapend;

	ckcurl(curl_easy_setopt(curl, CURLOPT_USERAGENT, "nostt"));
	ckcurl(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwd_data));

	page = argv[1];
	ckpage(page, &asked_subpg);

	while (1) {
		if (!(yajl = yajl_alloc(&callbacks, NULL, NULL))) {
			fprintf(stderr, "yajl_alloc failed\n");
			return 1;
		}

		if (json_key)
			free(json_key);
		if (next_subpg)
			free(next_subpg);

		json_depth = 0;
		json_key = NULL;
		next_subpg = NULL;

		if (!(url = malloc(sizeof(URL_BASE) + strlen(page)))) {
			fprintf(stderr, "out of memory\n");
			return 1;
		}

		strcpy(url, URL_BASE);
		strcat(url, page);

		ckcurl(curl_easy_setopt(curl, CURLOPT_URL, url));
		ckcurl(curl_easy_perform(curl));

		ckyajl(yajl_complete_parse(yajl));

		if (asked_subpg || !(page = next_subpg))
			break;

		ckpage(page, &tmp);

		free(url);
		yajl_free(yajl);
		yajl = NULL;

		putchar('\n');		
	} while (!asked_subpg && (page = next_subpg));

	return 0;
}
