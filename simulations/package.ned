package project.simulations;

import ned.IdealChannel;
import project.Receiver;
import project.Sender;


@license(LGPL);
//
// TODO documentation
//
network Network
{
    @display("bgb=452,263");
    submodules:
        sender: Sender {
            @display("p=94,100");
        }
        receiver: Receiver {
            @display("p=296,119");
        }
    connections:
        sender.port <--> IdealChannel <--> receiver.port;
}

