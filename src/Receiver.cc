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
int id=0;
Define_Module(Receiver);

void Receiver::initialize()
{
    // TODO - Generated method body
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
    EV << "At time=" << simTime()
       << " Receiver " << direction
       << " " << msgType
       << " [" << content << "], ID=" << seqId
       << ", modified=" << modified << "\n";
}

std::string Receiver::binaryToAscii(const std::string& bitString) {
    std::string result;
    for (size_t i = 0; i + 7 < bitString.length(); i += 8) {
        std::string byteStr = bitString.substr(i, 8);
        char c = static_cast<char>(std::bitset<8>(byteStr).to_ulong());
        result += c;
    }
    return result;
}
