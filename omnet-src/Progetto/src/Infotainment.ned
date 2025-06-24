package progetto;

simple Infotainment
{
    parameters:
        int id;                   // identificativo univoco della ECU
        int numECUs;              // numero totale di ECU (per scegliere destinazioni)
        double sendInterval @unit(s); // intervallo base per invio richieste (può essere funzione di time)
    gates:
        output out; // verso il Gateway
        input in;   // dalla Gateway
}
