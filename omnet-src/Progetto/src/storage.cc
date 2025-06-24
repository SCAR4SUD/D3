#include "storage.h"

Define_Module(Storage);

void Storage::initialize()
{
    // Leggiamo il parametro dal file .ned/.ini
    numEcu = par("numEcu");
    EV << "Modulo Storage inizializzato. Attendo dati da " << numEcu << " sorgenti.\n";

    // Creiamo e apriamo un file di log per ogni sorgente
    for (int i = 0; i < numEcu; ++i) {
        // Creiamo un nome di file dinamico
        std::string fileName = "log_module_" + std::to_string(i) + ".txt";

        // Apriamo il file in modalità 'append'
        // e lo associamo all'ID del modulo nella nostra mappa.
        fileStreams[i].open(fileName, std::ios_base::app);

        // Controllo di robustezza: verifichiamo se il file è stato aperto correttamente
        if (!fileStreams[i].is_open()) {
            // Se non riusciamo ad aprire un file, è un errore grave.
            // Lo segnaliamo con un'eccezione che fermerà la simulazione.
            throw cRuntimeError("Impossibile aprire il file di log: %s", fileName.c_str());
        }
        EV << "File di log creato: " << fileName << "\n";
    }
}

void Storage::handleMessage(cMessage *msg)
{
    //dynamic_cast meglio di check_and_cast
    //convertiamo il msg nel nostro messaggio.
    //MODIFICARE QUI IN CASO DI MESSAGGIO DIVERSO
    KeyResponse *packet = dynamic_cast<KeyResponse *>(msg);

    //Parametri che prende dal messaggio ricevuto
    int sourceId = packet->getSrc();
    const char* payload = packet->getKey();


    EV << "Ricevuto messaggio da sourceId: " << sourceId << " con payload: '" << payload << "'\n";

    // Troviamo lo stream corretto nella nostra mappa usando l'ID
    auto it = fileStreams.find(sourceId);
    if (it != fileStreams.end()) {
        // Scriviamo sul file.
        // Aggiungiamo un timestamp della simulazione per avere un log più utile.
        it->second << simTime() << ", " << payload << std::endl;
    } else {
        // Se riceviamo un ID non previsto, lo segnaliamo.
        // Non fermiamo la simulazione, ma lasciamo una traccia.
        EV_WARN << "Ricevuto messaggio da un sourceId sconosciuto: " << sourceId << ". Messaggio ignorato.\n";
    }

    // Deallochiamo il messaggio!
    //    in caso contrario rischio di un memory leak.
    //    Il modulo che riceve un messaggio è responsabile della sua distruzione.
    delete msg;
}

void Storage::finish()
{
    EV << "Simulazione terminata. Chiudo tutti i file di log.\n";

    // Iteriamo su tutta la mappa e chiudiamo ogni file stream, affinché venga bufferizzato
    for (auto const& [id, stream] : fileStreams) {
        if (stream.is_open()) {
            fileStreams[id].close();
        }
    }
}
