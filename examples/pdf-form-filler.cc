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
    std::cerr << "Usage: " << whoami << " infile.pdf [outfile.pdf [--annotate] [--use-pages]]" << std::endl
        << "Fill text form fields from stdin, list or annotate them." << std::endl
        << "  If no outfile.pdf is given, a list off all fillable fields is written to stdout." << std::endl
        << "  --annotate - Every field is filled with its own name." << std::endl
        << "  --use-pages - Page number is used to distinguish same-name fields from each other." << std::endl;
    exit(2);
}

void getFormValues(const bool use_pages)
{
    int number_of_values;
    std::cin >> number_of_values;
    std::cin.ignore();

    unsigned int tmp_page;
    std::string tmp_key;
    std::string tmp_value;

    for(auto i = 0; i < number_of_values; ++i) {
        if (use_pages) {
            std::cin >> tmp_page;
            std::cin.ignore();
        }
        std::getline (std::cin, tmp_key);
        std::getline (std::cin, tmp_value);
        form_values[use_pages ? tmp_page : 0].emplace(tmp_key, tmp_value);
    }
}

int main(int argc, char* argv[])
{
    whoami = QUtil::getWhoami(argv[0]);

    // For libtool's sake....
    if (strncmp(whoami, "lt-", 3) == 0) {
        whoami += 3;
    }

    if (argc < 2 || argc > 5) {
        usage();
    }

    char const* infilename = argv[1];
    char const* outfilename = (argc >= 3) ? argv[2] : nullptr;
    bool const list_fields = (argc == 2);
    bool annotate = false;
    bool use_pages = false;

    for (auto i = 3; i < argc; ++i) {
        auto const arg = std::string(argv[i]);
        if (arg == "--annotate") {
            annotate = true;
        } else if (arg == "--use-pages") {
            use_pages = true;
        }
    }

    try {
        QPDF qpdf;
        qpdf.processFile(infilename);

        QPDFAcroFormDocumentHelper form_helper(qpdf);
        QPDFPageDocumentHelper document_helper(qpdf);

        auto pages = document_helper.getAllPages();

        form_values.resize(use_pages ? pages.size() : 1);

        if (annotate == false && outfilename != nullptr) {
            getFormValues(use_pages);
        }

        unsigned int current_page = 0;
        for (auto& page: pages) {
            for (auto& annotation: form_helper.getWidgetAnnotationsForPage(page)) {
                auto field = form_helper.getFieldForAnnotation(annotation);

                if (field.getFieldType() != "/Tx") {
                    continue;
                }

                const auto field_name = field.getFullyQualifiedName();
                const auto field_id = (use_pages) ? std::to_string(current_page) + "-" + field_name : field_name;

                if (list_fields == true) {
                    std::cout << field_id << std::endl;
                }

                if (annotate == true) {
                    field.setV(field_id, true);
                    continue;
                }

                const auto& form_values_page = form_values[(use_pages) ? current_page : 0];
                if (form_values_page.find(field_name) == form_values_page.end()) {
                    continue;
                }

                field.setV(form_values_page.at(field_name), true);
            }
            ++current_page;
        }

        if (outfilename != nullptr) {
            QPDFWriter w(qpdf, outfilename);
            w.write();
        }
    } catch (std::exception &e) {
        std::cerr << whoami << " processing file " << infilename << ": " << e.what() << std::endl;
        exit(2);
    }

    return 0;
}
