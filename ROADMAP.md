# ROADMAP.md

Legenda: ✅ concluído · 🟡 em andamento / parcial · ⬜ não iniciado

| Fase | Entregável | Estado |
|---|---|---|
| 1. Auditoria | AUDITORIA, COMPARACAO_TECNICA, DEPENDENCIAS, ARQUITETURA | ✅ Aprovada em 13/11/2026 |
| 2. Fundação Android | APK inicial executável | 🟡 **Código escrito, APK ainda não compilado** (ver abaixo) |
| 3. Execução do jogo | Primeira execução real de OoT | ⬜ |
| 4. Renderização e áudio | Jogo com imagem e áudio | ⬜ |
| 5. Controles | Toque e controles externos; jogabilidade completa | ⬜ |
| 6. Otimização | Relatório de FPS, RAM, temperatura, estabilidade (no aparelho) | ⬜ |
| 7. Finalização | UI definitiva, configurações, saves, erros, APK, documentação de build | ⬜ |

## Fase 2 — o que falta para fechar
- [x] Verificador de ROM em C++20, testado com a ROM real no host.
- [x] Projeto Android (Gradle/Kotlin/CMake/JNI) e sonda de hardware, **escritos**.
- [x] Workflow de CI (APK + experimento do libultraship em arm64), **escrito**.
- [ ] Primeiro build real do APK (GitHub Actions ou PC do usuário).
- [ ] Corrigir o que o primeiro build quebrar.
- [ ] Rodar a sonda no Realme Note 50 e receber o relatório.
- [ ] Ler o resultado do job `lus-arm64-probe`.
- [ ] Decidir, com esses dados, o escopo da Fase 3.

Regra: uma fase por vez. A Fase 2 só fecha quando o APK compilar e abrir no aparelho.
