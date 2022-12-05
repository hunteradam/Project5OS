/**
 * This file contains implementations for methods in the VirtualAddress class.
 *
 * You'll need to add code here to make the corresponding tests pass.
 */

#include "virtual_address/virtual_address.h"
#include <iostream>
#include <string>
using namespace std;

VirtualAddress VirtualAddress::from_string(int process_id, string address) {
    int page = stoi(address.substr(0, 10),0,2);
    int offset = stoi(address.substr(10, 16),0,2);
    return VirtualAddress(process_id, page, offset);
}


string VirtualAddress::to_string() const {
    string address = decToBinary(this->page, 10) + decToBinary(this->offset, 6);
    return address;
}

string decToBinary(int decimal, int size){
    string result = "";
    
    for (int i = size - 1; i >= 0; i--) {
        int k = decimal >> i;
        if (k & 1)
            result += "1";
        else
            result += "0";
    }
    return result;
}

ostream& operator <<(ostream& out, const VirtualAddress& address) {
    string returnString = "";
    out << "PID " << address.process_id
    << " @ " << address.to_string()
    << " [page: " << address.page
    << "; offset: " << address.offset << "]";
    return out;
}
