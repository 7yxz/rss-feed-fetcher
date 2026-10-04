#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

struct Memory { char *data; size_t size; };

static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;
    char *new_ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!new_ptr) return 0;
    mem->data = new_ptr;
    memcpy(&(mem->data[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    return realsize;
}

void parse_and_print(const char *xml_data, size_t size) {
    xmlDocPtr doc = xmlReadMemory(xml_data, size, NULL, NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOWARNING);
    if (!doc) return;

    xmlNodePtr root = xmlDocGetRootElement(doc);
    for (xmlNodePtr cur = root; cur; cur = cur->next) {
        for (xmlNodePtr n = cur->children; n; n = n->next) {
            xmlNodePtr target = n;
            if (xmlStrcmp(n->name, (const xmlChar*)"channel") == 0) target = n->children;

            while (target) {
                if (xmlStrcmp(target->name, (const xmlChar*)"item") == 0 ||
                    xmlStrcmp(target->name, (const xmlChar*)"entry") == 0) {
                    
                    char *title = NULL, *link = NULL, *date = NULL;

                    for (xmlNodePtr sub = target->children; sub; sub = sub->next) {
                        if (!xmlStrcmp(sub->name, (const xmlChar*)"title")) {
                            title = (char*)xmlNodeGetContent(sub);
                        } else if (!xmlStrcmp(sub->name, (const xmlChar*)"link")) {
                            link = (char*)xmlGetProp(sub, (const xmlChar*)"href");
                            if (!link) link = (char*)xmlNodeGetContent(sub);
                        } else if (!xmlStrcmp(sub->name, (const xmlChar*)"pubDate") ||
                                   !xmlStrcmp(sub->name, (const xmlChar*)"updated")) {
                            date = (char*)xmlNodeGetContent(sub);
                        }
                    }

                    printf("Title: %s\nLink:  %s\nDate:  %s\n", 
                           title ? title : "N/A", link ? link : "N/A", date ? date : "N/A");
                    printf("--------------------------------------------------\n");

                    if (title) xmlFree(title);
                    if (link) xmlFree(link);
                    if (date) xmlFree(date);
                }
                target = target->next;
            }
        }
    }
    xmlFreeDoc(doc);
}

int main(int argc, char *argv[]) {
    const char *url = (argc > 1) ? argv[1] : "https://news.ycombinator.com/rss";

    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();

    struct Memory chunk = { .data = malloc(1), .size = 0 };
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    if (curl_easy_perform(curl) == CURLE_OK) {
        parse_and_print(chunk.data, chunk.size);
    }

    free(chunk.data);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return 0;
}
