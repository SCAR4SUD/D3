#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include </usr/include/jsoncpp/json/json.h>

using namespace std;

// Funzione per inviare e ricevere messaggi JSON
void send_request_print_response(const string& server_ip, int port, const Json::Value& request_json)
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[4096] = {0}; // Buffer grande per eventuali risposte lunghe

    // Creazione socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) <0)
    {
        cerr<<"\n Errore di creazione del socket \n";
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Conversione indirizzo IP da stringa a binario
    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <=0)
    {
        cerr<<"\n Indirizzo non valido / Indirizzo non supportato \n";
        close(sock);
        return;
    }

    // Connessione al server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cerr<<"\n Errore di connessione al server "<< server_ip <<":" <<port <<endl;
        close(sock);
        return;
    }

    Json::StreamWriterBuilder writer;
    writer["indentation"]="\t";
    string request_payload = Json::writeString(writer, request_json);
    cout<<"\n Invio richiesta al server:\n"<<request_payload<<endl;

    // Invia richiesta al server
    send(sock, request_payload.c_str(), request_payload.length(), 0);

    // Leggi risposta server
    long valread = read(sock, buffer, sizeof(buffer) - 1);
    if (valread >0)
    {
        buffer[valread] = '\0'; // Null terminate
        cout<<"\n Risposta del server:\n"<<buffer<<endl;
    }
    else if (valread == 0) {
        std::cout << "Connessione chiusa dal server.\n";
    } else {
        std::cerr << "Errore nella lettura della risposta.\n";
    }

    close(sock);
}


int main(int argc, char const *argv[]) {
    int port = 8080;
    std::string server_ip = "127.0.0.1"; // Indirizzo IP del server (localhost)

    std::cout << "GDPR Client. Connessione a " << server_ip << ":" << port << std::endl;
    std::cout << "Scegli un'opzione:\n";
    std::cout << " 1. Articolo 15 (Accesso)\n";
    std::cout << " 2. Articolo 16 (Rettifica)\n";
    std::cout << " 3. Articolo 17 (Cancellazione)\n";
    std::cout << " 4. Articolo 18 (Limitazione)\n";
    std::cout << " 5. Articolo 20 (Portabilità)\n";
    std::cout << " 6. Articolo 21 (Opposizione)\n";
    std::cout << " 7. Articolo 22 (Decisioni automatizzate)\n";
    std::cout << " 8. Articolo 23 (Esportazione Dati)\n";
    std::cout << " 0. Esci\n";
    std::cout << "Inserisci la tua scelta: ";

    int choice;
    std::cin >> choice;

    if (choice == 0) {
        return 0;
    }

    std::string password;
    std::cout << "Inserisci Password  ";
    std::cin >> password;
    std::cin.ignore(); // Consuma il newline residuo

    std::string email; // NUOVO: Richiede l'email
    std::cout << "Inserisci Email  ";
    std::cin >> email;
    std::cin.ignore(); // Consuma il newline residuo

    Json::Value request_json;
    request_json["password"] = password;
    request_json["email"] = email; // NUOVO: Aggiunge l'email alla richiesta JSON

    switch (choice) {
        case 1: // Articolo 15: Accesso
            request_json["gdpr_article"] = "15";
            break;
        case 2: { // Articolo 16: Rettifica
            request_json["gdpr_article"] = "16";
            Json::Value updates;
            std::cout << "Quanti campi vuoi aggiornare? ";
            int num_fields;
            std::cin >> num_fields;
            std::cin.ignore();
            for (int i = 0; i < num_fields; ++i) {
                std::string field, value_str;
                std::cout << "Campo " << i+1 << " (es. email, indirizzo, preferenze_sedile_auto, nuovo_campo): ";
                std::getline(std::cin, field);
                std::cout << "Nuovo valore per " << field << ": ";
                std::getline(std::cin, value_str);

                // Tentativo di inferire il tipo: se è "true"/"false" -> bool, se numerico -> int/double, altrimenti string
                Json::Value value_to_send;
                if (value_str == "true") value_to_send = true;
                else if (value_str == "false") value_to_send = false;
                else if (value_str.find_first_not_of("0123456789") == std::string::npos && !value_str.empty()) {
                    value_to_send = std::stoi(value_str);
                }
                else if (value_str.find_first_not_of("0123456789.") == std::string::npos && !value_str.empty() && value_str.find('.') != std::string::npos) {
                    value_to_send = std::stod(value_str);
                }
                else value_to_send = value_str;

                updates[field] = value_to_send;
            }
            request_json["updates"] = updates;
            break;
        }
        case 3: // Articolo 17: Cancellazione
            request_json["gdpr_article"] = "17";
            break;
        case 4: { // Articolo 18: Limitazione
            request_json["gdpr_article"] = "18";
            std::string reason;
            std::cout << "Inserisci il motivo della limitazione: ";
            std::getline(std::cin, reason);
            request_json["reason"] = reason;
            break;
        }
        case 5: // Articolo 20: Portabilità
            request_json["gdpr_article"] = "20";
            break;
        case 6: { // Articolo 21: Opposizione
            request_json["gdpr_article"] = "21";
            std::string reason;
            std::cout << "Inserisci il motivo dell'opposizione: ";
            std::getline(std::cin, reason);
            request_json["reason"] = reason;
            break;
        }
        case 7: // Articolo 22: Decisioni automatizzate
            request_json["gdpr_article"] = "22";
            break;
        case 8: // Articolo 23: Esportazione Dati
            request_json["gdpr_article"] = "23";
            break;
        default:
            std::cout << "Scelta non valida.\n";
            return 1;
    }

    send_request_print_response(server_ip, port, request_json);

    return 0;
}