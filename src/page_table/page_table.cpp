/**
 * This file contains implementations for methods in the PageTable class.
 *
 * You'll need to add code here to make the corresponding tests pass.
 */

#include "page_table/page_table.h"

using namespace std;


size_t PageTable::get_present_page_count() const {
    int total = 0;
    for (Row row : rows) {
        if (row.present)
            total += 1;
    }
    return total;
}


size_t PageTable::get_oldest_page() const {
    int time;
    int max = 0;
    int i = 0;
    bool pres = false;
    for (Row row : rows) {
        if (row.loaded_at < time && row.present) {
            time = row.loaded_at;
            max = i;
        }
        i++;
    }
    return max;
}


size_t PageTable::get_least_recently_used_page() const {
    int time;
    int max = 0;
    int i = 0;
    bool pres = false;
    for (Row row : rows) {
        if (row.last_accessed_at < time && row.present) {
            time = row.last_accessed_at;
            max = i;
        }
        i++;
    }
    return max;
}
