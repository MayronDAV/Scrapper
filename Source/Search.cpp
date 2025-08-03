#include "Search.h"
#include "Base.h"

// lib
#include <cpr/cpr.h>
#include <lexbor/html/html.h>
#include <lexbor/url/url.h>


namespace 
{
    lxb_dom_collection_t* MakeCollection(lxb_dom_document_t* p_Document, lxb_dom_element_t* p_Parent, const std::string& p_ClassName)
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

    bool IsTypeIn(const std::string& p_Type, const std::vector<std::string>& p_Types)
    {
        return std::find(p_Types.begin(), p_Types.end(), p_Type) != p_Types.end();
    }

    std::string FormatUrl(const std::string& p_Term, int p_DocsPerPage, int p_CurrentPage = 0, bool p_IsHomePage = true)
    {
        std::string search = p_Term;
        std::replace(search.begin(), search.end(), ' ', '+');

        std::string url = "";
        
        if (p_IsHomePage)
            url = std::format("https://scon.stj.jus.br/SCON/pesquisar.jsp?pesquisaAmigavel=+{0}&b=ACOR&tp=T&numDocsPagina={1}&i=1&O=&ref=&processo=&ementa=&nota=&filtroPorNota=&orgao=&relator=&uf=&classe=&juizo=&data=&dtpb=&dtde=&operador=e&thesaurus=JURIDICO&p=true&livre={0}", search, p_DocsPerPage);
        else
        {
            std::string searchUpper = search;
            std::transform(searchUpper.begin(), searchUpper.end(), searchUpper.begin(), ::toupper);

            url = std::format("https://scon.stj.jus.br/SCON/jurisprudencia/toc.jsp?numDocsPagina={2}&tipo_visualizacao=&filtroPorNota=&ref=&data=&p=true&b=ACOR&pesquisaAmigavel=+{0}&thesaurus=JURIDICO&i={3}&l={2}&tp=T&operador=e&livre={1}&b=ACOR", search, searchUpper, p_DocsPerPage, (p_DocsPerPage * p_CurrentPage) + 1);
        }

        return url;
    }

} // namespace

void Search::SearchTerm(const std::string &p_Term)
{
    m_Term = p_Term;
    m_CurrentPage = 0;
    m_CurrentUrl = FormatUrl(p_Term, m_DocsPerPage);

    if (!m_DeferredLoad)
        Load();

    m_HasPrevPage = false;
    m_HasNextPage = m_TotalResults > m_DocsPerPage;
}

void Search::FirstPage()
{
    m_CurrentUrl = FormatUrl(m_Term, m_DocsPerPage);
    
    if (!m_DeferredLoad)
        Load(); 

    m_CurrentPage = 0;
    m_HasPrevPage = false;
    m_HasNextPage = m_TotalResults > m_DocsPerPage;
}

void Search::NextPage()
{
    if (m_HasNextPage)
        m_CurrentPage++;

    m_CurrentUrl = FormatUrl(m_Term, m_DocsPerPage, m_CurrentPage, false);

    if (!m_DeferredLoad)
        Load();

    m_HasPrevPage = true;
    m_HasNextPage = m_CurrentPage < (m_TotalResults / m_DocsPerPage) - 1;
}

void Search::PrevPage()
{
    if (m_CurrentPage > 0)
        m_CurrentPage--;

    m_CurrentUrl = FormatUrl(m_Term, m_DocsPerPage, m_CurrentPage, m_CurrentPage == 0);

    if (!m_DeferredLoad)
        Load();

    m_HasNextPage = true;
    m_HasPrevPage = m_CurrentPage > 0;
}

void Search::LastPage()
{
    #ifdef min
        m_CurrentPage = min(int(m_TotalResults / m_DocsPerPage) - 1, 0);
    #else
        m_CurrentPage = std::min(int(m_TotalResults / m_DocsPerPage) - 1, 0);
    #endif

    m_CurrentUrl = FormatUrl(m_Term, m_DocsPerPage, m_CurrentPage, false);

    if (!m_DeferredLoad)
        Load();

    m_HasPrevPage = true;
    m_HasNextPage = false;
}

void Search::Load()
{
    if (m_Term.empty() && m_CurrentUrl.empty()) return;

    FetchAndParseHtml(m_CurrentUrl.empty() ? FormatUrl(m_Term, m_DocsPerPage) : m_CurrentUrl); 
}

void Search::ParseHtml(const std::string &p_Html)
{
    lxb_html_document_t* document = lxb_html_document_create();
    lxb_html_document_parse(document, reinterpret_cast<const lxb_char_t*>(p_Html.c_str()), p_Html.length());
    
    lxb_dom_collection_t* resultCounters = MakeCollection(&document->dom_document, lxb_dom_interface_element(document->body), "numDocs");
    
    if (resultCounters && lxb_dom_collection_length(resultCounters) > 0)
    {
        lxb_dom_element_t* counterElement = lxb_dom_collection_element(resultCounters, 0);
        std::string counterText = GetElementText(counterElement);
        
        // (ex: "15.575 acórdãos" -> 15575)
        counterText.erase(std::remove_if(counterText.begin(), counterText.end(), 
            [](char c) { return c < -1 ? true : !std::isdigit(c); }), counterText.end());
            
        m_TotalResults = std::stoul(counterText);
    }
    lxb_dom_collection_destroy(resultCounters, true);

    lxb_dom_collection_t* divs = MakeCollection(&document->dom_document, lxb_dom_interface_element(document->body), "paragrafoBRS");
        
    std::unordered_map<std::string, std::vector<std::string>> lawsuits;

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

                    if (classAttr == "docTitulo" && IsTypeIn(text, {"Processo", "Relator", "Relatora", "Data do Julgamento", "Ementa"}))
                    {
                        currentType = text == "Relatora" ? "Relator" : text;
                    }
                    
                    if (!currentType.empty() && classAttr == "docTexto")
                    {
                        std::string newText = text;
                        newText.erase(std::remove_if(newText.begin(), newText.end(), [](char c) 
                        {
                            return c == '\n' || c == '\t' || c == '\r';
                        }), newText.end());

                        newText.erase(std::unique(newText.begin(), newText.end(), [](char a, char b) {
                            return a == ' ' && b == ' ';
                        }), newText.end());

                        lawsuits[currentType].push_back(newText);
                        currentType = "";
                    }
                }
                
                child = lxb_dom_node_next(child);
            }
        }
        
        lxb_dom_collection_destroy(divs, true);
    }
    
    lxb_html_document_destroy(document);

    m_Lawsuits.clear();
    size_t size = lawsuits["Processo"].size();
    m_Lawsuits.reserve(size);

    for (size_t i = 0; i < size; i++)
    {
        Lawsuit lawsuit;
        lawsuit.Case            = lawsuits["Processo"][i];
        lawsuit.Rapporteur      = lawsuits["Relator"][i];
        lawsuit.JudgmentDate    = lawsuits["Data do Julgamento"][i];
        lawsuit.Headnote        = lawsuits["Ementa"][i];
        // TODO
        //lawsuit.PubDate       = lawsuits["Data da Publicação/Fonte"][i];
        //lawsuit.Decision      = lawsuits["Acórdão"][i];

        m_Lawsuits.push_back(lawsuit);
    }
}

void Search::FetchAndParseHtml(const std::string &p_Url)
{
    std::string html = FetchHtml(p_Url);
    ParseHtml(html);
}

std::string Search::FetchHtml(const std::string& p_Url)
{
    cpr::Response r = cpr::Get(cpr::Url{p_Url},
        cpr::Header{
            {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"},
            {"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
            {"Accept-Language", "pt-BR,pt;q=0.9,en-US;q=0.8,en;q=0.7"},
            {"Referer", "https://scon.stj.jus.br/"}
    });

    ASSERT(r.status_code == 200, std::format("Failed to access the site: {}", r.status_code));

    if (m_SaveHtml)
    {
        std::ofstream out("out.html", std::ios::out | std::ios::trunc);
        out << r.text;
        out.close();
    }

    return r.text;
}
