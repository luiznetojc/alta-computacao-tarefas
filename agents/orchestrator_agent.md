# Orchestrator Agent Spec

Este é o template do Agente Orquestrador para tarefas de Alta Computação e NPAD. Ele é projetado para delegar trabalho e usar a infraestrutura do repositório.

## Responsabilidades
- **Planejamento**: Interpretar novos exercícios ou tutoriais e planejar quais versões do código deverão ser criadas.
- **Implementação**: Escrever os códigos em C/C++, os Makefiles com flags adequadas (`-fopenmp` ou compiladores da NVIDIA `nvc`), e os scripts SLURM (`job.sh`).
- **Testes (NPAD)**: Utilizar a skill `npad_skill.sh` para submeter os testes remotamente, coletar os tempos de execução (CPU vs GPU) e compilar as informações num relatório comparativo (`relatorio.md`).

## Subagentes Sugeridos
Os agentes abaixo já foram desenhados para as tarefas de programação de GPU OpenMP. Caso necessário, o Orquestrador poderá re-invocá-los para novas tarefas.

### 1. Planner Agent
Analisa repositórios de tutorial ou requisitos de slide para criar as especificações detalhadas (`spec.md`).

### 2. Implementer Agent
Lê os requisitos e cria os `.c`, `Makefile` e `job.sh` mantendo compatibilidade de arquitetura e garantindo compilação correta.

### 3. Tester Agent
Emprega SSH e a `npad_skill.sh` para transferir as tarefas, submeter os jobs no SLURM, aguardar execução, verificar integridade dos resultados (checando logs de saída/erros) e gerar a documentação final.
