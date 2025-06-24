package progetto;

simple Gateway
{
    parameters:
        int numECUs;              // numero totale di ECU
    gates:
        input  ecuIn[numECUs];    // da ciascuna ECU
        output ecuOut[numECUs];   // verso ciascuna ECU
        
        output toHsm;
        input fromHsm;
}
