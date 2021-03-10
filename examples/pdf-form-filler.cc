#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFAcroFormDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>

static char const* whoami = 0;
std::vector<std::unordered_map<std::string, std::string>> form_values;

void usage()
{
    std::cerr << "Usage: " << whoami << " infile.pdf outfile.pdf [--annotate]" << std::endl
        << "Fill text form fields from stdin or annotate them" << std::endl;
    exit(2);
}

void getFormValues()
{
    int number_of_values;
    std::cin >> number_of_values;
    std::cin.ignore();

    unsigned int tmp_page;
    std::string tmp_key;
    std::string tmp_value;

    for(auto i = 0; i < number_of_values; ++i) {
        std::cin >> tmp_page;
        std::cin.ignore();
        std::getline (std::cin, tmp_key);
        std::getline (std::cin, tmp_value);
        form_values[tmp_page].emplace(tmp_key, tmp_value);
    }
}

int main(int argc, char* argv[])
{
    whoami = QUtil::getWhoami(argv[0]);

    // For libtool's sake....
    if (strncmp(whoami, "lt-", 3) == 0) {
        whoami += 3;
    }

    if (argc < 3 || argc > 4) {
        usage();
    }

    char const* infilename = argv[1];
    char const* outfilename = argv[2];
    bool const annotate = (argc == 4) && (std::string(argv[3]) == "--annotate");

    try {
        QPDF qpdf;
        qpdf.processFile(infilename);

        QPDFAcroFormDocumentHelper form_helper(qpdf);
        QPDFPageDocumentHelper document_helper(qpdf);

        auto pages = document_helper.getAllPages();

        form_values.resize(pages.size());

        if (annotate == false) {
            getFormValues();
        }

        unsigned int current_page = 0;
        for (auto& page: pages) {
            for (auto& annotation: form_helper.getWidgetAnnotationsForPage(page)) {
                auto field = form_helper.getFieldForAnnotation(annotation);

                if (field.getFieldType() != "/Tx") {
                    continue;
                }

                std::string field_name = field.getFullyQualifiedName();

                if (annotate == true) {
                    field.setV(std::to_string(current_page) + "-" + field_name, true);
                    continue;
                }

                if (form_values[current_page].find(field_name) == form_values[current_page].end()) {
                    continue;
                }

                field.setV(form_values[current_page][field_name], true);
            }
            ++current_page;
        }

        QPDFWriter w(qpdf, outfilename);
        w.write();
    } catch (std::exception &e) {
        std::cerr << whoami << " processing file " << infilename << ": " << e.what() << std::endl;
        exit(2);
    }

    return 0;
}
