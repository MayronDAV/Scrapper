#include "Search.h"
#include "Base.h"

// lib
#include <cpr/cpr.h>
#include <lexbor/html/html.h>
#include <lexbor/url/url.h>
#include <yaml-cpp/yaml.h>
#include <utf8.h>

#ifdef _WIN32
    #include <windows.h>
    std::string ConvertEncoding(const std::string& p_Input) 
    {
        int required_size = MultiByteToWideChar(CP_ACP, 0, p_Input.c_str(), -1, nullptr, 0);
        
        std::wstring wstr(required_size, 0);
        MultiByteToWideChar(CP_ACP, 0, p_Input.c_str(), -1, &wstr[0], required_size);
        
        required_size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        
        std::string result(required_size, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], required_size, nullptr, nullptr);
        
        return result;
    }
#else // UNTESTED!
    #include <iconv.h>
    std::string ConvertEncoding(const std::string& p_Input, const char* p_ToEncoding, const char* p_FromEncoding) 
    {
        iconv_t converter = iconv_open(p_ToEncoding, p_FromEncoding);
        if (converter == (iconv_t)-1) 
        {
            LOG("ERROR: Encoding conversion failed");
            return "";
        }
        
        size_t in_bytes = p_Input.size();
        char* in_ptr = const_cast<char*>(p_Input.data());
        size_t out_bytes = in_bytes * 4;
        std::string output(out_bytes, 0);
        char* out_ptr = &output[0];
        
        if (iconv(converter, &in_ptr, &in_bytes, &out_ptr, &out_bytes) == (size_t)-1) 
        {
            iconv_close(converter);
            LOG("ERROR: Conversion error");
            return "";
        }
        
        iconv_close(converter);
        output.resize(output.size() - out_bytes);
        return output;
    }
#endif




namespace SCPY
{
    namespace 
    {
        std::string SanitizeUTF8(const std::string& p_Input, char p_Replacement = ' ') 
        {
            std::string output;
            output.reserve(p_Input.size());
            
            auto end_it = utf8::find_invalid(p_Input.begin(), p_Input.end());     
            if (end_it != p_Input.end()) 
            {
                output.append(p_Input.begin(), end_it);
                output.push_back(p_Replacement);

                auto it = end_it;
                while (it != p_Input.end()) 
                {
                    try {
                        utf8::next(it, p_Input.end());
                        output.append(end_it+1, it);
                        end_it = it;
                    } catch (...) {
                        output.push_back(p_Replacement);
                        ++it;
                        end_it = it;
                    }
                }
            } 
            else 
            {
                output = p_Input;
            }
            
            return output;
        }

        std::string AutoFixBrokenEncoding(const std::string& p_Input)
        {
            if (utf8::is_valid(p_Input.begin(), p_Input.end())) {
                return p_Input;
            }

            #ifdef _WIN32
                std::string converted = ConvertEncoding(p_Input);
                if (utf8::is_valid(converted.begin(), converted.end())) 
                    return converted;
            #else
                const std::vector<std::string> encodings_to_try = {
                    "ISO-8859-1",  // Latin-1
                    "Windows-1252", // Western European
                    "ISO-8859-15"   // Latin-9
                };

                for (const auto& encoding : encodings_to_try) 
                {
                    std::string converted = ConvertEncoding(input, "UTF-8", encoding);
                    if (!converted.empty() && utf8::is_valid(converted.begin(), converted.end())) 
                        return converted;
                }
            #endif

            return SanitizeUTF8(p_Input);
        }

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
            auto str = std::string((const char*)text, len);
            return AutoFixBrokenEncoding(str);
        }

        std::string GetElementAttribute(lxb_dom_element_t* p_Element, const char* p_Attr) 
        {
            size_t len;
            const lxb_char_t* value = lxb_dom_element_get_attribute(
                p_Element, (const lxb_char_t*)p_Attr, strlen(p_Attr), &len);
            return value ? std::string((const char*)value, len) : "";
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

        std::string Clean(const std::string& p_Text)
        {
            std::string newText = p_Text;
            newText.erase(std::remove_if(newText.begin(), newText.end(), [](char c) 
            {
                return c == '\n' || c == '\t' || c == '\r';
            }), newText.end());

            newText.erase(std::unique(newText.begin(), newText.end(), [](char a, char b) {
                return a == ' ' && b == ' ';
            }), newText.end());

            auto pos = newText.find((char)0);
            if (pos != std::string::npos)
                newText.erase(pos);

            return newText;
        }

        std::u32string Utf8ToU32(const std::string& p_Text)
        {
            std::vector<uint32_t> utf32result;
            auto ptr = p_Text.c_str();
            utf8::utf8to32(ptr, ptr + p_Text.size(), std::back_inserter(utf32result));

            return std::u32string((char32_t*)utf32result.data(), utf32result.size());
        }

        std::string U32ToUtf8(const std::u32string& p_Text)
        {
            std::vector<unsigned char> utf8result;
            auto ptr = p_Text.c_str();
            utf8::utf32to8(ptr, ptr + p_Text.size(), std::back_inserter(utf8result));

            return std::string((char*)utf8result.data(), utf8result.size());
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

    void Search::ExportYML()
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Term" << YAML::Value << m_Term;
        out << YAML::Key << "Lawsuits" << YAML::Value << YAML::BeginSeq;
        
        for (auto& lawsuit : m_Lawsuits)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Case" << YAML::Value << lawsuit.Case;
            out << YAML::Key << "Rapporteur" << YAML::Value << lawsuit.Rapporteur;
            out << YAML::Key << "JudgmentDate" << YAML::Value << lawsuit.JudgmentDate;
            out << YAML::Key << "PubDate" << YAML::Value << lawsuit.PubDate;
            out << YAML::Key << "Headnote" << YAML::Value << lawsuit.Headnote;
            out << YAML::Key << "Decision" << YAML::Value << lawsuit.Decision;
            out << YAML::EndMap;
        }

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::string term = m_Term;
        std::replace(term.begin(), term.end(), ' ', '-');
        std::ofstream fout("Search-" + term + ".yml");
        fout << out.c_str();
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
            
        std::unordered_map<std::u32string, std::vector<std::string>> lawsuits;

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

                        if (classAttr == "docTitulo")
                        {
                            currentType = text == "Relatora" ? "Relator" : Clean(text);
                        }
                        
                        if (!currentType.empty() && classAttr == "docTexto")
                        {
                            lawsuits[Utf8ToU32(currentType)].push_back(Clean(text));
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

        size_t size = 0;
        if (lawsuits.find(U"Processo") != lawsuits.end())
            size = lawsuits[U"Processo"].size();
        m_Lawsuits.reserve(size);

        for (size_t i = 0; i < size; i++)
        {
            Lawsuit lawsuit;
            lawsuit.Case            = lawsuits[U"Processo"][i];
            lawsuit.Rapporteur      = lawsuits[U"Relator"][i];
            lawsuit.JudgmentDate    = lawsuits[U"Data do Julgamento"][i];
            lawsuit.Headnote        = lawsuits[U"Ementa"][i];
            lawsuit.PubDate         = lawsuits[U"Data da Publicação/Fonte"][i];
            lawsuit.Decision        = lawsuits[U"Acórdão"][i];

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
}
