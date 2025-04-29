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

#ifndef __PROJECT_SENDER_H_
#define __PROJECT_SENDER_H_

#include <omnetpp.h>
#include "MyMessage_m.h"
#include <fstream>
#include <vector>
#include <bitset>
using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Sender : public cSimpleModule
{
  private:
    std::vector<MyMessage *> messageVector;
    double st, to, td, tt, pt;
    cMessage *startEvent;
    cMessage *timeoutEvent;
    simtime_t sessionStartTime;
    simtime_t sessionFinishTime;
    int currentSeq = 0;
    int CurrentMessageIndex=0;
    MyMessage *currentMessage = nullptr;
    int totalpacketssent=0;
    bool waitforAck =false;
    int correctMessages=0;
    std::string unmodifiedpayload;

  protected:

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void loadMessageFromFile(std::string filename);
    void SendNextMessage();
    void finishSession();
    void SimulateErrors(MyMessage *msg);
    std::string BoolToString(bool b);
    std ::string stuffPayload(std::string payload);
    void sendFrame(MyMessage*msg,int duplicateorder);
    char calculateParityByte(std::string stuffedPayload);
    void modifybit(std::string &paylaodToSend);
};

#endif
