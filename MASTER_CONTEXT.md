# MASTER_CONTEXT.md — Ocarina Mobile Legacy

Última atualização: 13/11/2026 (sessão 1 — Fase 1, auditoria)

## Objetivo
Port Android nativo, dedicado só a *The Legend of Zelda: Ocarina of Time*, para o Realme Note 50 (Unisoc T612, Mali-G57 MP1, 4 GB, Android 14, arm64-v8a). Não é emulador genérico. Sem ROMs, assets, músicas, modelos ou texturas no repositório ou no APK. O usuário fornece uma cópia legítima.

## Regras do pedido (resumo)
- 7 fases, uma de cada vez: Auditoria → Fundação Android → Execução → Render/Áudio → Controles → Otimização → Finalização.
- Por fase: investigar, planejar, implementar, compilar, testar, documentar, registrar problemas.
- Nada fictício ou simulado. Recurso ou é funcional ou está marcado como pendente.
- Profiling antes de otimizar. Estabilidade acima de FPS.
- Stack pedida: C++20, NDK, CMake, JNI, Kotlin, OpenGL ES 3.0; Vulkan opcional; Oboe/AAudio.
- Metas: 30 FPS estáveis (avançado 40–60), até 500 MB de RAM adicional, sem aquecimento excessivo.
- **Aguardar aprovação do usuário depois da Fase 1 antes de implementar.**

## Estado atual (sessão 2, 13/11/2026)
- **Fase 1: aprovada pelo usuário.**
- **Fase 2: parcial.** Escritos: verificador de ROM (C++20, 18 testes de host passaram com a ROM real), projeto Android (Kotlin/CMake/JNI, arm64 apenas), sonda de hardware, workflow de CI, roteiro de teste no aparelho.
- **Não feito:** APK compilado; Kotlin compilado; qualquer execução no Realme Note 50; build do libultraship/SoH para arm64.
- Causa: o ambiente de trabalho não alcança Google Maven, Gradle nem o repositório do NDK (testado). Só compilação C++ no host é possível.
- Decisões assumidas (o usuário aprovou sem responder às 4 perguntas, então segui as recomendações): APK via GitHub Actions; usuário testa no aparelho e devolve o relatório; sem intenção de publicar assumida (a questão de licença segue aberta); SDL2_net e música customizada: decisão adiada para a Fase 3.

## Decisão proposta
Base: **Shipwright + libultraship + Torch.** Shell Android novo (Kotlin + `SDLActivity`), render GLES3 do LUS, áudio SDL2 no início, só `arm64-v8a`.
Descartados: `zeldaret/oot` direto (é decompilação, sem camada de plataforma) e Zelda64Recomp (só Majora's Mask hoje; RT64 exige Vulkan 1.2 de PC).

## Fatos verificados
- ROM fornecida: NTSC-U v1.2, z64, 32 MiB, SHA-1 `41b3bdc48d98c48529219919015a1af22f5057c2`, MD5 `57a9719ad547c516342e1a15d5c28c3d`, CRC1 `0x693BA2AE`. Suportada pelo Shipwright. **A ROM não está no repositório.**
- Commits auditados: oot `14759abf`; Shipwright `d4bf6528`; libultraship `da6daf1a` (pin do Shipwright); Torch `22d32182` (HEAD; pin é `2ab12fe9`); Zelda64Recomp `b65c482e`; Waterdish/Shipwright-Android `c9d8f4a1`.
- Heaps do SoH: sistema 4 MiB, áudio 3,5 MiB, estado de jogo 1 MiB.
- LUS: tem Android (CMake, `USE_OPENGLES`, SDL, `MobileImpl.cpp` só para teclado virtual). SoH upstream: nenhuma referência a Android.
- Sem arquivo de licença na raiz: Shipwright e `zeldaret/oot`. LUS e Torch: MIT. Zelda64Recomp: GPLv3 com política de proibir contribuições por IA.

## Perguntas em aberto (precisam de medição ou decisão)
1. O LUS + SoH compilam para Android arm64 sem ajustes grandes?
2. O Torch roda no aparelho (extração)? Se não, qual a causa?
3. Versão real de GLES/Vulkan do driver Mali do Realme Note 50.
4. Engasgos de compilação de shader e efeito do `mediump`.
5. RAM real do app e tempo de extração.
6. Como o `SaveManager` do SoH grava saves (atomicidade/backup).
7. SDL2_net e Ogg/Vorbis/Opus podem ser opcionais?

## Decisões pendentes do usuário
1. Como compilar o APK: GitHub Actions (recomendado), PC do usuário ou liberar instalação de pacotes nesta sessão.
2. Quem testa no aparelho e como devolve logs.
3. Se há intenção de publicar o APK (licença).
4. Se SDL2_net e música customizada podem ser removidos/opcionais.

## Restrições do ambiente de trabalho
Sessão sem Android SDK/NDK; rede do shell limitada; sem aparelho. Leitura de GitHub via proxy git funciona.

## Próximo passo
1. Usuário sobe o projeto num repositório GitHub (conferindo que nenhuma ROM entrou) e deixa o workflow rodar, ou compila no PC.
2. Usuário me envia: log do job `apk` (se falhar), log do job `lus-arm64-probe`, e o relatório da sonda do aparelho (`docs/ROTEIRO_TESTE_APARELHO.md`).
3. Com esses dados: corrigir o build, fechar a Fase 2 e definir a Fase 3.
