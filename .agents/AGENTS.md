## Geração de Relatórios em PDF

Quando solicitado para converter ou gerar relatórios em PDF com boa formatação (como a ABNT):
1. Utilize o `pandoc` com o motor `--pdf-engine=typst`. O Typst garante alinhamento justificado nativo, escalas de margem robustas e enquadramento dinâmico de imagens.
2. O formato do comando deve ser:
   `pandoc <arquivo.md> -o <saida.pdf> --pdf-engine=typst -V papersize=a4 -V margin-left=3cm -V margin-top=3cm -V margin-right=2cm -V margin-bottom=2cm -V mainfont="Arial" -V fontsize=12pt -V indent=true -V justify=true -V linestretch=1.5`
3. **Atenção à sintaxe do Markdown:** Lembre-se sempre de inserir uma **linha em branco antes do início de qualquer lista numerada** (ex: `1. ...`) ou com marcadores (`* ...`). Caso contrário, o renderizador pode tratar a lista como continuação do parágrafo anterior, quebrando a diagramação.
