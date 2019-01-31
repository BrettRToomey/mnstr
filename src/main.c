#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#include "hpdf.h"

#include "monsters.c"

jmp_buf env;

void errorHandler(HPDF_STATUS errorNo, HPDF_STATUS detailNo, void *userdata) {
    fprintf(stdout, "ERROR: %04x (%u)\n", (HPDF_UINT)errorNo, (HPDF_UINT)detailNo);
    longjmp(env, 1);
}

HPDF_Page pages[MonsterCount];

int main() {
    HPDF_Doc doc;
    HPDF_Font font;
    HPDF_Page page;

    doc = HPDF_New(errorHandler, NULL);
    if (!doc) {
        fprintf(stdout, "Failed to create PDF object\n");
        return 1;
    }

    if (setjmp(env)) {
        HPDF_Free(doc);
        return 1;
    }

    const char *fontName = HPDF_LoadTTFontFromFile(doc, "/Library/Fonts/Georgia.ttf", HPDF_TRUE);
    font = HPDF_GetFont (doc, fontName, NULL);
    
    for (int i = 0; i < MonsterCount; i += 1) {
        const char *pageTitle = names[i];

        page = HPDF_AddPage(doc);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A5, HPDF_PAGE_PORTRAIT);

        float pw = HPDF_Page_GetWidth(page);
        float ph = HPDF_Page_GetHeight(page);

        HPDF_Page_SetFontAndSize(page, font, 12);
        int tw = HPDF_Page_TextWidth(page, pageTitle);

        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, 20, ph - 32, pageTitle);
        HPDF_Page_EndText(page);

        // Footers

        tw = HPDF_Page_TextWidth(page, "1");
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, 10);
        HPDF_Page_TextOut(page, (pw - tw) / 2, 10, "1");
        HPDF_Page_EndText(page);
    }

    HPDF_SaveToFile(doc, "test.pdf");
    HPDF_Free(doc);

    return 0;
}
