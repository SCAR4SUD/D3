package progetto.simulations;

import progetto.ECU;
import progetto.Gateway;
import progetto.HSM;
import progetto.Infotainment;

network Rete_def
{
    parameters:
        int numECUs = default(4); // rimane 4, ma non useremo l’array per le submodules

        @display("bgb=1000,1000");
    submodules:

        gateway: Gateway {
            parameters:
                numECUs = parent.numECUs;
                @display("p=500,500;is=s");
        }

        hsm: HSM {
            @display("p=500,650;is=s");
        }

        ecu1: ECU {
            parameters:
                id = 1;
                numECUs = parent.numECUs;
                sendInterval = uniform(1s, 3s);
                @display("p=200,800;is=s");
        }
        ecu2: ECU {
            parameters:
                id = 2;
                numECUs = parent.numECUs;
                sendInterval = uniform(1s, 3s);
                @display("p=200,200;is=s");
        }
        ecu3: ECU {
            parameters:
                id = 3;
                numECUs = parent.numECUs;
                sendInterval = uniform(1s, 3s);
                @display("p=800,200;is=s");
        }
        Infotainment: Infotainment {
            parameters:
                id = 4;
                numECUs = parent.numECUs;
                sendInterval = uniform(1s, 3s);
                @display("p=800,800;is=s");
        }

    connections allowunconnected:
        //
        //   Creiamo manualmente le connessioni “in” e “out” 
        //   per ciascuna ECU con il Gateway (indice corrispondente -1).
        //
        ecu1.out --> {  delay = 1ms; } --> gateway.ecuIn[0];
        gateway.ecuOut[0] --> {  delay = 1ms; } --> ecu1.in;

        ecu2.out --> {  delay = 1ms; } --> gateway.ecuIn[1];
        gateway.ecuOut[1] --> {  delay = 1ms; } --> ecu2.in;

        ecu3.out --> {  delay = 1ms; } --> gateway.ecuIn[2];
        gateway.ecuOut[2] --> {  delay = 1ms; } --> ecu3.in;

        Infotainment.out --> {  delay = 1ms; } --> gateway.ecuIn[3];
        gateway.ecuOut[3] --> {  delay = 1ms; } --> Infotainment.in;

        // ----------------------------
        // Collegamento () Gateway <--> HSM 
        // ----------------------------
        gateway.toHsm --> {  delay = 1ms; } --> hsm.fromGateway;
        hsm.toGateway --> {  delay = 1ms; } --> gateway.fromHsm;
}
