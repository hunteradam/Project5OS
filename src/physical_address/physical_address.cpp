/**
 * This file contains implementations for methods in the PhysicalAddress class.
 *
 * You'll need to add code here to make the corresponding tests pass.
 */

#include "physical_address/physical_address.h"
#include <iostream>
using namespace std;

string PhysicalAddress::to_string() const {
    return decsToBinary(frame, FRAME_BITS) + decsToBinary(offset, OFFSET_BITS);
}

string decsToBinary(int decimal, int size){
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

ostream& operator <<(ostream& out, const PhysicalAddress& address) {
    out << decsToBinary(address.frame, address.FRAME_BITS) << decsToBinary(address.offset, address.OFFSET_BITS)
    << " [frame: " << address.frame 
    << "; offset: " << address.offset
    << "]";
    return out;
}
