#include "Search.h"
#include "Base.h"


int main() 
{
    Search search;
    search.SetDocsPerPage(50);
    search.SetSaveHtml(false);
    search.SetDeferredLoad(false);

    search.SearchTerm("furto");

    for (const auto& lawsuit : search.GetLawsuits())
    {
        LOG("============================================================");
        LOG("Case:         {}", lawsuit.Case);
        LOG("Rapporteur:   {}", lawsuit.Rapporteur);
        LOG("JudgmentDate: {}", lawsuit.JudgmentDate);
        LOG("PubDate:      {}", lawsuit.PubDate);
        LOG("Headnote:     {}", lawsuit.Headnote);
        LOG("Decision:     {}", lawsuit.Decision);
        LOG("============================================================");
    }
}