#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define STUDENT_ID_HEADER "54321" /* Replace with your actual Student ID */
#define PASS_THRESHOLD 50.0f
#define RISK_THRESHOLD 65.0f

/* 1. Student Record Structure */
typedef struct {
    int id;
    char name[64];
    char course[64];
    float grade;
} Student;

/* Dynamic container representing the database */
typedef struct {
    Student *records;
    size_t count;
    size_t capacity;
} StudentDatabase;

/* Function pointer type for menu operations */
typedef void (*MenuAction)(StudentDatabase *db);

/* --- Function Prototypes --- */
void display_header(void);
void clear_input_buffer(void);
bool get_valid_string(const char *prompt, char *buffer, size_t max_len);
int get_valid_int(const char *prompt);
float get_valid_float(const char *prompt, float min, float max);

/* Menu Actions dispatched via function pointers */
void action_add_student(StudentDatabase *db);
void action_display_students(StudentDatabase *db);
void action_sort_students(StudentDatabase *db);
void action_calculate_average(StudentDatabase *db);
void action_find_top_student(StudentDatabase *db);
void action_search_student(StudentDatabase *db);
void action_remove_student(StudentDatabase *db);
void action_custom_analysis(StudentDatabase *db);
void action_summary_report(StudentDatabase *db);
void action_exit_program(StudentDatabase *db);

/* Helper calculations utilizing pointer arithmetic */
float compute_average_pointer(const Student *start, size_t count);
const Student* find_extremum_pointer(const Student *start, size_t count, bool find_max);

/* --- Main Program --- */
int main(void) {
    StudentDatabase db = { .records = NULL, .count = 0, .capacity = 0 };

    /* Function Pointer Table for Menu Dispatch (1-indexed mapping) */
    MenuAction dispatch_table[] = {
        action_add_student,          /* 1 */
        action_display_students,     /* 2 */
        action_sort_students,        /* 3 */
        action_calculate_average,    /* 4 */
        action_find_top_student,     /* 5 */
        action_search_student,       /* 6 */
        action_remove_student,       /* 7 */
        action_custom_analysis,      /* 8 */
        action_summary_report,       /* 9 */
        action_exit_program          /* 10 */
    };
    const int total_actions = (int)(sizeof(dispatch_table) / sizeof(dispatch_table[0]));

    int choice = 0;
    while (choice != 10) {
        display_header();
        printf("1. Add a new student record\n");
        printf("2. Display all student records\n");
        printf("3. Sort students (Grade, Name, or ID)\n");
        printf("4. Calculate the average grade\n");
        printf("5. Identify the top-performing student\n");
        printf("6. Search for a student (ID or Name)\n");
        printf("7. Remove a student record\n");
        printf("8. Run custom academic analysis (Academic Risk Index)\n");
        printf("9. Display summary report\n");
        printf("10. Exit program\n");
        printf("----------------------------------------------------------------------\n");

        choice = get_valid_int("Enter your choice (1-10): ");
        if (choice >= 1 && choice <= total_actions) {
            printf("\n");
            /* Dispatch execution through function pointer */
            dispatch_table[choice - 1](&db);
        } else {
            printf("\n[Error] Invalid option. Please enter a value between 1 and %d.\n", total_actions);
        }
        printf("\n");
    }

    return EXIT_SUCCESS;
}

/* --- Input Validation & Utility Helpers --- */
void display_header(void) {
    printf("======================================================================\n");
    printf("Student ID: %s\n", STUDENT_ID_HEADER);
    printf("Project: Academic performance and risk analysis engine\n");
    printf("Custom Analysis: Academic risk index\n");
    printf("======================================================================\n");
}

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

bool get_valid_string(const char *prompt, char *buffer, size_t max_len) {
    printf("%s", prompt);
    if (fgets(buffer, (int)max_len, stdin) == NULL) {
        return false;
    }
    buffer[strcspn(buffer, "\r\n")] = '\0';
    if (strlen(buffer) == 0) {
        return false;
    }
    return true;
}

int get_valid_int(const char *prompt) {
    int val;
    char buffer[64];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &val) == 1) {
                return val;
            }
        }
        printf("  [!] Invalid integer. Try again.\n");
    }
}

float get_valid_float(const char *prompt, float min, float max) {
    float val;
    char buffer[64];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%f", &val) == 1) {
                if (val >= min && val <= max) {
                    return val;
                }
            }
        }
        printf("  [!] Value must be between %.2f and %.2f. Try again.\n", min, max);
    }
}

/* --- Pointer-Based Calculations --- */
float compute_average_pointer(const Student *start, size_t count) {
    if (count == 0 || start == NULL) return 0.0f;

    float sum = 0.0f;
    const Student *ptr = start;
    const Student *end = start + count;

    /* Pointer-based traversal: increment pointer rather than index */
    for (; ptr < end; ++ptr) {
        sum += ptr->grade;
    }
    return sum / (float)count;
}

const Student* find_extremum_pointer(const Student *start, size_t count, bool find_max) {
    if (count == 0 || start == NULL) return NULL;

    const Student *best = start;
    const Student *curr = start + 1;
    const Student *end = start + count;

    for (; curr < end; ++curr) {
        if (find_max) {
            if (curr->grade > best->grade) best = curr;
        } else {
            if (curr->grade < best->grade) best = curr;
        }
    }
    return best;
}

/* --- Menu Actions --- */

/* 1. Add Record (Expands memory via realloc) */
void action_add_student(StudentDatabase *db) {
    printf("--- Add New Student Record ---\n");
    int new_id = get_valid_int("Enter Student ID: ");

    /* Check for duplicate ID using pointer traversal */
    if (db->count > 0 && db->records != NULL) {
        const Student *curr = db->records;
        const Student *end = db->records + db->count;
        for (; curr < end; ++curr) {
            if (curr->id == new_id) {
                printf("[Rejected] A student record with ID %d already exists.\n", new_id);
                return;
            }
        }
    }

    char name_buf[64];
    while (!get_valid_string("Enter Student Name: ", name_buf, sizeof(name_buf))) {
        printf("  [!] Name cannot be empty.\n");
    }

    char course_buf[64];
    while (!get_valid_string("Enter Course Name: ", course_buf, sizeof(course_buf))) {
        printf("  [!] Course cannot be empty.\n");
    }

    float grade = get_valid_float("Enter Grade (0 - 100): ", 0.0f, 100.0f);

    /* Expand array dynamically */
    size_t new_cap = db->count + 1;
    Student *temp = (Student *)realloc(db->records, new_cap * sizeof(Student));
    if (temp == NULL) {
        printf("[Error] Reallocation failed. Cannot add student.\n");
        return;
    }

    db->records = temp;
    db->capacity = new_cap;

    /* Write new record using pointer offset */
    Student *target = db->records + db->count;
    target->id = new_id;
    strncpy(target->name, name_buf, sizeof(target->name) - 1);
    target->name[sizeof(target->name) - 1] = '\0';
    strncpy(target->course, course_buf, sizeof(target->course) - 1);
    target->course[sizeof(target->course) - 1] = '\0';
    target->grade = grade;

    db->count++;
    printf("[Success] Record for '%s' (ID: %d) added. Dynamic capacity is now %zu.\n",
           target->name, target->id, db->capacity);
}

/* 2. Display Records (Uses pointer traversal) */
void action_display_students(StudentDatabase *db) {
    if (db->count == 0) {
        printf("[Info] No student records available.\n");
        return;
    }

    printf("----------------------------------------------------------------------\n");
    printf("%-8s | %-22s | %-22s | %-6s\n", "ID", "Name", "Course", "Grade");
    printf("----------------------------------------------------------------------\n");

    const Student *ptr = db->records;
    const Student *end = db->records + db->count;
    for (; ptr < end; ++ptr) {
        printf("%-8d | %-22s | %-22s | %6.2f\n",
               ptr->id, ptr->name, ptr->course, ptr->grade);
    }
    printf("----------------------------------------------------------------------\n");
    printf("Total records: %zu\n", db->count);
}

/* 3. Sort Students */
static int cmp_grade_desc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    if (sb->grade > sa->grade) return 1;
    if (sb->grade < sa->grade) return -1;
    return 0;
}

static int cmp_name_asc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    return strcmp(sa->name, sb->name);
}

static int cmp_id_asc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    return (sa->id - sb->id);
}

void action_sort_students(StudentDatabase *db) {
    if (db->count < 2) {
        printf("[Info] Not enough records to sort.\n");
        return;
    }

    printf("Sort by:\n");
    printf("  1. Grade (Descending)\n");
    printf("  2. Name (Alphabetical)\n");
    printf("  3. Student ID (Ascending)\n");
    int opt = get_valid_int("Choice (1-3): ");

    switch (opt) {
        case 1:
            qsort(db->records, db->count, sizeof(Student), cmp_grade_desc);
            printf("[Success] Sorted by grade descending.\n");
            break;
        case 2:
            qsort(db->records, db->count, sizeof(Student), cmp_name_asc);
            printf("[Success] Sorted alphabetically by name.\n");
            break;
        case 3:
            qsort(db->records, db->count, sizeof(Student), cmp_id_asc);
            printf("[Success] Sorted by Student ID.\n");
            break;
        default:
            printf("[Error] Invalid sort option selected.\n");
            return;
    }
    action_display_students(db);
}

/* 4. Calculate Average */
void action_calculate_average(StudentDatabase *db) {
    if (db->count == 0) {
        printf("[Info] Database is empty.\n");
        return;
    }
    float avg = compute_average_pointer(db->records, db->count);
    printf("Class Average Grade: %.2f%%\n", avg);
}

/* 5. Identify Top Student */
void action_find_top_student(StudentDatabase *db) {
    if (db->count == 0) {
        printf("[Info] Database is empty.\n");
        return;
    }
    const Student *top = find_extremum_pointer(db->records, db->count, true);
    printf("Top Performing Student:\n");
    printf("  ID:     %d\n", top->id);
    printf("  Name:   %s\n", top->name);
    printf("  Course: %s\n", top->course);
    printf("  Grade:  %.2f\n", top->grade);
}

/* 6. Search for Student */
void action_search_student(StudentDatabase *db) {
    if (db->count == 0) {
        printf("[Info] Database is empty.\n");
        return;
    }

    printf("Search by:\n  1. ID\n  2. Name\n");
    int mode = get_valid_int("Choice (1-2): ");

    if (mode == 1) {
        int target_id = get_valid_int("Enter Student ID to find: ");
        const Student *curr = db->records;
        const Student *end = db->records + db->count;

        for (; curr < end; ++curr) {
            if (curr->id == target_id) {
                printf("\n[Found] ID: %d | Name: %s | Course: %s | Grade: %.2f\n",
                       curr->id, curr->name, curr->course, curr->grade);
                return;
            }
        }
        printf("[Search] No record found with ID %d.\n", target_id);
    } else if (mode == 2) {
        char query[64];
        if (!get_valid_string("Enter student name: ", query, sizeof(query))) {
            printf("[Error] Search query cannot be empty.\n");
            return;
        }

        bool found = false;
        const Student *curr = db->records;
        const Student *end = db->records + db->count;

        printf("\nMatching records:\n");
        for (; curr < end; ++curr) {
            if (strcmp(curr->name, query) == 0) {
                printf("  -> ID: %d | Name: %s | Course: %s | Grade: %.2f\n",
                       curr->id, curr->name, curr->course, curr->grade);
                found = true;
            }
        }
        if (!found) {
            printf("  None matching '%s' found.\n", query);
        }
    } else {
        printf("[Error] Invalid search option.\n");
    }
}

/* 7. Remove Record (Shrinks memory via realloc) */
void action_remove_student(StudentDatabase *db) {
    if (db->count == 0) {
        printf("[Info] No records available to remove.\n");
        return;
    }

    int target_id = get_valid_int("Enter Student ID to remove: ");
    size_t found_idx = db->count;

    for (size_t i = 0; i < db->count; ++i) {
        if ((db->records + i)->id == target_id) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == db->count) {
        printf("[Error] Student with ID %d was not found.\n", target_id);
        return;
    }

    /* Shift subsequent records backward using memmove */
    size_t elements_to_move = db->count - found_idx - 1;
    if (elements_to_move > 0) {
        memmove(db->records + found_idx,
                db->records + found_idx + 1,
                elements_to_move * sizeof(Student));
    }
    db->count--;

    /* Shrink the allocated heap buffer */
    if (db->count > 0) {
        Student *temp = (Student *)realloc(db->records, db->count * sizeof(Student));
        if (temp != NULL) {
            db->records = temp;
            db->capacity = db->count;
        }
    } else {
        free(db->records);
        db->records = NULL;
        db->capacity = 0;
    }

    printf("[Success] Record removed. Dynamic capacity shrunk to %zu.\n", db->capacity);
}

/* 8. Custom Analysis: Academic Risk Index (ARI) */
void action_custom_analysis(StudentDatabase *db) {
    if (db->count == 0) {
        printf("[Info] No data available for analysis.\n");
        return;
    }

    printf("================ Academic Risk Index (ARI) Analysis ================\n");
    printf("Algorithm: ARI = max(0.0, (%.1f - Grade) * 1.5)\n", RISK_THRESHOLD);
    printf("Status Tiers: Critical (ARI >= 30) | At-Risk (0 < ARI < 30) | Safe (0.0)\n");
    printf("--------------------------------------------------------------------\n");
    printf("%-8s | %-20s | %-6s | %-8s | %-12s\n", "ID", "Name", "Grade", "ARI Score", "Status");
    printf("--------------------------------------------------------------------\n");

    size_t critical_count = 0;
    size_t at_risk_count = 0;
    size_t safe_count = 0;

    const Student *curr = db->records;
    const Student *end = db->records + db->count;

    for (; curr < end; ++curr) {
        float deficit = RISK_THRESHOLD - curr->grade;
        float ari = (deficit > 0.0f) ? (deficit * 1.5f) : 0.0f;

        const char *status;
        if (ari >= 30.0f) {
            status = "CRITICAL";
            critical_count++;
        } else if (ari > 0.0f) {
            status = "AT-RISK";
            at_risk_count++;
        } else {
            status = "SAFE";
            safe_count++;
        }

        printf("%-8d | %-20s | %6.2f | %9.2f | %-12s\n",
               curr->id, curr->name, curr->grade, ari, status);
    }
    printf("--------------------------------------------------------------------\n");
    printf("Cohort Risk Summary: Critical: %zu | At-Risk: %zu | Safe: %zu\n",
           critical_count, at_risk_count, safe_count);
}

/* 9. Summary Report */
void action_summary_report(StudentDatabase *db) {
    if (db->count == 0) {
        printf("[Info] No records available for summary.\n");
        return;
    }

    float avg = compute_average_pointer(db->records, db->count);
    const Student *top = find_extremum_pointer(db->records, db->count, true);
    const Student *lowest = find_extremum_pointer(db->records, db->count, false);

    size_t meeting_pass = 0;
    size_t below_pass = 0;
    size_t high_risk = 0;

    const Student *curr = db->records;
    const Student *end = db->records + db->count;

    for (; curr < end; ++curr) {
        if (curr->grade >= PASS_THRESHOLD) {
            meeting_pass++;
        } else {
            below_pass++;
        }

        float deficit = RISK_THRESHOLD - curr->grade;
        if (deficit > 0.0f && (deficit * 1.5f) >= 30.0f) {
            high_risk++;
        }
    }

    printf("======================= Cohort Summary Report =======================\n");
    printf("Total Student Records:        %zu\n", db->count);
    printf("Class Average Grade:          %.2f%%\n", avg);
    printf("Highest Grade:                %.2f%% (%s, ID: %d)\n", top->grade, top->name, top->id);
    printf("Lowest Grade:                 %.2f%% (%s, ID: %d)\n", lowest->grade, lowest->name, lowest->id);
    printf("Students Passing (>= %.1f%%):   %zu (%.1f%%)\n",
           PASS_THRESHOLD, meeting_pass, (float)meeting_pass / db->count * 100.0f);
    printf("Students Below Pass (< %.1f%%): %zu (%.1f%%)\n",
           PASS_THRESHOLD, below_pass, (float)below_pass / db->count * 100.0f);
    printf("Custom Analysis Metric:       %zu critical-tier risk cases requiring immediate intervention\n", high_risk);
    printf("=====================================================================\n");
}

/* 10. Exit & Cleanup */
void action_exit_program(StudentDatabase *db) {
    printf("[System] Releasing heap-allocated memory...\n");
    if (db->records != NULL) {
        free(db->records);
        db->records = NULL;
    }
    db->count = 0;
    db->capacity = 0;
    printf("[System] Cleanup complete. Exiting.\n");
}
