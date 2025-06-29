# SCAR4SUD – Secure Software Development Policy (RFC 2119 Summary)

## 1. Politica di Secure Software Development
* Il manufacturer **MUST** definire, formalizzare e applicare un **Secure Development Lifecycle (SDL)** per l’intero SDLC, indipendentemente dal modello di processo (SCRUM, Waterfall).
* Per ogni progetto **MUST** essere scelti standard di riferimento (es. MISRA 2012, AUTOSAR, CERT C/C++).
* Il processo **MUST** includere requisiti di sicurezza (confidenzialità, integrità, disponibilità) già in fase di analisi e creare un **Threat Modeling** iniziale (es. STRIDE).

## 2. Fasi chiave del SDL
| Fase | Requisiti (MUST / SHOULD / MAY) |
|------|---------------------------------|
| **Analysis** | Il team **MUST** raccogliere requisiti funzionali e non funzionali; **SHOULD** aggiornare il threat model ad ogni variazione di scopo. |
| **Design** | L’architettura **MUST** incorporare principi di *least‑privilege* e *defense‑in‑depth*; modifiche **MUST** essere tracciate e revisionate periodicamente. |
| **Implementation** | Il codice **MUST** rispettare secure‑coding rules; branch protetti **MUST** essere usati; tool di SAST/DAST **MUST** girare in CI/CD con policy bloccanti. |
| **Verification & Test** | Copertura test **MUST** ≥ 80 % (≥ 90 % per safety‑critical); penetration test su ogni endpoint **MUST** essere pianificati; mutation score **SHOULD** ≥ 80 %. |
| **Release & Deployment** | Gli artefatti **MUST** essere firmati digitalmente; checklist di hardening **SHOULD** precedere ogni rilascio; patch delivery **MUST** avvenire via processo sicuro (es. OTA). |
| **Operations** | Processo di vulnerability disclosure **MUST** esistere; patching **MUST** rispettare gli SLA: Critical 7 g, High 30 g, Medium 90 g, Low 180 g. |

## 3. Ruoli & Responsabilità
* **Security Champion** **MUST** guidare la sicurezza a livello di team e **MUST** assicurare il rispetto delle policy.  
* **QA Lead** **SHOULD** garantire test e metriche di qualità.  
* **Release Manager** **MUST** mantenere la **Build Blocking Rate** < 5 % e validare i controlli finali.  
* **Developer** **MUST** applicare secure‑coding e **SHOULD** sanare violazioni prima del merge.  
* **Auditor Interno** **MUST** condurre audit semestrali secondo ISO 19011 e **MUST** chiudere ≥ 90 % delle non‑conformità entro 60 giorni.

## 4. Metriche di Performance
| Metrica | Soglia **MUST** | Owner |
|---------|-----------------|-------|
| Code Coverage | ≥ 90 % safety‑critical; ≥ 80 % altro | QA Lead |
| Mutation Score | ≥ 80 % | QA Lead |
| Density Defects | < 0.4 / KLOC | Dev Lead |
| Build Blocking Rate | < 5 % | Release Manager |
| False Positive Rate | < 15 % | Release Manager |
| Training Completati | ≥ 90 % | Security Champion |
| Finding Closure Rate | ≥ 90 % entro 60 gg | Auditor Interno |
| Audit Schedule Adherence | 100 % | CISO / Security Champion / Auditor |

Le metriche **MUST** essere monitorate in modo continuativo; deviazioni **SHOULD** innescare azioni correttive.

## 5. Formazione
Tutti i ruoli coinvolti **MUST** completare la formazione iniziale su Secure Coding & Threat Modeling e **SHOULD** ricevere aggiornamenti annuali (o semestrali se richiesto da normative automotive).

## 6. Audit
* Audit interni **MUST** essere pianificati su base semestrale e approvati dal CISO.  
* Audit esterni **SHOULD** includere evidenze di robustezza safety‑security.

## 7. Gestione Fornitori
* Ogni fornitore **MUST** compilare un Security Questionnaire.  
* I contratti **SHOULD** includere clausole di disclosure tempestiva di CVE.

## 8. Secure Coding Guidelines
* Principi di **Secure‑by‑Design**, *Fail‑Safe Defaults*, *Complete Mediation* e *Least Privilege* **MUST** essere seguiti.  
* Le regole catalogate come **MUST** (bloccanti), **SHOULD** (fortemente consigliate) o **MAY** (opzionali) **MUST** essere integrate nello SAST.  
* La checklist di code review **MUST** includere:  
  - Validazione input  
  - Error handling  
  - Uso di cryptography sicura  
  - Gestione segreti centralizzata  
  - Risoluzione di tutte le violazioni Critical/High prima del merge

---

Questo documento traduce i requisiti SCAR4SUD in termini formali RFC 2119 per facilitarne l’adozione nei processi aziendali.
