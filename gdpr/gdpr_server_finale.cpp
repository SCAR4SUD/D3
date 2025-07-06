#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include </usr/include/jsoncpp/json/json.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cmath> // Per std::abs
#include <fstream> // Per la gestione dei file (ifstream, ofstream)
#include <iterator> // Per std::istreambuf_iterator
#include <functional> // Per std::hash

using namespace std;

// --- INIZIO NUOVE FUNZIONI DI HASHING ---
// SALT hardcodato per l'hashing delle password.
// In un'applicazione reale, questo dovrebbe essere un segreto ben protetto
// e preferibilmente generato dinamicamente o gestito in modo sicuro.
const string HASH_SALT = "YourSuperSecretHardcodedSalt_GDPR_12345!";

// Funzione di hashing semplificata per la password (NON CRITTOGRAFICAMENTE SICURA!)
// Per produzione, usa SHA256, Argon2, bcrypt, ecc.
string hash_password(const string& password) {
    size_t hashed_val = std::hash<std::string>{}(password + HASH_SALT);
    return std::to_string(hashed_val); // Converti l'hash in stringa per usarlo come chiave
}
// --- FINE NUOVE FUNZIONI DI HASHING ---


// Simulazione del data store in memoria
// La chiave della mappa è ora la RAPPRESENTAZIONE HASHATA della password.
map<string, Json::Value> user_data_store;

// Definizione della struttura per le policy di conservazione
struct RetentionPolicy
{
    chrono::seconds min_retention_s; // tempo minimo di conservazione in secondi
    chrono::seconds max_retention_s; // tempo massimo di conservazione in secondi
};

// Mappa globale che definisce le policy di conservazione per ogni tag
map<string, RetentionPolicy> data_retention_policies;

void initialize_retention_policies()
{
    // Calcoli espliciti in secondi per maggiore compatibilità
    // 1 anno = 365 giorni * 24 ore/giorno * 60 minuti/ora * 60 secondi/minuto = 31,536,000 secondi
    // 1 giorno = 24 ore * 60 minuti/ora * 60 secondi/minuto = 86,400 secondi
    data_retention_policies["dati anagrafici"] = {chrono::seconds(10 * 31536000LL), chrono::seconds(15 * 31536000LL)};
    data_retention_policies["preferenze utente"] = {chrono::seconds(30 * 86400LL), chrono::seconds(2 * 31536000LL)};
    data_retention_policies["dato temporale"]= {chrono::seconds(1 * 31536000LL), chrono::seconds(5 * 31536000LL)};
    data_retention_policies["preferenze autovettura"] = {chrono::seconds(90 * 86400LL), chrono::seconds(3 * 31536000LL)};
    data_retention_policies["non categorizzato"] = {chrono::seconds(0), chrono::seconds(10 * 31536000LL)}; //default per dati senza tag
}

// Funzione per formattare una durata in secondi in un formato leggibile (anni, giorni, ore, minuti, secondi)
string format_duration_compact(chrono::seconds duration_s) {
    long long total_seconds = duration_s.count();
    string prefix = "";
    if (total_seconds < 0) {
        prefix = "-";
        total_seconds = std::abs(total_seconds); // Lavora con il valore assoluto
    }

    if (total_seconds == 0) {
        return "0 secondi";
    }

    long long years = total_seconds / (31536000LL); // Secondi in un anno (approssimazione 365 giorni)
    if (years > 0) {
        return prefix + to_string(years) + (years == 1 ? " anno" : " anni");
    }

    long long days = total_seconds / (86400LL);    // Secondi in un giorno
    if (days > 0) {
        return prefix + to_string(days) + (days == 1 ? " giorno" : " giorni");
    }

    long long hours = total_seconds / 3600LL;
    if (hours > 0) {
        return prefix + to_string(hours) + (hours == 1 ? " ora" : " ore");
    }

    long long minutes = total_seconds / 60LL;
    if (minutes > 0) {
        return prefix + to_string(minutes) + (minutes == 1 ? " minuto" : " minuti");
    }

    return prefix + to_string(total_seconds) + (total_seconds == 1 ? " secondo" : " secondi");
}

// Funzione per ottenere il timestamp corrente in formato ISO 8601 (stringa)
string get_current_timestamp_iso8601()
{
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss<<put_time(localtime(&now_c), "%Y-%m-%dT%H:%M:%SZ"); // formato ISO 8601
    return ss.str();
}

// Funzione per convertire un timestamp ISO 8601 in chrono::system_clock::time_point
chrono::system_clock::time_point parse_iso8601_timestamp(const string& timestamp_str)
{
    tm t={};
    stringstream ss(timestamp_str);
    // Parsifica la stringa del tempo nella struttura tm
    ss >> get_time(&t, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
        // Gestisci l'errore di parsing, es. lancia un'eccezione o restituisci un time_point di default
        cerr << "Errore nel parsing del timestamp: " << timestamp_str << endl;
        return chrono::system_clock::from_time_t(0);
    }
    return chrono::system_clock::from_time_t(mktime(&t));
}

// Funzione per creare un dato con un tag e un timestamp di inserimento
Json::Value createTaggedData(const Json::Value& value, const string& tag)
{
    Json::Value tagged_data;
    tagged_data["value"] = value;
    tagged_data["tag"] = tag;
    tagged_data["insertion_time"]= get_current_timestamp_iso8601(); // aggiungi il timestamp
    return tagged_data;
}

// Funzione per inizializzare dati di esempio con tag e password
void initialize_sample_data()
{
    // Esempio con password al posto di user_id
    string clear_password_user1 = "passwordUtente123";
    string hashed_password_user1 = hash_password(clear_password_user1); // Hash della password
    Json::Value user1_data;
    user1_data["email"] = createTaggedData("mario.rossi@example.com", "dati anagrafici");
    user1_data["nome"] = createTaggedData("Mario Rossi", "dati anagrafici");
    user1_data["indirizzo"] = createTaggedData("Via del corso 1, Roma", "dati anagrafici");
    user1_data["data_registrazione"] =  createTaggedData("2023-01-15", "dato temporale");
    user1_data["consenso_marketing"] = createTaggedData(true, "preferenze utente");
    Json::Value preferenze_sedile_auto_user1;
    preferenze_sedile_auto_user1["posizione_orizzontale"] = 10;
    preferenze_sedile_auto_user1["inclinazione_schienale"] = 30;
    user1_data["preferenze_sedile_auto"] = createTaggedData(preferenze_sedile_auto_user1, "preferenze autovettura");
    user_data_store[hashed_password_user1] = user1_data; // Usa l'hash come chiave

    string clear_password_user2 = "secretPass456";
    string hashed_password_user2 = hash_password(clear_password_user2); // Hash della password
    Json::Value user2_data;
    user2_data["email"] = createTaggedData("anna.verdi@example.com", "dati anagrafici");
    user2_data["nome"] = createTaggedData("Anna Verdi", "dati anagrafici");
    user2_data["indirizzo"] = createTaggedData("Corso Vittorio Emanuele 100, Milano", "dati anagrafici");
    user2_data["data_registrazione"] = createTaggedData("2024-03-20", "dato temporale");
    user2_data["consenso_marketing"] = createTaggedData(false, "preferenze utente");
    Json::Value preferenze_sedile_auto_user2;
    preferenze_sedile_auto_user2["posizione_orizzontale"] = 15;
    preferenze_sedile_auto_user2["inclinazione_schienale"] = 25;
    user2_data["preferenze_sedile_auto"] = createTaggedData(preferenze_sedile_auto_user2, "preferenze autovettura");
    user_data_store[hashed_password_user2] = user2_data; // Usa l'hash come chiave
}

// Funzione per caricare i dati utente da un file
void load_data_from_file() {
    ifstream infile("gdpr_data_store.txt");
    if (infile.is_open()) {
        Json::Reader reader;
        Json::Value root;
        string content((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
        infile.close();

        if (reader.parse(content, root)) {
            // Assumendo che la radice sia un oggetto le cui chiavi sono le password hashate
            for (Json::ValueConstIterator it = root.begin(); it != root.end(); ++it) {
                user_data_store[it.name()] = *it;
            }
            cout << "Dati utente caricati con successo da gdpr_data_store.txt" << endl;
        } else {
            cerr << "Errore nel parsing del file gdpr_data_store.txt. Inizializzazione con dati di esempio." << endl;
            initialize_sample_data(); // Fallback ai dati di esempio
        }
    } else {
        cout << "File gdpr_data_store.txt non trovato. Inizializzazione con dati di esempio." << endl;
        initialize_sample_data(); // Inizializza se il file non esiste
    }
}

// Funzione per salvare i dati utente su un file
void save_data_to_file() {
    Json::Value root;
    for (const auto& pair : user_data_store) {
        root[pair.first] = pair.second;
    }

    ofstream outfile("gdpr_data_store.txt");
    if (outfile.is_open()) {
        Json::StreamWriterBuilder writer;
        writer["indentation"] = "\t"; // Stampa JSON formattato
        string content = Json::writeString(writer, root);
        outfile << content;
        outfile.close();
        cout << "Dati utente salvati con successo in gdpr_data_store.txt" << endl;
    } else {
        cerr << "Errore: Impossibile salvare i dati nel file gdpr_data_store.txt" << endl;
    }
}


// Funzioni dedicate per ogni articolo GDPR
// Tutte le funzioni ora ricevono la 'hashed_password' come identificatore utente
// Articolo 15: Diritto di accesso
Json::Value handle_article_15_access(const string& hashed_password)
{
    Json::Value response;
    cout<< "[Art. 15] Gestione richiesta di accesso per un utente." <<endl; // Non stampare l'hash
    if(user_data_store.count(hashed_password))
    {
        response["status"] = "success";
        response["gdpr_article"] = "15";
        response["message"]= "Dati di accesso per l'utente recuperati.";
        response["data_subject_info"] = user_data_store[hashed_password]; // invia i dati con i tag

    }
    else
    {
        response["status"]= "error";
        response["gdpr_article"] = "15";
        response["message"]= "Errore interno: Utente non trovato dopo autenticazione. (Dovrebbe essere autenticato)";
    }
    return response;
}

// Articolo 16: Diritto di rettifica
Json::Value handle_article_16_rectification(const string& hashed_password, const Json::Value& updates)
{
    Json::Value response;
    cout<<"[Art. 16] Gestione richiesta di rettifica per un utente." <<endl; // Non stampare l'hash
    if (user_data_store.count(hashed_password))
    {
        bool changed = false;
        Json::Value errors_fields = Json::arrayValue; // per tracciare i campi non validi
        for (Json::ValueConstIterator it= updates.begin(); it!=updates.end(); ++it)
        {
            string field_name = it.name();
            const Json::Value& new_value = *it; // questo è il valore che arriva dal client
            string assigned_tag = ""; // inizializza a vuoto
            string current_insertion_time = get_current_timestamp_iso8601(); // default per nuovo campo

            bool field_exists = user_data_store[hashed_password].isMember(field_name);
            // Logica di validazione tag
            if (field_exists && user_data_store[hashed_password][field_name].isObject())
            {
                // Campo esistente: mantiene il tag e il timestamp originali
                if(user_data_store[hashed_password][field_name].isMember("tag"))
                {
                    assigned_tag = user_data_store[hashed_password][field_name]["tag"].asString();
                }
                if(user_data_store[hashed_password][field_name].isMember("insertion_time"))
                {
                    current_insertion_time = user_data_store[hashed_password][field_name]["insertion_time"].asString();
                }
            }
            else
            {
                // Tenta di assegnare un tag basato sul nome del campo
                if(field_name == "email" || field_name=="nome" || field_name=="indirizzo")
                {
                    assigned_tag = "dati anagrafici";
                }
                else if(field_name == "data_registrazione")
                {
                    assigned_tag = "dato temporale";
                }
                else if (field_name == "consenso_marketing")
                {
                    assigned_tag = "preferenze utente";
                }
                else if (field_name == "preferenze_sedile_auto")
                {
                    assigned_tag= "preferenze autovettura";
                }
                // Se il nome del campo non corrisponde ai campi predefiniti, controlla se è un tag valido
                else if(data_retention_policies.count(field_name))
                {
                    assigned_tag=field_name; // Se il nome del campo è un tag valido, usalo come tag
                }
                else
                {
                    // Campo non riconosciuto e non associabile a un tag esistente
                    Json::Value error_info;
                    error_info["field"] = field_name;
                    error_info["reason"] = "campo non riconosciuto o non coperto da un tag predefinito.";
                    errors_fields.append(error_info);
                    cout<<" - Campo '"<<field_name<<"' non modificabile: non riconosciuto o non ha un tag valido"<<endl;
                    continue; // Passa al prossimo campo nella richiesta di updates
                }
            }
            // Se il campo non esiste o non ha un tag assegnato (e non è stato gestito da continue sopra), usa il default "non categorizzato"
            if (assigned_tag.empty())
            {
                assigned_tag= "non categorizzato";
            }
            // Verifica che il tag assegnato sia valido nelle politiche di conservazione
            if(!data_retention_policies.count(assigned_tag))
            {
                Json::Value error_info;
                error_info["field"] = field_name;
                error_info["reason"] = "il tag '" + assigned_tag + "' assegnato al campo non ha una poltica di conservazione definita.";
                errors_fields.append(error_info);
                cout<<" - Campo '" <<field_name << "' non modificabile: tag non valido nelle policy"<<endl;
                continue; // Passa al prossimo campo
            }
            Json::Value current_value_obj = user_data_store[hashed_password][field_name];
            if (!current_value_obj.isMember("value") || current_value_obj["value"]!= new_value)
            {
                Json::Value rectified_data;
                rectified_data["value"] = new_value;
                rectified_data["tag"]=assigned_tag;
                rectified_data["insertion_time"]=current_insertion_time; // mantiene il timestamp originale
                user_data_store[hashed_password][field_name] = rectified_data;
                changed= true;
            }
        }
        if(errors_fields.empty())
        {

            if (changed)
            {
                response["status"]="success";
                response["gdpr_article"]="16";
                response["message"] = "Dati dell'utente rettificati con successo."; // Messaggio generico
                response["updated_data_preview"] = user_data_store[hashed_password]; // mostra i dati aggiornati con tag
                save_data_to_file(); // Salva le modifiche sul file

            }
            else
            {
                response["status"] = "success";
                response["gdpr_article"] = "16";
                response["message"] = "Nessuna modifica rilevata per l'utente. I dati erano già corretti.";
            }
        }
        else
        {
            response["status"] = "partial_error"; // indica che ci sono stati errori per alcuni campi
            response["gdpr_article"] = "16";
            response["message"] = "Alcuni campi non sono stati modificati/inseriti a causa di dati non riconosciuti o tag non validi.";
            response["errors_field"]= errors_fields;
            response["updated_data_preview"]= user_data_store[hashed_password]; // mostra comunque i dati aggiornati parzialmente
        }
    }
    else
    {
        response["status"] = "error";
        response["gdpr_article"] = "16";
        response["message"] = "Errore interno: Utente non trovato dopo autenticazione. (Dovrebbe essere autenticato)";
    }
    return response;
}
// Articolo 17: Diritto alla cancellazione ("diritto all'oblio") con check temporale
Json::Value handle_article_17_erasure_with_check(const string& hashed_password)
{
    Json::Value response;
    cout<<"[ART. 17] Gestione richiesta di cancellazione per un utente."<<endl; // Non stampare l'hash
    if(user_data_store.count(hashed_password))
    {
        bool can_delete_all=true;
        Json::Value undeletable_fields = Json::arrayValue;
        // Itera su tutti i campi dell'utente per controllare la policy di conservazione
        for(Json::ValueConstIterator it_field = user_data_store[hashed_password].begin(); it_field!=user_data_store[hashed_password].end(); ++it_field)
        {
            string field_name= it_field.name();
            const Json::Value& tagged_data= *it_field;
            if(tagged_data.isObject() && tagged_data.isMember("tag")&& tagged_data.isMember("insertion_time"))
            {
                string tag = tagged_data["tag"].asString();
                string insertion_time_str = tagged_data["insertion_time"].asString();
                auto it_policy = data_retention_policies.find(tag);
                if(it_policy != data_retention_policies.end()) // Policy trovata per questo tag
                {
                    RetentionPolicy policy = it_policy->second;
                    chrono::system_clock::time_point insertion_point = parse_iso8601_timestamp(insertion_time_str);
                    auto now = chrono::system_clock::now();
                    chrono::duration<double> age= now - insertion_point;
                    if(age<policy.min_retention_s)
                    {
                        can_delete_all= false;
                        Json::Value field_info;
                        field_info["field"]= field_name;
                        field_info["tag"]= tag;
                        // Utilizza format_duration_compact per visualizzare i tempi in modo leggibile
                        field_info["reason"]= "tempo minimo di conservazione non raggiunto(" +
                                            format_duration_compact(policy.min_retention_s) + " richiesti, " +
                                            format_duration_compact(chrono::duration_cast<chrono::seconds>(age)) + " trascorsi)";
                        undeletable_fields.append(field_info);
                        cout<<" Campo '"<<field_name<<"' (tag: "<<tag<<") non cancellabile: tempo minimo non raggiunto."<<endl;
                    }
                }
                else
                {
                    // Se un dato esistente non ha un tag riconosciuto, non può essere cancellato
                    can_delete_all=false;
                    Json::Value field_info;
                    field_info["field"]= field_name;
                    field_info["reason"]= "tag del dato non riconosciuto nelle policy di conservazione. impossibile valutarne la cancellazione.";
                    undeletable_fields.append(field_info);
                    cout<<" campo '" <<field_name<<"' (tag: "<<tag<<") non cancellabile: tag non riconosciuto."<<endl;
                }
            }
            else
            {
                // Se un dato esistente non ha la struttura corretta (manca tag o insertion_time)
                can_delete_all= false;
                Json::Value field_info;
                field_info["field"]= field_name;
                field_info["reason"] = "Struttura del dato non valida (manca 'tag' o 'insertion_time'). Impossibile valutarne la cancellazione.";
                undeletable_fields.append(field_info);
                cout << "  - Campo '" << field_name << "' non cancellabile: struttura dati non valida." << endl;

            }
        }
        if(can_delete_all)
        {
            user_data_store.erase(hashed_password); // Rimuovi l'utente dalla mappa
            response["status"] = "success";
            response["gdpr_article"] = "17";
            response["message"] = "Dati dell'utente cancellati con successo dopo aver verificato i tempi minimi di conservazione.";
            save_data_to_file(); // Salva le modifiche sul file
        }
        else
        {
            response["status"] = "warning"; // Usiamo "warning" o "partial_success" per indicare che non tutto può essere cancellato
            response["gdpr_article"] = "17";
            response["message"] = "Impossibile cancellare tutti i dati dell'utente. Alcuni dati non possono essere cancellati.";
            response["undeletable_data"] = undeletable_fields;
        }
    }
    else
    {
        response["status"]="error";
        response["gdpr_article"]="17";
        response["message"]="Errore interno: Utente non trovato dopo autenticazione. (Dovrebbe essere autenticato)";
    }
    return response;
}
// Articolo 18: Diritto di limitazione per il trattamento
Json::Value handle_article_18_restriction (const string& hashed_password, const string& reason)
{
    Json::Value response;
    cout<<"[Art. 18] Gestione richiesta di limitazione per un utente, motivo: "<<reason<<endl; // Non stampare l'hash
    if (user_data_store.count(hashed_password))
    {
        response["status"]="success";
        response["gdpr_article"] = "18";
        response["message"]="Richiesta di limitazione di trattamento per l'utente registrata. Motivo: "+ reason+".";
        response["restriction_status"] = "active_simulated";
    }
    else
    {
        response["status"]="error";
        response["gdpr_article"]="18";
        response["message"] ="Errore interno: Utente non trovato dopo autenticazione. (Dovrebbe essere autenticato)";
    }
    return response;
}
// Articolo 20: Diritto alla portabilità dei dati
Json::Value handle_article_20_portability(const string& hashed_password)
{
    Json::Value response;
    cout<<"[Art 20] Gestione richiesta di portabilità per un utente."<<endl; // Non stampare l'hash
    if(user_data_store.count(hashed_password))
    {
        response["status"]="success";
        response["gdpr_article"]= "20";
        response["message"]= "Dati per la portabilità per l'utente generati.";
        // Forniamo all'utente i dati portabili in un formato pulito senza il tag e il timestamp
        Json::Value portable_data_clean;
        for(Json::ValueConstIterator it= user_data_store[hashed_password].begin(); it!= user_data_store[hashed_password].end(); ++it)
        {
            string key = it.name();
            const Json::Value& tagged_value_obj = *it;
            if(tagged_value_obj.isObject() && tagged_value_obj.isMember("value"))
            {
                portable_data_clean[key] = tagged_value_obj["value"];
            }
            else
            {
                portable_data_clean[key]= tagged_value_obj; // Se non ha la struttura {"value":..., "tag":...}, lo passiamo così com'è
            }
        }
        response["portable_data"]= portable_data_clean;
    }
    else
    {
        response["status"] = "error";
        response["gdpr_article"] = "20";
        response["message"] = "Errore interno: Utente non trovato dopo autenticazione. (Dovrebbe essere autenticato)";
    }
    return response;
}

// Articolo 21: Diritto di opposizione
Json::Value handle_article_21_objection(const string& hashed_password, const string& reason)
{
    Json::Value response;
    cout<<"[Art. 21] Gestione richiesta di opposizione per un utente, motivo: "<<reason<<endl; // Non stampare l'hash
    if(user_data_store.count(hashed_password))
    {
        if(user_data_store[hashed_password].isMember("consenso_marketing") && user_data_store[hashed_password]["consenso_marketing"].isMember("value"))
        {
            user_data_store[hashed_password]["consenso_marketing"]["value"]= false; // aggiorna il valore booleano
            cout<<"Consenso marketing per l'utente impostato a false."<<endl;
            save_data_to_file(); // Salva le modifiche sul file
        }
        response["status"]="success";
        response["gdpr_article"]="21";
        response["message"]= "Richiesta di opposizione per l'utente registrata. Trattamento interrotto per motivo "+ reason+ ".";
    }
    else
    {
        response["status"]="error";
        response["gdpr_article"] = "21";
        response["message"] = "Errore interno: Utente non trovato dopo autenticazione. (Dovrebbe essere autenticato)";
    }
    return response;
}
// Articolo 22: Diritto di non essere soggetto a processi decisionali automatizzati
Json::Value handle_article_22_automated_decision(const string& hashed_password)
{
    Json::Value response;
    cout << "[Art. 22] Gestione richiesta verifica decisioni automatizzate per un utente." << endl; // Non stampare l'hash
    if(user_data_store.count(hashed_password))
    {
        response["status"] = "success";
        response["gdpr_article"] = "22";
        response["message"]="Verifica dello stato delle decisioni automatizzate per l'utente.";
        response["automated_decision_status"]= "Non soggetto a decisioni basate unicamente su trattamenti automatizzati.";
    }
    else
    {
        response["status"] = "error";
        response["gdpr_article"] = "22";
        response["message"] = "Errore interno: Utente non trovato dopo autenticazione. (Dovrebbe essere autenticato)";
    }
    return response;
}

// Articolo 23: Esportazione Dati in file TXT
Json::Value handle_article_23_export_data(const string& hashed_password)
{
    Json::Value response;
    cout << "[Art. 23] Gestione richiesta di esportazione dati per un utente." << endl; // Non stampare l'hash

    if (user_data_store.count(hashed_password))
    {
        // Recupera l'email per il nome del file dall'oggetto utente stesso
        string user_email_for_filename = "unknown";
        if (user_data_store[hashed_password].isMember("email") &&
            user_data_store[hashed_password]["email"].isMember("value") &&
            user_data_store[hashed_password]["email"]["value"].isString()) {
            user_email_for_filename = user_data_store[hashed_password]["email"]["value"].asString();
            // Semplifica l'email per un nome file valido (es. mario.rossi@example.com -> mario_rossi_example_com)
            for (char &c : user_email_for_filename) {
                if (c == '.' || c == '@') {
                    c = '_';
                }
            }
        }


        string filename = "user_data_" + user_email_for_filename + ".txt";
        ofstream outfile(filename);

        if (outfile.is_open())
        {
            outfile << "Dati utente per l'email: " << user_email_for_filename << "\n\n";
            outfile << "Esportato il: " << get_current_timestamp_iso8601() << "\n\n";
            outfile << "----------------------------------------\n";

            const Json::Value& user_data = user_data_store[hashed_password];
            for (Json::ValueConstIterator it = user_data.begin(); it != user_data.end(); ++it)
            {
                string field_name = it.name();
                const Json::Value& tagged_value_obj = *it;

                outfile << field_name << ": ";
                if (tagged_value_obj.isObject() && tagged_value_obj.isMember("value"))
                {
                    // Estrai e scrivi solo il valore effettivo, non l'intero oggetto "taggato"
                    // Gestisci diversi tipi JSON per una stampa più leggibile
                    if (tagged_value_obj["value"].isString()) {
                        outfile << tagged_value_obj["value"].asString() << "\n";
                    } else if (tagged_value_obj["value"].isBool()) {
                        outfile << (tagged_value_obj["value"].asBool() ? "true" : "false") << "\n";
                    } else if (tagged_value_obj["value"].isNumeric()) {
                        outfile << tagged_value_obj["value"].asDouble() << "\n"; // Usa asDouble per numeri generali
                    } else if (tagged_value_obj["value"].isObject() || tagged_value_obj["value"].isArray()) {
                        // Per oggetti/array nidificati, stampali come una stringa JSON compatta
                        Json::StreamWriterBuilder writer;
                        writer["indentation"] = ""; // Nessuna indentazione per output compatto
                        outfile << Json::writeString(writer, tagged_value_obj["value"]) << "\n";
                    } else {
                        outfile << "N/A (tipo sconosciuto)\n";
                    }
                }
                else
                {
                    // Se non è una struttura di dati "taggata", stampa così com'è
                    Json::StreamWriterBuilder writer;
                    writer["indentation"] = "";
                    outfile << Json::writeString(writer, tagged_value_obj) << "\n";
                }
            }
            outfile << "----------------------------------------\n";
            outfile.close();

            response["status"] = "success";
            response["gdpr_article"] = "23";
            response["message"] = "Dati dell'utente esportati con successo nel file: " + filename;
            response["filename"] = filename;
        }
        else
        {
            response["status"] = "error";
            response["gdpr_article"] = "23";
            response["message"] = "Impossibile aprire o creare il file per l'esportazione dei dati.";
        }
    }
    else
    {
        response["status"] = "error";
        response["gdpr_article"] = "23";
        response["message"] = "Errore interno: Utente non trovato dopo autenticazione. (Dovrebbe essere autenticato)";
    }
    return response;
}


// Funzione principale server
int main(int argc, char const *argv[])
{
    initialize_retention_policies(); // Inizializza le policy di conservazione
    load_data_from_file(); // Carica i dati dal file, o inizializza se non presente
    save_data_to_file(); // AGGIUNTA: Salva i dati immediatamente all'avvio (anche se inizializzati dai campioni)

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt=1;
    socklen_t addrlen= sizeof(address);
    char buffer[4096]= {0};// Aumento il buffer per eventuali richieste/risposte grandi
    int port=8080;

    // Creazione del socket file descriptor
    if((server_fd= socket(AF_INET, SOCK_STREAM, 0))== 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forza il riutilizzo dell'indirizzo della porta per evitare "address already in use"
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;// Ascolta su tutte le interfacce disponibili
    address.sin_port=htons(port);

    // Bind del socket all'indirizzo e alla porta
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Mettiti in ascolto delle connessioni in entrata
    if(listen(server_fd, 10) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    cout << "GDPR TCP API Server con Tagging Dati in ascolto sulla porta " << port << "..." << endl;
    cout << "In attesa di richieste client..." << endl;

    while(true)
    {
        // Accetta una nuova connessione
        if ((new_socket= accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0)
        {
            perror("accept");
            continue; // Continua ad ascoltare nuove connessioni
        }
        cout<<"\n Connessione accettata da "<<inet_ntoa(address.sin_addr)<<":"<<ntohs(address.sin_port)<<endl;

        // Leggi i dati del client
        memset(buffer, 0, sizeof(buffer)); // Resetta il buffer per ogni nuova richiesta
        long valread = read(new_socket, buffer, sizeof(buffer)-1); // -1 per lasciare spazio al null terminator
        if(valread <= 0)
        {
            cerr<<"Nessun dato ricevuto o errore di lettura. Chiusura connessione client"<<endl;
            close(new_socket);
            continue;
        }
        buffer[valread] = '\0';

        string request_str(buffer);
        cout<<"Richiesta JSON ricevuta:\n"<<request_str<<endl;

        Json::Reader reader;
        Json::Value request_json;
        Json::Value response_json;
         // Parsing della richiesta JSON
        if (reader.parse(request_str, request_json)) {
            // --- INIZIO NUOVA LOGICA DI AUTENTICAZIONE ---
            if (request_json.isMember("gdpr_article") && request_json.isMember("password") && request_json.isMember("email")) {
                string gdpr_article = request_json["gdpr_article"].asString();
                string clear_password = request_json["password"].asString(); // Password in chiaro dal client
                string client_email = request_json["email"].asString();     // Email dal client

                string hashed_password_key = hash_password(clear_password); // HASH della password ricevuta

                // Verifica se la password hashata esiste nel nostro store
                if (user_data_store.count(hashed_password_key)) {
                    const Json::Value& stored_user_data = user_data_store[hashed_password_key];
                    // Verifica che l'email nel data store corrisponda all'email fornita dal client
                    if (stored_user_data.isMember("email") && stored_user_data["email"].isMember("value") &&
                        stored_user_data["email"]["value"].isString() &&
                        stored_user_data["email"]["value"].asString() == client_email)
                    {
                        // Autenticazione riuscita! Procedi con la gestione dell'articolo GDPR
                        if (gdpr_article == "15") {
                            response_json = handle_article_15_access(hashed_password_key);
                        } else if (gdpr_article == "16") {
                            if (request_json.isMember("updates") && request_json["updates"].isObject()) {
                                response_json = handle_article_16_rectification(hashed_password_key, request_json["updates"]);
                            } else {
                                response_json["status"] = "error";
                                response_json["gdpr_article"] = "16";
                                response_json["message"] = "Richiesta Articolo 16: campo 'updates' mancante o non valido.";
                            }
                        } else if (gdpr_article == "17") {
                            response_json = handle_article_17_erasure_with_check(hashed_password_key);
                        } else if (gdpr_article == "18") {
                            if (request_json.isMember("reason") && request_json["reason"].isString()) {
                                response_json = handle_article_18_restriction(hashed_password_key, request_json["reason"].asString());
                            } else {
                                response_json["status"] = "error";
                                response_json["gdpr_article"] = "18";
                                response_json["message"] = "Richiesta Articolo 18: campo 'reason' mancante o non valido.";
                            }
                        } else if (gdpr_article == "20") {
                            response_json = handle_article_20_portability(hashed_password_key);
                        } else if (gdpr_article == "21") {
                            if (request_json.isMember("reason") && request_json["reason"].isString()) {
                                response_json = handle_article_21_objection(hashed_password_key, request_json["reason"].asString());
                            } else {
                                response_json["status"] = "error";
                                response_json["gdpr_article"] = "21";
                                response_json["message"] = "Richiesta Articolo 21: campo 'reason' mancante o non valido.";
                            }
                        } else if (gdpr_article == "22") {
                            response_json = handle_article_22_automated_decision(hashed_password_key);
                        } else if (gdpr_article == "23") {
                            response_json = handle_article_23_export_data(hashed_password_key);
                        }
                        else {
                            response_json["status"] = "error";
                            response_json["message"] = "Articolo GDPR '" + gdpr_article + "' non riconosciuto o non gestito.";
                        }
                    } else {
                        // Email non corrispondente
                        response_json["status"] = "error";
                        response_json["message"] = "Autenticazione fallita: L'email fornita non corrisponde a quella dell'utente.";
                    }
                } else {
                    // Password hashata non trovata
                    response_json["status"] = "error";
                    response_json["message"] = "Autenticazione fallita: Password o email non validi.";
                }
            } else {
                response_json["status"] = "error";
                response_json["message"] = "Formato JSON non valido: campi 'gdpr_article', 'password' o 'email' mancanti nella richiesta.";
            }
            // --- FINE NUOVA LOGICA DI AUTENTICAZIONE ---
        } else {
            response_json["status"] = "error";
            response_json["message"] = "Errore di parsing JSON della richiesta.";
        }

        // Serializzazione della risposta JSON
        Json::StreamWriterBuilder writer;
        writer["indentation"] = "\t"; // Per una bella formattazione
        string response_str = Json::writeString(writer, response_json);

        // Invia la risposta al client
        send(new_socket, response_str.c_str(), response_str.length(), 0);
        cout << "Risposta JSON inviata:\n" << response_str << endl;

        // Chiudi il socket del client
        close(new_socket);
    }

    // Chiudi il socket del server (questo codice non verrà raggiunto nel loop infinito)
    close(server_fd);
    return 0;
}