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


    startEvent = new cMessage("startEvent");
    timeoutEvent = new cMessage("timeoutEvent");

    scheduleAt(st, startEvent);
    //CurrentMessageIndex=17;
}

void Sender::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    if(msg == startEvent)
    {
        sessionStartTime = simTime();
        //EV<<"Session Started"<<endl;
        EV<<"The Session Started at time: "<<sessionStartTime.str()<<"s"<<endl;
        SendNextMessage();

    }
    else if(msg == timeoutEvent){
        EV<<"Timeout at Time :"<<simTime().str()<<",Resending Message "<<currentMessage->getM_Payload()<<" Id="<<std::to_string(currentMessage->getId())<<endl;
        sendFrame(currentMessage, 1);
    }
    else
    {
        MyMessage *ack = check_and_cast<MyMessage *>(msg);
        if (ack->getM_Type() == 1 && ack->getId() == currentMessage->getId()) {
            EV << "Sender ACK received for message ID: " << ack->getId() << endl;
            cancelEvent(timeoutEvent);
            waitforAck = false;
            correctMessages++;
            SendNextMessage();
        }
        else if(ack->getM_Type() == 2)
        {
            if(ack->getId() != currentMessage->getId())
            {
                EV<<"Sender Acknowledged that Receiver has Received an out of Order frame: "<<ack->getM_Payload()<<" with id: "<<ack->getId()<<endl;
                SendNextMessage();
            }
            else
            {
            EV << "NACK or wrong ACK received. Resending message ID: " << currentMessage->getId() << endl;
            currentMessage->setM_Payload(unmodifiedpayload.c_str());
            sendDelayed(currentMessage->dup(), tt, "port$o");
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
        bool modification = (line[0] == '1');
        bool duplication = (line[1] == '1');
        bool delay = (line[2] == '1');
        bool loss = (line[3] == '1');
        std::string payload = line.substr(5);

        MyMessage *msg = new MyMessage();
        msg->setId(idCounter++);
        msg->setM_Payload(payload.c_str());
        EV<<"Read Message: "<<payload<<endl;
        msg->setM_Header("");
        //msg->setM_Trailer("");
        msg->setM_Type(0);
        msg->setSending_time(0);


        msg->setModification(modification);
        msg->setDuplication(duplication);
        msg->setDelay(delay);
        msg->setLoss(loss);

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
        currentSeq =1 ^ currentSeq;
        EV<<"At time = "<<simTime().str()<<" Sender starts preparing message "<<currentMessage->getM_Payload()<<", id = "<<std::to_string(currentMessage->getId())<<endl;
        //EV<<"Loss= "<<BoolToString(currentMessage->getLoss())<<endl;
        SimulateErrors(currentMessage);
    }

}

void Sender::SimulateErrors(MyMessage *msg)
{
    bool loss = msg ->getLoss();
    bool delay = msg ->getDelay();
    bool duplication =msg->getDuplication();
    bool modify = msg->getModification();

    if(loss)
    {
        EV<<"At time = "<<simTime().str()<<" Sender losing message "<<msg->getM_Payload()<<", id = "<<std::to_string(msg->getId())<<",modified = "<<BoolToString(modify)<<
                ",loss = "<<BoolToString(loss)<<",delay = "<<BoolToString(delay)<<",duplication = "<<BoolToString(duplication)<<endl;
        if (!waitforAck)
        {
            waitforAck = true;
            scheduleAt(simTime() + to, timeoutEvent);
        }
        SendNextMessage();
        return;
    }

    if(delay)
    {
        scheduleAt(simTime() + td, msg->dup());
    }else {
        sendFrame(msg, 0);
    }

    if(duplication)
    {
        cMessage * dupmsg=currentMessage->dup();
        sendDelayed(dupmsg->dup(), tt+0.1, "port$o");;
    }

}

void Sender::sendFrame(MyMessage*msg,int duplicateorder)
{
    std::string stuffedPayload= stuffPayload(msg->getM_Payload());
    char parityByte = calculateParityByte(stuffedPayload);

    bool modification=msg->getModification();

    std::string paylaodToSend = stuffedPayload;

    unmodifiedpayload = paylaodToSend;
    if(modification && duplicateorder ==0)
    {
        modifybit(paylaodToSend);
    }

    std::string c=std::to_string(msg->getId());
    msg->setM_Header(c.c_str());
    msg->setM_Payload(paylaodToSend.c_str());
    msg->setM_Trailer(std::string(1,parityByte).c_str());

    currentMessage=msg;
    //send(msg->dup(),"port$o");
    sendDelayed(msg->dup(), tt, "port$o");
    //scheduleAt(simTime() + tt, msg);

    totalpacketssent++;


    EV<<"At time = "<<simTime().str()<<" Sender starts sending message "<<msg->getM_Payload()<<", id = "<<std::to_string(msg->getId())<<",modified = "<<BoolToString(msg->getModification())<<
                    ",loss = "<<BoolToString(msg->getLoss())<<",delay = "<<BoolToString(msg->getDelay())<<",duplication = "<<BoolToString(msg->getDuplication())<<endl;


    if(!waitforAck)
    {
        waitforAck=true;
        scheduleAt(simTime() + to, timeoutEvent);
    }
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

void Sender::modifybit(std::string &paylaodToSend)
{
    int Byte=intuniform(0, paylaodToSend.length()-1);
    paylaodToSend[Byte] ^= (1<<intuniform(0,7));
}

void Sender::finishSession()
{
    simtime_t TotalTransmition = sessionFinishTime - sessionStartTime;
    EV<<"total transmission time = "<<TotalTransmition.str()<<endl;
    EV<<"total number of transmission = "<<std::to_string(totalpacketssent)<<endl;
    EV<<"the network throughput = "<<endl;
}

std::string Sender::BoolToString(bool b)
{
    return b ? "1" : "0";
}
