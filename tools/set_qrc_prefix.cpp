/**
 * @file set_qrc_prefix.cpp
 * Sets the prefix in a `.qrc` file.
 *
 * @author Dan Keenan
 * @date 3/14/26
 * @copyright GPL-3.0
 */

#include <iostream>
#include <QDomDocument>
#include <QFile>

int main(int argc, char *argv[])
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <prefix> <input path> <output path>" << std::endl;
        return 1;
    }

    QFile input(QString::fromUtf8(argv[2]));
    if (!input.open(QFile::ReadOnly | QFile::Text)) {
        std::cerr << "Error opening input file: " << qPrintable(input.errorString()) << std::endl;
        return 1;
    }

    // Parse file.
    QDomDocument doc;
    const auto result = doc.setContent(&input, QDomDocument::ParseOption::UseNamespaceProcessing);
    if (!result) {
        std::cerr << "Error parsing qrc file at " << result.errorLine << ", " << result.errorColumn
                  << ": " << qPrintable(result.errorMessage) << std::endl;
        return 1;
    }

    // Find qresource element.
    auto qresource = doc.documentElement().firstChildElement("qresource");
    if (qresource.isNull()) {
        std::cerr << "Cannot find qresource element" << std::endl;
        return 1;
    }

    // Set new prefix.
    qresource.setAttribute("prefix", QString::fromUtf8(argv[1]));

    // Write output.
    QFile output(argv[3]);
    if (!output.open(QFile::WriteOnly | QFile::Text)) {
        std::cerr << "Error opening output file: " << qPrintable(output.errorString()) << std::endl;
        return 1;
    }
    output.write(doc.toByteArray());

    return 0;
}
