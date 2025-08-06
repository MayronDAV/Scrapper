#pragma once
#include <string>
#include <vector>
#include <unordered_map>


namespace SCPY
{
    struct Lawsuit
    {
        std::string Case;
        std::string Rapporteur;
        std::string JudgmentDate;
        std::string PubDate;
        std::string Headnote;
        std::string Decision;
    };

    class Search
    {
        public:
            Search() = default;

            void SearchTerm(const std::string& p_Term);

            void FirstPage();
            void NextPage();
            void PrevPage();
            void LastPage();

            void Load();

            void SetDocsPerPage(int p_DocsPerPage) { m_DocsPerPage = p_DocsPerPage; }
            void SetSaveHtml(bool p_SaveHtml) { m_SaveHtml = p_SaveHtml; }
            void SetDeferredLoad(bool p_DeferredLoad) { m_DeferredLoad = p_DeferredLoad; }

            bool HasPrevPage() const { return m_HasPrevPage; }
            bool HasNextPage() const { return m_HasNextPage; }
            bool IsHomePage() const { return m_CurrentPage == 0; }
            size_t GetTotalResults() const { return m_TotalResults; }
            int GetCurrentPage() const { return m_CurrentPage; }
            int GetDocsPerPage() const { return m_DocsPerPage; }
            std::string GetTerm() const { return m_Term; }
            const std::vector<Lawsuit>& GetLawsuits() const { return m_Lawsuits; }

        private:
            std::string FetchHtml(const std::string& p_Url);
            void ParseHtml(const std::string& p_Html);
            void FetchAndParseHtml(const std::string& p_Url);

        private:
            std::string m_Term = "";
            int m_CurrentPage = 0;
            int m_DocsPerPage = 10;
            size_t m_TotalResults = 0;
            bool m_HasNextPage = true;
            bool m_HasPrevPage = false;
            bool m_DeferredLoad = false;
            bool m_SaveHtml = false;

            std::string m_CurrentUrl = "";

            std::vector<Lawsuit> m_Lawsuits;
    };
} // namespace SCPY