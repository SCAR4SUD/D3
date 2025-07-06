# Framework Security Testing OWASP (WSTF)

## 0 · Il framework in sintesi

Il WSTF distribuisce il lavoro di sicurezza **in cinque fasi dello SDLC** invece di confinarlo a un unico penetration test finale.  
Ogni fase ha un **“gate”**: la release non avanza finché i criteri di uscita non sono soddisfatti.

---

## Fase 1 — Prima che inizi lo sviluppo

| Sotto‑fase | Obiettivi chiave | Deliverable | Strumenti consigliati |
|-----------|-----------------|-------------|-----------------------|
| **1.1 Definire un SDLC** | Formalizzare il modello di sviluppo e rendere la sicurezza un attributo di qualità intrinseco. | Documento di politica SDLC; standard di codice sicuro. | Analisi gap ISO/IEC 33020; template SDL. |
| **1.2 Rivedere policy & standard** | Verificare l’esistenza di standard di linguaggio e crittografia; colmare eventuali lacune. | Secure coding standard (Java, .NET, Go…), baseline crittografica, policy di gestione dipendenze. | OWASP ASVS; NIST SP 800‑218 (SSDF). |
| **1.3 Definire metriche** | Decidere **prima del codice** cosa misurare (es. densità vulnerabilità, MTTR) e come tracciare. | Piano di misurazione; campi Jira; schema dashboard. | SonarQube Quality Gates; OWASP DefectDojo. |

### Criteri di successo
* Almeno un KPI di sicurezza misurabile per *release train*.  
* Policy approvate dall’architecture board.

### Esempi di metriche
* % user story con abuso‑case collegato.  
* Difetti critici < 0,3/KSLOC nelle scansioni pre‑merge.

---

## Fase 2 — Durante definizione e design

### 2.1 Revisione requisiti di sicurezza
Controlla completezza e univocità di:
* gestione utenti & autenticazione
* autorizzazione
* confidenzialità & integrità dei dati
* accountability & gestione sessioni
* sicurezza del trasporto
* segregazione a livelli
* conformità legale (privacy, settore, ecc.)

**Deliverable:** Security Story Map firmata; catalogo di Abuso‑Case; test di accettazione.

### 2.2 Revisione design & architettura
Valuta C4/ADRs/DFD: cerca decisioni authZ distribuite e punti di validazione; proponi servizi centrali dove possibile.

**Strumenti:** Threat‑Dragon, IriusRisk, ArchiMate.

### 2.3 Creare & rivedere modelli UML
Assicurati che i diagrammi riflettano i flussi reali; sincronizza con il threat model.

### 2.4 Creare & rivedere threat model
Metodo STRIDE‑per‑elemento o PASTA. **Report:** rischi, contromisure, rischi residui firmati dal business.

### Gate d’uscita
* Tutte le minacce **HIGH/Critical** hanno mitigazione o accettazione firmata.  
* Verbale di arch‑review in Confluence.

---

## Fase 3 — Durante lo sviluppo

| Attività | Cosa fare | Tool tipici |
|---------|-----------|-------------|
| **3.1 Code walkthrough** | Dev & Security analizzano la feature branch: logica e flusso, non sintassi. | Pair programming, review ADR. |
| **3.2 Static code review** | Confronta il codice con requisiti CIA, OWASP Top 10, pitfall di linguaggio, normative di settore. | Semgrep, CodeQL, SonarQube, Talisman (secret scan). |

> L’analisi statica è il miglior compromesso costo/beneficio e dipende poco dall’abilità del tester.

### Hook CI
* **Pre‑commit:** scanner segreti.  
* **Pull request:** SAST + dependency check.  
* **Nightly:** SCA ramo completo (Syft + Grype).

### Gate d’uscita
* Nessun issue SAST **Critical** aperto.  
* Issue **High** collegate a ticket Jira con owner.

---

## 5 · Fase 4 — Durante il deployment

1. **4.1 Penetration test applicativo** – Black/grey‑box su ambiente «production like»; include abusi di logica e fuzzing API.  
2. **4.2 Test di configuration management** – Esamina IaC, immagini container e runtime config; verifica benchmark CIS e rotazione segreti.

### Controlli in pipeline
* Fai fallire il deploy se l’immagine container viola policy > Medium in Trivy/Kube‑Bench.  
* Genera automaticamente SBOM e firma artefatti (Sigstore).

### Gate d’uscita
* Report pentest: nessuna finding **Critical/High** sfruttabile.  
* Tutte le config in VCS e peer‑review.

---

## Fase 5 — Manutenzione & operazioni

| Sotto‑fase | Focus operativo | Artefatti |
|-----------|----------------|-----------|
| **5.1 Operational management review** | Patching, rotazione segreti, runbook incidenti. | SOP di sicurezza Ops. |
| **5.2 Health check periodici** | Riesami mensili/trimestrali di app e infrastruttura. | DAST automatico; report di drift compliance. |
| **5.3 Verifica dei change** | Test di sicurezza integrati nel change‑management e smoke test post‑deploy. | Ticket change con sign‑off sicurezza. |

---

## Consigli di implementazione Agile & DevSecOps

* Definisci una **“Security Definition of Done”** e collegala al backlog di sprint.  
* Nomina un **Security Champion** per squadra; ruota ogni trimestre.  
* Automatizza tutto il possibile; riserva i test manuali alla logica di business.  
* Imposta **librerie "paved‑road"** pre‑configurate in modo sicuro.  
* Conduci *post‑mortem* senza colpe per ogni vulnerabilità sfuggita.

---

## Glossario rapido

* **ASVS** – Application Security Verification Standard di OWASP.  
* **SAST / DAST / IAST** – Static / Dynamic / Interactive Application Security Testing.  
* **SBOM** – Software Bill of Materials.  
* **SCA** – Software Composition Analysis.  
* **STRIDE** – Spoofing, Tampering, Repudiation, Information disclosure, Denial of service, Elevation of privilege.

---

## Altre risorse consultabili

* OWASP Application Security Verification Standard 4.0.  
* NIST Secure Software Development Framework SP 800‑218.  
* Documentazione Microsoft Threat Modeling Tool. 
* Automotive ISAC ATM (Automotive Threat Matrix)

---

## Top 10 vulnerabilità in ambito automotive secondo OWASP

| # | Vulnerabilità | Descrizione sintetica |
|---|---------------|-----------------------|
| 1 | **Protocolli di comunicazione veicolo deboli** | I bus (es. CAN) non dispongono di autenticazione/cripto robuste → possibili comandi non autorizzati a freni, sterzo, ecc. |
| 2 | **Aggiornamenti Over‑the‑Air (OTA) insicuri** | Mancano autenticazione e cifratura negli update OTA → un aggressore può iniettare firmware malevolo. |
| 3 | **Sistemi telematici insicuri** | Unità telematica o API cloud con controlli insufficienti → accesso remoto a dati o impostazioni veicolo. |
| 4 | **Vulnerabilità nella supply chain software** | Componenti terze parti vulnerabili nell’infotainment o ECU → esecuzione di codice arbitrario. |
| 5 | **Exploit con accesso fisico** | Tramite porta OBD‑II o altri connettori si possono alterare parametri o firmware. |
| 6 | **Meccanismi di controllo accessi inadeguati** | Policy debole nei servizi o app mobile → escalation di privilegi non autorizzata. |
| 7 | **Meccanismi di autenticazione implementati male** | Password deboli o token prevedibili su app/servizi remoti → takeover account. |
| 8 | **Perdite di dati e violazioni privacy** | Canali di trasmissione o storage cloud non sicuri → esposizione di posizione e dati personali. |
| 9 | **Mancanza di sicurezza nei sistemi integrati** | Integrazione (infotainment, navigazione) senza confini chiari → pivot verso sistemi critici. |
|10 | **Sistemi legacy insicuri** | Vecchi modelli o ECU non patchati con protocolli obsoleti → sfruttamento di vulnerabilità note. |

---

## Reference

* The OWASP Testing Framework https://owasp.org/www-project-web-security-testing-guide/v42/3-The_OWASP_Testing_Framework/0-The_Web_Security_Testing_Framework
* Top 10 Automotive Security Vulnerabilities https://cheatsheetseries.owasp.org/cheatsheets/Automotive_Security.html


