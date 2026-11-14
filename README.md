# Ocarina Mobile Legacy

Port Android nativo de *The Legend of Zelda: Ocarina of Time*, dedicado só a esse jogo.

**Estado: Fase 1 aprovada; Fase 2 parcial (código escrito, APK ainda não compilado). O app ainda não executa o jogo.**

## O que este projeto será
Aplicativo Android com interface própria, controles de toque, áudio, salvamento e configurações gráficas, otimizado para o Realme Note 50 (Unisoc T612, Mali-G57 MP1, 4 GB, Android 14, arm64-v8a).

## O que este projeto não é
Não é emulador. Não inclui ROM, música, modelos nem texturas protegidas. O usuário fornece uma cópia legítima do jogo.

## Base técnica proposta
Shipwright + libultraship + Torch. Ver `docs/COMPARACAO_TECNICA.md`.

## Documentos
| Arquivo | Conteúdo |
|---|---|
| `docs/AUDITORIA.md` | Auditoria dos três repositórios e da ROM |
| `docs/COMPARACAO_TECNICA.md` | Comparação e decisão |
| `docs/DEPENDENCIAS.md` | Dependências, licenças, o que muda no Android |
| `docs/ARQUITETURA.md` | Arquitetura proposta, riscos, plano de build, primeiro protótipo |
| `MASTER_CONTEXT.md` | Contexto vivo do projeto |
| `ROADMAP.md`, `CHANGELOG.md`, `BUILD_INSTRUCTIONS.md`, `TEST_REPORT.md`, `KNOWN_ISSUES.md` | Documentação mestre |

## Aviso legal
Nintendo é dona dos direitos de Ocarina of Time. Este projeto não distribui o jogo. Não é afiliado à Nintendo nem aos projetos citados. Os repositórios auditados têm licenças diferentes e, em dois casos, nenhuma declarada na raiz; ver `docs/DEPENDENCIAS.md` §5.
