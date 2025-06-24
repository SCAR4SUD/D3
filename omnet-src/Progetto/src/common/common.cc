#include "common.h"

const char* ECU_ID1 = "ECU0001";
const char* ECU_ID2 = "ECU0002";
const char* ECU_ID3 = "ECU0003";
const char* ECU_ID4 = "ECU0004";
const char* ECU_ID5 = "ECU0005";
const char* HSM_ID  = "HSM0001";

//unsigned char aes_hsm_key[AES_KEY_LEN]; //chiave di comunicazione con hsm
std::string local_ecu_id;
const char* ecu_ids[] = {ECU_ID1, ECU_ID2, ECU_ID3, ECU_ID4, ECU_ID5};

void handle_errors(const std::string& context) {
    std::cerr << "[Errore] " << context << "\n";
    ERR_print_errors_fp(stderr);
    //exit(1);
}

void load_local_ecu_id() {  //il load dell'id tramite .txt è "non sicuro" solo a scopi progettuali, per facilità di uso su omninet, chiaramente su un prototipo l'id sarebbe hardcoded, e non servirebbe nemmeno la funzione
    std::ifstream infile("ecu_ids/ecu_id.txt");
    if (!infile.is_open()) {
        handle_errors("Impossibile aprire il file ECU ID");
    }

    std::string line;
    std::getline(infile, line);
    infile.close();

    bool valid_id = false;
    for (const char* id : ecu_ids) {
    if (line == id) {
        valid_id = true; 
        break;
    }
    }

    if (!valid_id) { 
        handle_errors("ECU ID non valido nel file"); 
    }

    local_ecu_id = line;
}


