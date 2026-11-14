# ROTEIRO_TESTE_APARELHO.md — Fase 2

Para você rodar no Realme Note 50 depois que o APK existir. Leva uns 5 minutos.

## Preparação
1. Instale o APK (`app-debug.apk` ou `app-release.apk`; ambos funcionam nesta fase).
2. Descompacte a ROM: o app **não** abre `.zip`. Precisa do arquivo `.z64` solto no armazenamento do celular.
3. Feche outros apps, para a medição de RAM ser realista.

## Passos
1. Abra o app. Ele mostra, sozinho, os blocos **Aparelho**, **Memória**, **Vulkan**, **Tela e energia**, **Biblioteca nativa** e **OpenGL ES**.
2. Espere uns segundos até aparecer o bloco **Benchmark de compilação de shader**.
3. Toque em **Escolher ROM** e selecione o `.z64`. O bloco **ROM** deve mostrar `status=Ok` e `version=NTSC 1.2 (US)`.
4. Toque em **Copiar** e cole o relatório numa mensagem para mim. Ou toque em **Salvar**: o arquivo `relatorio_sonda.txt` vai para a pasta do app (o aviso na tela mostra o caminho).

## O que cada resultado me diz
| Campo | Pergunta que responde |
|---|---|
| `GL_VERSION`, `GL_RENDERER` | Qual GLES o driver Mali do aparelho entrega de fato |
| `fragment_mediump_float` / `highp` | Se a hipótese do `precision mediump float` do libultraship é um risco real |
| `GL_NUM_PROGRAM_BINARY_FORMATS` | Se um cache de shaders em disco é possível |
| `GL_MAX_SAMPLES` | Se MSAA existe e até quanto |
| `primeira_ms`, `media_demais_ms`, `maxima_ms` | Ordem de grandeza do custo de compilar shader (engasgos) |
| `ram_total_mb`, `ram_disponivel_mb` | Quanta RAM sobra de verdade, com o sistema carregado |
| `memoryClass_mb`, `largeMemoryClass_mb` | Limite de heap que o Android dá ao app |
| `opengles_declarado`, linhas `android.hardware.vulkan.*` | Vulkan declarado pelo sistema |
| `estado_termico` | Aquecimento no momento da medição |

## Limites desta sonda
- O benchmark de shader é uma medida de ordem de grandeza, **não** o shader real do jogo.
- A sonda não mede FPS nem consumo do jogo, porque o jogo ainda não roda nesta fase.
- A ROM é lida inteira em memória (32 MiB) e uma cópia é feita para normalizar o formato. O pico é ~64 MiB temporários.
