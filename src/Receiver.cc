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
    bool haseBitError=hasSingleBitError(stuffedPayload, trailer);
    if(haseBitError)
    {
        EV<<"Detected a single bit error"<<endl;

    }
    bool errorDetected =haseBitError || msg->getId()!=id;
    if (errorDetected) {
        std::string unstuffedPayload = unstuffPayload(stuffedPayload);
        //result = binaryToAscii(unstuffedPayload);
        logAction("Received","message",unstuffedPayload,msg->getId(),1);
        MyMessage *nack = new MyMessage();
        nack->setM_Type(2);
        nack->setId(id);
//        if(haseBitError)
//        {
//            nack->setError_Type(0);
//        }
//        else
//        {
//            nack->setError_Type(1);
//        }

        //send(nack,"port$o");
        sendDelayed(nack, 5, "port$o");
        logAction("Sent","NACK",unstuffedPayload,id,0);
        }
    else {
        std::string unstuffedPayload = unstuffPayload(stuffedPayload);
        //result = binaryToAscii(unstuffedPayload);
        result =unstuffedPayload;
        logAction("Received","message",result,id,0);
        MyMessage *ack = new MyMessage();
        ack->setM_Type(1);
        ack->setId(id);
        sendDelayed(ack, 5, "port$o");
        Rec_Messages.push_back(result);
        logAction("Sent","ACK",unstuffedPayload,id,0);
        id = 1-id;
        }

}



std::string Receiver::unstuffPayload(const std::string& stuffed) {
    std::string unstuffed;
    for (size_t i = 1; i < stuffed.length()-1; ++i) {
        if (stuffed[i] == '/') {
            i++; // Skip the escape character
            if (i < stuffed.length()) {
                unstuffed += stuffed[i]; // Add the escaped character
            }
        } else {
            unstuffed += stuffed[i];
        }
    }
    //unstuffed -='$';
    return unstuffed;
}

bool Receiver::hasSingleBitError(const std::string& stuffedPayload, const std::string& parityByteStr) {

    //EV<<parityByteStr<<endl;
    if (parityByteStr.empty()){
        EV<<"Parity Byte is Empty"<<endl;
        return true; // Can't check without trailer
    }

    if (parityByteStr.length() != 1)
    {
        EV<<"Parity bit Length is longer than 1"<<endl;
    }

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
    std::ofstream outFile("../src/output3.txt", std::ios::app); // append mode
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
