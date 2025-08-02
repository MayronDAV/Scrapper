
// lib
#include <cpr/cpr.h>
#include <lexbor/html/html.h>
#include <lexbor/url/url.h>

// std
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#define LOG(...) std::cout << std::format(__VA_ARGS__) << "\n";


class STJScraper 
{
    public:
        struct Processo 
        {
            std::string Numero;
            std::string Relator;
            std::string Ementa;
            std::string Data;
            std::string Url;
        };

        std::vector<Processo> PesquisarProcessos(const std::string& p_Termo)
        {
            std::string search = p_Termo;
            std::replace(search.begin(), search.end(), ' ', '+');
            std::string url = "https://scon.stj.jus.br/SCON/pesquisar.jsp?pesquisaAmigavel=+" + search + "&b=ACOR&tp=T&numDocsPagina=50&i=1&O=&ref=&processo=&ementa=&nota=&filtroPorNota=&orgao=&relator=&uf=&classe=&juizo=&data=&dtpb=&dtde=&operador=e&thesaurus=JURIDICO&p=true&livre=" + search;

            std::string html = FetchHtml(url);               
            return ParseHtmlLexbor(html);
        }

    private:
        std::string FetchHtml(const std::string& p_Url)
        {
             cpr::Response r = cpr::Get(cpr::Url{p_Url},
                cpr::Header{
                    {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"},
                    {"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
                    {"Accept-Language", "pt-BR,pt;q=0.9,en-US;q=0.8,en;q=0.7"},
                    {"Referer", "https://scon.stj.jus.br/"}
                });

            if (r.status_code != 200) {
                throw std::runtime_error("Falha ao acessar o site: " + std::to_string(r.status_code));
            }

            return r.text;
        }

        lxb_dom_collection_t* GetElementsByClassName(lxb_dom_document_t* p_Document, 
                                           lxb_dom_element_t* p_Parent, 
                                           const std::string& p_ClassName)
        {
            lxb_dom_collection_t* collection = lxb_dom_collection_make(p_Document, 128);
            
            lxb_status_t status = lxb_dom_elements_by_class_name(
                p_Parent, 
                collection, 
                reinterpret_cast<const lxb_char_t*>(p_ClassName.c_str()), 
                p_ClassName.length());
            
            if (status != LXB_STATUS_OK) 
            {
                lxb_dom_collection_destroy(collection, true);
                return nullptr;
            }
            
            return collection;
        }

        std::string GetElementText(lxb_dom_element_t* p_Element) 
        {
            lxb_dom_node_t* node = lxb_dom_interface_node(p_Element);
            size_t len;
            const lxb_char_t* text = lxb_dom_node_text_content(node, &len);
            return std::string((const char*)text, len);
        }

        std::string GetElementAttribute(lxb_dom_element_t* p_Element, const char* p_Attr) 
        {
            size_t len;
            const lxb_char_t* value = lxb_dom_element_get_attribute(
                p_Element, (const lxb_char_t*)p_Attr, strlen(p_Attr), &len);
            return value ? std::string((const char*)value, len) : "";
        }

        std::vector<Processo> ParseHtmlLexbor(const std::string& p_Html)
        {
            lxb_html_document_t* document = lxb_html_document_create();
            lxb_html_document_parse(document, 
                reinterpret_cast<const lxb_char_t*>(p_Html.c_str()), p_Html.length());
            
            lxb_dom_collection_t* divs = GetElementsByClassName(
                &document->dom_document,
                lxb_dom_interface_element(document->body),
                "paragrafoBRS");
                
            std::unordered_map<std::string, std::vector<std::string>> processoValues;

            if (divs)
            {
                std::string currentType = "";
                for (size_t i = 0; i < lxb_dom_collection_length(divs); i++) 
                {
                    lxb_dom_element_t* div = lxb_dom_collection_element(divs, i);      
                    lxb_dom_node_t* child = lxb_dom_node_first_child(lxb_dom_interface_node(div));
                    
                    while (child != nullptr) 
                    {
                        if (child->type == LXB_DOM_NODE_TYPE_ELEMENT)
                        {
                            lxb_dom_element_t* element = lxb_dom_interface_element(child);
                            
                            std::string classAttr = GetElementAttribute(element, "class");
                            std::string text = GetElementText(element);

                            if (classAttr == "docTitulo" && 
                                ( text == "Processo" || text == "Relator" || text == "Relatora" || 
                                    text == "Data do Julgamento" || text == "Ementa"))
                            {
                                currentType = text == "Relatora" ? "Relator" : text;
                            }
                            
                            if (!currentType.empty() && classAttr == "docTexto")
                            {
                                processoValues[currentType].push_back(text);
                                currentType = "";
                            }
                        }
                        
                        child = lxb_dom_node_next(child);
                    }
                }
                
                lxb_dom_collection_destroy(divs, true);
            }
            
            lxb_html_document_destroy(document);

            std::vector<Processo> processos;
            size_t size = processoValues["Processo"].size();
            processos.reserve(size);
            for (size_t i = 0; i < size; i++)
            {
                Processo processo;
                processo.Numero = processoValues["Processo"][i];
                processo.Relator = processoValues["Relator"][i];
                processo.Data = processoValues["Data do Julgamento"][i];
                processo.Ementa = processoValues["Ementa"][i];
                processos.push_back(processo);
            }

            return processos;
        }
};


int main() 
{
    LOG("Hello World!");

    STJScraper scraper;
    auto processos = scraper.PesquisarProcessos("furto homicidio");

    for (const auto& processo : processos) 
    {
        LOG("Processo: {}", processo.Numero);
        LOG("Relator: {}", processo.Relator);
        LOG("Data: {}", processo.Data);
        LOG("Ementa: {}", processo.Ementa.substr(0, 100) + "...");
        LOG("URL: {}", processo.Url);
    }
}