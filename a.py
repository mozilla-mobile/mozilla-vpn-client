import tabula

# Specify the path to your PDF file
pdf_path = '~/Downloads/BM2245362887_Sentosa_SQ_HIV_Genotyping_Assay_LK550.hivdb-1.pdf'

# Use the read_pdf function to extract tables from all pages
all_tables = tabula.read_pdf(pdf_path, pages='all', multiple_tables=True, stream=True)

# Loop through all detected tables
for page_num, tables in enumerate(all_tables):
    if len(tables) > 0:
        print(f"Tables on Page {page_num + 1}:")
        for table_num, table_data in enumerate(tables):
            print(f"Table {table_num + 1}:")
            print(table_data)
            print("\n" + "-" * 20 + "\n")
    else:
        print(f"No tables found on Page {page_num + 1}.")
