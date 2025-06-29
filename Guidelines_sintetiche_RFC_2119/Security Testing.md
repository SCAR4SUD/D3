# Security Testing – Regole in conformità a ISO/IEC 27002

## 1. Governance e Policy
1. **POL-ST-01 – Politica di Security Testing**  
   * **MUST** esistere un documento approvato dal CISO che definisce: scopo, ambito applicativo, ruoli, severità/classificazione delle vulnerabilità e tempistiche di remediation.  
   * **MUST** essere riesaminato almeno annualmente o dopo incidenti / major release.

2. **POL-ST-02 – Ruoli e Responsabilità**  
   * *Dev Team* → esegue test statici, revisione codice e correzioni.  
   * *Security Team* → supervisiona, fornisce tool, conduce DAST e pen-test.  
   * *QA* → verifica criteri di accettazione sicurezza prima del Go-Live.  
   * *Change Advisory Board (CAB)* → valuta esiti test in fase di rilascio.

---

## 2. Pianificazione basata sul rischio
3. **RISK-ST-01 – Threat Modeling iniziale**  
   * **MUST** essere effettuato per ogni nuovo progetto o modifica di rilievo (> 25 % LOC).  
   * **SHOULD** usare metodologie STRIDE, LINDDUN o OWASP Threat Dragon.

4. **RISK-ST-02 – Piano di Test di Sicurezza**  
   * **MUST** derivare dal threat modeling e indicare copertura minima (es. 100 % delle API critiche), tool, responsabilità e metriche.  
   * **MUST** essere approvato prima di scrivere codice.

---

## 3. Integrazione “Shift-Left” nel SDLC
5. **SDLC-ST-01 – Commit Gate**  
   * **MUST** includere SAST e software-composition-analysis (SCA) automatici su ogni push.  
   * La build fallisce se vulnerabilità ad alta criticità non sono sanate o dispensate.

6. **SDLC-ST-02 – Build & CI/CD**  
   * **MUST** eseguire DAST su ambienti di staging tramite pipeline; risultati archiviati.  
   * **SHOULD** integrare test IaC, container scan e fuzzing per servizi esposti.

7. **SDLC-ST-03 – Pre-Production Gate**  
   * **MUST** prevedere penetration test manuale o red-team mirato per release major o applicazioni “alto impatto” (dati personali, pagamento).  
   * Go-Live consentito solo dopo chiusura/accettazione delle issue critiche.

---

## 4. Ambiente di test e dati
8. **ENV-ST-01 – Separazione degli ambienti**  
   * **MUST** esserci segregazione fisica o logica tra produzione, test e sviluppo.  
   * Accesso privilegiato con MFA e logging centralizzato.

9. **ENV-ST-02 – Protezione dei dati di test** (controllo 8.29)  
   * **MUST** usare dataset sintetici o mascherati; vietata copia full-prod.  
   * **SHOULD** applicare crittografia *at-rest* e *in-transit* alle basi di test.

---

## 5. Esecuzione dei test
10. **EXEC-ST-01 – Tipi di test minimi**

| Fase          | Test obbligatori         | Tool / Tecniche consigliate                       |
| ------------- | ------------------------ | ------------------------------------------------- |
| Code → Build  | SAST, SCA               | Semgrep, SonarQube, GitHub Dependabot             |
| Staging       | DAST, IAST              | OWASP ZAP, Burp Suite, Contrast                   |
| Pre-Prod      | Pen-test, fuzzing mirato | Kali / Pwntools, Jazzer                           |

Le deviazioni **MUST** essere documentate e approvate dal Security Lead.

11. **EXEC-ST-02 – Indipendenza dei tester**  
    * **MUST** esserci separazione di compiti: chi scrive il codice non approva i propri test di sicurezza critici.  
    * Pen-test **SHOULD** essere eseguito da team esterno almeno una volta l’anno.

---

## 6. Gestione dei risultati
12. **RES-ST-01 – Tracking vulnerabilità**  
    * **MUST** utilizzare un unico sistema (es. Jira Security) collegato alla pipeline.  
    * Ogni finding include CWE/CVSS, evidenza e raccomandazione di fix.

13. **RES-ST-02 – SLA di remediation**

| Gravità (CVSS)       | Tempo massimo di correzione |
| -------------------- | --------------------------- |
| ≥ 9.0 (Critica)      | 7 giorni di calendario      |
| 7.0 – 8.9 (Alta)     | 30 giorni                   |
| 4.0 – 6.9 (Media)    | 90 giorni                   |
| < 4.0 (Bassa)        | Nel backlog ordinario       |

Eccezioni **MUST** essere motivate e firmate da CISO o equivalente.

---

## 7. Metriche e reporting
14. **MET-ST-01 – Indicatori chiave**  
    * **MUST** misurare:  
      * Percentuale di build fallite per vulnerabilità > Alta.  
      * Tempo medio di remediation vs. SLA.  
      * Copertura test di sicurezza (LOC, API, componenti).  
    * Report mensile alla direzione; il trend **SHOULD** alimentare il programma di miglioramento continuo.

---

## 8. Tooling e manutenzione
15. **TOOL-ST-01 – Registro Tool**  
    * **MUST** elencare versioni, owner, licenze, date di aggiornamento e rad/rollback plan.  
    * Aggiornamenti critici di firma (es. SCA) **MUST** avvenire ≤ 24 h dal rilascio.

---

## 9. Formazione e consapevolezza
16. **TRN-ST-01 – Training obbligatorio**  
    * Dev e QA: **MUST** corso OWASP Top-10, MISRA 2012, altri corsi specifici nel settore automotive, secure coding entro 3 mesi dall’assunzione e refresh annuale.  
    * Security Champions in ogni squadra **SHOULD** ricevere training avanzato (SANS, OSWE, etc.).

---

## 10. Gestione dei fornitori
17. **SUP-ST-01 – Requisiti contrattuali**  
    * Contratti di sviluppo esterno **MUST** imporre il rispetto delle regole di questo documento e permettere audit.  
    * Fornitori critici **SHOULD** fornire report SAST/DAST prima della consegna.

---

## 11. Cambio, rilascio e accettazione
18. **REL-ST-01 – Security-Go/No-Go**  
    * CAB **MUST** verificare checklist di sicurezza firmata (test superati, SLA rispettati, documentazione aggiornata).  
    * Quality **MAY** bloccare il rilascio, se individua metriche non del tutto conformi
    * In caso di “No-Go” da parte del team di Quality il rilascio è bloccato finché le non-conformità non siano risolte o formalmente accettate.

---

## 12. Conservazione evidence e audit
19. **AUD-ST-01 – Log e report**  
    * Evidenze di test, scanner output, pen-test report e approvazioni **MUST** essere conservate ≥ 3 anni; accesso solo “need-to-know”.

---

## 13. Continual Improvement
20. **IMP-ST-01 – Post-mortem & Lessons Learned**  
    * Dopo ogni incidente o fail di release **MUST** esserci review di causa radice e aggiornamento processo/policy entro 30 giorni.

---

## 14. Allineamento normativo e interoperabilità
21. **REG-ST-01 – Mappature**  
    * **MUST** essere mantenuto un documento di tracciabilità che colleghi ogni regola ai controlli ISO 27002, OWASP SAMM, NIST SSDF, NIS2.  
    * Utile in audit per dimostrare copertura end-to-end.