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

#include "Sender.h"


Define_Module(Sender);

void Sender::initialize()
{
    // TODO - Generated method body
    st = par("st");
    to = par("to");
    td = par("td");
    tt = par("tt");
    pt = par("pt");

    loadMessageFromFile("../src/input0.txt");

    outputFile.open("../src/output0.txt", std::ios::app);
    if(outputFile.is_open())
    {
        EV<<"Opened output file!!!"<<endl;
    }
    startEvent = new cMessage("startEvent");
    timeoutEvent = new cMessage("timeoutEvent");

    scheduleAt(st, startEvent);
    //CurrentMessageIndex=17;
}

void Sender::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    if (msg == startEvent)
        {
            sessionStartTime = simTime();
            EV <<"At time="<<simTime()<< " The Session Started at time: " << sessionStartTime.str() << "s" << endl;
            outputFile <<"At time="<<simTime()<< " The Session Started at time: " << sessionStartTime.str() << "s" << endl;
            SendNextMessage();
        }
        else if (msg == timeoutEvent)
        {
            EV <<"At time= "<<simTime()<<"Sender Timed out, Resending Message ID = " << currentMessage->getId() << endl;
            outputFile <<"At time= "<<simTime()<<"Sender Timed out, Resending Message ID = " << currentMessage->getId() << endl;
            MyMessage *retransmit = currentMessage->dup();
            retransmit->setM_Payload(unmodifiedpayload.c_str());
            correctMessages++;
            sendDelayed(retransmit, tt+pt, "port$o");
            totalpacketssent++;
            scheduleAt(simTime() + to+pt, timeoutEvent);
        }
        else
        {
            MyMessage *ack = check_and_cast<MyMessage *>(msg);

            if (ack->getM_Type() == 1)  // ACK
            {
                if (ack->getId() == currentMessage->getId())
                {
                    totalpacketssent++;
                    EV << "At time = " << simTime()
                       << " Sender received ACK for message ID = " << ack->getId() << endl;

                    outputFile << "At time = " << simTime()
                                           << " Sender received ACK for message ID = " << ack->getId() << endl;

                    lastAckedSeq = ack->getId();
                    currentSeq = 1 - currentSeq;
                    cancelEvent(timeoutEvent);
                    waitforAck = false;
                    correctMessages++;

                    SendNextMessage();
                }
                else
                {
                    EV << "Received duplicate or unexpected ACK with ID = " << ack->getId()
                       << ", ignoring.\n";

                    outputFile << "Received duplicate or unexpected ACK with ID = " << ack->getId()
                                           << ", ignoring.\n";
                }
            }
            else if (ack->getM_Type() == 2)  // NACK
            {
                if (ack->getError_Type() == 0)
                {


                    EV <<"At time= "<<simTime()<<" Received NACK for current ID = " << ack->getId()
                       << ", Starts Preparing Message["<<unmodifiedpayload<<"] ID = " << currentMessage->getId() << endl;

                    outputFile <<"At time= "<<simTime()<<" Received NACK for current ID = " << ack->getId()
                                           << ", Starts Preparing Message["<<unmodifiedpayload<<"] ID = " << currentMessage->getId() << endl;

                    correctMessages++;
                    MyMessage *retransmit = currentMessage->dup();
                    retransmit->setM_Payload(unmodifiedpayload.c_str());
                    cancelEvent(timeoutEvent);
                    EV<<"At time ="<<simTime()+pt<<" Sender Sends message ["<<currentMessage->getM_Payload()<<"] ID= "<<currentMessage->getId()<<",modified= "<<0<<", duplicated="<<0<<", delayed="<<0<<" , lost="<<0<<endl;

                    outputFile<<"At time ="<<simTime()+pt<<" Sender Sends message ["<<currentMessage->getM_Payload()<<"] ID= "<<currentMessage->getId()<<",modified= "<<0<<", duplicated="<<0<<", delayed="<<0<<" , lost="<<0<<endl;

                    sendDelayed(retransmit, tt+pt, "port$o");
                    totalpacketssent+=2;
                    scheduleAt(simTime() + to+pt, timeoutEvent);
                }
                else if (ack->getError_Type()==1)
                {
                    totalpacketssent++;
                    EV <<"At time= "<<simTime()<< " Received NACK for already ACKed ID = " << ack->getId()
                       << " (likely a duplicate frame at receiver), ignoring.\n";
                    outputFile<<"At time= "<<simTime() << " Received NACK for already ACKed ID = " << ack->getId()
                                           << " (likely a duplicate frame at receiver), ignoring.\n";

                }
            }

            delete msg;
        }
}

void Sender::loadMessageFromFile(std::string filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
            EV << "File opened successfully!" << endl;

    }
    std::string line;
    int idCounter=0;
    while (getline(file, line)) {
        if (line.length() < 5) continue;
        size_t spacePos = line.find(' ');
        //std::string numberString = line.substr(0, spacePos);
        std::string numberString = line.substr(0, 4);
        errorMessages.push_back(numberString);

        std::string payload = line.substr(5);

        MyMessage *msg = new MyMessage();
        msg->setId(idCounter++);
        msg->setM_Payload(payload.c_str());

        EV<<"Read Message: "<<payload<<" Error code: "<<numberString<<endl;
        msg->setM_Header("");
        msg->setM_Type(0);
        msg->setSending_time(0);




        messageVector.push_back(msg);
    }
    EV << "Finished loading messages. Total messages: " << messageVector.size() << endl;
    file.close();
}

void Sender:: SendNextMessage(){
    if(CurrentMessageIndex >= messageVector.size())
    {
        sessionFinishTime =simTime();
        finishSession();
        return;
    }
    else{
        currentMessage=messageVector[CurrentMessageIndex++];
        currentMessage->setId(currentSeq);
        //currentSeq =1 ^ currentSeq;
        EV<<"At time = "<<simTime().str()<<" Sender Preparing message "<<currentMessage->getM_Payload()<<", id = "<<std::to_string(currentMessage->getId())<<endl;

        outputFile<<"At time = "<<simTime().str()<<" Sender Preparing message "<<currentMessage->getM_Payload()<<", id = "<<std::to_string(currentMessage->getId())<<endl;
        //EV<<"Loss= "<<BoolToString(currentMessage->getLoss())<<endl;
        SimulateErrors(currentMessage,errorMessages[CurrentMessageIndex-1]);
    }

}

void Sender::SimulateErrors(MyMessage *msg,std::string errorcode)
{
    std::string stuffedpayload= stuffPayload(msg->getM_Payload());
    char parity=calculateParityByte(stuffedpayload);
    msg->setM_Payload(stuffedpayload.c_str());
    msg->setM_Trailer(std::string(1,parity).c_str());


    bool modification = (errorcode[0] == '1');
    bool duplication = (errorcode[1] == '1');
    bool delay = (errorcode[2] == '1');
    bool loss = (errorcode[3] == '1');

    //no errors
    double delaytime=0;
    if(delay)
    {
        delaytime=td;
    }

    int modifiedbitpos=0;
    unmodifiedpayload=stuffedpayload;
    if(modification)
    {
        modifybit(stuffedpayload,modifiedbitpos);
        msg->setM_Payload(stuffedpayload.c_str());
    }
    if(!loss)
    {
        sendDelayed(msg->dup(), tt+pt+delaytime, "port$o");
        totalpacketssent++;
        if(!modification)
        {
            correctMessages++;
        }
    }
    currentMessage=msg;
    EV<<"At time ="<<simTime()+pt<<" Sender Sends message ["<<msg->getM_Payload()<<"] ID= "<<msg->getId()<<",modified= "<<modifiedbitpos<<", duplicated="<<BoolToString(duplication)<<", delayed="<<BoolToString(delay)<<" , lost="<<BoolToString(loss)<<endl;

    outputFile<<"At time ="<<simTime()+pt<<" Sender Sends message ["<<msg->getM_Payload()<<"] ID= "<<msg->getId()<<",modified= "<<modifiedbitpos<<", duplicated="<<BoolToString(duplication)<<", delayed="<<BoolToString(delay)<<" , lost="<<BoolToString(loss)<<endl;

    if(duplication && !loss)
    {
        sendDelayed(msg, tt+pt+delaytime+0.1, "port$o");
        totalpacketssent++;

        outputFile<<"At time ="<<simTime()+pt+0.1<<" Sender Sends message ["<<msg->getM_Payload()<<"] ID= "<<msg->getId()<<",modified= "<<modifiedbitpos<<", duplicated=2"<<", delayed="<<BoolToString(delay)<<" , lost="<<BoolToString(loss)<<endl;
        EV<<"At time ="<<simTime()+pt+0.1<<" Sender Sends message ["<<msg->getM_Payload()<<"] ID= "<<msg->getId()<<",modified= "<<modifiedbitpos<<", duplicated=2"<<", delayed="<<BoolToString(delay)<<" , lost="<<BoolToString(loss)<<endl;
    }

    scheduleAt(simTime() + to+pt, timeoutEvent);

}

std ::string  Sender::stuffPayload(std::string payload)
{
    std::string stuffed;
    stuffed += '$';
    for (char c : payload) {
        if (c == '$' || c == '/')
            stuffed += '/';
        stuffed += c;
    }
    stuffed += '$';
    return stuffed;
}

char Sender::calculateParityByte(std::string stuffedPayload)
{
    int onesCount = 0;
    for (char c : stuffedPayload) {
        std::bitset<8> bits(c);
        onesCount += bits.count();
    }
    return (onesCount % 2 == 0) ? '0' : '1';
}

void Sender::modifybit(std::string &paylaodToSend,int &modifiedindex)
{
    int byteIndex=intuniform(0, paylaodToSend.length()-1);
    int bitPosition = intuniform(0, 7);
    modifiedindex = byteIndex * 8 + bitPosition;
    paylaodToSend[byteIndex] ^= (1 << bitPosition);
}

void Sender::finishSession()
{
    simtime_t TotalTransmition = sessionFinishTime - sessionStartTime;
    EV<<"total transmission time = "<<TotalTransmition.str()<<endl;
    EV<<"total number of transmission = "<<std::to_string(totalpacketssent)<<endl;
    EV<<"the network throughput = "<<endl;

    outputFile<<"total transmission time = "<<TotalTransmition.str()<<endl;
    outputFile<<"total number of transmission = "<<std::to_string(totalpacketssent)<<endl;
    outputFile<<"the network throughput = "<<correctMessages/TotalTransmition<<endl;

    outputFile.close();
    finish();
}

std::string Sender::BoolToString(bool b)
{
    return b ? "1" : "0";
}
