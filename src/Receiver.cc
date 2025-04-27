//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Receiver.h"
#include "MyMessage_m.h"
#include <bitset>
#include <cstdio>
#include <string>
#include <fstream>
int id=0;
Define_Module(Receiver);

void Receiver::initialize()
{
    if (std::remove("receiver_log.txt") == 0) {
            EV << "receiver_log.txt deleted successfully at simulation start.\n";
        } else {
            EV << "receiver_log.txt does not exist or couldn't be deleted.\n";
        }
}

void Receiver::handleMessage(cMessage *msg1)
{
    MyMessage *msg = check_and_cast<MyMessage *>(msg1);
    std::string stuffedPayload = msg->getM_Payload();
    std::string trailer = msg->getM_Trailer();
    std::string result = "";
    bool errorDetected = hasSingleBitError(stuffedPayload, trailer) || msg->getId()!=id;
    if (errorDetected) {
        std::string unstuffedPayload = unstuffPayload(stuffedPayload);
        result = binaryToAscii(unstuffedPayload);
        logAction("Received","message",result,id,1);
        MyMessage *nack = new MyMessage();
        nack->setM_Type(2);
        nack->setId(id);
        send(nack,"port$o");
        logAction("Sent","NACK","",id,0);
        }
    else {
        std::string unstuffedPayload = unstuffPayload(stuffedPayload);
        result = binaryToAscii(unstuffedPayload);
        logAction("Received","message",result,id,0);
        MyMessage *nack = new MyMessage();
        nack->setM_Type(1);
        nack->setId(id);
        logAction("Sent","ACK","",id++,0);
        }

}




std::string Receiver::unstuffPayload(const std::string& stuffed) {
    std::string unstuffed;
    for (size_t i = 0; i < stuffed.length(); ++i) {
        if (stuffed[i] == '/') {
            i++; // Skip the escape character
            if (i < stuffed.length()) {
                unstuffed += stuffed[i]; // Add the escaped character
            }
        } else {
            unstuffed += stuffed[i];
        }
    }
    return unstuffed;
}

bool Receiver::hasSingleBitError(const std::string& stuffedPayload, const std::string& parityByteStr) {
    if (parityByteStr.empty()) return true; // Can't check without trailer

    // Convert trailer to char
    unsigned char parityByte = parityByteStr[0];

    // Count 1s in stuffed payload
    int oneCount = 0;
    for (char ch : stuffedPayload) {
        oneCount += __builtin_popcount((unsigned char)ch);
    }

    // Add parity byte bits
    oneCount += __builtin_popcount(parityByte);

    // Should be even if no error
    return (oneCount % 2 != 0); // true if there's an error
}

void Receiver::logAction(const std::string& direction, const std::string& msgType,
                         const std::string& content, int seqId, int modified) {
    // Log to OMNeT++ event log
    EV << "At time=" << simTime()
       << " Receiver " << direction
       << " " << msgType
       << " [" << content << "], ID=" << seqId
       << ", modified=" << modified << "\n";

    // Append to external text file
    std::ofstream outFile("receiver_log.txt", std::ios::app); // append mode
    if (outFile.is_open()) {
        outFile << "At time=" << simTime()
                << " Receiver " << direction
                << " " << msgType
                << " [" << content << "], ID=" << seqId
                << ", modified=" << modified << "\n";
        outFile.close();
    } else {
        EV << "Failed to open receiver_log.txt for writing.\n";
    }
}

std::string Receiver::binaryToAscii(const std::string& bitString) {
    std::string asciiStr = "";

    // Process the bit string in chunks of 8 bits
    for (size_t i = 0; i < bitString.length(); i += 8) {
        // Extract the next 8 bits (1 byte)
        std::string byteStr = bitString.substr(i, 8);

        // Convert the 8-bit string to a char
        char byte = 0;
        for (size_t j = 0; j < 8; ++j) {
            if (byteStr[j] == '1') {
                byte |= (1 << (7 - j)); // Set the corresponding bit in the byte
            }
        }

        // Add the character to the resulting ASCII string
        asciiStr += byte;
    }

    return asciiStr;
}
