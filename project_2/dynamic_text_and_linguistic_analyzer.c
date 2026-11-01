#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define STUDENT_ID "001"
#define SOURCE_TITLE "Structure and Interpretation of Computer Programs (SICP)"
#define SOURCE_AUTHORS "Harold Abelson, Gerald Jay Sussman, Julie Sussman (1996)"
#define WORD_BUF_SIZE 40

/* 2. Required Data Structure */
struct wordStat {
    char name[WORD_BUF_SIZE];
    int count;
};

/* Text mining engine dynamic state */
typedef struct {
    char *raw_text;
    size_t text_length;
    size_t text_capacity;

    struct wordStat *stats;
    size_t distinct_words;
    size_t stats_capacity;

    size_t total_words;
} TextEngine;

/* Function pointer type for menu operations */
typedef void (*EngineOperation)(TextEngine *engine);

/* --- Prototypes --- */
void display_header(void);
void clear_input_buffer(void);
int get_valid_int(const char *prompt, int min, int max);
void free_engine_memory(TextEngine *engine);
bool dynamic_text_read(TextEngine *engine);
void tokenize_and_build_stats(TextEngine *engine);

/* Core required operations dispatched via function pointers */
void op_count_words(TextEngine *engine);
void op_longest_word(TextEngine *engine);
void op_most_frequent(TextEngine *engine);
void op_display_word_statistics(TextEngine *engine);
void op_sort_word_statistics(TextEngine *engine);
void op_search_word(TextEngine *engine);
void op_generate_text_summary(TextEngine *engine);
void op_custom_analysis(TextEngine *engine);
void op_memory_inspection(TextEngine *engine);
void op_load_new_text(TextEngine *engine);
void op_exit_program(TextEngine *engine);

/* This is selected paragraph from SICP Chapter 1 (Baseline 150+ word paragraph from SICP Chapter 1) */
static const char *DEFAULT_CORPUS =
    "A computational process is indeed much like a sorcerer's idea of a spirit. "
    "It cannot be seen or touched. It does not contain matter at all. However, "
    "it is very real. It can perform intellectual work. It can answer questions. "
    "It can affect the world by disbursing money at a bank or by controlling a "
    "robot arm in a factory. The programs we use to conjure processes are like "
    "a sorcerer's spells. They are carefully composed from symbolic expressions "
    "in arcane and esoteric programming languages that prescribe the tasks we "
    "want our processes to perform. A computational process, in a correctly working "
    "computer, executes programs precisely and accurately. Thus, like the sorcerer's "
    "apprentice, novice programmers must learn to understand and to anticipate the "
    "consequences of their words. Well-conceived systems, like well-executed spells, "
    "are built step-by-step out of general and powerful abstractions.";

int main(void) {
    TextEngine engine = {
        .raw_text = NULL,
        .text_length = 0,
        .text_capacity = 0,
        .stats = NULL,
        .distinct_words = 0,
        .stats_capacity = 0,
        .total_words = 0
    };

    /* Allocate initial text buffer and load default corpus */
    size_t corpus_len = strlen(DEFAULT_CORPUS);
    engine.text_capacity = corpus_len + 1;
    engine.raw_text = (char *)malloc(engine.text_capacity * sizeof(char));
    if (engine.raw_text == NULL) {
        fprintf(stderr, "FATAL: Failed to allocate heap memory for text buffer.\n");
        return EXIT_FAILURE;
    }
    strcpy(engine.raw_text, DEFAULT_CORPUS);
    engine.text_length = corpus_len;

    /* Parse corpus and build word statistics */
    tokenize_and_build_stats(&engine);

    /* Function Pointer Dispatch Table */
    EngineOperation dispatch_table[] = {
        op_count_words,             /* 1 */
        op_longest_word,            /* 2 */
        op_most_frequent,           /* 3 */
        op_display_word_statistics, /* 4 */
        op_sort_word_statistics,    /* 5 */
        op_search_word,             /* 6 */
        op_generate_text_summary,   /* 7 */
        op_custom_analysis,         /* 8 */
        op_memory_inspection,       /* 9 */
        op_load_new_text,           /* 10 */
        op_exit_program             /* 11 */
    };
    const int total_actions = (int)(sizeof(dispatch_table) / sizeof(dispatch_table[0]));

    int choice = 0;
    while (choice != 11) {
        display_header();
        printf("1. Count total words\n");
        printf("2. Identify longest word\n");
        printf("3. Identify most frequent word\n");
        printf("4. Display distinct word frequencies\n");
        printf("5. Sort word statistics (Alphabetical or Frequency)\n");
        printf("6. Search for a word\n");
        printf("7. Generate text summary report\n");
        printf("8. Custom analysis: Lexical Diversity & Vocabulary Richness\n");
        printf("9. Inspect dynamic memory addresses\n");
        printf("10. Enter/Paste new text for analysis\n");
        printf("11. Exit\n");
        printf("----------------------------------------------------------------------\n");

        choice = get_valid_int("Select an operation (1-11): ", 1, total_actions);
        printf("\n");
        dispatch_table[choice - 1](&engine);
        printf("\n");
    }

    return EXIT_SUCCESS;
}

/* --- Header & Input Helpers --- */
void display_header(void) {
    printf("======================================================================\n");
    printf("Student ID: %s\n", STUDENT_ID);
    printf("Project: Dynamic Text Mining and Linguistic Pattern Analyzer\n");
    printf("Source: %s\n", SOURCE_TITLE);
    printf("Authors: %s\n", SOURCE_AUTHORS);
    printf("Custom Analysis: Lexical Diversity Score (Type-Token Ratio)\n");
    printf("======================================================================\n");
}

int get_valid_int(const char *prompt, int min, int max) {
    int val;
    char buffer[64];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &val) == 1 && val >= min && val <= max) {
                return val;
            }
        }
        printf("  [!] Please enter an integer between %d and %d.\n", min, max);
    }
}

void free_engine_memory(TextEngine *engine) {
    if (engine->raw_text != NULL) {
        free(engine->raw_text);
        engine->raw_text = NULL;
    }
    if (engine->stats != NULL) {
        free(engine->stats);
        engine->stats = NULL;
    }
    engine->text_length = 0;
    engine->text_capacity = 0;
    engine->distinct_words = 0;
    engine->stats_capacity = 0;
    engine->total_words = 0;
}

/* --- Dynamic Input & Tokenization (Pointer-based) --- */
bool dynamic_text_read(TextEngine *engine) {
    printf("Enter or paste your text (press ENTER, then Ctrl+D on Linux to finish):\n\n");

    size_t capacity = 512;
    size_t length = 0;
    char *buffer = (char *)malloc(capacity * sizeof(char));
    if (buffer == NULL) return false;

    int c;
    while ((c = getchar()) != EOF) {
        if (length + 2 >= capacity) {
            size_t new_cap = capacity * 2;
            char *temp = (char *)realloc(buffer, new_cap);
            if (temp == NULL) {
                free(buffer);
                return false;
            }
            buffer = temp;
            capacity = new_cap;
        }
        *(buffer + length) = (char)c;
        length++;
    }
    clearerr(stdin); /* Clear EOF indicator for future reads */

    *(buffer + length) = '\0';

    if (length == 0) {
        free(buffer);
        return false;
    }

    if (engine->raw_text != NULL) {
        free(engine->raw_text);
    }
    engine->raw_text = buffer;
    engine->text_length = length;
    engine->text_capacity = capacity;
    return true;
}

/* Traverses text using pointers, handles boundaries, normalization, and bounds */
void tokenize_and_build_stats(TextEngine *engine) {
    if (engine->stats != NULL) {
        free(engine->stats);
        engine->stats = NULL;
    }
    engine->distinct_words = 0;
    engine->stats_capacity = 16;
    engine->total_words = 0;

    engine->stats = (struct wordStat *)malloc(engine->stats_capacity * sizeof(struct wordStat));
    if (engine->stats == NULL) {
        fprintf(stderr, "FATAL: Word statistics allocation failed.\n");
        return;
    }

    const char *ptr = engine->raw_text;
    char token[WORD_BUF_SIZE];

    while (*ptr != '\0') {
        /* Skip leading non-alphanumeric delimiters */
        while (*ptr != '\0' && !isalnum((unsigned char)*ptr)) {
            ptr++;
        }
        if (*ptr == '\0') break;

        /* Extract token characters using pointer arithmetic */
        size_t token_len = 0;
        while (*ptr != '\0' && (isalnum((unsigned char)*ptr) || *ptr == '\'' || *ptr == '-')) {
            /* Treat apostrophes and hyphens inside words as valid, skip if trailing */
            if (*ptr == '\'' || *ptr == '-') {
                char next_char = *(ptr + 1);
                if (!isalnum((unsigned char)next_char)) {
                    break;
                }
            }

            if (token_len < WORD_BUF_SIZE - 1) {
                /* Normalize to lowercase */
                *(token + token_len) = (char)tolower((unsigned char)*ptr);
                token_len++;
            }
            /* Truncation rule: characters beyond 39 are discarded safely */
            ptr++;
        }
        *(token + token_len) = '\0';

        if (token_len == 0) continue;

        engine->total_words++;

        /* Search for existing word in statistics using pointer traversal and strcmp */
        struct wordStat *curr = engine->stats;
        struct wordStat *end = engine->stats + engine->distinct_words;
        bool found = false;

        for (; curr < end; ++curr) {
            if (strcmp(curr->name, token) == 0) {
                curr->count++;
                found = true;
                break;
            }
        }

        /* If distinct, append with dynamic capacity growth via realloc */
        if (!found) {
            if (engine->distinct_words >= engine->stats_capacity) {
                size_t new_cap = engine->stats_capacity * 2;
                struct wordStat *temp = (struct wordStat *)realloc(engine->stats,
                                        new_cap * sizeof(struct wordStat));
                if (temp == NULL) {
                    fprintf(stderr, "WARNING: Word table dynamic growth failed.\n");
                    return;
                }
                engine->stats = temp;
                engine->stats_capacity = new_cap;
            }

            struct wordStat *slot = engine->stats + engine->distinct_words;
            strcpy(slot->name, token);
            slot->count = 1;
            engine->distinct_words++;
        }
    }
}

/* --- Operation Implementations (Function Pointer Targets) --- */

/* 1. count_words() */
void op_count_words(TextEngine *engine) {
    printf("[Word Count Result]\n");
    printf("  Total words extracted:    %zu\n", engine->total_words);
    printf("  Distinct words cataloged: %zu\n", engine->distinct_words);
}

/* 2. longest_word() */
void op_longest_word(TextEngine *engine) {
    if (engine->distinct_words == 0) {
        printf("[Info] No words to analyze.\n");
        return;
    }

    const struct wordStat *longest = engine->stats;
    const struct wordStat *curr = engine->stats + 1;
    const struct wordStat *end = engine->stats + engine->distinct_words;
    size_t max_len = strlen(longest->name);

    for (; curr < end; ++curr) {
        size_t curr_len = strlen(curr->name);
        if (curr_len > max_len) {
            max_len = curr_len;
            longest = curr;
        }
    }

    printf("[Longest Word Result]\n");
    printf("  Word:   '%s'\n", longest->name);
    printf("  Length: %zu characters\n", max_len);
    printf("  Count:  %d occurrences\n", longest->count);
}

/* 3. most_frequent() */
void op_most_frequent(TextEngine *engine) {
    if (engine->distinct_words == 0) {
        printf("[Info] No words to analyze.\n");
        return;
    }

    const struct wordStat *frequent = engine->stats;
    const struct wordStat *curr = engine->stats + 1;
    const struct wordStat *end = engine->stats + engine->distinct_words;

    for (; curr < end; ++curr) {
        if (curr->count > frequent->count) {
            frequent = curr;
        }
    }

    printf("[Most Frequent Word Result]\n");
    printf("  Word:      '%s'\n", frequent->name);
    printf("  Frequency: %d occurrences (%.2f%% of total text)\n",
           frequent->count,
           ((float)frequent->count / (float)engine->total_words) * 100.0f);
}

/* 4. display_word_statistics() */
void op_display_word_statistics(TextEngine *engine) {
    if (engine->distinct_words == 0) {
        printf("[Info] Vocabulary array is empty.\n");
        return;
    }

    printf("----------------------------------------------------\n");
    printf("%-5s | %-30s | %-8s\n", "Index", "Word", "Count");
    printf("----------------------------------------------------\n");

    const struct wordStat *ptr = engine->stats;
    const struct wordStat *end = engine->stats + engine->distinct_words;
    size_t idx = 1;

    for (; ptr < end; ++ptr, ++idx) {
        printf("%-5zu | %-30s | %-8d\n", idx, ptr->name, ptr->count);
    }
    printf("----------------------------------------------------\n");
    printf("Total distinct entries: %zu\n", engine->distinct_words);
}

/* 5. sort_word_statistics() */
static int cmp_alpha_asc(const void *a, const void *b) {
    const struct wordStat *w1 = (const struct wordStat *)a;
    const struct wordStat *w2 = (const struct wordStat *)b;
    return strcmp(w1->name, w2->name);
}

static int cmp_freq_desc(const void *a, const void *b) {
    const struct wordStat *w1 = (const struct wordStat *)a;
    const struct wordStat *w2 = (const struct wordStat *)b;
    if (w2->count != w1->count) {
        return w2->count - w1->count;
    }
    return strcmp(w1->name, w2->name);
}

void op_sort_word_statistics(TextEngine *engine) {
    if (engine->distinct_words < 2) {
        printf("[Info] Insufficient records to sort.\n");
        return;
    }

    printf("Sort Ordering:\n");
    printf("  1. Alphabetically (A-Z)\n");
    printf("  2. By Frequency (Descending)\n");
    int mode = get_valid_int("Choice (1-2): ", 1, 2);

    if (mode == 1) {
        qsort(engine->stats, engine->distinct_words, sizeof(struct wordStat), cmp_alpha_asc);
        printf("[Success] Word statistics sorted alphabetically.\n");
    } else {
        qsort(engine->stats, engine->distinct_words, sizeof(struct wordStat), cmp_freq_desc);
        printf("[Success] Word statistics sorted by descending frequency.\n");
    }
    op_display_word_statistics(engine);
}

/* 6. search_word() */
void op_search_word(TextEngine *engine) {
    if (engine->distinct_words == 0) {
        printf("[Info] Vocabulary array is empty.\n");
        return;
    }

    char target[WORD_BUF_SIZE];
    printf("Enter word to search: ");
    if (fgets(target, sizeof(target), stdin) == NULL) return;
    target[strcspn(target, "\r\n")] = '\0';

    /* Normalize target input to lowercase */
    char *p = target;
    while (*p != '\0') {
        *p = (char)tolower((unsigned char)*p);
        p++;
    }

    const struct wordStat *curr = engine->stats;
    const struct wordStat *end = engine->stats + engine->distinct_words;
    bool found = false;

    for (; curr < end; ++curr) {
        if (strcmp(curr->name, target) == 0) {
            printf("\n[Found] '%s' occurs %d time(s) (Relative Frequency: %.2f%%)\n",
                   curr->name, curr->count,
                   ((float)curr->count / (float)engine->total_words) * 100.0f);
            found = true;
            break;
        }
    }

    if (!found) {
        printf("\n[Search] The word '%s' does not appear in the text.\n", target);
    }
}

/* 7. generate_text_summary() */
void op_generate_text_summary(TextEngine *engine) {
    if (engine->total_words == 0) {
        printf("[Info] No text loaded.\n");
        return;
    }

    /* Compute average word length using pointer traversal */
    size_t total_chars = 0;
    const struct wordStat *curr = engine->stats;
    const struct wordStat *end = engine->stats + engine->distinct_words;
    const struct wordStat *longest = engine->stats;
    const struct wordStat *most_freq = engine->stats;

    for (; curr < end; ++curr) {
        size_t len = strlen(curr->name);
        total_chars += (len * (size_t)curr->count);

        if (len > strlen(longest->name)) {
            longest = curr;
        }
        if (curr->count > most_freq->count) {
            most_freq = curr;
        }
    }

    float avg_len = (float)total_chars / (float)engine->total_words;
    float ttr = ((float)engine->distinct_words / (float)engine->total_words) * 100.0f;

    printf("======================== Overall Text Summary ========================\n");
    printf("Total Words:                  %zu\n", engine->total_words);
    printf("Distinct Words:               %zu\n", engine->distinct_words);
    printf("Average Word Length:          %.2f characters\n", avg_len);
    printf("Longest Word:                 '%s' (%zu characters)\n", longest->name, strlen(longest->name));
    printf("Most Frequent Word:           '%s' (%d occurrences)\n", most_freq->name, most_freq->count);
    printf("Lexical Diversity Score:      %.2f%% (Type-Token Ratio)\n", ttr);
    printf("======================================================================\n");
}

/* 8. Custom Analysis: Lexical Diversity Score (TTR) and Repetition Index */
void op_custom_analysis(TextEngine *engine) {
    if (engine->total_words == 0) {
        printf("[Info] No text loaded for analysis.\n");
        return;
    }

    float ttr = ((float)engine->distinct_words / (float)engine->total_words) * 100.0f;

    /* Count repeated words (count > 1) */
    size_t repeated_tokens = 0;
    size_t singletons = 0;
    const struct wordStat *curr = engine->stats;
    const struct wordStat *end = engine->stats + engine->distinct_words;

    for (; curr < end; ++curr) {
        if (curr->count > 1) {
            repeated_tokens++;
        } else {
            singletons++;
        }
    }

    float repetition_ratio = ((float)repeated_tokens / (float)engine->distinct_words) * 100.0f;

    printf("================ Custom Linguistic Analysis: Vocabulary Richness ================\n");
    printf("Formula 1: Type-Token Ratio (TTR) = (Distinct Words / Total Words) * 100\n");
    printf("Formula 2: Lexical Repetition Ratio = (Words with count > 1 / Distinct Words) * 100\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("Total Tokens (Words):          %zu\n", engine->total_words);
    printf("Total Types (Distinct):        %zu\n", engine->distinct_words);
    printf("Lexical Diversity Score (TTR): %.2f%%\n", ttr);
    printf("Unique Single-use Words:       %zu\n", singletons);
    printf("Repeated Word Types:           %zu\n", repeated_tokens);
    printf("Vocabulary Repetition Rate:    %.2f%%\n", repetition_ratio);
    printf("--------------------------------------------------------------------------------\n");
    printf("Interpretation:\n");
    if (ttr >= 60.0f) {
        printf("  High lexical diversity: Text uses varied vocabulary with minimal repetition.\n");
    } else if (ttr >= 40.0f) {
        printf("  Moderate lexical diversity: Balanced expository prose with standard repetitions.\n");
    } else {
        printf("  Low lexical diversity: Highly repetitive or restricted vocabulary.\n");
    }
    printf("================================================================================\n");
}

/* 6. Memory Address Inspection */
void op_memory_inspection(TextEngine *engine) {
    printf("================ Dynamic Memory Address Inspection ================\n");
    printf("Text Buffer Address:             %p\n", (void *)engine->raw_text);
    printf("  -> Represents the start of the dynamically allocated char buffer on the heap.\n");
    printf("  -> Allocated Capacity: %zu bytes | Current Length: %zu bytes\n\n",
           engine->text_capacity, engine->text_length);

    printf("Word Statistics Array Address:   %p\n", (void *)engine->stats);
    printf("  -> Represents the contiguous heap memory block for struct wordStat elements.\n");
    printf("  -> Allocated Capacity: %zu items | Populated Items: %zu\n\n",
           engine->stats_capacity, engine->distinct_words);

    if (engine->distinct_words > 0) {
        printf("First Entry (&stats[0]):         %p ('%s', count: %d)\n",
               (void *)(engine->stats), engine->stats->name, engine->stats->count);

        if (engine->distinct_words > 1) {
            struct wordStat *last_entry = engine->stats + (engine->distinct_words - 1);
            printf("Last Entry  (&stats[%zu]):       %p ('%s', count: %d)\n",
                   engine->distinct_words - 1, (void *)last_entry,
                   last_entry->name, last_entry->count);
        }
        printf("  -> Shows element address separation matching sizeof(struct wordStat) = %zu bytes.\n",
               sizeof(struct wordStat));
    }
    printf("===================================================================\n");
}

/* 10. Load New Text */
void op_load_new_text(TextEngine *engine) {
    if (dynamic_text_read(engine)) {
        tokenize_and_build_stats(engine);
        printf("[Success] New text parsed successfully: %zu total words, %zu distinct.\n",
               engine->total_words, engine->distinct_words);
    } else {
        printf("[Error] Failed to read or parse input text.\n");
    }
}

/* 11. Exit */
void op_exit_program(TextEngine *engine) {
    printf("[System] Releasing heap memory for text buffer and word statistics...\n");
    free_engine_memory(engine);
    printf("[System] Memory freed cleanly. Program terminated.\n");
}