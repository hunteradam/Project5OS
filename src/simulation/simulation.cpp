/**
 * This file contains implementations for the methods defined in the Simulation
 * class.
 *
 * You'll probably spend a lot of your time here.
 */

#include "simulation/simulation.h"
#include <stdexcept>
#include <vector>
#include <iterator>
int t  = 0;
int framesBegin = 0;
int framesEnd = 0;
int physicalCounter = 0;


Simulation::Simulation(FlagOptions& flags)
{
    this->flags = flags;
    this->frames.reserve(this->NUM_FRAMES);
}

void Simulation::run() {
    framesEnd = flags.max_frames;
    int lastProcessID = virtual_addresses[0].process_id;
    for (VirtualAddress address : virtual_addresses) {
        if (address.process_id != lastProcessID){
            if (frames.size() < framesEnd) {
                for (int i = frames.size()-1; i < framesEnd; i++) {
                    physicalCounter++;
                    Frame frame;
                    frame.page_number = 0;
                    frames.push_back(frame);
                }
            }
            framesBegin += flags.max_frames;
            framesEnd += flags.max_frames;
        }
        lastProcessID = address.process_id;
        perform_memory_access(address);
        t++;
        // free()
    }
    for (int i = 0; i < 512; i++){
        if(frames[i].contents == nullptr){
            free_frames.push_back(i);
        }
    }
    print_summary();
    
    
}

char Simulation::perform_memory_access(const VirtualAddress& virtual_address) {
    Process* process = processes.at(virtual_address.process_id);
    process->memory_accesses++;
    int counter = 0;
    
    for (int i = framesBegin; i < framesEnd && i < frames.size(); i++) {
        Frame frame = frames[i];
        // checking to see if process has a page in a frame
        if (frame.page_number == virtual_address.page) {
            //print in memory
            process->page_table.rows[virtual_address.page].last_accessed_at = t;
            std::cout << virtual_address << std::endl
                << "\t-> IN MEMORY" << std::endl
                << "\t-> physical address " << PhysicalAddress(counter+framesBegin-physicalCounter, virtual_address.offset) << std::endl
                << "\t-> RSS: " << process->get_rss() << std::endl << std::endl;
                return 0; 
        }
        counter ++;
    }
    // print Seg Fault
    if (!process->is_valid_page(virtual_address.page)) {
        std::cout << virtual_address << std::endl;
        std::cout << ("SEGFAULT - INVALID PAGE") << std::endl;
        exit(10);
    } 
    if (!process->pages[virtual_address.page]->is_valid_offset(virtual_address.offset)) {
        std::cout << virtual_address << std::endl
        << "\t-> PAGE FAULT" << std::endl
        << "\t-> physical address " << PhysicalAddress(counter+framesBegin-physicalCounter, virtual_address.offset) << std::endl
        << ("SEGFAULT - INVALID OFFSET") << std::endl;
        exit(10);
    }
    handle_page_fault(process, virtual_address.page);
    
    counter = 0;

    // print page fault
    for (int i = framesBegin; i < framesEnd && i < frames.size(); i++) {
        Frame frame = frames[i];
        // checking to see if process has a page in a frame
        if (frame.page_number == virtual_address.page) {
            //print in memeory
            std::cout << virtual_address << std::endl
                << "\t-> PAGE FAULT" << std::endl
                << "\t-> physical address " << PhysicalAddress(counter+framesBegin-physicalCounter, virtual_address.offset) << std::endl
                << "\t-> RSS: " << process->get_rss() << std::endl << std::endl;
        }
        counter ++;
    }
    return 0;
}

void Simulation::handle_page_fault(Process* process, size_t pageNumber) {
    
    process->page_faults++;
    page_faults++;
    if (frames.size() < framesEnd && frames.size() < 512){
        Frame frame;
        frame.set_page(process, pageNumber);
        frames.push_back(frame);
        process->page_table.rows[pageNumber].present = true;
        process->page_table.rows[pageNumber].loaded_at = t;
        process->page_table.rows[pageNumber].last_accessed_at = t;
        return;
    }
    if (flags.strategy == ReplacementStrategy::FIFO) { 
        int oldestPageNum = process->page_table.get_oldest_page();
        std::vector<Frame>::iterator it;
        it = frames.begin();
        for(int i = 0; i < framesBegin; i++){
            it++;
        }
        for (int i = framesBegin; i < framesEnd; i++) {
            Frame frame = frames[i];
            if (frame.page_number == oldestPageNum) {
                process->page_table.rows[oldestPageNum].present = false;
                frame.set_page(process, pageNumber);
                frames.erase(it);
                frames.insert(it, frame);
                process->page_table.rows[pageNumber].present = true;
                process->page_table.rows[pageNumber].loaded_at = t;
                process->page_table.rows[pageNumber].last_accessed_at = t;
                return;
            }
            it++;
        }
    }
    else if (flags.strategy == ReplacementStrategy::LRU){
        int leastUsedPageNum = process->page_table.get_least_recently_used_page();
        std::vector<Frame>::iterator it;
        it = frames.begin();
        for(int i = 0; i < framesBegin; i++){
            it++;
        }
        for (int i = framesBegin; i < framesEnd; i++) {
            Frame frame = frames[i];
            if (frame.page_number == leastUsedPageNum) {
                process->page_table.rows[leastUsedPageNum].present = false;
                frame.set_page(process, pageNumber);
                frames.erase(it);
                frames.insert(it, frame);
                process->page_table.rows[pageNumber].present = true;
                process->page_table.rows[pageNumber].loaded_at = t;
                process->page_table.rows[pageNumber].last_accessed_at = t;
                return;
            }
            it++;
        }
    }
}

void Simulation::print_summary() {
    if (!this->flags.csv) {
        boost::format process_fmt(
            "Process %3d:  "
            "ACCESSES: %-6lu "
            "FAULTS: %-6lu "
            "FAULT RATE: %-8.2f "
            "RSS: %-6lu\n");

        for (auto entry : this->processes) {
            std::cout << process_fmt
                % entry.first
                % entry.second->memory_accesses
                % entry.second->page_faults
                % entry.second->get_fault_percent()
                % entry.second->get_rss();
        }

        // Print statistics.
        boost::format summary_fmt(
            "\n%-25s %12lu\n"
            "%-25s %12lu\n"
            "%-25s %12lu\n");

        std::cout << summary_fmt
            % "Total memory accesses:"
            % this->virtual_addresses.size()
            % "Total page faults:"
            % this->page_faults
            % "Free frames remaining:"
            % this->free_frames.size();
    }

    if (this->flags.csv) {
        boost::format process_fmt(
            "%d,"
            "%lu,"
            "%lu,"
            "%.2f,"
            "%lu\n");

        for (auto entry : processes) {
            std::cout << process_fmt
                % entry.first
                % entry.second->memory_accesses
                % entry.second->page_faults
                % entry.second->get_fault_percent()
                % entry.second->get_rss();
        }

        // Print statistics.
        boost::format summary_fmt(
            "%lu,,,,\n"
            "%lu,,,,\n"
            "%lu,,,,\n");

        std::cout << summary_fmt
            % this->virtual_addresses.size()
            % this->page_faults
            % this->free_frames.size();
    }
}

int Simulation::read_processes(std::istream& simulation_file) {
    int num_processes;
    simulation_file >> num_processes;

    for (int i = 0; i < num_processes; ++i) {
        int pid;
        std::string process_image_path;

        simulation_file >> pid >> process_image_path;

        std::ifstream proc_img_file(process_image_path);

        if (!proc_img_file) {
            std::cerr << "Unable to read file for PID " << pid << ": " << process_image_path << std::endl;
            return 1;
        }
        this->processes[pid] = Process::read_from_input(proc_img_file);
    }
    return 0;
}

int Simulation::read_addresses(std::istream& simulation_file) {
    int pid;
    std::string virtual_address;

    try {
        while (simulation_file >> pid >> virtual_address) {
            this->virtual_addresses.push_back(VirtualAddress::from_string(pid, virtual_address));
        }
    } catch (const std::exception& except) {
        std::cerr << "Error reading virtual addresses." << std::endl;
        std::cerr << except.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Error reading virtual addresses." << std::endl;
        return 1;
    }
    return 0;
}

int Simulation::read_simulation_file() {
    std::ifstream simulation_file(this->flags.filename);
    // this->simulation_file.open(this->flags.filename);

    if (!simulation_file) {
        std::cerr << "Unable to open file: " << this->flags.filename << std::endl;
        return -1;
    }
    int error = 0;
    error = this->read_processes(simulation_file);

    if (error) {
        std::cerr << "Error reading processes. Exit: " << error << std::endl;
        return error;
    }

    error = this->read_addresses(simulation_file);

    if (error) {
        std::cerr << "Error reading addresses." << std::endl;
        return error;
    }

    if (this->flags.file_verbose) {
        for (auto entry: this->processes) {
            std::cout << "Process " << entry.first << ": Size: " << entry.second->size() << std::endl;
        }

        for (auto entry : this->virtual_addresses) {
            std::cout << entry << std::endl;
        }
    }

    return 0;
}
