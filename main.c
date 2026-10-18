#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "index/index.h"
#include "index/search.h"

// Вспомогательные функции для парсинга аргументов
TreeType parseType(const char* str) {
    if (strcmp(str, "avl") == 0) return TREE_AVL;
    if (strcmp(str, "rb") == 0) return TREE_RB;
    return TREE_BTREE;
}

const char* typeName(TreeType type) {
    if (type == TREE_AVL) return "avl";
    if (type == TREE_RB) return "rb";
    return "btree";
}

// Функция индексации (читает docs_full.jsonl и строит дерево)
static void runIndex(TreeType type, const char* data_path, const char* idx_path) {
    FILE* f = fopen(data_path, "r");
    if (!f) {
        fprintf(stderr, "[Error] Failed to open %s\n", data_path);
        return;
    }

    Index* idx = createIndex(type);
    static char line[1 << 20];  // строки с длинным телом вопроса бывают > 4 КБ
    int count = 0;

    printf("Building %s index from %s...\n", typeName(type), data_path);

    while (fgets(line, sizeof(line), f)) {
        // Простейший парсинг JSONL строки: {"doc_id": "123", "title": "abc", "tokens": ["t1", "t2"]}
        char* doc_id_str = strstr(line, "\"doc_id\": \"");
        char* title_str  = strstr(line, "\"title\": \"");
        char* tokens_str = strstr(line, "\"tokens\": [");

        if (!doc_id_str || !title_str || !tokens_str) continue;

        doc_id_str += 11;
        int doc_id = atoi(doc_id_str);

        // Читаем заголовок до закрывающей кавычки, раскрывая JSON-экранирование (\" \\ \n ...)
        title_str += 10;
        char* src = title_str;
        char* dst = title_str;
        while (*src && *src != '"') {
            if (*src == '\\' && src[1]) {
                src++;
                if (*src == 'n' || *src == 't' || *src == 'r') *src = ' ';
            }
            *dst++ = *src++;
        }
        *dst = '\0';

        tokens_str += 11;
        char* tokens_end = strchr(tokens_str, ']');
        if (tokens_end) *tokens_end = '\0';

        const char* tokens[256];
        int n_tokens = 0;
        char* token = strtok(tokens_str, "\", \t\r\n");
        while (token && n_tokens < 256) {
            tokens[n_tokens++] = token;
            token = strtok(NULL, "\", \t\r\n");
        }

        indexDocument(idx, doc_id, title_str, tokens, n_tokens);
        count++;
    }

    fclose(f);
    saveIndex(idx, idx_path);
    freeIndex(idx);
    printf("Successfully indexed %d documents. Saved to %s\n", count, idx_path);
}

// Запуск поиска по готовому индексу
static void runSearch(TreeType type, const char* idx_path, const char* query, int json_out) {
    Index* idx = loadIndex(idx_path, type);
    if (!idx) { 
        fprintf(stderr, "Failed to load index: %s\n", idx_path); 
        exit(1); 
    }

    SearchResults* sr = search(idx, query);
    
    if (json_out) printResultsJSON(sr);
    else          printResultsText(sr);

    freeSearchResults(sr);
    freeIndex(idx);
}

static void usage(const char* prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s index  --type=<avl|rb|btree> [--data=PATH] [--index=PATH]\n"
        "  %s search --type=<avl|rb|btree> [--index=PATH] [--json] \"query\"\n",
        prog, prog);
}

int main(int argc, char* argv[]) {
    if (argc < 3) { usage(argv[0]); return 1; }

    const char* mode = argv[1];
    TreeType    type = TREE_AVL;
    const char* data_path = "data/processed/docs_full.jsonl";
    char        idx_path[512] = {0};
    int         json_out = 0;
    const char* query    = NULL;

    for (int i = 2; i < argc; i++) {
        if      (strncmp(argv[i], "--type=",  7) == 0) type = parseType(argv[i] + 7);
        else if (strncmp(argv[i], "--data=",  7) == 0) data_path = argv[i] + 7;
        else if (strncmp(argv[i], "--index=", 8) == 0)
            strncpy(idx_path, argv[i] + 8, sizeof(idx_path) - 1);
        else if (strcmp(argv[i], "--json")    == 0)    json_out = 1;
        else if (argv[i][0] != '-')                    query = argv[i];
    }

    if (idx_path[0] == '\0')
        snprintf(idx_path, sizeof(idx_path), "data/index_%s.txt", typeName(type));

    if (strcmp(mode, "index") == 0) {
        runIndex(type, data_path, idx_path);
    } else if (strcmp(mode, "search") == 0) {
        if (!query) { fprintf(stderr, "No query provided\n"); return 1; }
        runSearch(type, idx_path, query, json_out);
    } else {
        fprintf(stderr, "Unknown mode: %s\n", mode);
        usage(argv[0]);
        return 1;
    }
    return 0;
}