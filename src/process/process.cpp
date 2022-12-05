/**
 * This file contains implementations for methods in the Process class.
 *
 * You'll need to add code here to make the corresponding tests pass.
 */

#include "process/process.h"
#include <iostream>
using namespace std;


Process* Process::read_from_input(std::istream& in) {
    std::vector<Page*> pages;
    while (!in.eof()) {
        Page *page = Page::read_from_input(in);
        pages.push_back(page);
    }

    int numBytes = 0;
    for (Page *page : pages){
        numBytes += page->size();
    }
    return new Process(numBytes, pages);
}


size_t Process::size() const
{
    return this->num_bytes;
}


bool Process::is_valid_page(size_t index) const
{
    if (pages.size() > index) {
        return true;
    }
    return false;
}


size_t Process::get_rss() const
{
    return pages.size()/8;
}


double Process::get_fault_percent() const
{
    if (memory_accesses == 0) {
        return 0.0;
    }
    return 100.0 * page_faults/memory_accesses;
}
