# FIAP - Faculdade de Informática e Administração Paulista

<p align="center">
<a href= "https://www.fiap.com.br/"><img src="assets/logo-fiap.png" alt="FIAP - Faculdade de Informática e Admnistração Paulista" border="0" width=40% height=40%></a>
</p>

<br>

# 💓 CardioIA – Monitoramento Contínuo (Fase 3)

## Nome do grupo
Grupo 6 

## 👨‍🎓 Integrantes: 
- [Ana Beatriz Duarte Domingues](https://www.linkedin.com/in/)
  **RM**: 560302
- [Junior Rodrigues da Silva](https://www.linkedin.com/in/jrsilva051/)
  **RM**: 559451
- [Carlos Emilio Castillo Estrada](https://www.linkedin.com/in/)
  **RM**: 559855

## 👩‍🏫 Professores:
### Tutor(a)
- [Lucas Gomes Moreira](https://www.linkedin.com/company/inova-fusca)
### Coordenador(a)
- [André Godoi Chiovato](https://www.linkedin.com/company/inova-fusca)

---

## 📜 Descrição
Este repositório reúne as **Partes 1 e 2** da Fase 3 do projeto **CardioIA**, que simula um sistema vestível de **monitoramento cardíaco com IoT**.  
O objetivo é demonstrar o ciclo completo de **coleta, armazenamento, transmissão e visualização de dados de saúde**, utilizando o **ESP32** no ambiente **Wokwi**.

---

## 📂 Estrutura do Repositório

Dentre os arquivos e pastas presentes na raiz do projeto, definem-se:

- **assets/** → imagens utilizadas na documentação do projeto.  
- **src/** → código-fonte desenvolvido para esta fase.  
- `Relatório – Projeto CardioIA Parte 1.pdf` → Relatório da Parte 1.
- `Relatório – Projeto CardioIA Parte 2.pdf` → Relatório da Parte 2 (MQTT + Dashboard).
- `link_projeto_wokwi` → Link do projeto.  
- **README.md** → guia geral do projeto (o arquivo que você está lendo).

--- 

## ⚙️ Funcionalidades
- **Coleta de dados vitais**:
  - Temperatura e umidade (sensor DHT22)
  - Batimentos cardíacos simulados (potenciômetro)
- **Armazenamento local (SPIFFS)**:
  - Os dados são gravados em um arquivo (`/dados.txt`) mesmo sem conexão.
- **Resiliência Offline**:
  - O sistema continua coletando dados quando offline e sincroniza quando volta a ficar online.
- **Indicador visual (LED RGB)**:
  - 🟢 Normal → parâmetros dentro das faixas ideais  
  - 🟡 Atenção → valores levemente alterados  
  - 🔴 Crítico → temperatura, umidade ou batimentos fora do limite  

---

## 🔍 Tecnologias Utilizadas
- **ESP32** (microcontrolador principal)  
- **DHT22** – sensor de temperatura e umidade  
- **Potenciômetro** – simulação de batimentos cardíacos (BPM)  
- **LED RGB** – indicador visual de estado  
- **SPIFFS** – sistema de arquivos para armazenamento local  
- **Arduino IDE / Wokwi** – ambiente de simulação e desenvolvimento

<img src="assets/estrutura_sensores.png" alt="Arquitetura dos sensores" width="500px">

> *Figura 1 – Arquitetura do projeto* 

---

## 🔗 Acessos ao Projeto
- 💻 **Simulação no Wokwi:** [Clique aqui para acessar](https://wokwi.com/projects/444880056645949441)  
- 📄 **Relatórios:**
  - `Relatório – Projeto CardioIA Parte 1.pdf`
  - `Relatório – Projeto CardioIA Parte 2.pdf`
 
---

## 🌐 Parte 2 – MQTT + Dashboard (Fog/Cloud)

### Como executar rapidamente
1. Execute `start_services.bat` (compila e sobe o Node‑RED).
2. No VS Code, abra o Wokwi Simulator e clique Play (carrega `build/tmp_sketch.ino.elf`).
3. No Node‑RED, importe `node-red/cardioia_dashboard.flow.json` e clique Deploy.
4. Abra `http://127.0.0.1:1880/ui` e gire o potenciômetro para ver o BPM variar.

### Conectividade
- Broker: `broker.hivemq.com:1883`
- Tópico: `cardioia/paciente/dados`
- Payload (JSON): `{ "temp": 36.7, "hum": 55, "bpm": 88 }`

### Evidências
<img src="assets/fluxo.png" alt="Fluxo Node-RED" width="520">

> *Figura 2 – Print do Fluxo* 

<img src="assets/dashboard.png" alt="Dashboard" width="520">

> *Figura 3 – Print dos resultados do dashboard* 

---

## 🗃 Histórico de lançamentos

* 0.1.0 - 16/10/2025
    *

## 📋 Licença

<img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/cc.svg?ref=chooser-v1"><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/by.svg?ref=chooser-v1"><p xmlns:cc="http://creativecommons.org/ns#" xmlns:dct="http://purl.org/dc/terms/"><a property="dct:title" rel="cc:attributionURL" href="https://github.com/agodoi/template">MODELO GIT FIAP</a> por <a rel="cc:attributionURL dct:creator" property="cc:attributionName" href="https://fiap.com.br">Fiap</a> está licenciado sobre <a href="http://creativecommons.org/licenses/by/4.0/?ref=chooser-v1" target="_blank" rel="license noopener noreferrer" style="display:inline-block;">Attribution 4.0 International</a>.</p>


