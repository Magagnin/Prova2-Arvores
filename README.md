🧱 1. Criar o projeto no Code::Blocks

Abra o Code::Blocks

Vá em File → New → Project... → Console Application → C → Next

Dê o nome do projeto, por exemplo: PatriciaRouter

Escolha uma pasta onde quer salvar e clique em Finish

📂 2. Adicionar os arquivos do código

Na janela "Projects", clique com o botão direito em Sources → Add files...

Agora adicione três arquivos novos (você pode criá-los diretamente no Code::Blocks):

Patricia.h

Patricia.c

RoteadorSim.c

PATRICIA ROUTER
/SOURCES
.../Patricia.c
.../RoteadorSim.c
/HEADERS
.../Patricia.h

💡 Dica: Salve cada um logo após colar.

⚙️ 3. Verificar o “Build Target”

Certifique-se de que o projeto está configurado para Console application e que todos os arquivos estão listados no painel esquerdo.
Se algum não estiver, clique com o botão direito → Add files... e selecione o .c.

🧩 4. Compilar e executar

Clique no botão Build and run (F9)
O Code::Blocks vai:

Compilar (gcc interno)

Executar no terminal integrado.

Você verá:

Tabela A:
PATRICIA:
  - edge="..."
...
--- Simulações ---
Roteando pacote para 192.168.1.5 a partir de A
  Router A: next-hop = C
  Router C: next-hop = LOCAL
  Pacote entregue em C (next-hop LOCAL)
