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

#ifndef __PROJECT_RECEIVER_H_
#define __PROJECT_RECEIVER_H_

#include <omnetpp.h>

#include "MyMessage_m.h"

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Receiver : public cSimpleModule
{
  protected:
    virtual void initialize() override;
    std::string unstuffPayload(const std::string& stuffed);
    bool hasSingleBitError(const std::string& stuffedPayload, const std::string& parityByteStr);
    void logAction(const std::string& direction, const std::string& msgType, const std::string& content, int seqId, int modified);
    virtual void handleMessage(cMessage *msg) override;
    std::string binaryToAscii(const std::string& bitString);
};

#endif
