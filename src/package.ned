package project;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    @display("bgb=1078,460");
    submodules:
        sender: Sender {
            @display("p=238,188;i=device/pc2");
        }
        receiver: Receiver {
            @display("p=769,194;i=device/antennatower");
        }
    connections:
        receiver.port <--> sender.port;
}

