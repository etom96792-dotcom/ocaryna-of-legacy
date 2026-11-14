# KNOWN_ISSUES.md

Riscos conhecidos na fase de auditoria. Nenhum foi reproduzido ainda; são hipóteses a verificar.

| # | Item | Tipo | Estado |
|---|---|---|---|
| 1 | SoH upstream sem alvo Android | Bloqueio de portabilidade | Aberto |
| 2 | Torch no Android não confirmado | Risco | Aberto |
| 3 | Possíveis engasgos por compilação de shader (sem cache de binário visto no backend GL) | Hipótese de desempenho | A medir |
| 4 | `precision mediump float` no backend GLES do LUS | Hipótese de correção visual | A testar |
| 5 | Perda de contexto EGL ao ir para segundo plano | Risco de estabilidade | A investigar |
| 6 | Interface ImGui feita para desktop | UX | Trabalho da Fase 5/7 |
| 7 | Sem overlay de toque no upstream | Recurso ausente | Fase 5 |
| 8 | Licença não declarada na raiz do Shipwright e do zeldaret/oot | Legal | Decisão do usuário |
| 9 | Ambiente de trabalho sem Android SDK/NDK e sem aparelho | Limitação de processo | Aguardando decisão |
| 10 | Versão real de GLES/Vulkan do Realme Note 50 não confirmada | Dado ausente | Sonda escrita; falta rodar no aparelho |
| 11 | APK e Kotlin nunca compilados. Versões escolhidas sem resolver: AGP 8.7.3, Kotlin 2.0.21, Gradle 8.14.3, NDK 26.1.10909125, CMake 3.22.1, compileSdk/targetSdk 34, minSdk 26 | Risco de build | Aberto: primeiro build real decide |
| 12 | A sonda lê a ROM inteira em memória (32 MiB + cópia de normalização, pico ~64 MiB) | Limitação conhecida | Aceitável para a sonda; revisar na Fase 3 |
| 13 | O benchmark de shader não é o shader real do Fast3D | Limitação da medição | Medida só de ordem de grandeza |
| 14 | Tabela de hashes aceita 19 versões (inclui GameCube, Master Quest, debug). O título do cabeçalho só é conferido quando o hash é desconhecido | Escopo | Fase 3 decide quais versões o app suporta de fato |
| 15 | Assinatura do release com chave de debug | Provisório | Chave própria na Fase 7 |
