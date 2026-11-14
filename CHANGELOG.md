# CHANGELOG.md

## [Não lançado]

### 2026-11-13 — Sessão 2 (Fase 2, fundação Android) — parcial
- Adicionado: `native/rom_verifier` (C++20): detecção de formato z64/v64/n64, CRC1, SHA-1 e tabela de 19 hashes gerada do `supportedHashes.json` do Shipwright. Rejeita `.zip`, tamanho errado, arquivo alterado.
- Adicionado: `tests/rom_verifier_test.cpp` — 18 verificações, todas passaram com a ROM real (NTSC-U v1.2).
- Adicionado: projeto Android em `android/` (Kotlin, CMake, JNI, só `arm64-v8a`, sem permissões de armazenamento): sonda de hardware (GLES real, precisão de float, formatos de binário de programa, benchmark simplificado de compilação de shader, RAM, Vulkan declarado, estado térmico) e verificação de ROM pelo seletor do sistema.
- Adicionado: `.github/workflows/android.yml` (testes de host, APK, experimento libultraship arm64) e `.gitignore` que bloqueia ROMs e `.o2r`.
- Adicionado: `docs/ROTEIRO_TESTE_APARELHO.md`.
- **Não compilado:** o APK e o código Kotlin nunca foram compilados (ambiente sem acesso a Google Maven, Gradle e NDK).

### 2026-11-13 — Sessão 1 (Fase 1, auditoria)
- Adicionado: `docs/AUDITORIA.md`, `docs/COMPARACAO_TECNICA.md`, `docs/DEPENDENCIAS.md`, `docs/ARQUITETURA.md`.
- Adicionado: `README.md`, `MASTER_CONTEXT.md`, `ROADMAP.md`, `BUILD_INSTRUCTIONS.md`, `TEST_REPORT.md`, `KNOWN_ISSUES.md`.
- Verificada a ROM fornecida (NTSC-U v1.2) contra tabelas do Shipwright e do zeldaret/oot.
- Nenhum código do app. Nenhum build. Nenhum teste em aparelho.

### 2026-11-13 — Correção (Fase 2)
- Workflow: `./gradlew` trocado por `sh ./gradlew`, porque o upload pelo navegador do GitHub não preserva a permissão de execução do arquivo.
