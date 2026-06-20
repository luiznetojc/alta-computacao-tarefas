from fpdf import FPDF
import markdown

class ABNTPDF(FPDF):
    def footer(self):
        self.set_y(-20)
        self.set_font('Arial', '', 10)
        self.cell(0, 10, str(self.page_no()), align='R')

def convert(md_file, pdf_file):
    pdf = ABNTPDF()
    pdf.set_margins(left=30, top=30, right=20)
    pdf.add_page()
    pdf.set_font("Arial", size=12)
    
    with open(md_file, 'r', encoding='utf-8') as f:
        md_text = f.read()

    html_text = markdown.markdown(md_text, extensions=['tables', 'fenced_code'])
    
    # fpdf2 write_html
    pdf.write_html(html_text)
    pdf.output(pdf_file)
    print(f"Salvo {pdf_file}")

convert('20-tarefa/relatorio.md', '20-tarefa/relatorio_abnt.pdf')
convert('21-tarefa/relatorio.md', '21-tarefa/relatorio_abnt.pdf')
