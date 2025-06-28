# SonarQube C/C++ + MISRA C 2012 — End‑to‑End Setup Guide

> **Audience ↠** DevOps engineers, embedded C/C++ developers, and project leads who want **clean code, standards compliance, and automated quality gates** in every pull request. The guide starts from *zero* and ends with **fully automated CI/CD pipelines** on the four most common platforms (GitHub Actions, GitLab CI/CD, Jenkins, Azure DevOps).

---

## 1 ▸ Prerequisites

| Requirement           | Min. Version          | Purpose                            |
| --------------------- | --------------------- | ---------------------------------- |
| **Java JDK**          | **17** (LTS)          | Runs SonarQube server & Scanner    |
| **Docker Engine**     | 24 or newer           | Simplest server deployment         |
| **Docker Compose**    | v2 plugin             | Or use Podman Compose equivalently |
| **Git**               | Any recent            | CI examples assume a Git repo      |
| **Build Tool**        | `make`, CMake, Ninja… | Needed to wrap the build           |

---

## 2 ▸ Deployment Options

### 2.1 Docker Compose **(recommended)**

Create *docker-compose.yml* at repo root:

```yaml\iversion:
services:
  sonarqube:
    image: sonarqube:lts
    container_name: sonarqube
    restart: unless-stopped
    hostname: sonarqube
    ports:
      - '9000:9000'
    environment:
      SONAR_JDBC_URL: 'jdbc:postgresql://db:5432/sonarqube'
      SONAR_JDBC_USERNAME: 'sonar'
      SONAR_JDBC_PASSWORD: 'sonar'
      # JVM tuning for 4 GB host
      SONAR_WEB_JAVAOPTS: '-Xms512m -Xmx2048m'
    volumes:
      - sonarqube_data:/opt/sonarqube/data
      - sonarqube_extensions:/opt/sonarqube/extensions
    depends_on:
      - db

  db:
    image: postgres:15-alpine
    container_name: sonar-db
    restart: unless-stopped
    environment:
      POSTGRES_USER: 'sonar'
      POSTGRES_PASSWORD: 'sonar'
      POSTGRES_DB: 'sonarqube'
    volumes:
      - postgresql:/var/lib/postgresql/data
volumes:
  sonarqube_data:
  sonarqube_extensions:
  postgresql:
```

Launch:

```bash
sudo docker compose up -d
```

First‑time startup may take 1‑2 minutes while plugins are unpacked.

### 2.2 Bare‑metal / VM install

1. **Download** the matching bundle from [https://www.sonarqube.org/downloads/](https://www.sonarqube.org/downloads/).
2. **Create** a dedicated system user `sonar`.
3. **Place** the unzipped folder under */opt/sonarqube* and `chown -R sonar:sonar`.
4. **Configure** database in */opt/sonarqube/conf/sonar.properties*.
5. **Systemd** unit example:
   ```ini
   [Unit]
   Description=SonarQube service
   After=network.target

   [Service]
   Type=forking
   ExecStart=/opt/sonarqube/bin/linux-x86-64/sonar.sh start
   ExecStop=/opt/sonarqube/bin/linux-x86-64/sonar.sh stop
   User=sonar
   LimitNOFILE=65536
   Restart=on-failure

   [Install]
   WantedBy=multi-user.target
   ```
6. `sudo systemctl enable --now sonarqube`.

---

## 3 ▸ First‑Time Configuration

1. Browse to [http://localhost:9000](http://localhost:9000) → **Login** `admin` / `admin` → change password.
2. **License**: C/C++ analysis (CFamily engine) requires **Developer Edition** or higher. Activate a trial on **Administration ▸ License Manager**.
3. **Update plugins** (Administration ▸ Marketplace) until no pending updates.
4. **Create** a **Project Key** (e.g. `my‑firmware`).
5. **Generate** a long‑lived **User Token** (`Settings ▸ Security ▸ Tokens`). Store as `SONAR_TOKEN` secret in your CI platform.
6. **Define Quality Profile**
   - Quality Profiles ▸ Languages ▸ C / C++
   - *Duplicate* the built‑in profile ↠ rename to `MISRA‑C 2012 Strict`.
   - *Search* → \*\*Tag = \*\*\`\` → **Bulk Select ▸ Activate**.
   - Optionally deactivate rules not relevant to your hardware target.
7. **Set Default** profile for C/C++ so every new project inherits it.

---

## 4 ▸ Local Analysis Workflow

### 4.1 Install SonarScanner CLI

```bash
# Linux
wget https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-5.0.1.3006-linux.zip
unzip sonar-scanner-cli-*.zip -d /opt
export PATH="$PATH:/opt/sonar-scanner-5.0.1.3006-linux/bin"
```

### 4.2 Download Build Wrapper

- Linux x64: [https://sonar-cxx-build-wrapper.s3.amazonaws.com/build-wrapper-linux-x86.zip](https://sonar-cxx-build-wrapper.s3.amazonaws.com/build-wrapper-linux-x86.zip)

```bash
unzip build-wrapper-linux-x86.zip -d /opt
```

### 4.3 `sonar-project.properties` template

```properties
sonar.projectKey=my-firmware
sonar.projectName=My Firmware
sonar.projectVersion=1.0.0
sonar.sources=src,include
sonar.cfamily.build-wrapper-output=bw-output
# Exclude generated or third‑party code
sonar.exclusions=src/3rd_party/**/*
sonar.host.url=http://localhost:9000
sonar.login=${SONAR_TOKEN}
```

### 4.4 Run analysis locally

```bash
# 1️⃣ wrap your build
/opt/build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir bw-output \
  make clean all
# 2️⃣ scan
sonar-scanner
```

Results appear in the web UI within \~30 seconds.

---

## 5 ▸ CI/CD Integration

> All pipelines share the same three secrets: *`SONAR_HOST_URL`* (usually `https://sonar.example.com`), *`SONAR_TOKEN`* (user token created above), and optional *`BUILD_WRAPPER`* path if not checking the wrapper into the repo.

### 5.1 GitHub Actions — `.github/workflows/ci.yml`

```yaml
name: C /C++ CI with Sonar
on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install deps
        run: sudo apt-get update && sudo apt-get install -y build-essential make cmake
      - name: Download build-wrapper
        run: |
          curl -sSL -o bw.zip https://sonar-cxx-build-wrapper.s3.amazonaws.com/build-wrapper-linux-x86.zip
          unzip bw.zip
      - name: Build (wrapped)
        run: ./build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir bw-output make -j$(nproc)
      - name: Sonar Scan
        env:
          SONAR_TOKEN: ${{ secrets.SONAR_TOKEN }}
        run: |
          curl -sSL -o scanner.zip https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-5.0.1.3006-linux.zip
          unzip scanner.zip
          ./sonar-scanner-*/bin/sonar-scanner \
            -Dsonar.host.url=${{ secrets.SONAR_HOST_URL }} \
            -Dsonar.login=${{ secrets.SONAR_TOKEN }}
```

### 5.2 GitLab CI — `.gitlab-ci.yml`

```yaml
stages: [build, scan]

variables:
  SONAR_USER_HOME: "$CI_PROJECT_DIR/.sonar"  # cache scanners

build:
  stage: build
  image: gcc:14
  script:
    - apt-get update && apt-get install -y wget unzip make
    - wget -qO bw.zip https://sonar-cxx-build-wrapper.s3.amazonaws.com/build-wrapper-linux-x86.zip && unzip bw.zip
    - ./build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir bw-output make -j$(nproc)
  artifacts:
    paths:
      - bw-output

sonar_scan:
  stage: scan
  image: openjdk:17
  dependencies: [build]
  script:
    - wget -qO scanner.zip https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-5.0.1.3006-linux.zip && unzip scanner.zip
    - ./sonar-scanner-*/bin/sonar-scanner -Dsonar.host.url=$SONAR_HOST_URL -Dsonar.login=$SONAR_TOKEN
```

### 5.3 Jenkins (Declarative Pipeline)

```groovy
pipeline {
  agent any
  environment {
    SONAR_HOST_URL = 'https://sonar.example.com'
    SONAR_TOKEN    = credentials('sonar-token')
  }
  stages {
    stage('Checkout') { steps { checkout scm } }
    stage('Build + Wrap') {
      steps {
        sh '''
        curl -sSL -o bw.zip https://sonar-cxx-build-wrapper.s3.amazonaws.com/build-wrapper-linux-x86.zip
        unzip bw.zip
        ./build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir bw-output make -j$(nproc)
        '''
      }
    }
    stage('SonarQube Analysis') {
      steps {
        withSonarQubeEnv('My Sonar') {
          sh 'sonar-scanner -Dsonar.cfamily.build-wrapper-output=bw-output'
        }
      }
    }
    stage('Quality Gate') {
      steps {
        waitForQualityGate abortPipeline: true
      }
    }
  }
}
```

### 5.4 Azure DevOps — `azure-pipelines.yml`

```yaml
trigger:
  branches: { include: ['main'] }

pool: { vmImage: 'ubuntu-latest' }

steps:
  - checkout: self
  - bash: |
      sudo apt-get update && sudo apt-get install -y build-essential
      curl -sSL -o bw.zip https://sonar-cxx-build-wrapper.s3.amazonaws.com/build-wrapper-linux-x86.zip
      unzip bw.zip
      ./build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir bw-output make -j$(nproc)
    displayName: 'Build and wrap'
  - task: SonarQubePrepare@5
    inputs:
      SonarQube: 'SonarCloud'
      scannerMode: 'CLI'
      configMode: 'manual'
      cliProjectKey: 'my-firmware'
      cliProjectName: 'My Firmware'
      extraProperties: |
        sonar.cfamily.build-wrapper-output=bw-output
  - task: SonarQubeAnalyze@5
  - task: SonarQubePublish@5
    inputs:
      pollingTimeoutSec: '300'
```

> 🏁 Each pipeline halts on a failed **Quality Gate**, ensuring MISRA compliance before merge.

---

## 6 ▸ Quality Gates & Badges

1. **Create** a gate: Administration ▸ Quality Gates ▸ ➕ .
2. Recommended conditions for safety‑critical code:
   - New Code Reliability Rating ≤ A
   - New Code Security Rating ≤ A
   - New Code Maintainability Rating ≤ A
   - New Code Coverage ≥ 80 %
   - New Code MISRA‑C Violations = 0
3. Add badge in main README:
   ```markdown
   ![Quality Gate](https://sonar.example.com/api/project_badges/quality_gate?projectKey=my-firmware)
   ```

---

## 7 ▸ Upgrading & Maintenance

| Task               | Frequency | Command                               |
| ------------------ | --------- | ------------------------------------- |
| **Backup DB**      | Daily     | `pg_dump -Fc sonarqube > backup.sqlc` |
| **Plugin updates** | Monthly   | GUI ▸ Marketplace                     |
| **Server upgrade** | LTS → LTS | Replace image tag & run DB migration  |
| **Prune logs**     | Weekly    | `docker system prune -f`              |

---

## 8 ▸ Security Hardening Checklist

- Terminate TLS in front of SonarQube (Traefik / Nginx Ingress).
- Restrict UI access to VPN or corporate SSO.
- Rotate user tokens every 90 days.
- Use OpenID Connect for authentication (Settings ▸ Authentication).
- Enforce strong password policy and disable anonymous project creation.
- Configure Webhook to Slack/MS Teams for failed Quality Gates.

---

## 9 ▸ Troubleshooting & FAQ

| Symptom                           | Likely Cause                 | Fix                                |
| --------------------------------- | ---------------------------- | ---------------------------------- |
| `Web server is down`              | JVM lacks RAM                | Increase `-Xmx`, allocate 3+ GB    |
| `sonar-scanner: Not authorized`   | Wrong token / missing secret | Double‑check `SONAR_TOKEN`         |
| `No analyses have been performed` | Forgot build wrapper         | Verify `bw-output` folder uploaded |
| `CE task timed out`               | Large project & low CPU      | Add `worker threads=2`, scale CPU  |
| `Rule can not be activated`       | Using Community Edition      | Switch to Developer+               |

---

## 10 ▸ Adding custom rules

For an official, step‑by‑step guide on creating, packaging, and distributing custom coding rules, consult the SonarSource documentation: https://docs.sonarsource.com/sonarqube-server/latest/extension-guide/adding-coding-rules/

By following that guide you can extend SonarQube with your own rules in C, C++, Java, and any other language supported by the platform.

---

## 11 ▸ References & Further Reading

- Official docs: [https://docs.sonarsource.com/sonarqube/latest/](https://docs.sonarsource.com/sonarqube/latest/)
- MISRA‑C 2012 summary: [https://www.misra.org.uk](https://www.misra.org.uk)
- Sonar CFamily engine: [https://docs.sonarsource.com/cpp/](https://docs.sonarsource.com/cpp/)
- Sample open‑source firmware: [https://github.com/ST‑Microelectronics/STM32Cube](https://github.com/ST‑Microelectronics/STM32Cube)

---

© 2025 — Feel free to copy‑adapt under the MIT License.

